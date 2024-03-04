#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

int main(int argc, char* argv[])
{
	printf("Server started\n");
	int sockfd;
	int clilen;
	struct sockaddr_in cli_addr, serv_addr;

    	memset(&serv_addr, 0, sizeof(serv_addr)); 
    	memset(&cli_addr, 0, sizeof(cli_addr)); 

	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		perror("Cannot create socket\n");
		exit(0);
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(atoi(argv[1]));

	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
	{
		perror("Unable to bind local address\n");
		exit(0);
	}

	listen(sockfd, 5);

	while (1)
	{
		int buf;
		clilen = sizeof(cli_addr);
        	recvfrom(sockfd, &buf, sizeof(int), 0, (struct sockaddr *) &cli_addr, &clilen);
		printf("Data received from %s:%d is %d\n", inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port), buf);
	}
	
	close(sockfd);
	return 0;
}