FASTQ (de)interleave tools
--------------------------

**fastq-leave** is a fast and lightweight tool for interleaving and deinterleavng FASTQ files.

Installation
------------

```
git clone https://github.com/ndaniel/fastq-leave.git
cd fastq-leave
make
```

Usage
-----
```
Usage:   fastq-leave <command> <arguments>

Command: interleave        interleave two paired-end FASTQ files
         deinterleave      splits one interleave FASTQ file
```

Examples
--------------

* Interleave two FASTQ paired-end files:

        fastq-leave interleave reads1.fq reads2.fq out.fq
        fastq-leave interleave reads1.fq reads2.fq - | gzip > out.fq.gz

* Deinterleave a FASTQ file:

        fastq-leave deinterleave reads.fq ou1.fq ou2.fq
        zcat reads.fq.gz | fastq-leave deinterleave - ou1.fq ou2.fq

* Count reads from a FASTQ file:

        fastq-leave count reads.fq count.txt

* Summary statistics regarding lengths of reads from a FASTQ file:

        fastq-leave lengths reads.fq lengths.txt

* Count of reads and summary statistics regarding lengths of reads from a FASTQ file:

        fastq-leave count-lengths reads.fq count.txt lengths.txt

* Convert a FASTQ file into a text tab-delimited file with four columns:

        fastq-leave tab4 reads.fq fastq.txt
        
* Convert an interleaved FASTQ file into a text tab-delimited file with 8 columns:

        fastq-leave tab8 reads.fq fastq.txt

* Convert back text tab-delimited file with 4 or 8 columns into a FASTQ file:

        fastq-leave detab fastq.txt reads.fq

* Remove the reads that are strictly shorter than 30bp from a FASTQ file:

        fastq-leave drop-short 30 reads.fq out.fq
        
* Replace all Ns in reads sequences from a FASTQ file:

        fastq-leave NtoA reads.fq out.fq

* Trim 10bp from 5' end of the reads sequences from FASTQ file:

        fastq-leave trim5 10 reads.fq out.fq
        
* Trim 10bp from 3' end of the reads sequences from FASTQ file:

        fastq-leave trim3 10 reads.fq out.fq
        
* Retain the first 70bp from 5' end of the reads sequences and trim the rest:

        fastq-leave retain5 70 reads.fq out.fq
        
* Retain the first 70bp from 3' end of the reads sequences and trim the rest:

        fastq-leave retain3 70 reads.fq out.fq
        
* Trim N or Ns from both ends of the reads sequences from FASTQ file:

        fastq-leave trim-N reads.fq out.fq

* Trim polyA, which are strictly or equally longer than 15bp, from both ends of the reads sequences from FASTQ file:

        fastq-leave trim-polyA 15 reads.fq out.fq

* Compress the reads ids from a FASTQ file:

        fastq-leave count reads.fq count.txt
        fastq-leave compress-id @ count.txt reads.fq out.fq
        
        
