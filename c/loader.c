#include "loader.h"

// Build the basedir /sys/fs/bpf/xdp/<iface>/ and the final
// endpoint /sys/fs/bpf/xdp/<iface>/carnx_map
int prepare_pin_endpoints(char *iface, char *pin_dir, char *path_to_map)
{
    int len;

    if (mkdir(PIN_BASEDIR, 0700) < 0)
    {
        switch (errno)
        {
        case EEXIST:
            warnf("loader", "The directory %s already exists", PIN_BASEDIR);
            break;
        default:
            infof("loader", "Failed to create directory %s: %s", PIN_BASEDIR, strerror(errno));
            return -1;
        }
    }
    else
    {
        infof("loader", "The directory %s has been created", PIN_BASEDIR);
    }

    len = snprintf(pin_dir, PATH_MAX, "%s/%s",
                   PIN_BASEDIR, iface);
    if (len < 0)
    {
        errorf("loader", "Error while building the directory of pinned map");
        return len;
    }

    len = snprintf(path_to_map, PATH_MAX, "%s/%s/%s",
                   PIN_BASEDIR, iface, XDP_CARNX_MAP_NAME);
    if (len < 0)
    {
        errorf("loader", "Error while building pinned map path");
        return len;
    }

    return 0;
}

// This function loads a BPF program (XDP) into the kernel. It fills a bpf_object
// and also returns the program file descriptor
int _load_bpf_program(char *filename, struct bpf_object **obj, int *prog_fd)
{
    // Attributes for BPF program
    struct bpf_prog_load_attr prog_load_attr = {
        .prog_type = BPF_PROG_TYPE_XDP, // XDP program
        .file = filename,               // Compiled BPF program
    };

    // load BPF program
    int err = bpf_prog_load_xattr(&prog_load_attr, obj, prog_fd);
    if (err < 0)
    {
        errorf("loader", "Error while loading BPF program: %d", strerror(-err));
        return err;
    }
    infof("loader", "BPF program '%s' loaded", filename);
    return 0;
}

struct bpf_object *load_bpf_program(char *filename, int *prog_fd)
{
    struct bpf_object *obj;
    if (_load_bpf_program(filename, &obj, prog_fd) < 0)
        return NULL;
    return obj;
}

int unload_bpf_program(struct bpf_object *obj)
{
    int err = bpf_object__unload(obj);
    if (err < 0)
    {
        errno = -err;
        errorf("loader", "Error while unloading BPF program: %s", strerror(-err));
        return err;
    }
    infof("loader", "BPF program '%s' unloaded", bpf_object__name(obj));
    return 0;
}

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
int attach_bpf_program(int prog_fd, char *iface, unsigned int xdp_flags)
{
    int err;
    // find if interface exists
    int ifindex = if_nametoindex(iface);
    if (!ifindex)
    {
        errorf("loader", "Interface %s not found (%s)", iface, strerror(errno));
        return -errno;
    }

    err = bpf_set_link_xdp_fd(ifindex, prog_fd, xdp_flags);
    if (err < 0)
    {
        errno = -err;
        errorf("loader", "Error while attaching XDP program to '%s': %s", iface, strerror(errno));
        return err;
    }
    infof("loader", "BPF program attached to '%s'", iface);
    return 0;
}

int detach_bpf_program(char *iface, unsigned int xdp_flags)
{
    int err;
    // find if interface exists
    int ifindex = if_nametoindex(iface);
    if (!ifindex)
    {
        errorf("loader", "Interface %s not found (%s)", iface, strerror(errno));
        return -errno;
    }

    err = bpf_set_link_xdp_fd(ifindex, -1, xdp_flags);
    if (err < 0)
    {
        errno = -err;
        errorf("loader", "Error while detaching XDP program from '%s': %s", iface, strerror(errno));
        return err;
    }
    infof("loader", "BPF program detached from '%s'", iface);
    return 0;
}

// Remove maps endpoints (on the filesystem) if they exist
int unpin_maps_if_exist(struct bpf_object *obj, char *pin_dir, char *path_to_map)
{
    int err;
    if (access(path_to_map, F_OK) != -1)
    {
        debugf("Previous pinned map found at %s", path_to_map);
        err = bpf_object__unpin_maps(obj, pin_dir);
        if (err)
        {
            errorf("loader", "Error while unpinning map within %s", pin_dir);
            errno = -err;
            return err;
        }
        infof("loader", "Previous maps at %s is now unpinned", path_to_map);
    }
    return 0;
}

int pin_maps(struct bpf_object *obj, char *pin_dir)
{
    int err = bpf_object__pin_maps(obj, pin_dir);
    if (err)
    {
        errorf("loader", "Error while pinning maps: %s", strerror(-err));
        errno = -err;
        return err;
    }
    infof("loader", "Map pinned in %s", pin_dir);
    return 0;
}

// Does everything but return the object
struct bpf_object *load_attach_pin(char *filename, char *iface, unsigned int xdp_flags)
{
    struct bpf_object *obj;
    int prog_fd = -1;
    char path_to_map[PATH_MAX];
    char pin_dir[PATH_MAX];

    // Load the kernel code
    if (_load_bpf_program(filename, &obj, &prog_fd))
        return NULL;

    // Attach the program to the interface
    if (attach_bpf_program(prog_fd, iface, xdp_flags))
        return NULL;

    // Prepare paths. After this point pin_dir and path_to_map
    // are initialized
    if (prepare_pin_endpoints(iface, pin_dir, path_to_map) < 0)
        return NULL;

    // Unpin previous maps
    if (unpin_maps_if_exist(obj, pin_dir, path_to_map))
        return NULL;

    // Pin maps
    if (pin_maps(obj, pin_dir))
        return NULL;

    return obj;
}

int get_map_fd_from_name(struct bpf_object *obj, char *map_name)
{
    struct bpf_map *map = bpf_object__find_map_by_name(obj, map_name);
    if (!map)
    {
        errorf("user", "Cannot find map with name %s", map_name);
        return -1;
    }
    return bpf_map__fd(map);
}