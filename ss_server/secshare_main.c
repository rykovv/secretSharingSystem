#include <stdio.h>
#include <stdlib.h>
#include "secshare.h"

int main(int argc, char ** argv){
	int i = 0;

	/*
	long unsigned int n = 9;
	long unsigned int k = 11;
	1,3,5,7 ... 
	BIGNUM ** little_vs = get_n_first_primes(k);
	/* big primes 
	BIGNUM ** big_primes = generate_n_big_primes(n, 256);
	/* secret 
	BIGNUM * s = BN_new();
	char num[100] = "7353643343254354353533243543";
	BN_dec2bn(&s, num);
	/* modulo 
	const BIGNUM * m = generate_big_prime(300);
	/* shares 
	BIGNUM ** shares = compute_shares(big_primes, little_vs, k, n, s, m);	
	
	for(; i < k; i++){
		printf("share=%s\n", BN_bn2dec(shares[i]));
	}

	printf("pre sec=%s\n", BN_bn2dec(s));

	BIGNUM * sec = compute_secret(shares, little_vs, n, m);
	
	printf("cal sec=%s\n", BN_bn2dec(sec));

	BN_free(sec);
	BNs_free(little_vs, k);
	BNs_free(big_primes, n);
	BNs_free(shares, k);
	*/

	FILE * fp, *fs;
	fs = fopen("secret.ss", "wr");
	fp = fopen(argv[1], "r");
	fseek(fp, 0L, SEEK_END);
	long unsigned int sz = ftell(fp);
	fseek(fp, 0L, SEEK_SET);

	printf("File size %lu bytes\n", sz);
	printf("File size %lu bits\n", sz*8);

	char * buffer;
	size_t nread;
	size_t chunk_size = 64; // read by 64 bytes -> 512 bits

	BIGNUM * test = BN_new();

	buffer = (char *) malloc(chunk_size);
	if (!buffer) {
		fprintf(stderr, "Memory error!");
		fclose(fp);
		return;
	}

	fread(buffer, 1, chunk_size, fp);
	printf("%s\n", buffer);
	fread(buffer, 1, chunk_size, fs);
	printf("%s\n", buffer);

	
	if (fp) {
    	while ((nread = fread(buffer, 1, chunk_size, fp)) > 0){
        	printf("buffer read= %d\n", nread); //fwrite(buffer, 1, nread, stdout);
        	BN_dec2bn(&test, buffer);
        	printf("bignum read=%s\n", BN_bn2dec(test));
        	
        	fwrite(test, 1, sizeof(*test), fs);
    	}
    
    	if (ferror(fp)) {
        // deal with error 
    	}

    	fclose(fp);
	}
	

	//fclose(fp);
	fclose(fs);

	return 0;
}