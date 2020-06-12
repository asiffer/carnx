#include "common.h"

int debug_status = 0;

// Activate/Desactivate debug log output
void set_debug(int d)
{
    if (d != 0)
        debug_status = 1;
    else
        debug_status = 0;
}
// ========================================================================== //
// UTILS ==================================================================== //
// ========================================================================== //

// Print a formatted message with red color (with a newline)
void errorf(const char *module, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    printf("[%s]\t%6s: ", __error, module);
    vprintf(fmt, args);
    printf("\n");
    va_end(args);
}

// Print a formatted message with green color (with a newline)
void infof(const char *module, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    printf("[%s]\t%6s: ", __info, module);
    vprintf(fmt, args);
    printf("\n");
    va_end(args);
}

// Print a formatted message with brown color (with a newline)
void warnf(const char *module, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    printf("[%s]\t%6s: ", __warn, module);
    vprintf(fmt, args);
    printf("\n");
    va_end(args);
}

// Print a formatted message with gray color (with a newline)
void debugf(const char *module, const char *fmt, ...)
{
    if (debug_status == 0)
        return;
    va_list args;
    va_start(args, fmt);
    printf("[%s]\t%6s: ", __debug, module);
    vprintf(fmt, args);
    printf("\n");
    va_end(args);
}

int nb_counters()
{
    return (int)__END_OF_COUNTERS__;
}

int reverse_lookup(int c, char *name)
{
    switch ((enum Counter)c)
    {
    case PKT:
        strcpy(name, "PKT");
        break;
    case IP:
        strcpy(name, "IP");
        break;
    case IP6:
        strcpy(name, "IP6");
        break;
    case TCP:
        strcpy(name, "TCP");
        break;
    case UDP:
        strcpy(name, "UDP");
        break;
    case ICMP:
        strcpy(name, "ICMP");
        break;
    case ICMP6:
        strcpy(name, "ICMP6");
        break;
    case ARP:
        strcpy(name, "ARP");
        break;
    case ACK:
        strcpy(name, "ACK");
        break;
    case SYN:
        strcpy(name, "SYN");
        break;
    // case NEWCOUNTER:
    //      strcpy(name, "NEWCOUNTER");
    //      break;
    default:
        return -1;
    }
    return 0;
}