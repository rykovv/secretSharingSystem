CC = gcc

sources = ss_client_tcp.c

secshare: $(sources)
	$(CC) $(sources) -lm -lssl -lcrypto -g -o $@

clean:
	rm ss_client
