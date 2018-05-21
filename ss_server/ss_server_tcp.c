#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>

void natural_qsort(char str[]);

main(int argc, char ** argv) {
	if(argc != 2){
		perror("Wrong number of arguments: <port number>\n");
		return -1;
	}
    int 						sockfd, newsockfd, clilen, n;
	struct sockaddr_in   		cli_addr, serv_addr;
	char 	             		buffer[1024];
	int 						received_bytes;
	FILE 						*fp;

	if ((sockfd = socket(AF_INET, SOCK_STREAM,0)) < 0){
		perror("server: can't open stream socket") ;
		return -1;
	}

  	serv_addr.sin_family        	= AF_INET;
  	serv_addr.sin_addr.s_addr   	= htonl(0);
	serv_addr.sin_port          	= htons(atoi(argv[1]));

	if (bind(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) {
		perror("server: can't bind local address") ;
    	return -1;
	}

	while (1) {
		printf("Waiting for a new client ...\n");
		listen(sockfd, 5);
		clilen = sizeof(cli_addr);
		newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
		
		if (newsockfd < 0) {
		    perror("server: accept error") ;
		    return -1 ;
		}

		char *ip_in = inet_ntoa(cli_addr.sin_addr);
		
		printf("Connection accepted: <%s> : <%d>\n", ip_in, cli_addr.sin_port);

		int finished = 0;

		bzero(buffer, sizeof(buffer));
		printf("Receiving a message ...\n");

		fp = fopen("secret", "w+");

		recv(newsockfd, buffer, sizeof(buffer), 0);
		unsigned fsz = atoi(buffer);

		printf("size %d\n", fsz);
		
		bzero(buffer, sizeof(buffer));

		while((received_bytes = recv(newsockfd, buffer, sizeof(buffer), 0)) > 0){
			fwrite(buffer, sizeof(char), sizeof(buffer), fp);
		}

		fclose(fp);
		truncate("secret", fsz);

		/*


	   	if ((n = recv(newsockfd, buffer, sizeof(buffer), 0)) < 0){
		    perror("ERROR: recv");
		    return -1;
		}

		if(!strcmp(buffer, "FINISH\n") || n <= 0){
			if(!strcmp(buffer, "FINISH\n"))
				send(newsockfd, "OK\n", 4, 0);
			finished = 1;
		}
		else {
			printf("Received %d bytes\nReceived message: %s\n", n ,buffer);

			natural_qsort(buffer);
			strcat(buffer, "\n");

			if ((n = send(newsockfd, buffer, strlen(buffer), 0)) != strlen(buffer)) {
			    perror("ERROR: send");
			    return -1;
			}
		}
		*/
		close(newsockfd);
		printf("Connection is closed.\n");
	}
}

/* qsort function that order a string according the cmpfunc (in our case the natural order) */
void natural_qsort(char str[]) {
  qsort(str, (size_t) strlen(str), (size_t) sizeof(char), strcmp);
}
