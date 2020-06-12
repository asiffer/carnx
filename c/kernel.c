// common
#include "common.h"
// helper functions (local)
#include "libbpf/src/bpf_helpers.h"

// Simple map to store the counters
struct bpf_map_def SEC("maps") XDP_CARNX_MAP = {
    .type = BPF_MAP_TYPE_PERCPU_ARRAY,
    .key_size = sizeof(unsigned int), // counter_name
    .value_size = sizeof(u64),        // counter_balue
    .max_entries = MAX_NB_COUNTERS,   // number of keys
    .map_flags = 0,
};

static inline int increment_counter(int key)
{
    int localkey = key;
    u64 *count = bpf_map_lookup_elem(&carnx_map, &localkey);
    if (count)
    {
        *count += 1;
        return 0;
    }
    return 1;
}

static inline int update_eth_based_counter(struct ethhdr *eth)
{
    int index;
    switch (htons(eth->h_proto))
    {
    case ETH_P_IP:
        index = IP;
        break;
    case ETH_P_ARP:
        index = ARP;
        break;
    default:
        index = -1;
        break;
    }
    if (index < 0)
        return -1;

    return increment_counter(index);
}

static inline int update_ip_based_counter(struct iphdr *ip)
{
    int index;

    switch (ip->protocol)
    {
    case IPPROTO_TCP:
        index = TCP;
        break;
    case IPPROTO_UDP:
        index = UDP;
        break;
    case IPPROTO_ICMP:
        index = ICMP;
        break;
    case IPPROTO_ICMPV6:
        index = ICMP6;
        break;
    default:
        index = -1;
        break;
    }
    if (index < 0)
        return -1;

    return increment_counter(index);
}

static inline int update_tcp_based_counter(struct tcphdr *tcph)
{
    if (tcph->ack)
        increment_counter(ACK);
    if (tcph->syn)
        increment_counter(SYN);
    return 0;
}

SEC("prog")
int xdp_update_counters(struct xdp_md *ctx)
{
    void *end = (void *)(long)ctx->data_end;
    void *data = (void *)(long)ctx->data;
    u64 offset = 0;

    // a packet is received
    increment_counter(PKT);

    struct ethhdr *eth = data;
    offset += sizeof(struct ethhdr);
    /* make sure the bytes you want to read are within 
       the packet's range before reading them */
    if (data + offset > end)
    {
        return XDP_ABORTED;
    }

    // update counters using ethernet frame
    update_eth_based_counter(eth);

    struct iphdr *iph = data + offset;
    offset += sizeof(struct iphdr);
    /* make sure the bytes you want to read are within 
       the packet's range before reading them */
    if (data + offset > end)
    {
        return XDP_ABORTED;
    }

    // update counters using ip frame
    update_ip_based_counter(iph);

    if (iph->protocol == IPPROTO_TCP)
    {
        struct tcphdr *tcph = data + offset;
        offset += sizeof(struct tcphdr);
        /* make sure the bytes you want to read are within 
           the packet's range before reading them */
        if (data + offset > end)
        {
            return XDP_ABORTED;
        }
        // update counters using ip frame
        update_tcp_based_counter(tcph);
    }

    return XDP_PASS;
}

char __license[] SEC("license") = "GPL";