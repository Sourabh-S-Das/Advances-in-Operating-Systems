#include "vmlinux.h"
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_endian.h>

struct {
   __uint(type, BPF_MAP_TYPE_QUEUE);
   __uint(max_entries, 128);
   __type(value, int);
} queue SEC(".maps");

struct {
   __uint(type, BPF_MAP_TYPE_HASH);
   __uint(max_entries, 3);
   __type(key, int);
   __type(value, int);
} cntr SEC(".maps");

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
    
    int max_threads = 5;
    int serv1 = 1, serv2 = 2, serv3 = 3;
    int port1 = 8081, port2 = 8082, port3 = 8083;
    
    bpf_map_update_elem(&cntr, &serv1, &max_threads, BPF_NOEXIST);
    bpf_map_update_elem(&cntr, &serv2, &max_threads, BPF_NOEXIST);
    bpf_map_update_elem(&cntr, &serv3, &max_threads, BPF_NOEXIST);
    
    int src_port = bpf_ntohs(udp->source);
    bpf_printk("Source port : %d, Dest port : %d\n", src_port, dest_port);
    
    /*
    This is the checksum part which doesn't work for now.
    
    uint32_t res = 0;
    uint32_t pseudo_header[10];
    pseudo_header[0] = ip->saddr;
    pseudo_header[1] = ip->daddr;
    pseudo_header[2] = 0;
    pseudo_header[3] = 17;
    pseudo_header[4] = 8;
    
    int length =  sizeof(pseudo_header) / sizeof(uint16_t);
    uint32_t sum = 0;
    uint16_t *data1 = pseudo_header;

    while (length > 1) {
        sum += *data1++;
        length -= 2;
    }

    if (length > 0) {
        sum += *(uint8_t *)data1;
    }

    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }
    
    sum = ~sum;
    
    res += sum;
    
    length = 4;
    data1 = udp;
    sum = 0;
    while (length > 1) {
        sum += *data1++;
        length -= 2;
    }

    if (length > 0) {
        sum += *(uint8_t *)data1;
    }

    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }
    
    sum = ~sum;
    res += sum;
    
    while (res >> 16) {
        res = (res & 0xFFFF) + (res >> 16);
    }
    
    res = ~res;

    
    
    bpf_printk("CKSUM : %u, obtained : %u\n", udp->check, res);
    */
    int val;
    
    if (src_port == 8081)
    {
    	if (bpf_map_pop_elem(&queue, &val))
    	{
    		int *res = bpf_map_lookup_elem(&cntr, &serv1);
    		if (!res)
    		{
    			return XDP_PASS;
    		}
    		*res = *res + 1;
    		bpf_map_update_elem(&cntr, &serv1, res, BPF_EXIST);
    		bpf_printk("Server 1 counter incremented\n");
    	}
    	else
    	{
    		*content = val;
    		udp->dest = udp->source;
    		udp->source = bpf_htons(8080);
    		bpf_printk("Packet dequeued and sent to server 1\n");
    		return XDP_PASS;
    	}
    }
    
    else if (src_port == 8082)
    {
    	if (bpf_map_pop_elem(&queue, &val))
    	{
    		int *res = bpf_map_lookup_elem(&cntr, &serv2);
    		if (!res)
    		{
    			return XDP_PASS;
    		}
    		*res = *res + 1;
    		bpf_map_update_elem(&cntr, &serv2, res, BPF_EXIST);
    		bpf_printk("Server 2 counter incremented\n");
    	}
    	else
    	{
    		*content = val;
    		udp->dest = udp->source;
    		udp->source = bpf_htons(8080);
    		bpf_printk("Packet dequeued and sent to server 2\n");
    		return XDP_PASS;
    	}
    }
    
    else if (src_port == 8083)
    {
    	if (bpf_map_pop_elem(&queue, &val))
    	{
    		int *res = bpf_map_lookup_elem(&cntr, &serv3);
    		if (!res)
    		{
    			return XDP_PASS;
    		}
    		*res = *res + 1;
    		bpf_map_update_elem(&cntr, &serv3, res, BPF_EXIST);
    		bpf_printk("Server 3 counter incremented\n");
    	}
    	else
    	{
    		*content = val;
    		udp->dest = udp->source;
    		udp->source = bpf_htons(8080);
    		bpf_printk("Packet dequeued and sent to server 3\n");
    		return XDP_PASS;
    	}
    }
    
    else
    {
    	int *res = bpf_map_lookup_elem(&cntr, &serv1);
    	if (!res)
    	{
    		return XDP_PASS;
    	}
    	if (*res != 0)
    	{
    		*res -= 1;
    		bpf_map_update_elem(&cntr, &serv1, res, BPF_EXIST);
    		udp->dest = bpf_htons(8081);
    		bpf_printk("Packet redirected to server 1\n");
    		return XDP_PASS;
    	}
    	
    	res = bpf_map_lookup_elem(&cntr, &serv2);
    	if (!res)
    	{
    		return XDP_PASS;
    	}
    	if (*res != 0)
    	{
    		*res -= 1;
    		bpf_map_update_elem(&cntr, &serv2, res, BPF_EXIST);
    		udp->dest = bpf_htons(8082);
    		bpf_printk("Packet redirected to server 2\n");
    		return XDP_PASS;
    	}
    	
    	res = bpf_map_lookup_elem(&cntr, &serv3);
    	if (!res)
    	{
    		return XDP_PASS;
    	}
    	if (*res != 0)
    	{
    		*res -= 1;
    		bpf_map_update_elem(&cntr, &serv3, res, BPF_EXIST);
    		udp->dest = bpf_htons(8083);
    		bpf_printk("Packet redirected to server 3\n");
    		return XDP_PASS;
    	}
    	
    	val = *content;
    	bpf_map_push_elem(&queue, &val, BPF_EXIST);
    	bpf_printk("Packet enqueued\n");
    	
    }
    
    return XDP_DROP;
}

char LICENSE[] SEC("license") = "GPL";
