#include "secshare.h"
#include <math.h>
#include <limits.h>
#include <string.h>
#include <sys/stat.h>

int isPrime(long unsigned int number){
	int prm = 1;
	long unsigned int sqrtn = sqrt(number), i;

	/* interger approx passes it as a prime */
	if(number == 2)
		return 0;

	for (i = 2; prm && (i <= sqrtn); i++){
		if(!(number % i)){
			prm = 0;
		}
	}
	
	return prm;
}

BIGNUM ** get_n_first_primes(long unsigned int n){
	long unsigned int ctr = 0, i;
	BIGNUM ** prms = (BIGNUM **) malloc(sizeof(BIGNUM *)*n);

	for(i = 1; i < ULONG_MAX && ctr < n; i++){
		if(isPrime(i)){
			prms[ctr++] = convert_int2bn(i);
		}
	}

	return prms;
}

BIGNUM * compute_share(BIGNUM ** prms, BIGNUM * v, long unsigned int n, const BIGNUM * s, const BIGNUM * m){	
	long unsigned int i;
	BIGNUM * res = BN_dup(s);
	BIGNUM * exp, * deg, * mult;

	BN_CTX * ctx = BN_CTX_new();
	BN_CTX_start(ctx);
	BN_RECP_CTX * recp = BN_RECP_CTX_new();
	BN_RECP_CTX_set(recp, m, ctx);

	for(i = 1; i < n; i++){
		exp = BN_new();
		mult = BN_new();
		deg = convert_int2bn(i);

		BN_mod_exp(exp, v, deg, m, ctx);
		
		BN_mod_mul_reciprocal(mult, prms[i-1], exp, recp, ctx);
		
		BN_mod_add(res, res, mult, m, ctx);

		BN_free(exp);
		BN_free(mult);
		BN_free(deg);
	}

	BN_CTX_end(ctx);
	BN_CTX_free(ctx);
	BN_RECP_CTX_free(recp);
	
	return res;
}

BIGNUM ** compute_shares(BIGNUM ** prms, BIGNUM ** vs, long unsigned k, long unsigned int n, const BIGNUM * s, const BIGNUM * m){
	BIGNUM ** shrs = (BIGNUM **) malloc(sizeof(BIGNUM *)*k);
	long unsigned int i; 
	
	for(i = 0; i < k; i++){
		shrs[i] = compute_share(prms, vs[i], n, s, m);
	}

	return shrs;
}

BIGNUM * compute_secret(BIGNUM ** shrs, BIGNUM ** vs, long unsigned int n, BIGNUM * m){
	long unsigned int * indexes, i, j;
	BIGNUM ** sub_vs, * s, * nom, * den, * temp_den, * temp_mult, * temp_sum;
	s = BN_new();

	BN_CTX * ctx = BN_CTX_new();
	BN_CTX_start(ctx);
	BN_RECP_CTX * recp = BN_RECP_CTX_new();
	BN_RECP_CTX_set(recp, m, ctx);

	for(i = 0; i < n; i++){
		temp_den = BN_new();
		temp_sum = BN_new();
		temp_mult = BN_new();

		indexes = serie_excluded_index(n, i);
		sub_vs = arr_mod_subs(vs, vs[i], m, n);

		nom = multiplyer(vs, indexes, m, n-1);

		temp_den = multiplyer(sub_vs, indexes, m, n-1);
		den = BN_mod_inverse(NULL, temp_den, m, ctx);

		BN_mod_mul_reciprocal(temp_mult, nom, den, recp, ctx);
		BN_mod_mul_reciprocal(temp_sum, temp_mult, shrs[i], recp, ctx);

		BN_mod_add(s, s, temp_sum, m, ctx);

		BN_free(temp_den);
		BN_free(temp_sum);
		BN_free(temp_mult);
		BN_free(nom);
		BN_free(den);
		BNs_free(sub_vs, n);
		free(indexes);
	}

	BN_CTX_end(ctx);
	BN_CTX_free(ctx);
	BN_RECP_CTX_free(recp);

	return s;
}

long unsigned int * serie_excluded_index(long unsigned int num_elem, long unsigned int i_ex){
	long unsigned int * serie = (long unsigned int *) malloc(sizeof(long unsigned int)*(num_elem-1));
	long unsigned int i, j;

	for(i = 0, j = 0; j < (num_elem-1); i++){
		if(i != i_ex){
			serie[j++] = i;
		}
	}

	return serie;
}

BIGNUM * multiplyer(BIGNUM ** elems, long unsigned int * indexes, const BIGNUM * m, long unsigned int i_size){
	BIGNUM * res = BN_new();
	BN_one(res);
	BIGNUM * aux;
	long unsigned int i;

	BN_CTX * ctx = BN_CTX_new();
	BN_CTX_start(ctx);
	BN_RECP_CTX * recp = BN_RECP_CTX_new();
	BN_RECP_CTX_set(recp, m, ctx);

	for(i = 0; i < i_size; i++){
		aux = BN_new();
		BN_mod_mul_reciprocal(aux, res, elems[indexes[i]], recp, ctx);
		res = BN_copy(res, aux);
		BN_free(aux);
	}

	BN_CTX_end(ctx);
	BN_CTX_free(ctx);
	BN_RECP_CTX_free(recp);

	return res;
}

BIGNUM ** arr_mod_subs(BIGNUM ** arr, BIGNUM * sub, const BIGNUM * m, long unsigned int size){
	BIGNUM ** subs = (BIGNUM **) malloc(sizeof(BIGNUM *)*size);
	long unsigned int i;

	BN_CTX * ctx = BN_CTX_new();
	BN_CTX_start(ctx);

	for(i = 0; i < size; i++){
		subs[i] = BN_new();
		BN_mod_sub(subs[i], sub, arr[i], m, ctx);
	}

	BN_CTX_end(ctx);
	BN_CTX_free(ctx);

	return subs;
}

BIGNUM * generate_big_prime(long unsigned int bits){
	BIGNUM * ret = BN_new();
	
	if(!BN_generate_prime_ex(ret, bits, 1, NULL, NULL, NULL)) {
		perror("BN_generate_prime_ex FAILED\n");
	}

	return ret;
}

BIGNUM ** generate_n_big_primes(long unsigned int n, long unsigned int bits){
	BIGNUM ** big_prms = (BIGNUM **) malloc(sizeof(BIGNUM *)*n);
	long unsigned int i;

	for(i = 0; i < n; i++){
		big_prms[i] = generate_big_prime(bits);
	}

	return big_prms;
}

BIGNUM * convert_int2bn(long unsigned int n){
	char n2str[32];
	sprintf(n2str, "%lu", n);

	BIGNUM * ret = generate_big_prime(bn_size);
	BN_dec2bn(&ret, n2str);

	return ret;
}

void BNs_free(BIGNUM ** array, long unsigned int size){
	long unsigned int i;

	for(i = 0; i < size; i++){
		BN_free(array[i]);
	}
	free(array);
}

void init_folder_files(int num_files){
	long unsigned int i;

	FILE * fp;

	char folder_name[50];
	char number[20];
	char temp[50];

	struct stat st = {0};

	if(stat("shares", &st) == -1) {
		mkdir("shares", 0700);
	}

	strcpy(folder_name, "shares");

	if(stat(folder_name, &st) == -1){
		mkdir(folder_name, 0700);
	}

	strcat(folder_name, "/share_");

	for(i = 0; i < num_files; i++){
		strcpy(temp, folder_name);
		sprintf(number, "%ld", i);
		strcat(temp, number);
		fp = fopen(temp, "w+");
		//fclose(secfiles[i]); move to finishing routine
		strcpy(temp, folder_name);
		fclose(fp);
	}
}

void init(){

}

void write_layer(BIGNUM ** shrs, long unsigned int k, const BIGNUM * m){
	long unsigned int i;
	char file_name[100];
	char file_din[100];
	char number[20];
	FILE * fp;
	char buffer[bn_size*2 + 1];

	strcpy(file_name, "shares/share_");

	for(i = 0; i < k; i++){
		strcpy(file_din, file_name);

		sprintf(number, "%ld", i);
		strcat(file_din, number);

		fp = fopen(file_din, "a+");

		if (ferror(fp)) {
        	fprintf(stderr, "%s No such file or directory.\n", file_din);
    	}

		if (m != NULL) {
			sprintf(buffer, "%lu", BN_num_bytes(m));
			buffer[16] = '\0';
			fwrite(buffer, 1, 16, fp);
        	
        	BN_bn2bin(m, buffer);
        	buffer[chunk_size*2] = '\0';
        	fwrite(buffer, 1, chunk_size*2, fp);
		} else {
			sprintf(buffer, "%lu", BN_num_bytes(shrs[i]));
			buffer[16] = '\0';
			fwrite(buffer, 1, 16, fp);

        	BN_bn2bin(shrs[i], buffer);
    		buffer[chunk_size] = '\0';
			fwrite(buffer, 1, chunk_size, fp);
		}

		fclose(fp);
	}
}

void secret_from_files(long unsigned int n, long unsigned int k){
	long unsigned int i = 2;
	int fin = 0;
	BIGNUM ** shrs;
	BIGNUM * secret = BN_new();

	char * buffer;

	FILE * revealed = fopen("shares/secret", "a+");

	buffer = (char *) malloc(chunk_size*2 + 1);
	if (!buffer) {
		fprintf(stderr, "Memory error!");
		return;
	}

	// leer M y Vs de fold
	BIGNUM ** m = read_layer(k, 0);
	BIGNUM ** vs = read_layer(k, 1);

	while(!fin){
		shrs = read_layer(5, i);
		if(shrs[k-1] == NULL){
			fin = 1;
		} else {
			secret = compute_secret(shrs, vs, n, m[0]);
			BNs_free(shrs, k);
			printf("Part %d copmuted\n%s\n", i-2, BN_bn2dec(secret));
			// write secret to a file
			BN_bn2bin(secret, buffer);
			fwrite(buffer, 1, BN_num_bytes(secret), revealed);
			i++;
			BN_free(secret);
		}
	}

	printf("Secret decyphered correctly\n");

	fclose(revealed);
	BNs_free(vs, k);
	BNs_free(m, k);
	free(buffer);
}

BIGNUM ** read_layer(long unsigned int k, long unsigned int layer_offset){
	BIGNUM ** bns = (BIGNUM **) malloc(sizeof(BIGNUM *)*k);
	
	FILE * fp;
	char file_name[200];
	char folder_name[200];
	char number[20];
	char buffer[chunk_size*2 + 1];
	long unsigned int i, size;
	size_t nread;
	char last_layer = 0;

	strcpy(folder_name, "shares/share_");

	for(i = 0; (i < k) && !last_layer; i++){
		strcpy(file_name, folder_name);

		sprintf(number, "%ld", i);
		strcat(file_name, number);

		fp = fopen(file_name, "r");

		fseek(fp, 0L, SEEK_END);
		long unsigned int sz = ftell(fp);
		fseek(fp, 0L, SEEK_SET);

		if(sz >= (layer_offset?(layer_offset-1)*(chunk_size+16)+(chunk_size*2+16)+(chunk_size+16):(chunk_size*2+16))){
			// set fseek
			fseek(fp, layer_offset?(layer_offset-1)*(chunk_size+16)+(chunk_size*2+16):0, SEEK_SET);
		
			fread(buffer, 1, 16, fp);
			buffer[16] = '\0';
			size = atol(buffer);
			fread(buffer, 1, size, fp);
			buffer[size] = '\0';
			bns[i] = BN_new();
			BN_bin2bn(buffer, size, bns[i]);
		} else {
			last_layer = 1;
		}
		fclose(fp);
	}

	return bns;
}

void start_thread(char * file, long unsigned int n, long unsigned int k){
	int i = 0;

	// Se crean k pequeños primos 
	BIGNUM ** little_vs = get_n_first_primes(k);
	// Luego n big primes
	BIGNUM ** big_primes = generate_n_big_primes(n, bn_size);
	// se crea el modulo para el sistema
	const BIGNUM * m = generate_big_prime(bn_size + 10);
	// variable para las comparticiones
	BIGNUM ** shares;
	
	FILE * fp;
	// Abrimos el fichero y averiguamos su tamaño
	fp = fopen(file, "r");
	fseek(fp, 0L, SEEK_END);
	long unsigned int sz = ftell(fp);
	fseek(fp, 0L, SEEK_SET);

	printf("File size %lu bytes\n", sz);
	printf("File size %lu bits\n", sz*8);

	// bufer de lectura y numero de bytes leidos
	char buffer[chunk_size + 1];
	size_t nread, nread_cum = 0;

	// inicializamos ficheros y carpetas para su escritura posterior
	init_folder_files(k);

	// Primero se escriben M (módulo) y Vs
	write_layer(NULL, k, m);
	write_layer(little_vs, k, NULL);

	// un bignum auxiliar 
	BIGNUM * temp_bn = BN_new();
	
	if (fp) {
		// leemos el fuchero al buffer
    	while ((nread = fread(buffer, 1, chunk_size, fp)) > 0){
        	bzero(buffer, sizeof(buffer));
        	nread_cum += nread;
        	// convertimos el buffer en el bignum
        	BN_bin2bn(buffer, chunk_size, temp_bn);
        	buffer[BN_num_bytes(temp_bn)] = '\0';
        	printf("Working %.1f%%\n", (float) nread_cum/sz*100);
        	// calculamos comparticiones para el pedazo leido
        	shares = compute_shares(big_primes, little_vs, k, n, temp_bn, m);
        	// escribimos las comparticiones en los ficheros creados 
        	write_layer(shares, k, NULL);
        	// liberamos la memoria alocada para las comparticiones
        	BNs_free(shares, k);
    	}

    
    	if (ferror(fp)) {
        	printf("Error creating shares\n");
    	} else {
    		printf("Shares created succesfully\n");
    	}
	}

	// limpiamos los recursos
	fclose(fp);
	BN_free(temp_bn);
	BNs_free(little_vs, k);
	BNs_free(big_primes, n);
}

void ex_test(char * fold){
	int k = 5;

	start_thread(fold, 3, k);

	secret_from_files(3, k);
	
	/*
	size_t nread;
	char buffer[chunk_size + 1];
	BIGNUM * temp_bn = BN_new();

	FILE * fp, *fs;
	fp = fopen(fold, "r");
	fs = fopen("resec.txt", "ab+");


	if (fp) {
		// leemos el fuchero al buffer
    	while ((nread = fread(buffer, 1, chunk_size, fp)) > 0){
        	//printf("buffer read= %d\n", nread); //fwrite(buffer, 1, nread, stdout);
        	// convertimos el buffer en el bignum
        	BN_bin2bn(buffer, chunk_size, temp_bn);
        	buffer[chunk_size] = '\0';
        	printf("leido %s\n", BN_bn2dec(temp_bn));

        	//?????????
        	char pp[BN_num_bytes(temp_bn) + 1];
        	pp[BN_num_bytes(temp_bn)] = '\0';
        	BN_bn2bin(temp_bn, pp);
        	fwrite(pp, 1, sizeof(pp), fs);
    	}

    	printf("finished seccesfully\n");
    
    	if (ferror(fp)) {
        	printf("finished with errors\n");
    	}
	}
	*/
	/*
	FILE * fp;
	fp = fopen("resec.txt", "w+");
	
	BIGNUM * test = generate_big_prime(bn_size/2);// convert_int2bn(10);
	char buffer[chunk_size + 1];
	buffer[chunk_size/2] = '\0';
	printf("creado 1\n%s\n", BN_bn2dec(test));
	BN_bn2bin(test, buffer);
	i = fwrite(buffer, 1, chunk_size/2, fp);
	printf("Escrito %d bytes\n", i);

	fclose(fp);
	fp = fopen("resec.txt", "a");

	BIGNUM * test2 = generate_big_prime(bn_size);
	BN_dec2bn(&test2, "2");
	printf("creado 2\n%s\n", BN_bn2dec(test2));
	BN_bn2bin(test2, buffer);
	buffer[chunk_size] = '\0';
	i = fwrite(buffer, 1, chunk_size, fp);
	printf("Escrito %d bytes\n", i);

	fclose(fp);

	fp = fopen("resec.txt", "a");

	BIGNUM * test3 = generate_big_prime(bn_size);
	printf("creado 3\n%s\n", BN_bn2dec(test3));
	BN_bn2bin(test3, buffer);
	buffer[chunk_size] = '\0';
	i = fwrite(buffer, 1, chunk_size, fp);
	printf("Escrito %d bytes\n", i);

	fclose(fp);

	printf("-----------------------\n");

	fp = fopen("resec.txt", "r");
	memset(buffer, 0, strlen(buffer));

	BIGNUM * test11 = BN_new();
	i = fread(buffer, 1, chunk_size/2, fp);
	buffer[chunk_size/2] = '\0';
	BN_bin2bn(buffer, chunk_size/2, test11);
	printf("leido 1 %d bytes\n%s\n", i, BN_bn2dec(test11));

	fclose(fp);
	
	fp = fopen("resec.txt", "r");
	fseek(fp, chunk_size/2, SEEK_SET);
	memset(buffer, 0, strlen(buffer));
	BIGNUM * test22 = BN_new();
	i = fread(buffer, 1, chunk_size, fp);
	buffer[chunk_size] = '\0';
	BN_bin2bn(buffer, 1, test22);
	printf("leido 2 %d bytes\n%s\n", i, BN_bn2dec(test22));
	
	fclose(fp);
	
	fp = fopen("resec.txt", "r");
	fseek(fp, chunk_size*3/2, SEEK_SET);

	memset(buffer, 0, strlen(buffer));
	BIGNUM * test33 = BN_new();
	i = fread(buffer, 1, chunk_size, fp);
	buffer[chunk_size] = '\0';
	BN_bin2bn(buffer, chunk_size, test33);
	printf("leido 3 %d bytes\n%s\n", i, BN_bn2dec(test33));
	

	BN_free(test2);
	BN_free(test);
	BN_free(test22);
	BN_free(test11);
	BN_free(test3);
	BN_free(test33);
	fclose(fp);
	*/
}