#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>

typedef struct request_data{
    int data;
    struct sockaddr_in cliaddr; 
}request_data;

request_data rq;
int handle_request = 0;
int num_free_thread_chng = 0;
int tot_time_thread[5]; 

pthread_t req_handlers[5], sender_thread, recv_thread;
pthread_mutex_t req_data_lock, num_free_thr_lock, print_lock;
pthread_cond_t req_data_cond, num_free_thr_cond;

int sockfd; 
struct sockaddr_in servaddr, cliaddr;

void *process_request(void *args)
{
    int* thr_ptr = (int*)args;
    int thr_no = (*thr_ptr);

    pthread_mutex_lock(&print_lock);
    printf("Thread %d is ready to process requests\n", thr_no);
    pthread_mutex_unlock(&print_lock);

    int send_sockfd = sockfd;
/*
    int send_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(send_sockfd < 0)
    {
        perror("Socket creation in sender thread failed\n");
        exit(EXIT_FAILURE);
    }
*/
    struct sockaddr_in lbaddr; 
    memset(&lbaddr, 0, sizeof(lbaddr));
    lbaddr.sin_family = AF_INET;
    lbaddr.sin_port = htons(8080);
    inet_aton("127.0.0.1", &lbaddr.sin_addr); 

    int val = 0;

    while(1)
    {
        int time = 0;
        struct sockaddr_in caddr;
        pthread_mutex_lock(&req_data_lock);
        while(!handle_request)
        {
            pthread_cond_wait(&req_data_cond, &req_data_lock);
        }
        time = rq.data;
        caddr = rq.cliaddr;
        handle_request = 0;
        pthread_mutex_unlock(&req_data_lock);
        pthread_cond_broadcast(&req_data_cond);

        pthread_mutex_lock(&print_lock);
        printf("Thread %d is handling the request of client at %s:%d with data = %d\n", thr_no, inet_ntoa(caddr.sin_addr), ntohs(caddr.sin_port), time);
        pthread_mutex_unlock(&print_lock);

        sleep(time); // processing the request

        tot_time_thread[thr_no - 1] += time;

        pthread_mutex_lock(&print_lock);
        printf("Thread %d has completed processing the request\n", thr_no);
        printf("Total amount of time the thread %d has processed till now = %d\n", thr_no, tot_time_thread[thr_no - 1]);
        pthread_mutex_unlock(&print_lock);

        pthread_mutex_lock(&num_free_thr_lock);
	/*
        while(num_free_thread_chng) // This is done to make sure that the previous send request has been handled before adding a new one
        {
            pthread_cond_wait(&num_free_thr_cond, &num_free_thr_lock);
        }
	*/
        // num_free_thread_chng = 1;
	sendto(send_sockfd, &val, sizeof(int), 0, (const struct sockaddr *) &lbaddr, sizeof(lbaddr));
	printf("Data is sent\n");
        printf("Thread %d is now free\n", thr_no);
        pthread_mutex_unlock(&num_free_thr_lock);
        pthread_cond_broadcast(&num_free_thr_cond);
    }
    pthread_exit(NULL);
    return NULL;
}

void *send_to_lb(void *args)
{
    int send_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(send_sockfd < 0)
    {
        perror("Socket creation in sender thread failed\n");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in lbaddr; 
    memset(&lbaddr, 0, sizeof(lbaddr));
    lbaddr.sin_family = AF_INET;
    lbaddr.sin_port = htons(8082);
    inet_aton("127.0.0.1", &lbaddr.sin_addr); 

    int val = 0;

    pthread_mutex_lock(&print_lock);
    printf("----------------------Sender Thread is ready-----------------------\n");
    pthread_mutex_unlock(&print_lock);

    while(1)
    {
        pthread_mutex_lock(&num_free_thr_lock);
        while(!num_free_thread_chng)
        {
            pthread_cond_wait(&num_free_thr_cond, &num_free_thr_lock);
        }

        sendto(send_sockfd, &val, sizeof(int), 0, (const struct sockaddr *) &lbaddr, sizeof(lbaddr));
        pthread_mutex_lock(&print_lock);
        printf("Message sent to load balancer about free thread\n");
        pthread_mutex_unlock(&print_lock);

        pthread_mutex_unlock(&num_free_thr_lock);
        pthread_cond_broadcast(&num_free_thr_cond);
    }

    close(send_sockfd);
    pthread_exit(NULL);
    return NULL;
}

void *recv_handler(void *args)
{
   pthread_mutex_lock(&print_lock);
   printf("---------------------Receiver Thread is ready------------------------\n");
   pthread_mutex_unlock(&print_lock);

   while(1)
   {
        int n; 
        socklen_t len;
        int recv_val = 0;
        len = sizeof(cliaddr);
        recvfrom(sockfd, &recv_val, sizeof(recv_val), 0, (struct sockaddr*)(&cliaddr), &len);
        pthread_mutex_lock(&print_lock);
        printf("A new request received at recv thread with value: %d\n", recv_val);
        pthread_mutex_unlock(&print_lock);

        pthread_mutex_lock(&req_data_lock);
        while(handle_request) // until previous request has been taken up it will not update
        {
            pthread_cond_wait(&req_data_cond, &req_data_lock);
        }
        rq.data = recv_val;
        rq.cliaddr = cliaddr;
        handle_request = 1;
        pthread_mutex_unlock(&req_data_lock);
        pthread_cond_broadcast(&req_data_cond);
   }
    pthread_exit(NULL);
    return NULL;
}

int main(int argc, char **argv)
{      
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if ( sockfd < 0 ) { 
        perror("socket creation failed\n"); 
        exit(EXIT_FAILURE); 
    } 
      
    memset(&servaddr, 0, sizeof(servaddr)); 
    memset(&cliaddr, 0, sizeof(cliaddr)); 
      
    servaddr.sin_family = AF_INET; 
    servaddr.sin_addr.s_addr = INADDR_ANY; 
    servaddr.sin_port = htons(atoi(argv[1])); 
       
    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0 ) 
    { 
        perror("bind failed\n"); 
        exit(EXIT_FAILURE); 
    }

    memset(tot_time_thread, 0, sizeof(tot_time_thread));

    pthread_mutex_init(&req_data_lock, NULL);
    pthread_mutex_init(&num_free_thr_lock, NULL);
    pthread_mutex_init(&print_lock, NULL);
    pthread_cond_init(&req_data_cond, NULL);
    pthread_cond_init(&num_free_thr_cond, NULL);

    int args_arr[5];
    for(int i = 0; i<5; i++)
    {
        args_arr[i] = i+1;
        pthread_create(&req_handlers[i], NULL, process_request, &args_arr[i]);
    }

    // pthread_create(&sender_thread, NULL, send_to_lb, NULL);
    pthread_create(&recv_thread, NULL, recv_handler, NULL);

    for(int i = 0; i<5; i++)
    {
        pthread_join(req_handlers[i], NULL);
    }

    pthread_join(sender_thread, NULL);
    pthread_join(recv_thread, NULL);

    pthread_mutex_destroy(&req_data_lock);
    pthread_mutex_destroy(&num_free_thr_lock);
    pthread_mutex_destroy(&print_lock);
    pthread_cond_destroy(&req_data_cond);
    pthread_cond_destroy(&num_free_thr_cond);

    close(sockfd);
    return 0;
}