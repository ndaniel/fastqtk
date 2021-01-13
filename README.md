FASTQ tools
------------

**fastqtk** is a fast and lightweight tool for interleaving/deinterleaving/counting/trimming FASTQ files.

Installation
------------

```
git clone https://github.com/ndaniel/fastqtk.git
cd fastqtk
make
```

Usage
-----
```
Usage:   fastqtk <command> <arguments>

Command: interleave        interleaves two paired-end FASTQ files.
         deinterleave      splits an (already) interleaved FASTQ file.
         count             counts all reads from a FASTQ file.
         lengths           summary statistics for lengths of reads from a FASTQ file.
         count-lengths     number of reads and summary statistics for lengths of reads from a FASTQ file.
         tab4              converts a FASTQ file to a text tab-delimited file with 4 columns.
         tab8              converts a (interleaved paired-end) FASTQ file to text tab-delimited file with 8 columns.
         detab             converts a text tab-delimited file with 4 or 8 columns (which was converted using tab4 or tab8) to FASTQ file.
         drop-short        drops reads that have short sequences (below a given threshold).
         NtoA              replaces all Ns in reads sequences with As in a FASTQ file.
         trim5             trims 5' end of the reads from a FASTQ file.
         trim3             trims 3' end of the reads from a FASTQ file.
         retain5           retains the first N bp from 5'end of the reads from a FASTQ file.
         retain3           retains the last N bp from 3'end of the reads from a FASTQ file.
         trim-id           trims the reads ids from a FASTQ file from the end to the first blank in the string.
         trim-N            trim N at both ends of the reads from a FASTQ file.
         trim-polyA        trim polyA at both ends of the reads from a FASTQ file.
         compress-id       lossy compression of the reads ids from a FASTQ file.

```

Examples
--------------

* Interleave two FASTQ paired-end files:

        fastqtk interleave reads1.fq reads2.fq out.fq
        fastqtk interleave reads1.fq reads2.fq - | gzip > out.fq.gz

* Deinterleave a FASTQ file:

        fastqtk deinterleave reads.fq ou1.fq ou2.fq
        zcat reads.fq.gz | fastqtk deinterleave - ou1.fq ou2.fq

* Count reads from a FASTQ file:

        fastqtk count reads.fq count.txt

* Summary statistics regarding lengths of reads from a FASTQ file:

        fastqtk lengths reads.fq lengths.txt

* Count of reads and summary statistics regarding lengths of reads from a FASTQ file:

        fastqtk count-lengths reads.fq count.txt lengths.txt

* Convert a FASTQ file into a text tab-delimited file with four columns:

        fastqtk tab4 reads.fq fastq.txt
        
* Convert an interleaved FASTQ file into a text tab-delimited file with 8 columns:

        fastqtk tab8 reads.fq fastq.txt

* Convert back text tab-delimited file with 4 or 8 columns into a FASTQ file:

        fastqtk detab fastq.txt reads.fq

* Remove the reads that are strictly shorter than 30bp from a FASTQ file:

        fastqtk drop-short 30 reads.fq out.fq
        
* Replace all Ns in reads sequences from a FASTQ file:

        fastqtk NtoA reads.fq out.fq

* Trim 10bp from 5' end of the reads sequences from FASTQ file:

        fastqtk trim5 10 reads.fq out.fq
        
* Trim 10bp from 3' end of the reads sequences from FASTQ file:

        fastqtk trim3 10 reads.fq out.fq
        
* Retain the first 70bp from 5' end of the reads sequences and trim the rest:

        fastqtk retain5 70 reads.fq out.fq
        
* Retain the first 70bp from 3' end of the reads sequences and trim the rest:

        fastqtk retain3 70 reads.fq out.fq
        
* Trim N or Ns from both ends of the reads sequences from FASTQ file:

        fastqtk trim-N reads.fq out.fq

* Trim polyA, which are strictly or equally longer than 15bp, from both ends of the reads sequences from FASTQ file:

        fastqtk trim-polyA 15 reads.fq out.fq

* Compress the reads ids from a FASTQ file:

        fastqtk count reads.fq count.txt
        fastqtk compress-id @ count.txt reads.fq out.fq
        
        
