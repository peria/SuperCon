#include <assert.h>
#include "supercon.h"

#define MSB  (0x8000000000000000UL)
#define MSK  ((1UL<<32)-1)
#define SIZE (256*256)

void blshift(bignum *x, int b);
void brshift(bignum *x, int b);

/* (x+y)%n */
bignum madd(bignum x, bignum y, bignum n)
{
  if( bge(x, n) ) x = brem(x, n);
  if( bge(y, n) ) y = brem(y, n);
  bignum z = badd(x, y);
  if( bge(z, n) ) z = bsub(z, n);
  return z;
}

/* (x-y)%n */
bignum msub(bignum x, bignum y, bignum n)
{
  if( bge(x, n) ) x = brem(x, n);
  if( bge(y, n) ) y = brem(y, n);
  bignum z = badd(x, y);
  if( bge(z, n) ) z = badd(z, n);
  return z;
}

/* (x*y)%n */
bignum mmul(bignum x, bignum y, bignum n)
{
  bignum z = {0, 0};
  unsigned long b;

  for( b = 1UL << 63 ; b ; b >>= 1 ) if( x.h & b ) break;
  if( b ) { z = y; b >>= 1; }
  for( ; b ; b >>= 1 ) {
    blshift(&z, 1);
    if( bge(z,n) ) z = bsub(z, n);
    if( x.h & b )  z = badd(z, y);
    if( bge(z,n) ) z = bsub(z, n);
  }
  for( b = 1UL << 63 ; b ; b >>= 1 ) {
    blshift(&z, 1);
    if( bge(z,n) ) z = bsub(z, n);
    if( x.l & b )  z = badd(z, y);
    if( bge(z,n) ) z = bsub(z, n);
  }
  return z;
}

/* x<y */
bool blt(bignum x, bignum y)
{
  return ( x.h < y.h ||
	  (x.h == y.h && x.l < y.l) );
}
/* x<=y */
bool ble(bignum x, bignum y)
{
  return ( x.h < y.h ||
	  (x.h == y.h && x.l <= y.l) );
}
/* x>y */
bool bgt(bignum x, bignum y)
{
  return ( x.h > y.h ||
	  (x.h == y.h && x.l > y.l) );
}
/* x>=y */
bool bge(bignum x, bignum y)
{
  if( x.h > y.h ) return true;
  return ( x.h == y.h && x.l >= y.l );
}
/* x==y */
bool beq(bignum x, bignum y)
{
  return ( x.h == y.h && x.l == y.l );
}
/* x!=y */
bool bne(bignum x, bignum y)
{
  return ( x.h != y.h || x.l != y.l );
}

/* x+y */
bignum badd(bignum x, bignum y)
{
  bignum z;
  z.h = x.h + y.h;
  z.l = x.l + y.l;
  if( z.l < x.l ) z.h++;
  return z;
}
/* x-y */
bignum bsub(bignum x, bignum y)
{
  bignum z;
  z.h = x.h - y.h;
  z.l = x.l - y.l;
  if( z.l > x.l ) z.h--;
  return z;
}
/* x*y */
bignum bmul(bignum x, bignum y)
{
  bignum xl = {x.l & MSK, x.l >> 32};
  bignum yl = {y.l & MSK, y.l >> 32};
  bignum z  = {xl.l*yl.l, xl.h*yl.h};
  unsigned long c = xl.l * yl.h;
  unsigned long d = c + xl.h * yl.l;
  if( d < c ) z.h += (1UL << 32);
  z.l += (d << 32);
  z.h += (d >> 32);
  if( z.l < xl.l*yl.l ) z.h++;
  z.h += x.l*y.h + x.h*y.l;
  return z;
}
/* x/y */
bignum bquo(bignum x, bignum y)
{
  bignum q = {0, 0}, r = {0, 0};
  bdiv(x, y, &q, &r);
  return q;
}
/* x%y */
bignum brem(bignum x, bignum y)
{
  bignum q = {0, 0}, r = {0, 0};
  bdiv(x, y, &q, &r);
  return r;
}
void bdiv(bignum x, bignum y, bignum *q, bignum *r)
{
  if( y.h == 0 && y.l == 0 ) return; // Zero division error
  int i;
  for( i = 0 ; (y.h & MSB) == 0 && bgt(x, y) ; ++i ) blshift(&y, 1);
  q->h = q->l = 0;
  for( *r = x ; i >= 0 ; --i, brshift(&y, 1) ) {
    blshift(q, 1);
    if( bge(*r, y) ) {
      q->l = q->l + 1;
      *r = bsub(*r, y);
    }
  }
}

/* import of crypt-image */
int getimage(FILE *fp, int eimage[])
{
  if( fp == NULL || feof(fp) ) return 1;
  int i;
  for( i = 0 ; i < SIZE && !feof(fp) ; ++i ) {
    fscanf(fp, " %d ", &eimage[i]);
  }
  return (i != SIZE);
}

/* import of hint file */
int gethint(FILE *fp, bignum *rsaN, bignum *rsaE,
	    int hardness[], bignum hint[])
{
  char line[1024];
  int j, hard;
  bignum hnt;

  if( fp == NULL ) return 1;

  for( int i = 0 ; !feof(fp) ; ) {
    fgets(line, 1023, fp);
    if( line[0] == '#' ) continue;
    switch( i ) {
    case 0: /* for N */
      sscanf(line, "%lx %lx", &(rsaN->h), &(rsaN->l));
      break;
    case 1: /* for E */
      sscanf(line, "%lx %lx", &(rsaE->h), &(rsaE->l));
      break;
    default:
      sscanf(line, " %d %d %lx %lx ", &j, &hard, &hnt.h, &hnt.l);
      hardness[j] = hard;
      hint[j] = hnt;
    }
    ++i;
  }
  return 0;
}

static double start_time = -1;
void outstarttime(void)
{
  assert(start_time < 0);
  MPI_Barrier(MPI_COMM_WORLD);
  start_time = MPI_Wtime();
}
void outresult(int image[])
{
  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  if( rank ) return;
  if( start_time < 0 ) {
    fprintf(stderr, "Problem is not initialized\n");
    return ;
  }

  static bool isRead = false;
  static int  ans[SIZE];
  if( !isRead ) {
    FILE* fp = fopen("answer.txt", "r");
    for( int i = 0 ; i < SIZE ; ++i ) fscanf(fp, "%d", &ans[i]);
    fclose(fp);
    isRead = true;
  }
  int cnt = 0;
  for( int i = 0 ; i < SIZE ; ++i ) {
    cnt += (image[i] == ans[i]);
  }
  fprintf(stderr, "%d/%d (%4.1f%%) is correct (@%.3fsec)\n",
	  cnt, SIZE, cnt*100./SIZE, MPI_Wtime() - start_time);

  char filename[100];
  sprintf(filename, "answer%f.txt", MPI_Wtime() - start_time);
  FILE* fp = fopen(filename, "w");
  for( int i = 0 ; i < SIZE ; ++i ) {
    fprintf(fp, " %d", image[i]);
    if( i % 256 == 255 ) fputs("\n", fp);
  }
  fclose(fp);
  fprintf(stderr, "Output image is stored in %s\n", filename);
}

void getnum16(bignum *x)
{
  scanf("%lx", &(x->l));
}
void getnu10(bignum *x)
{
  scanf("%lu", &(x->l));
}
void putnum16(bignum x)
{
  if( x.h == 0 ) printf("%lx", x.l);
  else printf("%lx%016lx", x.h, x.l);
}
void putnum10(bignum x)
{
  printf("%lu", x.l);
}

// ===================================================================
// Original functions
void blshift(bignum *x, int b)
{
  x->h = (x->h << b) + (x->l >> (64 - b));
  x->l = x->l << b;
}
void brshift(bignum *x, int b)
{
  x->l = (x->l >> b) + (x->h << (64 - b));
  x->h = x->h >> b;
}

