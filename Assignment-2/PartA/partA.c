#include "vmlinux.h"
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_endian.h>

SEC("xdp")
int packet_process(struct xdp_md *skb){
    void *data = (void *)(long)skb->data;
    void *data_end = (void *)(long)skb->data_end;

    struct ethhdr *eth = data;
    if ((void *)(eth + 1) > data_end) {
        return XDP_PASS;
    }

    struct iphdr *ip = (struct iphdr *)(eth + 1);
    if ((void *)(ip + 1) > data_end) {
        return XDP_PASS;
    }

    if (ip->protocol != IPPROTO_UDP) {
        return XDP_PASS;
    }

    struct udphdr *udp = (struct udphdr *)(ip + 1);
    if ((void *)(udp + 1) > data_end) {
        return XDP_PASS;
    }
    
    int *content = (int *)(udp + 1);
    if ((void *)(content + 1) > data_end) {
        return XDP_PASS;
    }

    int dest_port = bpf_ntohs(udp->dest);
    
    if (dest_port != 8080) {
    	return XDP_PASS;
    }
    
    int src_port = bpf_ntohs(udp->source);
    bpf_printk("Source port : %d, Dest port : %d\n", src_port, dest_port);
    
    bpf_printk("Data in the packet : %d\n", *content);
    
    if ((*content) % 2 == 0) {
    	bpf_printk("Packet dropped\n");
    	return XDP_DROP;
    }
    
    bpf_printk("Packet not dropped\n");
    return XDP_PASS;
}

char LICENSE[] SEC("license") = "GPL";
