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

BIGNUM * compute_secret(BIGNUM ** shrs, BIGNUM ** vs, long unsigned int n, const BIGNUM * m){
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

	//print_array(elems, i_size+1);
	//print_array(indeces, i_size);
	//printf("mul=%d\n", res);

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

	BIGNUM * ret = BN_new();
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

	FILE ** secfiles = (FILE **) malloc(sizeof(FILE *)*num_files);

	char sfname[50];
	char number[20];


	struct stat st = {0};

	if (stat("shares", &st) == -1) {
		mkdir("shares", 0700);
	}

	for(i = 0; i < num_files; i++){
		strcpy(sfname, "shares/secret");
		sprintf(number, "%d", i);
		strcat(sfname, number);
		secfiles[i] = fopen(sfname, "a+");
		fclose(secfiles[i]);
	}

	free(secfiles);
}