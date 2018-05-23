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
	FILE 						*tosec_fp;
	FILE 						*email_fp;
	long unsigned int 			num_shares;
	long unsigned int 			tot_shares;
	int 						option;
	int 						state;

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

		recv(newsockfd, buffer, sizeof(buffer), 0);
		option = atoi(buffer);

		if(option == 1) { /* Compartir el secreto */

			tosec_fp = fopen("f2sec", "w+");

			recv(newsockfd, buffer, sizeof(buffer), 0);
			unsigned fsz = atoi(buffer);
			
			bzero(buffer, sizeof(buffer));

			while((received_bytes = recv(newsockfd, buffer, sizeof(buffer), 0)) > 0){
				fwrite(buffer, sizeof(char), sizeof(buffer), tosec_fp);
			}

			fclose(tosec_fp);
			truncate("f2sec", fsz);

			fprintf(stdout, "File for sharing received and saved, %d bytes.\n", fsz);



			email_fp = fopen("emails", "w+");

			recv(newsockfd, buffer, sizeof(buffer), 0);
			fsz = atoi(buffer);

			bzero(buffer, sizeof(buffer));

			while((received_bytes = recv(newsockfd, buffer, sizeof(buffer), 0)) > 0){
				fwrite(buffer, sizeof(char), sizeof(buffer), email_fp);
			}

			fclose(email_fp);
			truncate("emails", fsz);

			fprintf(stdout, "Emails file reveived\n");

			bzero(buffer, sizeof(buffer));
			recv(newsockfd, buffer, sizeof(buffer), 0);
			num_shares = atol(buffer);

			bzero(buffer, sizeof(buffer));
			recv(newsockfd, buffer, sizeof(buffer), 0);
			tot_shares = atol(buffer);

			state = start_thread("file", num_shares, tot_shares);

			// parse emails and send shares
			system("echo 'This is the message body' | mutt -a '/path/to/file.to.attach' -s 'subject of message' -- recipient@domain.com");

			// clean resources

			// send state to the client
			sprintf(buffer, "%d", state);
			send(newsockfd, buffer, sizeof(buffer), 0);

		} else if(option == 0) {
			// receive shares

			//decipher

			// send back secret
		} else {
			fprintf(stderr, "Not supportable option %d\n", option);
		}

		close(newsockfd);
		printf("Connection is closed.\n");
	}
}

/* qsort function that order a string according the cmpfunc (in our case the natural order) */
void natural_qsort(char str[]) {
  qsort(str, (size_t) strlen(str), (size_t) sizeof(char), strcmp);
}
