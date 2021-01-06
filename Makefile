CC=gcc
CFLAGS=-g -Wall -O2 -Wno-unused-function

all:fastq-leave

fastq-leave:fastq-leave.c
		$(CC) $(CFLAGS) fastq-leave.c -o $@ -lz -lm

clean:
		rm -fr *.o ext/*.o a.out fastq-leave *~ *.a *.dSYM session*
