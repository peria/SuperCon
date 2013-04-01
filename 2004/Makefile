SRC	= decrypt.c supercon.c
OBJ	= $(SRC:.c=.o)
HEAD	= supercon.h

CC	= mpicc
CFLAGS	= -O3 -std=c99 -W -Wall
LDFLAGS = -O3 -std=c99

LIBS	= 

.PHONY: all clean

all: decrypt
	cp -f earth.txt answer.txt
	time mpirun -np 2 ./decrypt

A:
	cp -f monkey.txt answer.txt
	time mpirun -np 2 ./decrypt A

C:
	cp -f monkey.txt answer.txt
	time mpirun -np 2 ./decrypt C


decrypt: $(OBJ) Makefile
	$(CC) $(LDFLAGS) $(OBJ) -o $@ $(LIBS)

%.o: %.c $(HEAD) Makefile
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *~ *.o *.gmon
