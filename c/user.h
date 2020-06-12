#include "common.h"
#include "loader.h"
// system imports

// local
#include "libbpf/src/libbpf.h"
#include "libbpf/src/bpf.h"

#ifndef _USER_H_
#define _USER_H_

#define IFACE_LENGTH 32

struct context
{
    struct bpf_object *obj;
    int prog_fd;
    int map_fd;
    struct bpf_map *map;
    char iface[IFACE_LENGTH];
    unsigned int xdp_flags;
    bool is_loaded;
    bool is_attached;
};

// debug
void debug_context();

// Get the counter value related to the given key
u64 get_counter(int key);

// Get all counter values. You must ensure that the
// returned buffer has length >= nb_counters()
int get_all_counters(u64 *array);

// Get all counter values. You must ensure that the
// returned buffer has length >= nb_counters(). It returns
// the timestamp of the action (seconds and nanoseconds)
int get_all_counters_with_timestamp(u64 *array, u64 *sec, u64 *nsec);

// Load an eBPF program into the kernel
int load(char *filename);

// Attach the XDP program on the interface
int attach(char *iface, unsigned int xdp_flags);

// Load and attach the eBPF program. If the program is already loaded
// it reloads it but through a different id (different file descriptor)
int load_and_attach(char *filename, char *iface, unsigned int xdp_flags);

// Detach the XDP program from the interface
int detach();

// Unload the eBPF program from the kernel
int unload();

// return the load status
bool is_loaded();

// return the attach status
bool is_attached();

// Return the interface where the program
// is attached. You must ensure that dest
// points to a buffer with IFACE_LENGTH size.
int get_iface(char *dest);

#endif // _USER_H_