CC=gcc
CFLAGS=-g -Wall -O3 -march=native -flto -Wno-unused-function

all:fastqtk

fastqtk:fastqtk.c
		$(CC) $(CFLAGS) fastqtk.c -o $@ -lz -lm

clean:
		rm -fr *.o ext/*.o a.out fastqtk *~ *.a *.dSYM session*
