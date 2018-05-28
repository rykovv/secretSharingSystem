#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>

#define BUFSIZE 1024

#define TCP_NODELAY 1
#define SOL_TCP	6

main(int argc, char ** argv) {
	if(argc != 2){
		perror("Wrong number of arguments: <port number>\n");
		return -1;
	}
    int 						sockfd, newsockfd, clilen, received_bytes, read_bytes;
	int 						option, state, finished, rest_file, n, i, fsz;
	struct sockaddr_in   		cli_addr, serv_addr;
	char 	             		buffer[BUFSIZE], file_name[BUFSIZE], file_din[BUFSIZE], number[20];
	FILE 						*tosec_fp, *email_fp, *secret, *share;
	long unsigned int 			num_shares, tot_shares;

	
	if(1){

		ex_test("emails");

		return 0;
	}
	


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
		printf("Esperando por un cliente ...\n");
		listen(sockfd, 5);
		clilen = sizeof(cli_addr);
		newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);

		int one = 1;
		setsockopt(newsockfd, SOL_TCP, TCP_NODELAY, &one, sizeof(one));
		
		if (newsockfd < 0) {
		    perror("server: accept error") ;
		    return -1 ;
		}

		char *ip_in = inet_ntoa(cli_addr.sin_addr);
		
		printf("Concexión aceptada: <%s> : <%d>\n", ip_in, cli_addr.sin_port);

		finished = 0;

		bzero(buffer, BUFSIZE);
		printf("Esperando que el cliente elija la operación ...\n");

		recv(newsockfd, buffer, BUFSIZE, 0);
		option = atoi(buffer);

		if(option == 1) { /* Compartir el secreto */
	
			fprintf(stdout, "Opcion compartir un secreto.\n");

			/* ----------------------------------------- */

			recv(newsockfd, buffer, BUFSIZE, 0);
			unsigned fsz = atoi(buffer);
			rest_file = fsz;

			fprintf(stdout, "El tamaño del fichero a compartir %d bytes\n", fsz);
			
			tosec_fp = fopen("f2sec", "w+");
			bzero(buffer, BUFSIZE);

			while(rest_file > 0){
				received_bytes = recv(newsockfd, buffer, BUFSIZE, 0);
				fwrite(buffer, sizeof(char), received_bytes, tosec_fp);
				fprintf(stdout, "%d - %d = %d\n", rest_file, received_bytes, rest_file - received_bytes);
				fprintf(stdout, "Escrito %d bytes del fichero.\n", received_bytes);
				rest_file -= received_bytes;
			}

			fclose(tosec_fp);
			truncate("f2sec", fsz);

			fprintf(stdout, "El fichero a compartir fue recibido y almacendado.\n");

			/* ----------------------------------------- */

			email_fp = fopen("emails", "w+");

			recv(newsockfd, buffer, BUFSIZE, 0);
			fsz = atoi(buffer);
			rest_file = fsz;

			fprintf(stdout, "El tamaño del fichero de emails = %d bytes\n", fsz);

			//sleep(1);
			bzero(buffer, BUFSIZE);

			while(rest_file > 0){
				received_bytes = recv(newsockfd, buffer, BUFSIZE, 0);
				fwrite(buffer, sizeof(char), received_bytes, email_fp);
				rest_file -= received_bytes;
				fprintf(stdout, "Recibido y escrito %d bytes del fichero emails\n", received_bytes);
			}

			fclose(email_fp);
			truncate("emails", fsz);

			fprintf(stdout, "El fichero con emails fue recibido.\n");

			/* ----------------------------------------- */

			bzero(buffer, BUFSIZE);
			recv(newsockfd, buffer, BUFSIZE, 0);
			num_shares = atol(buffer);

			fprintf(stdout, "Número mínimo de comparticiones = %d\n", num_shares);

			bzero(buffer, BUFSIZE);
			recv(newsockfd, buffer, BUFSIZE, 0);
			tot_shares = atol(buffer);

			fprintf(stdout, "Número total de comparticiones = %d\n", tot_shares);			

			/* ----------------------------------------- */

			fprintf(stdout, "Se están realizando los cáclulos...\n");

			state = start_thread("f2sec", num_shares, tot_shares);

			/* ----------------------------------------- */

			// parse emails and send shares
			if(system("./sendShares.sh") < 0){
				fprintf(stdout, "Hubo un error en el envío de comparticiones.\n");
				state = 1;
			} else {
				fprintf(stdout, "Las comparticiones fueron creadas y enviadas con éxito.\n");
			}

			/* ----------------------------------------- */

			// clean resources
			//system("./cleanRes.sh");

			/* ----------------------------------------- */

			// send state to the client
			sprintf(buffer, "%d", state);
			send(newsockfd, buffer, sizeof(buffer), 0);

			/* ----------------------------------------- */

		} else if(option == 0) {
			// receive shares
			fprintf(stdout, "Opcion calcular el secreto.\nRecibiendo comparticiones...\n");
			finished = 0;
			i = 0;

			//bzero(buffer, BUFSIZE);
			recv(newsockfd, buffer, BUFSIZE, 0);
			tot_shares = atoi(buffer);

			fprintf(stdout, "Número total de comparticiones = %d\n", tot_shares);

			recv(newsockfd, buffer, BUFSIZE, 0);
			num_shares = atoi(buffer);

			fprintf(stdout, "Número mínimo de comparticiones = %d\n", num_shares);
			//bzero(buffer, BUFSIZE);

			strcpy(file_name, "shares/share_");
			
			while((!finished) && (i < tot_shares)){

				bzero(buffer, BUFSIZE);
				//fprintf(stdout, "buf %s\n", buffer);
				while(recv(newsockfd, buffer, BUFSIZE, 0) != 4);
				//fprintf(stdout, "buf %s\n", buffer);
				fsz = atoi(buffer);
				rest_file = fsz;

				fprintf(stdout, "El tamaño de la compartición %d = %d bytes\n", i, fsz);

				strcpy(file_din, file_name);
				sprintf(number, "%ld", i);
				strcat(file_din, number);
				//bzero(buffer, BUFSIZE);
				share = fopen(file_din, "w+");

				while(rest_file > 0){
					received_bytes = recv(newsockfd, buffer, BUFSIZE, 0);
					fwrite(buffer, sizeof(char), received_bytes, share);
					rest_file -= received_bytes;
					fprintf(stdout, "Recibido y escrito %d bytes de la compartición %d\n", received_bytes, i);
				}

				fclose(share);
				truncate(file_din, fsz);

				fprintf(stdout, "La compartición %d fue recibida.\n", i);

				//bzero(buffer, BUFSIZE);
				recv(newsockfd, buffer, BUFSIZE, 0);
				finished = atoi(buffer);

				i++;
			}

			fprintf(stdout, "Todas las comparticiones fueron recibidas\n");

			//decipher
			secret_from_files(num_shares, tot_shares);

			// send back secret
			secret = fopen("shares/secret", "r");

			fseek(secret, 0L, SEEK_END);
			fsz = ftell(secret);
			fseek(secret, 0L, SEEK_SET);

			sprintf(buffer, "%d", fsz);
			//sleep(5);
			fprintf(stdout, "El tamaño del secreto a enviar es %s\n", buffer);
			send(newsockfd, buffer, sizeof(fsz), 0);
			sleep(1);
			while(((read_bytes = fread(buffer, sizeof(char), BUFSIZE, secret)) > 0) && (fsz > 0)) {
				send(newsockfd, buffer, read_bytes, 0);
				fsz -= read_bytes;
				printf("Sent %d bytes secret %d\n", read_bytes, i);
			}
			fprintf(stdout, "El secreto fue enviado al cliente.\n");
			fclose(secret);

		} else {
			fprintf(stderr, "Not supportable option %d\n", option);
		}

		close(newsockfd);
		printf("Connection is closed.\n");
	}
}
