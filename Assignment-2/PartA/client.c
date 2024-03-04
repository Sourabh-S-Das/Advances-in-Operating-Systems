#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

int main(int argc, char* argv[])
{
	printf("Client started\n");
	int sockfd;
	struct sockaddr_in serv_addr, cli_addr;

	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		perror("Unable to create socket\n");
		exit(0);
	}

	memset(&cli_addr, 0, sizeof(cli_addr));
    	memset(&serv_addr, 0, sizeof(serv_addr));

	//cli_addr.sin_family = AF_INET;
	//cli_addr.sin_addr.s_addr = INADDR_ANY;
	//cli_addr.sin_port = htons(atoi(argv[2]));

	if (bind(sockfd, (struct sockaddr *) &cli_addr, sizeof(cli_addr)) < 0)
	{
		perror("Unable to bind local address\n");
		exit(0);
	}

	serv_addr.sin_family = AF_INET;
	inet_aton("127.0.0.1", &serv_addr.sin_addr);
	serv_addr.sin_port = htons(atoi(argv[1]));
	for(int i = 0; i < 10; i++){
	int buf = rand() % 1000;
	sendto(sockfd, &buf, sizeof(int), 0, (const struct sockaddr *) &serv_addr, sizeof(serv_addr));
	printf("Data sent to %s:%d is %d\n", inet_ntoa(serv_addr.sin_addr), ntohs(serv_addr.sin_port), buf);
	//usleep(500000);
	}

	close(sockfd);

	return 0;
}