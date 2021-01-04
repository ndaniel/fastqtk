FASTQ (de)interleave tools
--------------------------

*fastq-leave* is a fast and lightweight tool for interleaving and deinterleavng FASTQ files.

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

* Interleave two FASTQ files:

        fastq-leave interleave reads1.fq reads2.fq out.fq
        fastq-leave interleave reads1.fq reads2.fq - | gzip > out.fq.gz

* Deinterleave a FASTQ file:

        fastq-leave deinterleave reads.fq ou1.fq ou2.fq
        zcat reads.fq.gz | fastq-leave deinterleave - ou1.fq ou2.fq
