#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

int main()
{
	printf("Client started\n");
	for(int i = 0; i < 10; i++){
	int sockfd;
	struct sockaddr_in serv_addr, cli_addr;

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("Unable to create socket\n");
		exit(0);
	}

	//cli_addr.sin_family = AF_INET;
	//cli_addr.sin_addr.s_addr = INADDR_ANY;
	//cli_addr.sin_port = htons(20001);

	if (bind(sockfd, (struct sockaddr *) &cli_addr, sizeof(cli_addr)) < 0)
	{
		perror("Unable to bind local address\n");
		exit(0);
	}

	serv_addr.sin_family = AF_INET;
	inet_aton("127.0.0.1", &serv_addr.sin_addr);
	serv_addr.sin_port = htons(20000);

	int one = 1;
	setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &one, sizeof(one));

	if ((connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr))) < 0)
	{
		perror("Unable to connect to server\n");
		exit(0);
	}

		char buf = 'a' + rand() % 26;
		send(sockfd, &buf, 1, 0);
		printf("Data sent to %s:%d is %c\n", inet_ntoa(serv_addr.sin_addr), ntohs(serv_addr.sin_port), buf);
		//usleep(500000);
	

	close(sockfd);
}

	return 0;
}