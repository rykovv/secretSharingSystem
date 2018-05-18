#include <openssl/bn.h>
#include <signal.h>
/* Funciones para el compartidor de secretos */
#ifndef __SECSHARE_H__
#define __SECSHARE_H__

size_t chunk_size = 32; // read by chunks of 64 bytes -> 512 bits
size_t bn_size = 256;

/* -------
	Funciones para generar las compraticiones 
   ------- */

/* Funcion para generar un BIGNUM de 'bits' bits */
BIGNUM * generate_big_prime(long unsigned int bits);

/* Funcion que genera un array de n BIGNUMs de 'bits' bits */
BIGNUM ** generate_n_big_primes(long unsigned int n, long unsigned int bits);

/* Funcion que comprueba si number es un numero primo */
int isPrime(long unsigned int number);

/* Funcion que devuelve n primeros numeros primos */ 
BIGNUM ** get_n_first_primes(long unsigned int n);

/* Funcion que convierte long unsigned int n a BIGNUM */
BIGNUM * convert_int2bn(long unsigned int n);

/* calcula una comparticion dados primos, Vi, grado del polinomio y el secreto s con modulo m */
BIGNUM * compute_share(BIGNUM ** prms, BIGNUM * v, long unsigned int n, const BIGNUM * s, const BIGNUM * m);

/* Funcion que calcula comparticiones dados primos, cantidad de comparticiones, el secreto y el primo p */
BIGNUM ** compute_shares(BIGNUM ** prms, BIGNUM ** vs, long unsigned k, long unsigned int n, const BIGNUM * s, const BIGNUM * m);

void BNs_free(BIGNUM ** array, long unsigned int size);

/* Calcula el secreto */
BIGNUM * compute_secret(BIGNUM ** shrs, BIGNUM ** vs, long unsigned int n, BIGNUM * m);

/* create a serie size of num_elem excluding i_ex */
long unsigned int * serie_excluded_index(long unsigned int num_elem, long unsigned int i_ex);

/* cumulative selective multiplyer */
BIGNUM * multiplyer(BIGNUM ** elems, long unsigned int * indexes, const BIGNUM * m, long unsigned int i_size);

/* array modulo substraction */
BIGNUM ** arr_mod_subs(BIGNUM ** arr, BIGNUM * sub, const BIGNUM * m, long unsigned int size);

/* Computes secret file from files */
void secret_from_files(long unsigned int n, long unsigned int k);

/* ---------
	Funciones para calcular el secreto 
   --------- */

/* Funcion para comprobar si un sistema dado tiene solucion unica */
int has_unique_solution(int ** system, int p);

/* Funcion para calcular el secreto */
int get_unique_solution(int ** system, int p);

/* Management folders and files. Returns a pointer to created and opened files */
void init_folder_files(int num_files);

/* Server initialization routine */
void init();

/* Write shares into files */
void write_layer(BIGNUM ** shrs, long unsigned int k, const BIGNUM * m);

/* Start thread with a file to make a secret and split into k shares */
void start_thread(char * file, long unsigned int n, long unsigned int k);

/* Gets one layer from k shares stored in folder_name directory in form of k BIGNUMs */
BIGNUM ** read_layer(long unsigned int k, long unsigned int offset);

void ex_test(char * fold);

#endif