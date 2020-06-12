// api.go

package api

const (
	// ServiceName is the systemd name
	ServiceName = "carnx"
	// Username is the owner of the socket (the user who
	// runs the client)
	// Username = "root"

	// Socket is the path to the socket
	Socket = "/run/carnx.sock"
	// XDPFlagsSkbMode is the slow, driver-independent "generic" XDP variant.
	// It should be used when the driver dos not have native XDP
	XDPFlagsSkbMode = uint(1)
	// XDPFlagsDrvMode is the driver native mode.
	// It requests that the automatic fallback to the generic XDP
	// variant be disabled and in case driver is not XDP-capable
	// error should be returned. It also disables hardware offloads.
	XDPFlagsDrvMode = uint(2)
	// XDPFlagsHwMode is the hardware offload mode. The kernel code is re-verified and compiled
	// to be run by the hardware firmware
	XDPFlagsHwMode = uint(4)
)

func main() {}
