#ifndef SUPERCON__H
#define SUPERCON__H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

typedef struct _bignum {
  unsigned long l;
  unsigned long h;
} bignum;
enum _bool { false = 0, true = 1 };
typedef enum _bool bool;

bignum madd(bignum x, bignum y, bignum n); /* (x+y)%n */
bignum msub(bignum x, bignum y, bignum n); /* (x-y)%n */
bignum mmul(bignum x, bignum y, bignum n); /* (x*y)%n */
bool blt(bignum x, bignum y); /* x<y */
bool ble(bignum x, bignum y); /* x<=y */
bool bgt(bignum x, bignum y); /* x>y */
bool bge(bignum x, bignum y); /* x>=y */
bool beq(bignum x, bignum y); /* x==y */
bool bne(bignum x, bignum y); /* x!=y */
bignum badd(bignum x, bignum y); /* x+y */
bignum bsub(bignum x, bignum y); /* x-y */
bignum bmul(bignum x, bignum y); /* x*y */
bignum bquo(bignum x, bignum y); /* x/y */
bignum brem(bignum x, bignum y); /* x%y */
void bdiv(bignum x, bignum y, bignum *q, bignum *r);
/* return 0 if reading failed, 1 if it succeeded. */
int getimage(FILE *fp, int eimage[]); /* import of crypt-image */
int gethint(FILE *fp, bignum *rsaN, bignum *rsaE, int hardness[], bignum hint[]);
void outstarttime(void);
void outresult(int image[]);
/* I/O for <=19 decimal digits numbers */
void getnum16(bignum *x);
void getnu10(bignum *x);
void putnum16(bignum x);
void putnum10(bignum x);
#endif

