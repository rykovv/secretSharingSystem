CC = gcc

sources = ss_server_tcp.c secshare_functions.c
dependencies = secshare.h

secshare: $(sources) $(dependencies)
	$(CC) $(sources) -lm -lssl -lcrypto -g -o $@

clean:
	rm secshare f2sec emails
	rm -R shares/
