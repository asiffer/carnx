#include "user.h"

// ========================================================================== //
// GLOBAL VARIABLES ========================================================= //
// ========================================================================== //

// Main structure
struct context CONTEXT = {
    .map = NULL,
    .obj = NULL,
    .prog_fd = 0,
    .xdp_flags = 0,
    .iface = {0},
    .map_fd = 0,
    .is_loaded = false,
    .is_attached = false,
};

// last timestamp
struct timespec last_timestamp = {
    .tv_sec = 0,
    .tv_nsec = 0,
};

// period
int period = 1;

// ========================================================================== //
// ROUTINES ================================================================= //
// ========================================================================== //

void debug_context()
{
    debugf("user", "BPF fd: %d, Map fd: %d", CONTEXT.prog_fd, CONTEXT.map_fd);
}

// Load an eBPF program into the kernel
int load(char *filename)
{
    // load and return the object
    CONTEXT.obj = load_bpf_program(filename, &CONTEXT.prog_fd);
    if (CONTEXT.obj == NULL)
    {
        errorf("user", "Error while loading BPF program %s", filename);
        return -1;
    }
    // set flag
    CONTEXT.is_loaded = true;

    // Get back the map
    CONTEXT.map_fd = get_map_fd_from_name(CONTEXT.obj, XDP_CARNX_MAP_NAME);
    if (CONTEXT.map_fd < 0)
    {
        errorf("user", "Error while retrieving the file descriptor of map %s", XDP_CARNX_MAP_NAME);
        return -2;
    }
    return 0;
}

// Attach the XDP program on the interface with given xdp_flags
int attach(char *iface, unsigned int xdp_flags)
{
    if (CONTEXT.obj == NULL)
        return -1;
    if (CONTEXT.is_attached == true)
    {
        warnf("user", "The program is already attached");
        return 0;
    }

    // attach
    int r = attach_bpf_program(CONTEXT.prog_fd, iface, xdp_flags);
    if (r < 0)
        return r;

    // update context
    CONTEXT.xdp_flags = xdp_flags;
    strncpy(CONTEXT.iface, iface, IFACE_LENGTH);
    CONTEXT.is_attached = true;
    return 0;
}

// Load and attach the eBPF program. If the program is already loaded
// it reloads it but through a different id (different file descriptor)
// It returns a negative integer when an error occured
// -1: Error while loading BPF program
// -2: Error while retrieving the file descriptor of the map
// -3: Error while attaching program
int load_and_attach(char *filename, char *iface, unsigned int xdp_flags)
{
    // init the object, load and attach everything
    CONTEXT.obj = load_bpf_program(filename, &CONTEXT.prog_fd);
    if (CONTEXT.obj == NULL)
    {
        errorf("user", "Error while loading BPF program %s", filename);
        return -1;
    }
    // set flag
    CONTEXT.is_loaded = true;

    // Get back the map
    CONTEXT.map_fd = get_map_fd_from_name(CONTEXT.obj, XDP_CARNX_MAP_NAME);
    if (CONTEXT.map_fd < 0)
    {
        errorf("user", "Error while retrieving the file descriptor of map %s", XDP_CARNX_MAP_NAME);
        return -2;
    }

    // attach
    if (attach_bpf_program(CONTEXT.prog_fd, iface, CONTEXT.xdp_flags) < 0)
    {
        errorf("user", "Error while attaching program to %s", iface);
        return -3;
    }

    // update context
    strncpy(CONTEXT.iface, iface, sizeof(CONTEXT.iface));
    CONTEXT.xdp_flags = xdp_flags;
    CONTEXT.is_attached = true;
    return 0;
}

// Get the counter value related to the given key
u64 get_counter(int key)
{
    /* For percpu maps, userspace gets a value per possible CPU */
    unsigned int nr_cpus = libbpf_num_possible_cpus();
    u64 values[nr_cpus];
    u64 accumulator = 0;
    int i = 0;

    if (bpf_map_lookup_elem(CONTEXT.map_fd, &key, values) != 0)
    {
        errorf("user", "BPF map lookup failed, failed key:0x%X ", key);
        return 0;
    }

    /* Sum values from each CPU */
    for (i = 0; i < nr_cpus; i++)
    {
        accumulator += values[i];
    }
    return accumulator;
}

// Get all counter values. You must ensure that the
// returned buffer has length >= nb_counters()
int get_all_counters(u64 *array)
{
    int i;
    for (i = 0; i < nb_counters(); i++)
    {
        array[i] = get_counter(i);
    }
    return 0;
}

// Get all counter values. You must ensure that the
// returned buffer has length >= nb_counters(). It returns
// the timestamp of the action (seconds and nanoseconds)
int get_all_counters_with_timestamp(u64 *array, u64 *sec, u64 *nsec)
{
    /* Update time. Several clocks are available:
     * CLOCK_REALTIME (0)
     *     System-wide realtime clock. Setting this clock requires appropriate privileges. 
     * CLOCK_MONOTONIC (1)
     *     Clock that cannot be set and represents monotonic time since some unspecified starting point. 
     * CLOCK_PROCESS_CPUTIME_ID (2)
     *     High-resolution per-process timer from the CPU. 
     * CLOCK_THREAD_CPUTIME_ID (3)
     *     Thread-specific CPU-time clock. 
     */
    clock_gettime(0, &last_timestamp);
    *sec = (u64)last_timestamp.tv_sec;
    *nsec = (u64)last_timestamp.tv_nsec;
    // fallback to the above function
    return get_all_counters(array);
}

// Detach the XDP program from the interface
int detach()
{
    if (CONTEXT.obj == NULL)
    {
        errorf("user", "The program seems not loaded");
        return -1;
    }
    if (CONTEXT.is_attached == false)
    {
        warnf("user", "The program is not attached");
        return 0;
    }
    int r = detach_bpf_program(CONTEXT.iface, CONTEXT.xdp_flags);
    if (r < 0)
        return r;

    // update context
    CONTEXT.is_attached = false;
    return 0;
}

// unload the eBPF program from the kernel
int unload()
{
    if (CONTEXT.is_loaded == false)
    {
        warnf("user", "The program is not loaded");
        return 0;
    }
    if (CONTEXT.is_attached)
    {
        errorf("user", "The program must be detached first");
        return -1;
    }
    int r = unload_bpf_program(CONTEXT.obj);
    // set flag
    if (r == 0)
        CONTEXT.is_loaded = false;
    return r;
}

// return the load status
bool is_loaded()
{
    return CONTEXT.is_loaded;
}

// return the attach status
bool is_attached()
{
    return CONTEXT.is_attached;
}

// Return the interface where the program
// is attached. You must ensure that dest
// points to a buffer with IFACE_LENGTH size.
int get_iface(char *dest)
{
    if (CONTEXT.is_attached == false)
    {
        errorf("user", "No program attached");
        return -1;
    }
    // copy
    strncpy(dest, CONTEXT.iface, IFACE_LENGTH);
    return 0;
}
