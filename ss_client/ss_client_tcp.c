/* Cliente de ECO sobre TCP */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <openssl/evp.h>
#include <openssl/aes.h>
#include <openssl/err.h>
#include <unistd.h>

main(int argc, char ** argv) {
	if(argc != 3){
		perror("Wrong number of arguments: <server's IP> <server's port>\n");
		return -1;
	}

	int           			sockfd, n;
	struct sockaddr_in     	serv_addr ;
	char 		 			buffer[1024];
	char 					file_name[1024];
	char 					password[1024];
	size_t					sent_bytes;
	int 					option;
	int 					state;
	long unsigned int 		num_shares;
	long unsigned int 		tot_shares;

	FILE 					*tosec_fp;
	FILE 					*email_fp;


	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("client: can't open stream socket") ;
		return -1;
	}

	printf("Bienvenido a la aplicación de compartición de secretos!\n");

	serv_addr.sin_family        	= AF_INET ;
	serv_addr.sin_addr.s_addr   	= inet_addr(argv[1]); 
	serv_addr.sin_port          	= htons(atoi(argv[2]));

	printf("Conectando al servidor ...\n");

   	if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) != 0)  {
		perror("client: can't connect with server") ;
		return -1;
	}
	printf("Conectado al servidor con IP <%s> : <%s> !\n", argv[1], argv[2]);

	fprintf(stdout, "Indique la acción que quiere realizar:\n\t0 - Recuperar el secreto.\n\t1 - Compartir el sercreto\n");

	int finished = 0;

	while(!finished) {
		fgets(buffer, "%s", sizeof(char), stdin);
		option = atoi(buffer);
		
		if(option != 1 || option != 0){
			fprintf(stdout, "Opción inválida %d. (0/1):\n", );
		} else {
			finished = 1;
		}
	}

	if (option == 1) { /* Compartir el secreto */
		finished = 0;

		fprintf(stdout, "Indique el camino al fichero que quiere compartir:\n");
		while(!finished){

			fgets(file_name, sizeof(file_name), stdin);
			char* nl = strchr(file_name, '\n');
	    	if(nl != 0) {
	        	*nl = 0;
	    	} 

			if (access(file_name, F_OK) == -1) {
			    fprintf(stdout ,"Fichero %s no existe. Intente de nuevo:\n", file_name);
			} else {
				finished = 1;
			}
		}

		fprintf(stdout, "Introduzca la contraseña para el fichero (AES):\n");

		fgets(password, sizeof(password), stdin);
		sprintf(buffer, "openssl enc -aes-128-cbc -in %s -out f2send -k %s", file_name, password);

		system(buffer);

		tosec_fp = fopen("f2send", "r");

		/*printf("Write a message to send :\n");
		gets(buffer);

		if(strlen(buffer) == 0){
			printf("Cannot send empty string.\n");
			continue;
		}
		strcat(buffer, "\n");
		*/
		fseek(tosec_fp, 0L, SEEK_END);
		int fsz = ftell(tosec_fp);
		fseek(tosec_fp, 0L, SEEK_SET);

		bzero(buffer, sizeof(buffer));

		sprintf(buffer, "%d", fsz);
		send(sockfd, buffer, sizeof(fsz), 0);

		while((sent_bytes = fread(buffer, sizeof(char), sizeof(buffer), tosec_fp)) > 0) {
			if((sent_bytes = send(sockfd, buffer, sizeof(buffer), 0)) > 0){
				//printf("Sent %d bytes\n", sent_bytes);
			}
		}

		finished = 0;
		fprintf(stdout, "Introduzca el camino al fichero con emails: (email0,email1,...,emailn)\n");
		while(!finished){

			fgets(buffer, sizeof(buffer), stdin);
			char* nl = strchr(file_name, '\n');
	    	if(nl != 0) {
	        	*nl = 0;
	    	} 

			if (access(buffer, F_OK) == -1) {
			    fprintf(stdout ,"Fichero %s no existe. Intente de nuevo:\n", buffer);
			} else {
				finished = 1;
			}
		}



		fseek(email_fp, 0L, SEEK_END);
		int fsz = ftell(email_fp);
		fseek(email_fp, 0L, SEEK_SET);

		bzero(buffer, sizeof(buffer));

		sprintf(buffer, "%d", fsz);
		send(sockfd, buffer, sizeof(fsz), 0);

		while((sent_bytes = fread(buffer, sizeof(char), sizeof(buffer), email_fp)) > 0) {
			if((sent_bytes = send(sockfd, buffer, sizeof(buffer), 0)) > 0){
				//printf("Sent %d bytes\n", sent_bytes);
			}
		}

		fprintf(stdout, "Indique el numero total de comparticiones:\n");
		sprintf(buffer, "%lu", tot_shares);
		send(sockfd, buffer, sizeof(tot_shares), 0);

		fprintf(stdout, "Indique el numero mínimo de comparticiones:\n");
		sprintf(buffer, "%lu", num_shares);
		send(sockfd, buffer, sizeof(num_shares), 0);
		
		fprintf(stdout, "El proceso se está realizando.....\n");

		recv(sockfd, buffer, sizeof(buffer), 0);
		state = atoi(buffer);
		if(!state) {
			fprintf(stdout, "El secreto se ha compartido con éxito!\n");
		} else {
			fprintf(stdout, "Han habido errores en la compartición.\n");
		}

	} else if (option == 0) { /* Recuperar el secreto */

	} else {
		fprintf(stderr, "Opción inválida %d\n", option);
	}



	/*
	if ((n = send(sockfd, buffer, strlen(buffer), 0)) != strlen(buffer)) {
	  perror("client: error in send");
	  return -1;
  	}

	printf("Client: sends %d bytes\nSent message: %s\n", n, buffer);
    bzero(buffer,sizeof(buffer));

	if ((n = recv(sockfd, buffer, sizeof(buffer), 0)) < 0) {
	  perror("client: error in recv\n");
	  return -1;
	}

	if(n <= 0 || !strcmp(buffer, "OK\n"))
		finished = 1;
	else {
		printf("Client: receives %d bytes\nReceived message: %s\n", n, buffer);
		bzero(buffer,sizeof(buffer));
	}
	*/

	close(sockfd);
	printf("Connetion is closed.\n");
}
