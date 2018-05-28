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
#include <fts.h>

#define BUFSIZE 1024

#define TCP_NODELAY 1
#define SOL_TCP	6

main(int argc, char ** argv) {
	if(argc != 3){
		perror("Wrong number of arguments: <server's IP> <server's port>\n");
		return -1;
	}

	int           			sockfd, n, option = 1, state, fsz, i;
	struct sockaddr_in     	serv_addr ;
	char 		 			buffer[BUFSIZE], file_name[BUFSIZE], password[BUFSIZE];
	size_t					sent_bytes, read_bytes, rest_file, received_bytes;
	long unsigned int 		num_shares = 0, tot_shares = 0;
	char					*const shares_path[1] = {"shares"};

	FILE 					*tosec_fp, *email_fp, *secret;;


	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("client: can't open stream socket") ;
		return -1;
	}

	printf("Bienvenido a la aplicación de compartición de secretos!\n");

	serv_addr.sin_family        	= AF_INET ;
	serv_addr.sin_addr.s_addr   	= inet_addr(argv[1]); 
	serv_addr.sin_port          	= htons(atoi(argv[2]));

	/* ----------------------------------------- */

	printf("Conectando al servidor ...\n");

   	if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) != 0)  {
		perror("client: can't connect with server") ;
		return -1;
	}
	printf("Conectado al servidor con IP <%s> : <%s> !\n", argv[1], argv[2]);

	int one = 1;

	setsockopt(sockfd, SOL_TCP, TCP_NODELAY, &one, sizeof(one));


	/* ----------------------------------------- */

	fprintf(stdout, "Indique la acción que quiere realizar:\n\t0 - Recuperar el secreto.\n\t1 - Compartir el sercreto\n");

	int finished = 0;

	while(!finished) {
		fgets(buffer, sizeof(char)*3, stdin);
		char* nl = strchr(buffer, '\n');
    	if(nl != 0) {
        	*nl = 0;
    	}
		option = atoi(buffer);

		if(option == 1 || option == 0){
			finished = 1;
		} else {
			fprintf(stdout, "Opción inválida %d. (0/1):\n", option);
		}
	}

	send(sockfd, buffer, sizeof(option), 0);

	/* ----------------------------------------- */

	if (option == 1) { /* Compartir el secreto */
		finished = 0;

		/* ----------------------------------------- */

		fprintf(stdout, "Indique el camino al fichero que quiere compartir:\n");
		while(!finished){

			fgets(file_name, BUFSIZE, stdin);
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

		/* ----------------------------------------- */

		fprintf(stdout, "Introduzca la contraseña para el fichero (AES):\n");

		fgets(password, BUFSIZE, stdin);
		sprintf(buffer, "openssl enc -aes-128-cbc -in %s -out f2send -k %s", file_name, password);

		system(buffer);

		/* ----------------------------------------- */

		tosec_fp = fopen("f2send", "r");

		fseek(tosec_fp, 0L, SEEK_END);
		fsz = ftell(tosec_fp);
		fseek(tosec_fp, 0L, SEEK_SET);

		bzero(buffer, BUFSIZE);

		sprintf(buffer, "%d", fsz);
		send(sockfd, buffer, sizeof(fsz), 0);
		sleep(1);

		while(((read_bytes = fread(buffer, sizeof(char), BUFSIZE, tosec_fp)) > 0) && (fsz > 0)) {
			send(sockfd, buffer, read_bytes, 0);
			fprintf(stdout, "%d - %d = %d\n", fsz, read_bytes, fsz - read_bytes);
			fsz -= read_bytes;
			printf("Sent %d bytes file to share\n", read_bytes);
		}
		fclose(tosec_fp);
		fprintf(stdout, "El fichero fue encriptado y enviado al servidor.\n");

		/* ----------------------------------------- */

		
		finished = 0;
		fprintf(stdout, "Introduzca el camino al fichero con emails: (email0,email1,...,emailn)\n");

		while(!finished){
			fgets(buffer, BUFSIZE, stdin);
			char* nl = strchr(buffer, '\n');
	    	if(nl != 0) {
	        	*nl = 0;
	    	} 

			if (access(buffer, F_OK) == -1) {
			    fprintf(stdout ,"Fichero %s no existe. Intente de nuevo:\n", buffer);
			} else {
				finished = 1;
			}
		}
		
		/* ----------------------------------------- */
		email_fp = fopen(buffer, "r");

		fseek(email_fp, 0L, SEEK_END);
		fsz = ftell(email_fp);
		fseek(email_fp, 0L, SEEK_SET);

		bzero(buffer, BUFSIZE);

		sprintf(buffer, "%d", fsz);
		sleep(1);
		send(sockfd, buffer, sizeof(fsz), 0);
		sleep(1);
		while(((read_bytes = fread(buffer, sizeof(char), BUFSIZE, email_fp)) > 0) && (fsz > 0)) {
			send(sockfd, buffer, read_bytes, 0);
			fsz -= read_bytes;
			printf("Sent %d bytes emails\n", read_bytes);
		}
		fprintf(stdout, "Los emails fue enviado al servidor.\n");
		fclose(email_fp);

		/* ----------------------------------------- */

		fprintf(stdout, "Indique el numero mínimo de comparticiones:\n");

		fgets(buffer, sizeof(long unsigned int), stdin);
		send(sockfd, buffer, sizeof(long unsigned int), 0);


		fprintf(stdout, "Indique el numero total de comparticiones:\n");

		fgets(buffer, sizeof(long unsigned int), stdin);
		send(sockfd, buffer, sizeof(long unsigned int), 0);

		/* ----------------------------------------- */
		
		fprintf(stdout, "El proceso se está realizando.....\n");

		recv(sockfd, buffer, BUFSIZE, 0);
		state = atoi(buffer);
		if(!state) {
			fprintf(stdout, "El secreto se ha compartido con éxito!\n");
		} else {
			fprintf(stdout, "Han habido errores en la compartición.\n");
		}

		/* ----------------------------------------- */

		// enviar la contraseña a emails
		sprintf(buffer, "./sendPassword.sh %s", password);
		if(system(buffer) < 0){
			fprintf(stdout, "La clave no se ha enviado correctamente.\n");
		} else {
			fprintf(stdout, "La clave se ha enviado con éxito.\n");
		}

		/* ----------------------------------------- */

	} else if (option == 0) { /* Recuperar el secreto */
		i = 0;
		
		fprintf(stdout, "Indique el número de comparticiones que va a enviar:\n");
		//bzero(buffer, BUFSIZE);
		fgets(buffer, BUFSIZE, stdin);
		tot_shares = atol(buffer);
		send(sockfd, buffer, BUFSIZE, 0);
		fprintf(stdout, "Número total de comparticiones = %d\n", tot_shares);

		fprintf(stdout, "Indique el número mínimo y necesario de comparticiones para revelar el sercreto:\n");
		//bzero(buffer, BUFSIZE);
		fgets(buffer, BUFSIZE, stdin);
		num_shares = atol(buffer);
		send(sockfd, buffer, BUFSIZE, 0);
		fprintf(stdout, "Número mínimo de comparticiones = %d\n", num_shares);


		//fprintf(stdout, "Coloquen las comparticiones en carpeta shares. Después pulse Enter\n");
		//getchar();
		
		FTS *ftsp;
		FTSENT *p, *chp;

		if ((ftsp = fts_open(shares_path, FTS_NOCHDIR, NULL)) == NULL) {
    		warn("fts_open");
		}

		chp = fts_children(ftsp, 0);

		if (chp == NULL) {
               /* no files to traverse */
		} else {
			 while (((p = fts_read(ftsp)) != NULL) && (i < tot_shares)) {
			 	if((p->fts_info) == FTS_F){
			 		
			 		tosec_fp = fopen(p->fts_path, "r");
			 		fprintf(stdout, "Se abre el fichero %s de ", p->fts_path);

			 		//bzero(buffer, BUFSIZE);

			 		fseek(tosec_fp, 0L, SEEK_END);
					fsz = ftell(tosec_fp);
					fseek(tosec_fp, 0L, SEEK_SET);

					sprintf(buffer, "%d", fsz);
					buffer[sizeof(fsz)] = '\0';
					fprintf(stdout, "%s bytes\n", buffer);
					sleep(1);
					send(sockfd, buffer, sizeof(fsz), 0);
					sleep(1);
					while(((read_bytes = fread(buffer, sizeof(char), BUFSIZE, tosec_fp)) > 0) && (fsz > 0)) {
						send(sockfd, buffer, read_bytes, 0);
						fsz -= read_bytes;
						//printf("Sent %d bytes share %d\n", read_bytes, i);
					}
					fprintf(stdout, "La compartición %s fue enviada al servidor.\n", p->fts_name);
					fclose(tosec_fp);

			 		// enviar finished = 0
					sprintf(buffer, "%d", 0);
					sleep(1);
					int ret = 0;
					send(sockfd, buffer, sizeof(int), ret);
					sleep(1);
					i++;
			 	} else {
			 		int ret = 1;
					send(sockfd, buffer, sizeof(int), ret);
					sleep(1);
			 	}
			}
			fts_close(ftsp);
		}

		fprintf(stdout, "Esperando la respuesta del servidor...\n");

		recv(sockfd, buffer, BUFSIZE, 0);
		fsz = atoi(buffer);
		rest_file = fsz;

		fprintf(stdout, "El tamaño del secreto calculado %d bytes\n", fsz);
		
		secret = fopen("shares/secret", "w+");
		bzero(buffer, BUFSIZE);

		while(rest_file > 0){
			received_bytes = recv(sockfd, buffer, BUFSIZE, 0);
			fwrite(buffer, sizeof(char), received_bytes, secret);
			fprintf(stdout, "%d - %d = %d\n", rest_file, received_bytes, rest_file - received_bytes);
			fprintf(stdout, "Escrito %d bytes del fichero.\n", received_bytes);
			rest_file -= received_bytes;
		}

		fclose(secret);
		truncate("shares/secret", fsz);

		fprintf(stdout, "Introduzca la contraseña del secreto:\n");
		fgets(password, BUFSIZE, stdin);
		sprintf(buffer, "openssl enc -aes-128-cbc -d -in shares/secret -out shares/yourSecret -k %s", password);
		system(buffer);

		fprintf(stdout, "El secreto está guardado en la carpeta shares como fichero 'yourSecret'.\n");

	} else {
		fprintf(stderr, "Opción inválida %d\n", option);
	}


	close(sockfd);
	printf("Connetion is closed.\n");
}
