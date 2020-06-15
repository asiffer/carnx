// grpc_server.go
//

package main

// #cgo LDFLAGS: -Llib/ -lcarnx -lbpf
// #cgo CFLAGS: -Ic/
//
// #include "user.h"
//
// void __server_debug(char * msg) {
//     return debugf("server", msg);
// }
//
// void __server_info(char * msg) {
//     return infof("server", msg);
// }
//
// void __server_warn(char * msg) {
//     return warnf("server", msg);
// }
//
// void __server_error(char * msg) {
//     return errorf("server", msg);
// }
//
import "C"

import (
	"bytes"
	"carnx/api"
	"context"
	"encoding/binary"
	"fmt"
	"net"
	"os"
	"os/signal"
	"os/user"
	"runtime"
	"strings"
	"syscall"
	"time"
	"unsafe"

	"github.com/coreos/go-systemd/activation"

	"google.golang.org/grpc"
	"google.golang.org/grpc/reflection"
)

const (
	// SizeOfUint64 is the number of bytes to represent a uint64
	SizeOfUint64 = 8
)

var (
	// nbCounters is the number of available counters
	nbCounters = int(C.nb_counters())
	// lookUp is the structure which returns the key given
	// the name of the counter
	lookUp = make(map[string]int)
	// debug
	debug = false
)

func init() {
	checkUser()
	initLookUp()
}

// ========================================================================== //
// UTILS ==================================================================== //
// ========================================================================== //

func errorf(format string, a ...interface{}) {
	msg := C.CString(fmt.Sprintf(format, a...))
	C.__server_error(msg)
}

func warnf(format string, a ...interface{}) {
	msg := C.CString(fmt.Sprintf(format, a...))
	C.__server_warn(msg)
}

func infof(format string, a ...interface{}) {
	msg := C.CString(fmt.Sprintf(format, a...))
	C.__server_info(msg)
}

func debugf(format string, a ...interface{}) {
	if !debug {
		return
	}
	msg := C.CString(fmt.Sprintf(format, a...))
	C.__server_debug(msg)
}

func initLookUp() {
	buffer := C.CString(strings.Repeat("X", 32))
	for i := 0; i < nbCounters; i++ {
		C.reverse_lookup(C.int(i), buffer)
		lookUp[C.GoString(buffer)] = i
	}
	C.free(unsafe.Pointer(buffer))
	debugf("Counter lookup is done")
}

// checkUser checks that the program is run by root
func checkUser() {
	u, err := user.Current()
	if err != nil {
		errorf("Cannot retrieve current user: %v", err)
		return
	}
	if u.Uid != "0" {
		warnf("This program should be run as root")
	}
}

// activate the debugging
func setDebug() {
	debug = true
	C.set_debug(1)
}

// get the name of the current function
func currentFunction() string {
	pc := make([]uintptr, 15)
	n := runtime.Callers(2, pc)
	frames := runtime.CallersFrames(pc[:n])
	frame, _ := frames.Next()
	tree := strings.Split(frame.Function, ".")
	if len(tree) >= 3 {
		return tree[2]
	}
	return frame.Function
}

func toUint64Slice(raw []byte) ([]uint64, error) {
	// create an io.Reader from these bytes
	buffer := bytes.NewReader(raw)
	// init a slice of float64
	data := make([]uint64, len(raw)/SizeOfUint64)
	// Read bytes and copy them into the float64 slice
	if err := binary.Read(buffer, binary.LittleEndian, &data); err != nil {
		return nil, fmt.Errorf("Error while retrieving counters: %v", err)
	}
	return data, nil
}

// return the interface where the xdp program is
// attached
func getIface() string {
	ptr := C.malloc(C.sizeof_char * C.IFACE_LENGTH)
	defer C.free(unsafe.Pointer(ptr))
	C.get_iface((*C.char)(ptr))
	b := C.GoBytes(ptr, C.IFACE_LENGTH)
	return string(b)
}

func removeFileIfExists(file string) error {
	if _, err := os.Stat(file); os.IsNotExist(err) {
		return nil
	}
	return os.Remove(file)
}

// ========================================================================== //
// CarnxServer ENDPOINTS ==================================================== //
// ========================================================================== //

// CarnxServer is the type to manage RPC calls
type CarnxServer struct{}

// GetNbCounters returns the number of counters
func (a *CarnxServer) GetNbCounters(ctx context.Context, garbage *api.Garbage) (*api.NbCounters, error) {
	debugf("Receving call to '%s'")
	return &api.NbCounters{NbCounters: uint32(nbCounters)}, nil

}

// Ping aims to check the connection
func (a *CarnxServer) Ping(ctx context.Context, garbage *api.Garbage) (*api.Garbage, error) {
	debugf("Receiving call to %s", currentFunction())
	return garbage, nil
}

// GetCounter returns the value of a counter given its key
func (a *CarnxServer) GetCounter(ctx context.Context, cid *api.CounterID) (*api.CounterValue, error) {
	debugf("Receiving call to %s", currentFunction())
	key := C.int(cid.Id)
	return &api.CounterValue{Value: uint64(C.get_counter(key))}, nil
}

// GetCounterByName returns the value of a counter given its name
func (a *CarnxServer) GetCounterByName(ctx context.Context, counter *api.CounterName) (*api.CounterValue, error) {
	debugf("Receiving call to %s", currentFunction())
	return a.GetCounter(ctx, &api.CounterID{Id: uint32(lookUp[counter.Name])})
}

// GetCounterNames returns the list of the counters (in the right order)
func (a *CarnxServer) GetCounterNames(ctx context.Context, garbage *api.Garbage) (*api.CounterList, error) {
	debugf("Receiving call to %s", currentFunction())
	names := make([]string, len(lookUp))
	for n, index := range lookUp {
		names[index] = n
	}
	return &api.CounterList{Counters: names}, nil
}

// Snapshot returns the current values of the counters
func (a *CarnxServer) Snapshot(ctx context.Context, garbage *api.Garbage) (*api.Snap, error) {
	debugf("Receiving call to %s", currentFunction())
	sec := C.ulong(0)
	nsec := C.ulong(0)

	values := make([]uint64, nbCounters)
	// get values
	C.get_all_counters_with_timestamp((*C.ulong)(&values[0]), &sec, &nsec)

	data := make(map[string]uint64)
	// fill the map
	data["TIME"] = uint64(time.Unix(int64(sec), int64(nsec)).UnixNano())
	// (*record)["NSEC"] = uint64(nsec)
	for name, i := range lookUp {
		data[name] = values[i]
	}

	return &api.Snap{Data: data}, nil
}

// Load an eBPF program into the kernel
func (a *CarnxServer) Load(ctx context.Context, attr *api.LoadParameters) (*api.ReturnCode, error) {
	ret := int32(C.load(C.CString(attr.BpfProgram)))
	returnCode := api.ReturnCode{Code: ret}

	if ret < 0 {
		return &returnCode, fmt.Errorf("Error while loading BPF program %s", attr.BpfProgram)
	}
	return &returnCode, nil
}

// LoadAndAttach aims to init the XDP program. It loads the program
// into the kernel and attach it to the given interface with
// the flags
func (a *CarnxServer) LoadAndAttach(ctx context.Context, attr *api.LoadAttachParameters) (*api.ReturnCode, error) {
	debugf("Receiving call to %s", currentFunction())
	debugf("Loading %s to interface %s", attr.BpfProgram, attr.Interface)
	ret := int32(C.load_and_attach(C.CString(attr.BpfProgram),
		C.CString(attr.Interface),
		C.uint(attr.XdpFlags)))

	returnCode := api.ReturnCode{Code: ret}
	// check errors
	switch ret {
	case -1:
		msg := "Error while loadinf BPF program"
		errorf(msg)
		return &returnCode, fmt.Errorf(msg)
	case -2:
		msg := "Error while retrieving the file descriptor of the map"
		errorf(msg)
		return &returnCode, fmt.Errorf(msg)
	case -3:
		msg := "Error while attaching program"
		errorf(msg)
		return &returnCode, fmt.Errorf(msg)
	default:
		infof("Program loaded and attached to %s", attr.Interface)
		// startMonitoring()
		return &returnCode, nil
	}
}

// Unload the eBPF program from the kernel
func (a *CarnxServer) Unload(ctx context.Context, g *api.Garbage) (*api.ReturnCode, error) {
	debugf("Receiving call to %s", currentFunction())
	ret := int32(C.unload())

	returnCode := api.ReturnCode{Code: ret}
	if ret < 0 {
		return &returnCode, fmt.Errorf("Error while unloading the BPF program")
	}
	infof("BPF program unloaded")
	return &returnCode, nil
}

// Attach the XDP program onto the given interface
func (a *CarnxServer) Attach(ctx context.Context, attr *api.AttachParameters) (*api.ReturnCode, error) {
	debugf("Receiving call to %s", currentFunction())
	debugf("Attaching to %s", attr.Interface)
	ret := int32(C.attach(C.CString(attr.Interface), C.uint(attr.XdpFlags)))

	returnCode := api.ReturnCode{Code: int32(ret)}
	if ret < 0 {
		return &returnCode, fmt.Errorf("The BPF program is not loaded")
	}
	infof("Program is attached to %s", attr.Interface)
	// start prometheus
	// startMonitoring()
	return &returnCode, nil
}

// Detach the XDP program from the interface previously given
func (a *CarnxServer) Detach(ctx context.Context, g *api.Garbage) (*api.ReturnCode, error) {
	debugf("Receiving call to %s", currentFunction())
	iface := getIface()
	debugf("Detaching from %s", iface)
	ret := int32(C.detach())

	returnCode := api.ReturnCode{Code: ret}
	if ret < 0 {
		return &returnCode, fmt.Errorf("The BPF program is not loaded")
	}
	infof("Program detached from %s", iface)
	// stop prometheus
	// stopMonitoring()
	return &returnCode, nil
}

// IsLoaded check if the program is loaded into the kernel
func (a *CarnxServer) IsLoaded(ctx context.Context, g *api.Garbage) (*api.LoadStatus, error) {
	debugf("Receiving call to %s", currentFunction())
	return &api.LoadStatus{Loaded: bool(C.is_loaded())}, nil
}

// IsAttached check if the program is attached to the interface
func (a *CarnxServer) IsAttached(ctx context.Context, g *api.Garbage) (*api.AttachStatus, error) {
	debugf("Receiving call to %s", currentFunction())
	return &api.AttachStatus{Attached: bool(C.is_attached())}, nil
}

// ========================================================================== //
// Server =================================================================== //
// ========================================================================== //

// ServeGRPC starts a gRPC server
func ServeGRPC(unixPath string) error {
	lis, err := net.Listen("unix", unixPath)
	if err != nil {
		errorf("Failed to listen: %v", err)
		return err
	}
	grpcServer := grpc.NewServer()
	api.RegisterCarnxServer(grpcServer, &CarnxServer{})
	// determine whether to use TLS
	infof("Listening on %s", unixPath)
	return grpcServer.Serve(lis)
}

func clean(s *CarnxServer) {
	// detach program
	if _, err := s.Detach(nil, &api.Garbage{}); err != nil {
		errorf("%v", err)
	}

	// unload program
	if _, err := s.Unload(nil, &api.Garbage{}); err != nil {
		errorf("%v", err)
	}
}

// serves expose the grpc endpoint given a listener
func serve(l net.Listener) error {
	// À la réception du signal, ferme en douceur le serveur et
	// attend 5 secondes la fin des connexions en cours.
	done := make(chan struct{})
	quit := make(chan os.Signal, 1)
	server := grpc.NewServer()
	s := &CarnxServer{}
	api.RegisterCarnxServer(server, s)
	reflection.Register(server)
	signal.Notify(quit, syscall.SIGINT, syscall.SIGTERM)

	go func() {
		<-quit
		fmt.Println("")
		infof("Server is shutting down")
		server.GracefulStop()
		clean(s)
		close(done)
	}()

	// accept new connection
	infof("Listening to %s", l.Addr())
	server.Serve(l)

	// wait for the end of the connections
	<-done
	return nil
}

// ServeUnix serves the gRPC endpoint through
// a user defined unix socket
func ServeUnix(unixPath string) error {
	lis, err := net.Listen("unix", unixPath)
	if err != nil {
		return err
	}
	return serve(lis)
}

// ServeSystemd serves the gRPC endpoint through
// a systemd socket
func ServeSystemd() error {
	listeners, err := activation.Listeners()
	if err != nil {
		return fmt.Errorf("Cannot retrieve listeners: %s", err)
	}
	if len(listeners) != 1 {
		return fmt.Errorf("unexpected number of socket activation (%d != 1)",
			len(listeners))
	}
	return serve(listeners[0])
}
