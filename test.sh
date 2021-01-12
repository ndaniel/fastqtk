#!/usr/bin/env bash

make clean
make

FILE=reads_1.fq
if [ ! -f "$FILE" ]; then
    wget https://sourceforge.net/projects/fusioncatcher/files/test/reads_1.fq.gz -O reads_1.fq.gz
    wget https://sourceforge.net/projects/fusioncatcher/files/test/reads_2.fq.gz -O reads_2.fq.gz
    gzip -f -d *.gz
fi

# test 1
echo "Test 1"
./fastq-leave interleave reads_1.fq reads_2.fq ri.fq
./fastq-leave deinterleave ri.fq r1.fq r2.fq
diff r1.fq reads_1.fq
diff r2.fq reads_2.fq

# test 1
echo "Test 2"
./fastq-leave interleave reads_1.fq reads_2.fq - | ./fastq-leave deinterleave - r1_.fq r2_.fq
diff r1_.fq reads_1.fq
diff r2_.fq reads_2.fq


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

echo "Test 12"
./fastq-leave drop-short 101 reads_1.fq r1_dropped.fq
cat reads_1.fq | ./fastq-leave drop-short 101 - - | cat >r1_dropped_.fq

echo "Test 13"
./fastq-leave NtoA reads_1.fq reads_1_n2a.fq
#sed 's/GAAAAAA/GNNNNNN/g' reads_1.fq | ./fastq-leave  NtoA - reads_1_n2a.fq
#diff reads_1_n2a.fq reads_1.fq

echo "Test 14"
./fastq-leave trim5 7 reads_1.fq reads_1_trim5_7.fq
./fastq-leave trim5 99 reads_1.fq reads_1_trim5_99.fq
./fastq-leave trim5 0 reads_1.fq reads_1_trim5_0.fq

echo "Test 15"
./fastq-leave trim3 7 reads_1.fq reads_1_trim3_7.fq
./fastq-leave trim3 99 reads_1.fq reads_1_trim3_99.fq
./fastq-leave trim3 0 reads_1.fq reads_1_trim3_0.fq

echo "Test 16"
./fastq-leave retain5 7 reads_1.fq reads_1_retain5_7.fq
./fastq-leave retain5 99 reads_1.fq reads_1_retain5_99.fq
./fastq-leave retain5 100 reads_1.fq reads_1_retain5_100.fq

echo "Test 17"
./fastq-leave retain3 7 reads_1.fq reads_1_retain3_7.fq
./fastq-leave retain3 99 reads_1.fq reads_1_retain3_99.fq
./fastq-leave retain3 100 reads_1.fq reads_1_retain3_100.fq

echo "Test 18"
./fastq-leave trim-id reads_1.fq reads_1_trim-id.fq
./fastq-leave trim-id reads_2.fq reads_2_trim-id.fq

echo "Test 19"
./fastq-leave trim-N reads_1.fq reads_1_trimN.fq
./fastq-leave trim-N reads_2.fq reads_2_trimN.fq

echo "Test 20"
./fastq-leave trim-polyA 12 reads_1.fq reads_1_trim-polya.fq
./fastq-leave trim-polyA 12 reads_2.fq reads_2_trim-polya.fq

echo "Test 21"
./fastq-leave compress-id @ count_reads_1.txt reads_1.fq reads_1_compressed-ids.fq
./fastq-leave compress-id /1 count_reads_1.txt reads_1.fq reads_1_compressed-ids-1.fq
./fastq-leave compress-id /2 count_reads_1.txt reads_2.fq reads_2_compressed-ids-2.fq
./fastq-leave compress-id _1 count_reads_1.txt reads_1.fq reads_1_compressed-ids-1_.fq
./fastq-leave compress-id _2 count_reads_1.txt reads_2.fq reads_2_compressed-ids-2_.fq
./fastq-leave compress-id @@ count_ri.txt ri.fq ri_compressed-ids.fq
./fastq-leave compress-id /12 count_ri.txt ri.fq ri_compressed-ids-12.fq
./fastq-leave compress-id _12 count_reads_1.txt ri.fq ri_compressed-ids_12.fq
