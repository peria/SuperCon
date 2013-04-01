#include <time.h>
#include "supercon.h"
#define SIZE (256*256)

bignum rsaN, rsaE;
int    eimage[SIZE];
int    hard[SIZE];
bignum hint[SIZE];
int    mpiRank, mpiSize;

#define debug() do{ printf("Line:%d\n", __LINE__); }while(0)

int input(int argc, char* argv[]);
int factor(bignum* p, bignum* q, bignum N);
bignum bgcd(bignum a, bignum b);
bignum bminv(bignum a, bignum m);
bignum bmpow(bignum a, bignum e, bignum m);

int main(int argc, char* argv[])
{
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &mpiRank);
  MPI_Comm_size(MPI_COMM_WORLD, &mpiSize);
  if( mpiRank == 0 ) {
    printf("MPI Size = %d\n", mpiSize);
  }

  outstarttime();

  if( !input(argc, argv) ) {
    if( mpiRank == 0 ) {
      fprintf(stderr, "Cannot open image/hint file\n");
    }
    goto FINALIZE;
  }
  if( mpiRank == 0 ) fprintf(stderr, "Data input\n");

  // Factorize N
  bignum p, q;
  if( factor(&p, &q, rsaN) == false ) goto FINALIZE;
  if( mpiRank == 0 ) {
    puts("Factored!");
    printf("p:"); putnum16(p); puts("");
    printf("q:"); putnum16(q); puts("");
  }

  // Get secret key
  if( --p.l == 0xffffffffffffffffUL ) --p.h;
  if( --q.l == 0xffffffffffffffffUL ) --q.h;
  bignum phi = bmul(p, q);
  bignum rsaD = bminv(rsaE, phi);
  if( mpiRank == 0 ) {
    printf("phi = ");
    putnum16(phi);
    printf("\nD = ");
    putnum16(rsaD);
    puts("");
  }

  // Decode
  int st = SIZE * mpiRank / mpiSize;
  int ed = SIZE * (mpiRank + 1) / mpiSize;
  if( mpiRank == 0 ) st = 0;
  if( mpiRank + 1 == mpiSize ) ed = SIZE;
  for( int i = st ; i < ed ; ++i ) {
    bignum a = {i, 0}; // a = i;
    bignum c = bmpow(a, rsaD, rsaN);
    eimage[i] ^= (c.l & 1);
  }

  // Share
  int tag = 12;
  if( mpiRank == 0 ) {
    MPI_Status stt;
    for( int i = 1 ; i < mpiSize ; ++i ) {
      int st = SIZE * i / mpiSize;
      int ed = SIZE * (i + 1) / mpiSize;
      if( i == mpiSize - 1 ) ed = SIZE;
      ed -= st;
      if( ed ) {
	MPI_Recv(&eimage[st], ed, MPI_INT, i, tag, MPI_COMM_WORLD, &stt);
      }
    }
  } else {
    ed -= st;
    if( ed ) {
      MPI_Send(&eimage[st], ed, MPI_INT, 0, tag, MPI_COMM_WORLD);
    }
  }

  // Check
  if( mpiRank == 0 ) outresult(eimage);

 FINALIZE:
  MPI_Finalize();
  return 0;
}

// ===================================================================
// Input image data
// ===================================================================
int input(int argc, char* argv[])
{
  char filename[512];
  sprintf(filename, "eimage%s.txt", (argc<2)?"":argv[1]);
  FILE* fp = fopen(filename, "r");
  int ret1 = getimage(fp, eimage);
  fclose(fp);

  sprintf(filename, "hint%s.txt", (argc<2)?"":argv[1]);
  fp = fopen(filename, "r");
  int ret2 = gethint(fp, &rsaN, &rsaE, hard, hint);
  fclose(fp);

  // Information output
  if( mpiRank == 0 ) {
    printf("N = "); putnum16(rsaN); puts("");
    printf("E = "); putnum16(rsaE); puts("");
  }

  return !(ret1 + ret2);
}

// ===================================================================
// Factorize with Pollard's rho method
// ===================================================================
int rho(bignum* p, bignum* q, bignum N)
{
  bignum x = {random(), random()};
  x.l = (x.l << 32) + random();
  bignum c = {random(), random()};
  c.l = (c.l << 32) + random();
  bignum g = {1, 0}; // g = 1;
  int ngcd = 1;

  for( long j = 1 ; j <= 1 << 20 ; j <<= 1 ) {
    bignum y = x;
    for( long i = j ; i <= (j >> 1) * 3 ; ++i ) {
      x = mmul(x, x, N);
      x = madd(x, c, N);
    }
    for( long i = (j >> 1) * 3 + 1 ; i <= j * 2 ; ++i ) {
      if( bgt(x, y) ) *p = bsub(x, y);
      else            *p = bsub(y, x);
      if( p->h == 0 && p->l == 0 ) continue;

      if( ++ngcd < 200 ) {
	g = mmul(g, *p, N);
      } else {
	g = mmul(g, *p, N);
	*q = bgcd(N, g);
	ngcd = 1;
	if( q->h == N.h && q->l == N.l ) {
	  return 0; // not found
	}
	if( q->h != 0 || q->l != 1 ) {
	  *p = bquo(N, *q);
	  return 1; // factored
	}
      }

      x = mmul(x, x, N);
      x = madd(x, c, N);
    }
  }

  *q = bgcd(N, g);
  if( (q->h != N.h || q->l != N.l) &&
      (q->h != 0   || q->l != 1) ) {
    *p = bquo(N, *q);
    return 1; // factored
  }

  return 0; // not found
}

int factor(bignum* p, bignum* q, bignum N)
{
  srandom(mpiRank * 10 + time(NULL));

  int idFact = 0;
  for( int i = 0, fact = 0 ; ; ++i ) { // 30 \simeq 180[sec] / 5.5[sec/task]
    double st = MPI_Wtime();
    if( rho(p, q, N) ) {
      fact = mpiRank + 1;
      printf("[id:%d] Find factors (N = ", mpiRank);
      putnum16(*p); printf(" * ");
      putnum16(*q); printf(") (%.2fsec)\n", MPI_Wtime() - st);
    }
    MPI_Allreduce(&fact, &idFact, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
    if( idFact ) break;
    fprintf(stderr, "[id:%d] %d th failed (%.2fsec)\n", mpiRank, i + 1, MPI_Wtime() - st);
  }

  MPI_Barrier(MPI_COMM_WORLD);
  if( idFact ) {
    --idFact;
    MPI_Bcast(p, 2, MPI_LONG, idFact, MPI_COMM_WORLD);
    MPI_Bcast(q, 2, MPI_LONG, idFact, MPI_COMM_WORLD);
    return true;
  }

  return false; // Failed
}

// ===================================================================
// GCD in Bignum
// ===================================================================
bignum bgcd(bignum a, bignum b)
{
  bignum c = brem(a, b);
  while( c.h || c.l ) {
    a = b;
    b = c;
    c = brem(a, b);
  }
  return b;
}

bignum bminv(bignum x, bignum y)
{
  bignum r0 = x, r1 = y;
  bignum a0 = {1, 0}, a1 = {0, 0}; // a0 = 1

  int sa0 = 1, sa1 = 1;
  while( r1.h || r1.l ) {
    bignum q, r2, a2, a3;
    int sa2;
    bdiv(r0, r1, &q, &r2);
    a3 = bmul(q, a1);
    if( sa0 == sa1 ) {
      if( bge(a3, a0) ) {
	a2 = bsub(a3, a0);
	sa2 = -sa0;
      } else {
	a2 = bsub(a0, a3);
	sa2 = sa0;
      }
    } else {
      a2 = badd(a0, a3);
      sa2 = sa0;
    }
    r0 = r1; a0 = a1; sa0 = sa1;
    r1 = r2; a1 = a2; sa1 = sa2;
  }
  if( sa0 < 1 ) {
    a0 = bsub(y, a0);
  }

  return a0;
}

bignum bmpow(bignum a, bignum e, bignum m)
{
  bignum x = {1, 0}; // x = 1;
  unsigned long b = 1UL << 63;
  for( b = 1UL << 63 ; b ; b >>= 1 ) if( e.h & b ) break;

  for( ; b ; b >>= 1 ) {
    x = mmul(x, x, m);
    if( e.h & b ) x = mmul(x, a, m);
  }
  for( b = 1UL << 63 ; b ; b >>= 1 ) {
    x = mmul(x, x, m);
    if( e.l & b ) x = mmul(x, a, m);
  }
  return x;
}

