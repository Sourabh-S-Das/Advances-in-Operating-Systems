#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/perf_event.h>
#include <linux/bpf.h>
#include <net/if.h>
#include <errno.h>
#include <assert.h>
#include <sys/sysinfo.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <bpf/libbpf.h>
#include <bpf/bpf.h>
#include <sys/resource.h>
#include <libgen.h>
#include <linux/if_link.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>

struct bpf_object *bpfobj;

int main(){
    
    int err  = 0;
    struct rlimit rlim = {
        .rlim_cur = 512UL << 20,
        .rlim_max = 512UL << 20,
    };
    struct bpf_link *link = NULL;

    err = setrlimit(RLIMIT_MEMLOCK, &rlim);
    if (err) {
        printf("failed to change rlimit\n");
        exit(EXIT_FAILURE);
    }
    
    bpfobj = bpf_object__open_file("bpf.object.o", NULL);
    if(libbpf_get_error(bpfobj)){
        printf("failed to open and/or load BPF object\n");
        exit(EXIT_FAILURE);
    }
    
    if(bpf_object__load(bpfobj)){
        printf("failed to load BPF object %d\n", err);
        bpf_link__destroy(link);
        bpf_object__close(bpfobj);
        exit(EXIT_FAILURE);
    }
    
    int ifindex = if_nametoindex("lo");
    if(!ifindex){
        printf("Invalid interface name\n");
        exit(EXIT_FAILURE);
    }
    
    int progfd = bpf_program__nth_fd(bpf_object__find_program_by_name(bpfobj, "packet_process"), 0);
    if (progfd < 0) {
        printf("Error: Finding XDP program\n");
        bpf_object__close(bpfobj);
        exit(EXIT_FAILURE);
    }
    
    if(bpf_set_link_xdp_fd(ifindex, progfd, XDP_FLAGS_UPDATE_IF_NOEXIST)){
        printf("Error: Attaching XDP program to interface\n");
        close(progfd);
        bpf_object__close(bpfobj);
        exit(EXIT_FAILURE);
    }
    printf("eBPF XDP code loaded successfully\n");
    printf("Enter any character to unload and exit the program\n");
    char c;
    scanf("%c", &c);
    bpf_set_link_xdp_fd(ifindex, -1, XDP_FLAGS_UPDATE_IF_NOEXIST);
    close(progfd);
    bpf_object__close(bpfobj);
}

