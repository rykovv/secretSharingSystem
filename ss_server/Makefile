CC = gcc

sources = secshare_main.c secshare_functions.c
dependencies = secshare.h

secshare: $(sources) $(dependencies)
	$(CC) $(sources) -lm -lssl -lcrypto -g -o $@

clean:
	rm secshare
	rm -R shares/