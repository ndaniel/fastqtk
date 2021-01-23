#!/usr/bin/env bash


FILE=reads_1.fq
if [ ! -f "$FILE" ]; then
    wget https://sourceforge.net/projects/fusioncatcher/files/test/reads_1.fq.gz -O reads_1.fq.gz
    wget https://sourceforge.net/projects/fusioncatcher/files/test/reads_2.fq.gz -O reads_2.fq.gz
    gzip -f -d *.gz
fi


echo "Test"
./fastqtk interleave reads_1.fq reads_2.fq - | ./fastqtk deinterleave - r1.fq r2.fq

if diff "reads_1.fq" "r1.fq" >/dev/null ; then
  echo -e "\n\n\n\033[33;7m   Testing of FASTQTK went fine!   \033[0m\n"
else
  echo -e "\n\n\n\033[33;7m   ERROR: Something wrong with FASTQTK! Testing did not go well!   \033[0m\n"
fi

if diff "reads_2.fq" "r2.fq" >/dev/null ; then
  echo -e "\n"
else
  echo -e "\n\n\n\033[33;7m   ERROR: Something wrong with FASTQTK! Testing did not go well!   \033[0m\n"
fi

