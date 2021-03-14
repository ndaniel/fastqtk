#!/usr/bin/env bash


FILE=reads_1.fq
if [ ! -f "$FILE" ]; then
    wget https://sourceforge.net/projects/fusioncatcher/files/test/reads_1.fq.gz -O reads_1.fq.gz
    wget https://sourceforge.net/projects/fusioncatcher/files/test/reads_2.fq.gz -O reads_2.fq.gz
    gzip -f -d *.gz
fi


echo "Testing.."
./fastqtk interleave reads_1.fq reads_2.fq - | ./fastqtk deinterleave - r1.fq r2.fq

if diff "reads_1.fq" "r1.fq" >/dev/null ; then
  echo -e "  * Test 1 -> Ok!\n"
else
  echo -e "\n\n\n\033[33;7m   ERROR: Something wrong with FASTQTK! Testing did not go well!   \033[0m\n"
  exit 1
fi

if diff "reads_2.fq" "r2.fq" >/dev/null ; then
  echo -e "  * Test 2 -> Ok!\n"
else
  echo -e "\n\n\n\033[33;7m   ERROR: Something wrong with FASTQTK! Testing did not go well!   \033[0m\n"
  exit 1
fi

cat reads_1.fq | echo $((`wc -l`/4)) > n_wc.txt
./fastqtk count reads_1.fq  n_ftk.txt

if diff "n_wc.txt" "n_ftk.txt" >/dev/null ; then
  echo -e "  * Test 3 -> ok!\n"
else
  echo -e "\n\n\n\033[33;7m   ERROR: Something wrong with FASTQTK! Testing did not go well!   \033[0m\n"
  exit 1
fi

./fastqtk count-lengths reads_1.fq c_ftk.txt l_ftk.txt
cat reads_1.fq | echo $((`wc -l`/4)) > c_wc.txt

cat reads_1.fq | paste - - - - | cut -f 2 | LC_ALL=C awk '{ print length($0) }' | sort -run > l_awk.txt 

if diff "c_wc.txt" "c_ftk.txt" >/dev/null ; then
  echo -e "  * Test 4 -> ok!\n"
else
  echo -e "\n\n\n\033[33;7m   ERROR: Something wrong with FASTQTK! Testing did not go well!   \033[0m\n"
  exit 1
fi


if diff "l_awk.txt" "l_ftk.txt" >/dev/null ; then
  echo -e "  * Test 5 -> ok!\n"
else
  echo -e "\n\n\n\033[33;7m   ERROR: Something wrong with FASTQTK! Testing did not go well!   \033[0m\n"
  exit 1
fi



echo -e "\n\033[33;7m   Testing of FASTQTK went fine!   \033[0m\n"

rm c_wc.txt
rm c_ftk.txt
rm l_ftk.txt
rm l_awk.txt
rm r1.fq
rm r2.fq
rm n_wc.txt
rm n_ftk.txt
