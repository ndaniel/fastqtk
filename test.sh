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
./fastqtk interleave reads_1.fq reads_2.fq ri.fq
./fastqtk deinterleave ri.fq r1.fq r2.fq
diff r1.fq reads_1.fq
diff r2.fq reads_2.fq

# test 1
echo "Test 2"
./fastqtk interleave reads_1.fq reads_2.fq - | ./fastqtk deinterleave - r1_.fq r2_.fq
diff r1_.fq reads_1.fq
diff r2_.fq reads_2.fq


echo "Test 3"
./fastqtk count reads_1.fq count_reads_1.txt

echo "Test 4"
./fastqtk count ri.fq count_ri.txt


echo "Test 5"
./fastqtk lengths reads_1.fq lengths_reads_1.txt

echo "Test 6"
./fastqtk lengths ri.fq lengths_ri.txt

echo "Test 7"
./fastqtk count-lengths ri.fq count_ri_.txt lengths_ri_.txt

echo "Test 8"
./fastqtk tab-4 ri.fq tab4_ri.txt

echo "Test 9"
./fastqtk tab-8 ri.fq tab8_ri.txt

echo "Test 10"
./fastqtk detab tab4_ri.txt ri_4back.fq

echo "Test 11"
./fastqtk detab tab8_ri.txt ri_8back.fq

echo "Test 12"
./fastqtk drop-short 101 reads_1.fq r1_dropped.fq
cat reads_1.fq | ./fastqtk drop-short 101 - - | cat >r1_dropped_.fq

echo "Test 13"
./fastqtk NtoA reads_1.fq reads_1_n2a.fq
#sed 's/GAAAAAA/GNNNNNN/g' reads_1.fq | ./fastqtk  NtoA - reads_1_n2a.fq
#diff reads_1_n2a.fq reads_1.fq

echo "Test 14"
./fastqtk trim-5 7 reads_1.fq reads_1_trim5_7.fq
./fastqtk trim-5 99 reads_1.fq reads_1_trim5_99.fq
./fastqtk trim-5 0 reads_1.fq reads_1_trim5_0.fq

echo "Test 15"
./fastqtk trim-3 7 reads_1.fq reads_1_trim3_7.fq
./fastqtk trim-3 99 reads_1.fq reads_1_trim3_99.fq
./fastqtk trim-3 0 reads_1.fq reads_1_trim3_0.fq

echo "Test 16"
./fastqtk retain-5 7 reads_1.fq reads_1_retain5_7.fq
./fastqtk retain-5 99 reads_1.fq reads_1_retain5_99.fq
./fastqtk retain-5 100 reads_1.fq reads_1_retain5_100.fq

echo "Test 17"
./fastqtk retain-3 7 reads_1.fq reads_1_retain3_7.fq
./fastqtk retain-3 99 reads_1.fq reads_1_retain3_99.fq
./fastqtk retain-3 100 reads_1.fq reads_1_retain3_100.fq

echo "Test 18"
./fastqtk trim-id reads_1.fq reads_1_trim-id.fq
./fastqtk trim-id reads_2.fq reads_2_trim-id.fq

echo "Test 19"
./fastqtk trim-poly N 1 reads_1.fq reads_1_trimN.fq
./fastqtk trim-poly N 1 reads_2.fq reads_2_trimN.fq

echo "Test 20"
./fastqtk trim-poly A 12 reads_1.fq reads_1_trim-polya.fq
./fastqtk trim-poly A 12 reads_2.fq reads_2_trim-polya.fq

echo "Test 21"
./fastqtk compress-id @ count_reads_1.txt reads_1.fq reads_1_compressed-ids.fq
./fastqtk compress-id count_reads_1.txt reads_1.fq reads_1_compressed-ids_.fq
./fastqtk compress-id 200000000 reads_1.fq reads_1_compressed-ids__.fq
./fastqtk compress-id /1 count_reads_1.txt reads_1.fq reads_1_compressed-ids-1.fq
./fastqtk compress-id /2 count_reads_1.txt reads_2.fq reads_2_compressed-ids-2.fq
./fastqtk compress-id _1 count_reads_1.txt reads_1.fq reads_1_compressed-ids-1_.fq
./fastqtk compress-id _2 count_reads_1.txt reads_2.fq reads_2_compressed-ids-2_.fq
./fastqtk compress-id @@ count_ri.txt ri.fq ri_compressed-ids.fq
./fastqtk compress-id /12 count_ri.txt ri.fq ri_compressed-ids-12.fq
./fastqtk compress-id _12 count_reads_1.txt ri.fq ri_compressed-ids_12.fq


echo "Test 22"

./fastqtk drop-se ri.fq ri_dropse.fq
./fastqtk drop-se rik.fq rik_dropse.fq

./fastqtk drop-se reads_1.fq reads_1_dropse.fq

#./fastqtk interleave reads_1_compressed-ids-1.fq reads_2_compressed-ids-2.fq reads_compressed-ids.fq

./fastqtk drop-se reads_compressed-ids.fq reads_compressed-ids_dropse.fq

echo "Test 23"

./fastqtk fq2fa reads_1.fq reads_1.fa

echo "Test 24"

./fastqtk fa2fq reads_1.fa reads_1_back.fq

echo "Test 25"

./fastqtk rev-com reads_1.fq reads_1_revcom.fq

echo "Test 26"

./fastqtk trim-poly A 15 poly.fq poly_polyA.fq
./fastqtk trim-poly C 15 poly.fq poly_polyC.fq
./fastqtk trim-poly A 1 poly.fq poly_polyA1.fq
./fastqtk trim-poly ACGT 15 poly.fq poly_polyACGT.fq
./fastqtk trim-poly N 2 poly.fq poly_N.fq

