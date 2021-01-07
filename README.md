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

* Interleave two paired-end FASTQ files:

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
 
