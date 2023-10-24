#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

int main()
{
	printf("Server started\n");
	int sockfd, newsockfd;
	int clilen;
	struct sockaddr_in cli_addr, serv_addr;

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("Cannot create socket\n");
		exit(0);
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(20000);

	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
	{
		perror("Unable to bind local address\n");
		exit(0);
	}

	listen(sockfd, 5);

	while (1)
	{
		char buf;
		clilen = sizeof(cli_addr);
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen) ;

		if (newsockfd < 0)
		{
			perror("Accept error\n");
			exit(0);
		}
		while (recv(newsockfd, &buf, 1, 0) > 0)
		{
			printf("Data received from %s:%d is %c\n", inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port), buf);
		}

		close(newsockfd);
	}

	return 0;
}