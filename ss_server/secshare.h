#include <openssl/bn.h>
/* Funciones para el compartidor de secretos */
#ifndef __SECSHARE_H__
#define __SECSHARE_H__
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
BIGNUM * compute_secret(BIGNUM ** shrs, BIGNUM ** vs, long unsigned int n, const BIGNUM * m);

/* create a serie size of num_elem excluding i_ex */
long unsigned int * serie_excluded_index(long unsigned int num_elem, long unsigned int i_ex);

/* cumulative selective multiplyer */
BIGNUM * multiplyer(BIGNUM ** elems, long unsigned int * indexes, const BIGNUM * m, long unsigned int i_size);

/* array modulo substraction */
BIGNUM ** arr_mod_subs(BIGNUM ** arr, BIGNUM * sub, const BIGNUM * m, long unsigned int size);

/* ---------
	Funciones para calcular el secreto 
   --------- */

/* Funcion para comprobar si un sistema dado tiene solucion unica */
int has_unique_solution(int ** system, int p);

/* Funcion para calcular el secreto */
int get_unique_solution(int ** system, int p);

#endif