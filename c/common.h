// Common data for user and kernel
//
#ifndef _COMMON_H_
#define _COMMON_H_

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <time.h>

#include <linux/bpf.h>
#include <linux/if_link.h>
#include <linux/if_ether.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/types.h>

#include <arpa/inet.h>
#include <net/if.h>

// types
typedef unsigned long u64;
typedef unsigned int u32;
typedef unsigned short u16;
typedef unsigned char u8;

// file path
#define PATH_MAX 512

// map name
#define XDP_CARNX_MAP carnx_map
#define XDP_CARNX_MAP_NAME "carnx_map"
#define PIN_BASEDIR "/sys/fs/bpf/xdp"
#define MAX_NB_COUNTERS 256

// log constants
#define __debug "\033[37;1mDEBUG\033[0m"
#define __info "\033[32;1mINFO\033[0m"
#define __warn "\033[33;1mWARN\033[0m"
#define __error "\033[31;1mERROR\033[0m"

// Indices in the eBPF Map
enum Counter
{
    PKT,
    IP,
    IP6,
    TCP,
    UDP,
    ICMP,
    ICMP6,
    ARP,
    ACK,
    SYN,
    __END_OF_COUNTERS__, // aims to loop over the counters (it must be the last item)
};

// Activate/Desactivate debug log output
void set_debug(int active);

// Print a formatted message with gray color (with a newline)
void debugf(const char *module, const char *fmt, ...);

// Print a formatted message with green color (with a newline)
void infof(const char *module, const char *fmt, ...);

// Print a formatted message with red color (with a newline)
void errorf(const char *module, const char *fmt, ...);

// Print a formatted message with brown color (with a newline)
void warnf(const char *module, const char *fmt, ...);

// return the number of counters
int nb_counters();

// reverse counter lookup (id -> name)
int reverse_lookup(int c, char *name);

#endif // _COMMON_H_