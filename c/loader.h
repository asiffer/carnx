#include "libbpf/src/libbpf.h"
#include "common.h"

#ifndef _LOADER_H_
#define _LOADER_H_

// Build the basedir /sys/fs/bpf/xdp/<iface>/ and the final
// endpoint /sys/fs/bpf/xdp/<iface>/carnx_map
int prepare_pin_endpoints(char *iface, char *pin_dir, char *path_to_map);

// This function loads a BPF program (XDP) into the kernel. It fills a bpf_object
// and also returns the program file descriptor
int _load_bpf_program(char *filename, struct bpf_object **obj, int *prog_fd);

// This function loads a BPF program (XDP) into the kernel. It returns
// the pointer to the bpf_object (NULL is an error occured)
struct bpf_object *load_bpf_program(char *filename, int *prog_fd);

// This function unloads a program from the kernel
int unload_bpf_program(struct bpf_object *obj);

/* Attach a BPF program on a specific interface (iface). The program is given through the
 * file descriptor and the flags controls the way to load:
 *   XDP_FLAGS_SKB_MODE		Generic mode.
 *                          Slow, driver-independent "generic" XDP variant.
 *                          It should be used when the driver dos not have native XDP
 *   XDP_FLAGS_DRV_MODE		Driver native mode.
 *                          Request that the automatic fallback to the generic XDP 
 *                          variant be disabled and in case driver is not XDP-capable 
 *                          error should be returned. It also disables hardware offloads.
 *   XDP_FLAGS_HW_MODE		Hardware offload mode. The kernel code is re-verified and compiled 
 *                          to be run by the hardware firmware
 */
int attach_bpf_program(int prog_fd, char *iface, unsigned int xdp_flags);

// Remove all XDP programs from the interface
int detach_bpf_program(char *iface, unsigned int xdp_flags);

// Remove maps endpoints (on the filesystem) if they exist
int unpin_maps_if_exist(struct bpf_object *obj, char *pin_dir, char *path_to_map);

// Pin maps of a BPF object. The maps can then be accessed
// at pin_dir/<map_name>
int pin_maps(struct bpf_object *obj, char *pin_dir);

// Does everything
struct bpf_object *load_attach_pin(char *filename, char *iface, unsigned int xdp_flags);

// Get back the map file descriptor from the name
int get_map_fd_from_name(struct bpf_object *obj, char *map_name);

#endif // _LOADER_H_