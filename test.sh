#!/usr/bin/env bash

make clean
make

wget https://sourceforge.net/projects/fusioncatcher/files/test/reads_1.fq.gz -O reads_1.fq.gz
wget https://sourceforge.net/projects/fusioncatcher/files/test/reads_2.fq.gz -O reads_2.fq.gz

gzip -f -d *.gz

# test 1
echo "Test 1"
./fastq-leave interleave reads_1.fq reads_2.fq ri.fq
./fastq-leave deinterleave ri.fq r1.fq r2.fq
diff r1.fq reads_1.fq
diff r2.fq reads_2.fq

# test 1
echo "Test 2"
./fastq-leave interleave reads_1.fq reads_2.fq - | ./fastq-leave deinterleave - r1.fq r2.fq
diff r1.fq reads_1.fq
diff r2.fq reads_2.fq


echo "Test 3"
./fastq-leave count reads_1.fq count_reads_1.txt

echo "Test 4"
./fastq-leave count ri.fq count_ri.txt


echo "Test 5"
./fastq-leave lengths reads_1.fq lengths_reads_1.txt

echo "Test 6"
./fastq-leave lengths ri.fq lengths_ri.txt

echo "Test 7"
./fastq-leave count-lengths ri.fq count_ri_.txt lengths_ri_.txt

echo "Test 8"
./fastq-leave tab4 ri.fq tab4_ri.txt

echo "Test 9"
./fastq-leave tab8 ri.fq tab8_ri.txt

echo "Test 10"
./fastq-leave detab tab4_ri.txt ri_4back.fq

echo "Test 11"
./fastq-leave detab tab8_ri.txt ri_8back.fq

