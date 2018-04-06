CC = gcc

sources = secshare_main.c secshare_functions.c
dependencies = secshare.h

secshare: $(sources) $(dependencies)
	$(CC) $(sources) -lm -lssl -lcrypto -o $@

clean:
	rm secshare