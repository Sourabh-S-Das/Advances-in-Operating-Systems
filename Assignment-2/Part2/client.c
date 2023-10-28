#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h>
#include <time.h> 

int main()
{
    int sockfd; 
    struct sockaddr_in servaddr; 

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if ( sockfd < 0 ) { 
        perror("socket creation failed\n"); 
        exit(EXIT_FAILURE); 
    } 
  
    memset(&servaddr, 0, sizeof(servaddr)); 
 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_port = htons(8080); 
    inet_aton("127.0.0.1", &servaddr.sin_addr); 

    int val = 0;
    printf("Do you want to manually type the message (press 1) or send it randomly (press 0) ? : ");
    scanf("%d",&val);

    if(val)
        while(1)
        {
            int time = 0;
            printf("Enter the time to send to the server: ");
            scanf("%d", &time);
            sendto(sockfd, &time, strlen(time), 0, (const struct sockaddr *) &servaddr, sizeof(servaddr));
        }
    else
        while(1)
        {
            int time = rand(time(NULL))%50;
            printf("Sending data = %d to the server\n", time);
            sendto(sockfd, &time, strlen(time), 0, (const struct sockaddr *) &servaddr, sizeof(servaddr));
            printf("Waiting before sending next data ...\n");
            sleep(10); // waiting before next send
        }

    close(sockfd);
    return 0;
}