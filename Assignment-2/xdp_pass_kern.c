#include <linux/bpf.h>
#include <linux/if_ether.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/in.h>
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_endian.h>

SEC("xdp")
int xdp_sequence_number_and_port(struct xdp_md *skb) {
    void *data = (void *)(long)skb->data;
    void *data_end = (void *)(long)skb->data_end;

    struct ethhdr *eth = data;
    if ((void *)(eth + 1) > data_end) {
        return XDP_PASS;
    }

    if (eth->h_proto != __constant_htons(ETH_P_IP)) {
        return XDP_PASS;
    }

    struct iphdr *ip = (struct iphdr *)(eth + 1);
    if ((void *)(ip + 1) > data_end) {
        return XDP_PASS;
    }

    if (ip->protocol != IPPROTO_TCP) {
        return XDP_PASS;
    }

    struct tcphdr *tcp = (struct tcphdr *)(ip + 1);
    if ((void *)(tcp + 1) > data_end) {
        return XDP_PASS;
    }

    // Print the sequence number and source port
    int seq = bpf_ntohs(tcp->seq), port = bpf_ntohs(tcp->dest);
    
    if (port != 20000) {
    	return XDP_PASS;
    }
    
    bpf_printk("Seq: %u, Dest Port: %u\n", seq, port);

    if (seq % 2 == 0) {
    	bpf_printk("Packet dropped\n");
    	return XDP_PASS;
    }
    else {
    	bpf_printk("Packet passed\n");	// Needs change from passed to forwarded
    	return XDP_PASS;
    }
}

char _license[] SEC("license") = "GPL";
