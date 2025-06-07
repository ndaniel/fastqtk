/* The MIT License

   Copyright (c) 2010-2021 Daniel Nicorici <daniel.nicorici@gmail.com>

   Permission is hereby granted, free of charge, to any person obtaining
   a copy of this software and associated documentation files (the
   "Software"), to deal in the Software without restriction, including
   without limitation the rights to use, copy, modify, merge, publish,
   distribute, sublicense, and/or sell copies of the Software, and to
   permit persons to whom the Software is furnished to do so, subject to
   the following conditions:

   The above copyright notice and this permission notice shall be
   included in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
   NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
   BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
   ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
   CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>


#define BUFFER_SIZE  32 * 1024 * 1024
//define BUFFER_SIZE  32 * 1024 * 1024
//#define BUFFER_SIZE   3731

//maximum length of reads
#define MAX_LEN  32*1024

#define SIZE_OF_CHAR sizeof(char)

////////////////////////////////////////////////////////////////////////////////
const char CARS[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
const int CARS_LEN = strlen(CARS);
const char newline = '\n';

char toCARS[256];

char DIGITS = 0;

char COUNTER[] = "----------------------------------------------------";
int COUNTER_LEN_MAX;
int COUNTER_LEN;

char EXTRA[100] = "\n";

void nextid(void) {
    
    int n;
    char x;
    int i;
    char flag;
    
    if(COUNTER[0]!='@') {
        // first call -- initialize 
        COUNTER_LEN_MAX = strlen(COUNTER);
        
        for (i=0;i<256;i++) {
            toCARS[i] = 0;
        }
        for (i=0;i<CARS_LEN;i++) {
            toCARS[(int)CARS[i]] = (char) i + 1;
        }
        
        for (i=1;i<DIGITS+1;i++) {
            COUNTER[i] = CARS[0];
        }
        n = strlen(EXTRA);
        if (n == 1) {
            COUNTER[i] = 10; // newline
            COUNTER_LEN = DIGITS + 2;
        } else if (n == 3 || n == 2) {
            COUNTER[i++] = EXTRA[0];
            COUNTER[i++] = EXTRA[1];
            COUNTER[i] = 10; //newline
            COUNTER_LEN = DIGITS + 4;
        }
        COUNTER[0] = (int) '@';
    } else {
        flag = 0;
        if (COUNTER[1] == CARS[CARS_LEN-1]) {
            flag = 1;
        }
        for (i=DIGITS;i>0;i--) {
            x = toCARS[(int)COUNTER[i]];
            if (x==CARS_LEN) {
                COUNTER[i] = CARS[0];
            } else {
                COUNTER[i] = CARS[(int)x];
                break;
            }
        }
        if ((flag == 1) && (COUNTER[1] == CARS[0])) {
            fprintf(stderr,"ERROR: Counter too small (%s)!",COUNTER);
            exit(2);
        }


    }

}

////////////////////////////////////////////////////////////////////////////////
FILE* myfopen(char* filename, char* mode) {
    FILE *ptr = fopen(filename, mode);
    if (ptr == NULL) {
        fprintf(stderr, "ERROR: Failed to open the file '%s'!\n",filename);
        exit(2);
    }
    return ptr;
}

////////////////////////////////////////////////////////////////////////////////
char* mymalloc(size_t n) 
{
    char *ptr = (char *)malloc(SIZE_OF_CHAR * n);
    if (ptr == NULL) {
        fprintf(stderr, "ERROR: Failed to allocate memory!\n");
        exit(2);
    }
    return ptr;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
int main(int argc, char * argv[])
{

    char is_stdin = 0;
    char is_stdout = 0;
    char r1 = 0;
    char end1 = 0;
    char end2 = 0;
    
    char len[MAX_LEN];

    long long int u;

    char *buffer;
    char *b1;
    char *b2;
    char flag = 0;
    char usage = 0;
    char windows = 0;
    

    size_t  bytes_read,bytes_read1,bytes_read2,x;
    size_t  i,j,k,b1i,b2i,l;
    size_t j1,j2,k1,k2,k1old,k2old;
    long int threshold,trim,retain;

    FILE *fip, *fr1, *fr2;
    FILE *fop, *fi1p, *fi2p;
    

    usage = 1;
    if (argc > 1) {
        if (strcmp(argv[1],"deinterleave") == 0) {
            if (argc != 5) {
                fprintf(stderr, "\n");
                fprintf(stderr, "Usage:   fastqtk  deinterleave  <in.fq>  <out1.fq>  <out2.fq>\n\n");
                fprintf(stderr, "It splits an interleaved input FASTQ file into two paired-end FASTQ files.\n");
                fprintf(stderr, "For input from STDIN use - instead of <in.fq>.\n\n");
                return 1;
            } else {
                usage = 2;
            }
        } else if (strcmp(argv[1],"interleave") == 0) {
            if (argc != 5) {
                fprintf(stderr, "\n");
                fprintf(stderr, "Usage:   fastqtk  interleave  <in1.fq>  <in2.fq>  <out.fq>\n\n");
                fprintf(stderr, "It interleaves two input paired-end FASTQ files into one output FASTQ file.\n");
                fprintf(stderr, "For redirecting output to STDOUT use - instead of <out.fq>.\n\n");
                return 1;
            } else {
                usage = 3;
            }
        } else if (strcmp(argv[1],"count") == 0) {
            if (argc != 4) {
                fprintf(stderr, "\n");
                fprintf(stderr, "Usage:   fastqtk  count  <in.fq>  <out.txt>\n\n");
                fprintf(stderr, "It provides the total number of reads from an input FASTQ file and outputs it to a text file.\n");
                fprintf(stderr, "For redirecting to STDOUT/STDIN use - instead of file name.\n\n");
                return 1;
            } else {
                usage = 4;
            }
        } else if (strcmp(argv[1],"lengths") == 0) {
            if (argc != 4) {
                fprintf(stderr, "\n");
                fprintf(stderr, "Usage:   fastqtk  lengths  <in.fq>  <out.txt>\n\n");
                fprintf(stderr, "It provides a summary statistics regarding the lengths of the reads from an input FASTQ file and outputs it to a text file.\n");
                fprintf(stderr, "The output text file contains the unique lengths of the reads found in the input file, which are sorted in descending order.\n");
                fprintf(stderr, "For redirecting to STDOUT/STDIN use - instead of file name.\n\n");
                return 1;
            } else {
                usage = 5;
            }
        } else if (strcmp(argv[1],"count-lengths") == 0) {
            if (argc != 5) {
                fprintf(stderr, "\n");
                fprintf(stderr, "Usage:   fastqtk  count-lengths  <in.fq>  <count.txt>  <statistics.txt>\n\n");
                fprintf(stderr, "It provides total number of reads and a summary statistics regarding the lengths of the reads from an input FASTQ file and outputs it to a text file.\n");
                return 1;
            } else {
                usage = 6;
            }
        } else if (strcmp(argv[1],"tab-4") == 0) {
            if (argc != 4) {
                fprintf(stderr, "\n");
                fprintf(stderr, "Usage:   fastqtk  tab-4  <in.fq>  <fastq.txt>\n\n");
                fprintf(stderr, "It converts a FASTQ file into a tab-delimited text file with four columns.\n");
                fprintf(stderr, "For redirecting to STDOUT/STDIN use - instead of file name.\n\n");
                return 1;
            } else {
                usage = 7;
            }
        } else if (strcmp(argv[1],"tab-8") == 0) {
            if (argc != 4) {
                fprintf(stderr, "\n");
                fprintf(stderr, "Usage:   fastqtk  tab-8  <in.fq>  <fastq.txt>\n\n");
                fprintf(stderr, "It converts an interleaved paired-end FASTQ file into a tab-delimited text file with 8 columns.\n");
                fprintf(stderr, "For redirecting to STDOUT/STDIN use - instead of file name.\n\n");
                return 1;
            } else {
                usage = 8;
            }
        } else if (strcmp(argv[1],"detab") == 0) {
            if (argc != 4) {
                fprintf(stderr, "\n");
                fprintf(stderr, "Usage:   fastqtk  detab  <fastq.txt>  <out.fq>\n\n");
                fprintf(stderr, "It converts a 4 or 8 columns text tab-delimited file into a FASTQ file.\n");
                fprintf(stderr, "For redirecting to STDOUT/STDIN use - instead of file name.\n\n");
                return 1;
            } else {
                usage = 9;
            }
        } else if (strcmp(argv[1],"drop-short") == 0) {
            if (argc != 5) {
                fprintf(stderr, "\n");
                fprintf(stderr, "Usage:   fastqtk  drop-short  <N>  <in.fq>  <out.fq>\n\n");
                fprintf(stderr, "It drops the reads that have the sequences stricly shorter than N. N is a non-zero positive integer.\n");
                fprintf(stderr, "For redirecting to STDOUT/STDIN use - instead of file name.\n\n");
                return 1;
            } else {
                usage = 10;
            }
        } else if (strcmp(argv[1],"NtoA") == 0) {
            if (argc != 4) {
                fprintf(stderr, "\n");
                fprintf(stderr, "Usage:   fastqtk  NtoA  <in.fq>  <out.fq>\n\n");
                fprintf(stderr, "It replaces all the As from reads sequences with As in a FASTQ file.\n");
                fprintf(stderr, "For redirecting to STDOUT/STDIN use - instead of file name.\n\n");
                return 1;
            } else {
                usage = 11;
            }
        } else if (strcmp(argv[1],"trim-5") == 0) {
            if (argc != 5) {
                fprintf(stderr, "\n");
                fprintf(stderr, "Usage:   fastqtk  trim-5  <N>  <in.fq>  <out.fq>\n\n");
                fprintf(stderr, "It trims N nucleotides from 5' end of the reads from a FASTQ file. N is a non-zero positive integer.\n");
                fprintf(stderr, "For redirecting to STDOUT/STDIN use - instead of file name.\n\n");
                return 1;
            } else {
                usage = 12;
            }
        } else if (strcmp(argv[1],"trim-3") == 0) {
            if (argc != 5) {
                fprintf(stderr, "\n");
                fprintf(stderr, "Usage:   fastqtk  trim-3  <N>  <in.fq>  <out.fq>\n\n");
                fprintf(stderr, "It trims N nucleotides from 3' end of the reads from a FASTQ file. N is a non-zero positive integer.\n");
                fprintf(stderr, "For redirecting to STDOUT/STDIN use - instead of file name.\n\n");
                return 1;
            } else {
                usage = 13;
            }
        } else if (strcmp(argv[1],"retain-5") == 0) {
            if (argc != 5) {
                fprintf(stderr, "\n");
                fprintf(stderr, "Usage:   fastqtk  retain-5  <N>  <in.fq>  <out.fq>\n\n");
                fprintf(stderr, "It retains the first N nucleotides from 5' end of the reads from a FASTQ file. N is a non-zero positive integer.\n");
                fprintf(stderr, "For redirecting to STDOUT/STDIN use - instead of file name.\n\n");
                return 1;
            } else {
                usage = 14;
            }
        } else if (strcmp(argv[1],"retain-3") == 0) {
            if (argc != 5) {
                fprintf(stderr, "\n");
                fprintf(stderr, "Usage:   fastqtk  retain-3  <N>  <in.fq>  <out.fq>\n\n");
                fprintf(stderr, "It retains the last N nucleotides from 3' end of the reads from a FASTQ file. N is a non-zero positive integer.\n");
                fprintf(stderr, "For redirecting to STDOUT/STDIN use - instead of file name.\n\n");
                return 1;
            } else {
                usage = 15;
            }
        } else if (strcmp(argv[1],"trim-id") == 0) {
            if (argc != 4) {
                fprintf(stderr, "\n");
                fprintf(stderr, "Usage:   fastqtk  trim-id  <in.fq>  <out.fq>\n\n");
                fprintf(stderr, "It retains the beginning part of the reads ids all the way to the first blank space or newline. Basically the reads ids are\n");
                fprintf(stderr, "truncated after the first blank space if they have one. Also the trims ids for the quality sequences (every third line is changed to +).\n");
                fprintf(stderr, "For redirecting to STDOUT/STDIN use - instead of file name.\n\n");
                return 1;
            } else {
                usage = 16;
            }
        } else if (strcmp(argv[1],"trim-poly") == 0) {
            if (argc != 6) {
                fprintf(stderr, "\n");
                fprintf(stderr, "Usage:   fastqtk  trim-poly <A|C|G|T|N|ACGT>  <M> <in.fq>  <out.fq>\n\n");
                fprintf(stderr, "It trims poly-A/C/G/T/N tails at both ends of the reads sequences from a FASTQ file.\n");
                fprintf(stderr, "For redirecting to STDOUT/STDIN use - instead of file name.\n\n");
                fprintf(stderr, "Options:\n");
                fprintf(stderr, "     *  A|C|G|T|N|ACGT  (nucleotide or nucleotides that form the poly tails that will be trimmed):\n");
                fprintf(stderr, "               A             - poly-A tails are trimmed.\n");
                fprintf(stderr, "               C             - poly-C tails are trimmed.\n");
                fprintf(stderr, "               G             - poly-G tails are trimmed.\n");
                fprintf(stderr, "               T             - poly-T tails are trimmed.\n");
                fprintf(stderr, "               N             - poly-N tails are trimmed.\n");
                fprintf(stderr, "               ACGT          - poly-A, poly-C, poly-G, and poly-T tails are trimmed.\n");
                fprintf(stderr, "     *  M  (the length of the poly tail; all poly-tails equal or longer than M will be trimmed).\n\n");
                return 1;
            } else {
                usage = 17;
            }
        } else if (strcmp(argv[1],"compress-id") == 0) {
            if (!(argc == 6 || argc == 5 || argc == 4)) {
                fprintf(stderr, "\n");
                fprintf(stderr, "Usage:   fastqtk  compress-id  [@|@@|/1|_1|/2|_2|/12|_12]  [N|counts.txt]  <in.fq>  <out.fq>\n\n");
                fprintf(stderr, "It does lossy compression on the reads ids from a FASTQ file.\n");
                fprintf(stderr, "For redirecting to STDOUT/STDIN use - instead of file name.\n\n");
                fprintf(stderr, "Options:\n");
                fprintf(stderr, "     *  N|counts.txt  (count reads; if here is a dot then it is considered that a file has been given):\n");
                fprintf(stderr, "               N             - number of reads in the input FASTQ file (or a much larger number). [500000].\n");
                fprintf(stderr, "               counts.txt    - file that contains number of reads in the input FASTQ file <in.fq>, that was generated\n");
                fprintf(stderr, "                               using 'fastqtk count in.fq counts.txt' beforehand.\n");
                fprintf(stderr, "     *  @|@@|/1|_1|/2|_2|/12|_12  (settings for generating reads ids):\n");
                fprintf(stderr, "               @             - does NOT add /1 or /2 to reads ids. (FASTQ file is NOT interleaved) [DEFAULT]\n");
                fprintf(stderr, "               @@            - does NOT add /1 or /2 to reads ids. (FASTQ file is INTERLEAVED)\n\n");
                fprintf(stderr, "               /1            - adds /1 to the end of the reads ids. (FASTQ file is NOT interleaved)\n");
                fprintf(stderr, "               /2            - adds /2 to the end of the reads ids. (FASTQ file is NOT interleaved)\n");
                fprintf(stderr, "               /12           - adds /1 and /2 to the end of the reads ids. (FASTQ file is INTERLEAVED\n");
                fprintf(stderr, "               _1            - adds _1 to the end of the reads ids. (FASTQ file is NOT interleaved)\n");
                fprintf(stderr, "               _2            - adds _2 to the end of the reads ids. (FASTQ file is NOT interleaved)\n");
                fprintf(stderr, "               _12           - adds _1 and _2 to the end of the reads ids. (FASTQ file is INTERLEAVED)\n\n");
                return 1;
            } else {
                usage = 19;
            }
        } else if (strcmp(argv[1],"drop-se") == 0) {
            if (argc != 4) {
                fprintf(stderr, "\n");
                fprintf(stderr, "Usage:   fastqtk  drop-se  <in.fq>  <out.fq>\n\n");

                fprintf(stderr, "It drops the unparied reads from an interleaved FASTQ file.\n");
                fprintf(stderr, "For redirecting to STDOUT/STDIN use - instead of file name.\n\n");
                return 1;
            } else {
                usage = 20;
            }
        } else if (strcmp(argv[1],"fq2fa") == 0) {
            if (argc != 4) {
                fprintf(stderr, "\n");
                fprintf(stderr, "Usage:   fastqtk  fq2fq  <in.fq>  <out.fa>\n\n");

                fprintf(stderr, "It a FASTQ file to FASTA file.\n");
                fprintf(stderr, "For redirecting to STDOUT/STDIN use - instead of file name.\n\n");
                return 1;
            } else {
                usage = 21;
            }
        } else if (strcmp(argv[1],"fa2fq") == 0) {
            if (argc != 4) {
                fprintf(stderr, "\n");
                fprintf(stderr, "Usage:   fastqtk  fa2fq  <in.fa>  <out.fq>\n\n");

                fprintf(stderr, "It a FASTA (generated using 'fastqtk fq2fa') file to FASTQ file.\n");
                fprintf(stderr, "For redirecting to STDOUT/STDIN use - instead of file name.\n\n");
                return 1;
            } else {
                usage = 22;
            }
        } else if (strcmp(argv[1],"rev-com") == 0) {
            if (argc != 4) {
                fprintf(stderr, "\n");
                fprintf(stderr, "Usage:   fastqtk  rev-com  <in.fq>  <out.fq>\n\n");

                fprintf(stderr, "It reverse complements all the reads from a FASTQ file.\n");
                fprintf(stderr, "For redirecting to STDOUT/STDIN use - instead of file name.\n\n");
                return 1;
            } else {
                usage = 23;
            }
        }
    }
    if (usage == 1) {
        fprintf(stderr, "\n");
        fprintf(stderr, "Usage:   fastqtk  <command>  <arguments>\n");
        fprintf(stderr, "Version: 0.28\n\n");
        fprintf(stderr, "Command:\n");
        fprintf(stderr, "      interleave       interleaves two paired-end FASTQ files.\n");
        fprintf(stderr, "      deinterleave     splits an (already) interleaved (paired-end) FASTQ file.\n");
        fprintf(stderr, "      count            counts all reads from a FASTQ file.\n");
        fprintf(stderr, "      lengths          summary statistics for lengths of reads from a FASTQ file.\n");
        fprintf(stderr, "      count-lengths    number of reads and summary statistics for lengths of reads from a FASTQ file.\n");
        fprintf(stderr, "      tab-4            converts a FASTQ file to a text tab-delimited file with 4 columns.\n");
        fprintf(stderr, "      tab-8            converts a (interleaved paired-end) FASTQ file to text tab-delimited file with 8 columns.\n");
        fprintf(stderr, "      detab            converts a text tab-delimited file with 4/8 columns (converted using tab4/tab8) to FASTQ file.\n");
        fprintf(stderr, "      retain-5         retains the first N bp from 5'end of the reads from a FASTQ file.\n");
        fprintf(stderr, "      retain-3         retains the last N bp from 3'end of the reads from a FASTQ file.\n");
        fprintf(stderr, "      trim-5           trims 5' end of the reads from a FASTQ file.\n");
        fprintf(stderr, "      trim-3           trims 3' end of the reads from a FASTQ file.\n");
        fprintf(stderr, "      trim-id          trims reads ids (removes everything after first space) from a FASTQ file.\n");
        fprintf(stderr, "      trim-poly        trims poly-A/C/G/T/N tails at both ends of the reads sequences from a FASTQ file.\n");
        fprintf(stderr, "      drop-se          drops unpaired reads from an interleaved paired-end FASTQ file.\n");
        fprintf(stderr, "      drop-short       drops reads that have short sequences (below a given threshold).\n");
        fprintf(stderr, "      fq2fa            converts a FASTQ file to FASTA file.\n");
        fprintf(stderr, "      fa2fq            converts a FASTA file to FASTQ file.\n");
        fprintf(stderr, "      compress-id      lossy compression of the reads ids from a FASTQ file.\n");
        fprintf(stderr, "      NtoA             replaces all Ns in reads sequences with As in a FASTQ file.\n");
        fprintf(stderr, "      rev-com          reverse complements all reads in a FASTQ file.\n");


        fprintf(stderr, "\n");
        return 1;
    }

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
    /*
    DEINTERLEAVE
    */
    if (usage == 2) {
        is_stdin = 0;
        if (strcmp(argv[2],"-")==0) {
            fip = stdin;
            is_stdin = 1;
        } else {
            fip = myfopen(argv[2],"r");
        }
        
        fr1 = myfopen(argv[3],"w");
        
        fr2 = myfopen(argv[4],"w");

        buffer = mymalloc(BUFFER_SIZE * SIZE_OF_CHAR + 1);
        
        b1 = mymalloc(BUFFER_SIZE * SIZE_OF_CHAR + 1);

        b2 = mymalloc(BUFFER_SIZE * SIZE_OF_CHAR + 1);

        
        i = 0;
        j = 0;
        k = 0;

        r1 = 0;
        b1i = 0;
        b2i = 0;
        // for(;;)


        while(1) {

            bytes_read = fread(buffer, SIZE_OF_CHAR, BUFFER_SIZE, fip);

            if (ferror(fip)) {
                    fprintf(stderr, "ERROR: Failed reading the input file.\n");
                    return 2;
            }

            if (bytes_read == 0 && feof(fip)) {
                    break;
            }


            if (feof(fip)) {
                if(buffer[bytes_read-1] != '\n') {
                    bytes_read = bytes_read + 1;
                    buffer[bytes_read-1] = '\n';
                } else if (bytes_read > 1 && buffer[bytes_read-2] == '\n' && buffer[bytes_read-1] == '\n') {
                    bytes_read = bytes_read - 1;
                }
            }


            flag = 0;
            for (i=0;i<bytes_read;i++) {
                flag = 0;
                if (buffer[i] == '\n') {
                    j = j + 1;
                    if (j==4) {
                        if (r1==0) {
                            l = i - k + 1;
                            memcpy(b1+b1i,buffer+k,l);
                            b1i = b1i + l;
                            r1 = 1;
                        } else {
                            l = i - k + 1;
                            memcpy(b2+b2i,buffer+k,l);
                            b2i = b2i + l;
                            r1 = 0;
                        }
                        j = 0;
                        k = i + 1;
                    }
                }
            }

            if (flag == 0) {
                i = bytes_read - 1;
                if(r1 == 0){
                    l = i - k + 1;
                    memcpy(b1+b1i,buffer+k,l);
                    b1i = b1i + l;
                } else {
                    l = i - k + 1;
                    memcpy(b2+b2i,buffer+k,l);
                    b2i = b2i + l;
                }
            }
            

            fwrite(b1, SIZE_OF_CHAR, b1i, fr1);
            fwrite(b2, SIZE_OF_CHAR, b2i, fr2);
            
            b1i = 0;
            b2i = 0;
            k = 0;
        }
        
        
        if (is_stdin == 0) {
            fclose(fip);
        }


        fclose(fr1);
        fclose(fr2);

        free(buffer);
        free(b1);
        free(b2);

        return 0;
    } // end DEINTERLEAVE

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
    /*
    INTERLEAVE
    */
    if (usage == 3) {
        is_stdout = 0;
        if (strcmp(argv[4],"-")==0) {
            fop = stdout;
            is_stdout = 1;
        } else {
            fop = myfopen(argv[4],"w");
        }

        fi1p = myfopen(argv[2],"r");

        fi2p = myfopen(argv[3],"r");


        buffer = mymalloc(BUFFER_SIZE * SIZE_OF_CHAR + 1);
        
        b1 = mymalloc(BUFFER_SIZE * SIZE_OF_CHAR + 1);

        b2 = mymalloc(BUFFER_SIZE * SIZE_OF_CHAR + 1);
        
        i = 0;
        j = 0;
        j1 = 0;
        j2 = 0;
        k = 0;
        k1 = 0;
        k1old = 0;
        k2 = 0;
        k2old = 0;

        r1 = 0;
        b1i = 0;
        b2i = 0;
        
        bytes_read1 = 0;
        bytes_read2 = 0;
        
        end1 = 0;
        end2 = 0;
        
        while(1) {
            
            if (end1 == 1 && end2 == 1) {
                break;
            }
            ////////////////////////////////////////
            if (r1 == 0) {
                if (k1 == 0) {
                    bytes_read1 = fread(b1, SIZE_OF_CHAR, BUFFER_SIZE, fi1p);
                    if (ferror(fi1p)) {
                        fprintf(stderr, "ERROR: Failed reading the input file.\n");
                        return 2;
                    }
                    if (bytes_read1 == 0 && feof(fi1p)) {
                            end1 = 1;
                            r1 = 1;
                            continue;
                    }
                    if (feof(fi1p)) {
                        if(b1[bytes_read1-1] != '\n') {
                            bytes_read1 = bytes_read1 + 1;
                            b1[bytes_read1-1] = '\n';
                        } else if (bytes_read1 > 1 && b1[bytes_read1-2] == '\n' && b1[bytes_read1-1] == '\n') {
                            bytes_read1 = bytes_read1 - 1;
                        }
                    }
                }

                flag = 0;
                for (i=k1;i<bytes_read1;i++) {
                    flag = 0;
                    if (b1[i] == '\n') {
                        j1 = j1 + 1;
                        if (j1==4) {
                            flag = 1;
                            j1 = 0;
                            k1old = k1;
                            k1 = i + 1;
                            break;
                        }
                    }
                }

                if (flag == 0) {
                    k1old = k1;
                    k1 = bytes_read1;
                    r1 = 0;
                    // read more
                } else {
                    r1 = 1;
                }


                l = k1 - k1old;
                if(BUFFER_SIZE < k + l + 10) { 
                    // empty the buffer
                    fwrite(buffer, SIZE_OF_CHAR, k, fop);
                    k = 0;
                }
                memcpy(buffer+k,b1+k1old,l);
                k = k + l;

                if (flag == 0 || k1 == bytes_read1) {
                    k1 = 0;
                    k1old = 0;
                }

            ////////////////////////////////////////////////////////////////
            } else {
                if (k2 == 0) {
                    bytes_read2 = fread(b2, SIZE_OF_CHAR, BUFFER_SIZE, fi2p);
                    if (ferror(fi2p)) {
                        fprintf(stderr, "ERROR: Failed reading the input file.\n");
                        return 2;
                    }
                    if (bytes_read2 == 0 && feof(fi2p)) {
                            end2 = 1;
                            r1 = 0;
                            continue;
                    }
                    if (feof(fi2p)) {
                        if(b2[bytes_read2-1] != '\n') {
                            bytes_read2 = bytes_read2 + 1;
                            b2[bytes_read2-1] = '\n';
                        } else if (bytes_read2 > 1 && b2[bytes_read2-2] == '\n' && b2[bytes_read2-1] == '\n') {
                            bytes_read2 = bytes_read2 - 1;
                        }
                    }
                }

                flag = 0;
                for (i=k2;i<bytes_read2;i++) {
                    flag = 0;
                    if (b2[i] == '\n') {
                        j2 = j2 + 1;
                        if (j2==4) {
                            flag = 1;
                            j2 = 0;
                            k2old = k2;
                            k2 = i + 1;
                            break;
                        }
                    }
                }

                if (flag == 0) {
                    k2old = k2;
                    k2 = bytes_read2;
                    r1 = 1;
                    // read more
                } else {
                    r1 = 0;
                }


                l = k2 - k2old;
                if(BUFFER_SIZE < k + l + 10) {
                    fwrite(buffer, SIZE_OF_CHAR, k, fop);
                    k = 0;
                }
                memcpy(buffer+k,b2+k2old,l);
                k = k + l;

                if (flag == 0 || k2 == bytes_read2) {
                    k2 = 0;
                    k2old = 0;
                }

            }


        }


        if (k!=0) {
            fwrite(buffer, SIZE_OF_CHAR, k, fop);
        }

        if (is_stdout==0) {
            fclose(fop);
        }


        fclose(fi1p);
        fclose(fi2p);

        free(buffer);
        free(b1);
        free(b2);
        return 0;
    } // end INTERLEAVE

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
    /*
    COUNT
    */
    if (usage == 4) {
        is_stdin = 0;
        if (strcmp(argv[2],"-")==0) {
            fip = stdin;
            is_stdin = 1;
        } else {
            fip = myfopen(argv[2],"r");
        }
        is_stdout = 0;
        if (strcmp(argv[3],"-")==0) {
            fop = stdout;
            is_stdout = 1;
        } else {
            fop = myfopen(argv[3],"w");
        }
        

        buffer = mymalloc(BUFFER_SIZE * SIZE_OF_CHAR + 1);
        

        u = 0;
        

        // for(;;)


        while(1) {

            bytes_read = fread(buffer, SIZE_OF_CHAR, BUFFER_SIZE, fip);

            if (ferror(fip)) {
                    fprintf(stderr, "ERROR: Failed reading the input file.\n");
                    return 2;
            }
            if (bytes_read == 0 && feof(fip)) {
                    break;
            }
            
            if (feof(fip)) {
                if(buffer[bytes_read-1] != '\n') {
                    bytes_read = bytes_read + 1;
                    buffer[bytes_read-1] = '\n';
                } else if (bytes_read > 1 && buffer[bytes_read-2] == '\n' && buffer[bytes_read-1] == '\n') {
                    bytes_read = bytes_read - 1;
                }
            }

            for (i=0;i<bytes_read;i++) {
                if (buffer[i] == '\n') {
                    u = u + 1;
                }
            }
        }

        if ( u%4 == 0) {
            u = u/4;
        } else {
            if (u>2) {
                u = u/4 + 1;
            } else {
                u = 0;
            }
        }
        
        fprintf(fop,"%lld\n",u);
        
        if (is_stdin == 0) {
            fclose(fip);
        }
        if (is_stdout == 0) {
            fclose(fop);
        }

        free(buffer);
        return 0;
    } // end COUNT

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
    /*
    LENGTHS
    */
    if (usage == 5) {
    
        
        is_stdin = 0;
        if (strcmp(argv[2],"-")==0) {
            fip = stdin;
            is_stdin = 1;
        } else {
            fip = myfopen(argv[2],"r");
        }
        is_stdout = 0;
        if (strcmp(argv[3],"-")==0) {
            fop = stdout;
            is_stdout = 1;
        } else {
            fop = myfopen(argv[3],"w");
        }
        

        buffer = mymalloc(BUFFER_SIZE * SIZE_OF_CHAR + 1);

        for (i=0;i<MAX_LEN;i++) {
            len[i] = 0;
        }

        j = 2;
        k = 0;
        l = 0;
        windows = 0;

        // for(;;)


        while(1) {

            bytes_read = fread(buffer, SIZE_OF_CHAR, BUFFER_SIZE, fip);

            if (ferror(fip)) {
                    fprintf(stderr, "ERROR: Failed reading the input file.\n");
                    return 2;
            }
            if (bytes_read == 0 && feof(fip)) {
                    break;
            }
            

            flag = 0;
            for (i=0;i<bytes_read;i++) {
                if (buffer[i] == '\n') {
                    j = j + 1;
                    if (j==3) {
                        k = i;
                        flag = 1;
                    } else if (j==4) {
                        l = l + i - k - 1;
                        if (l < MAX_LEN && l - 1 > 0) {
                            len[l] = 1;
                        } else {
                            fprintf(stderr, "ERROR: Too long read sequence found. Maximum length of supported reads is %d.\n",MAX_LEN);
                            free(buffer);
                            return 2;
                        }
                        j = 0;
                        l = 0;
                        flag = 0;
                    }
                }
            }
            if (flag == 1) {
                flag = 0;
                i = bytes_read;
                l = i - k - 1;
                k = 0;
            }
            if (k != 0 && k != 1 && buffer[k-1] == '\r') {
                windows = 1;
            }
            
        }

        for (i=MAX_LEN-1;i>0;i--) {
            if (len[i] != 0) {
                fprintf(fop,"%ld\n",i-windows);
            }
        }
        if (len[0] != 0 && windows == 0) {
            fprintf(fop,"0\n");
        }
        
        if (is_stdin == 0) {
            fclose(fip);
        }
        if (is_stdout == 0) {
            fclose(fop);
        }

        free(buffer);
        return 0;
    } // end LENGTHS

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
    /*
    COUNT-LENGTHS
    */
    if (usage == 6) {
        is_stdin = 0;
        if (strcmp(argv[2],"-")==0) {
            fip = stdin;
            is_stdin = 1;
        } else {
            fip = myfopen(argv[2],"r");
        }

        fr1 = myfopen(argv[3],"w");
        
        fr2 = myfopen(argv[4],"w");

        buffer = mymalloc(BUFFER_SIZE * SIZE_OF_CHAR + 1);

        for (i=0;i<MAX_LEN;i++) {
            len[i] = 0;
        }

        u = 0;
        j = 2;
        k = 0;
        l = 0;
        windows = 0;

        // for(;;)


        while(1) {

            bytes_read = fread(buffer, SIZE_OF_CHAR, BUFFER_SIZE, fip);

            if (ferror(fip)) {
                    fprintf(stderr, "ERROR: Failed reading the input file.\n");
                    return 2;
            }
            if (bytes_read == 0 && feof(fip)) {
                    break;
            }
            

            flag = 0;
            for (i=0;i<bytes_read;i++) {
                if (buffer[i] == '\n') {
                    j = j + 1;
                    u = u + 1;
                    if (j==3) {
                        k = i;
                        flag = 1;
                    } else if (j==4) {
                        l = l + i - k - 1;
                        if (l<MAX_LEN) {
                            len[l] = 1;
                        } else {
                            fprintf(stderr, "ERROR: Too long read sequence found. Maximum length of supported reads is %d.\n",MAX_LEN);
                            free(buffer);
                            return 2;
                        }
                        j = 0;
                        l = 0;
                        flag = 0;
                    }
                }
            }
            
            if (flag == 1) {
                flag = 0;
                i = bytes_read - 1;
                l = i - k - 1;
                k = 0;
            }
            if (k != 0 && k != 1 && buffer[k-1] == '\r') {
                windows = 1;
            }
        }

        if ( u%4 == 0 ) {
            u = u/4;
        } else {
            if (u>2) {
                u = u/4 + 1;
            } else {
                u = 0;
            }
        }
        fprintf(fr1,"%lld\n",u);
        
        for (i=MAX_LEN-1;i>0;i--) {
            if (len[i] != 0) {
                fprintf(fr2,"%ld\n",i-windows);
            }
        }
        if (len[0] != 0 && windows == 0) {
            fprintf(fr2,"0\n");
        }

        if (is_stdin == 0) {
            fclose(fip);
        }

        fclose(fr1);
        fclose(fr2);

        free(buffer);
        return 0;
    } // end COUNT-LENGTHS

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

    /*
    TAB4 & TAB8
    */
    if (usage == 7 || usage == 8) {
        is_stdin = 0;
        if (strcmp(argv[2],"-")==0) {
            fip = stdin;
            is_stdin = 1;
        } else {
            fip = myfopen(argv[2],"r");
        }
        is_stdout = 0;
        if (strcmp(argv[3],"-")==0) {
            fop = stdout;
            is_stdout = 1;
        } else {
            fop = myfopen(argv[3],"w");
        }
        
        r1 = 4;
        if (usage == 8) {
            r1 = 8;
        }

        buffer = mymalloc(BUFFER_SIZE * SIZE_OF_CHAR + 1);
        

        

        // for(;;)
        j = 0;


        while(1) {

            bytes_read = fread(buffer, SIZE_OF_CHAR, BUFFER_SIZE, fip);

            if (ferror(fip)) {
                    fprintf(stderr, "ERROR: Failed reading the input file.\n");
                    return 2;
            }
            if (bytes_read == 0 && feof(fip)) {
                    break;
            }

            
            if (feof(fip)) {
                if(buffer[bytes_read-1] != '\n') {
                    bytes_read = bytes_read + 1;
                    buffer[bytes_read-1] = '\n';
                } else if (bytes_read > 1 && buffer[bytes_read-2] == '\n' && buffer[bytes_read-1] == '\n') {
                    bytes_read = bytes_read - 1;
                }
            }

            for (i=0;i<bytes_read;i++) {
                if (buffer[i] == '\n') {
                    j = j + 1;
                    if (j==r1) {
                        j = 0;
                    } else {
                        buffer[i] = '\t';
                    }
                }
            }
            
            fwrite(buffer, SIZE_OF_CHAR, bytes_read, fop);
        }


        
        if (is_stdin == 0) {
            fclose(fip);
        }
        if (is_stdout == 0) {
            fclose(fop);
        }

        free(buffer);
        return 0;
    } // end TAB4 & TAB8

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

    /*
    DETAB
    */
    if (usage == 9) {
        is_stdin = 0;
        if (strcmp(argv[2],"-")==0) {
            fip = stdin;
            is_stdin = 1;
        } else {
            fip = myfopen(argv[2],"r");
        }
        is_stdout = 0;
        if (strcmp(argv[3],"-")==0) {
            fop = stdout;
            is_stdout = 1;
        } else {
            fop = myfopen(argv[3],"w");
        }
        
        r1 = 4;
        if (usage == 8) {
            r1 = 8;
        }

        buffer = mymalloc(BUFFER_SIZE * SIZE_OF_CHAR + 1);
        

        

        // for(;;)
        j = 0;


        while(1) {

            bytes_read = fread(buffer, SIZE_OF_CHAR, BUFFER_SIZE, fip);

            if (ferror(fip)) {
                    fprintf(stderr, "ERROR: Failed reading the input file.\n");
                    return 2;
            }
            if (bytes_read == 0 && feof(fip)) {
                    break;
            }

            
            if (feof(fip)) {
                if(buffer[bytes_read-1] != '\n') {
                    bytes_read = bytes_read + 1;
                    buffer[bytes_read-1] = '\n';
                } else if (bytes_read > 1 && buffer[bytes_read-2] == '\n' && buffer[bytes_read-1] == '\n') {
                    bytes_read = bytes_read - 1;
                }
            }

            for (i=0;i<bytes_read;i++) {
                if (buffer[i] == '\t') {
                    buffer[i] = '\n';
                }
            }
            fwrite(buffer, SIZE_OF_CHAR, bytes_read, fop);
        }


        
        if (is_stdin == 0) {
            fclose(fip);
        }
        if (is_stdout == 0) {
            fclose(fop);
        }

        free(buffer);
        return 0;
    } // end DETAB


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
    /*
    DROP-SHORT
    */
    if (usage == 10) {
        threshold = atoi(argv[2]);
        is_stdin = 0;
        if (strcmp(argv[3],"-")==0) {
            fip = stdin;
            is_stdin = 1;
        } else {
            fip = myfopen(argv[3],"r");
        }
        is_stdout = 0;
        if (strcmp(argv[4],"-")==0) {
            fop = stdout;
            is_stdout = 1;
        } else {
            fop = myfopen(argv[4],"w");
        }
        

        buffer = mymalloc(BUFFER_SIZE * SIZE_OF_CHAR + 1);
        b1 = mymalloc(BUFFER_SIZE * SIZE_OF_CHAR + 1);

        i = 0;
        j = 0;


        k = 0;

        j1 = 0;
        j2 = 0;
        
        k1 = 0;
        k2 = 0;
        
        b1i = 0;

        while(1) {

            bytes_read = fread(buffer + k, SIZE_OF_CHAR, BUFFER_SIZE - k, fip);
            bytes_read = k + bytes_read;
            k = 0;
            j2 = 0;

            if (ferror(fip)) {
                    fprintf(stderr, "ERROR: Failed reading the input file.\n");
                    return 2;
            }
            if (bytes_read == 0) { // end of file?
                if (feof(fip)) {
                    break;
                }
            }
            
            if (feof(fip)) {
                if(buffer[bytes_read-1] != '\n') {
                    bytes_read = bytes_read + 1;
                    buffer[bytes_read-1] = '\n';
                } else if (bytes_read > 1 && buffer[bytes_read-2] == '\n' && buffer[bytes_read-1] == '\n') {
                    bytes_read = bytes_read - 1;
                }
            }

            flag = 0;
            for (i=0;i<bytes_read;i++) {
                flag = 0;
                if (buffer[i] == '\n') {
                    j = j + 1;
                    if (j==1) {
                        k1 = i + 1;
                    } else if (j==2) {
                        k2 = i;
                    } else if (j==4) {
                        if (k2-k1>=threshold) {
                            l = i - j2 + 1;
                            if (b1i + l > BUFFER_SIZE - 10) {
                                fwrite(b1, SIZE_OF_CHAR, b1i, fop);
                                b1i = 0;
                            }
                            memcpy(b1+b1i,buffer+j2,l);
                            b1i = b1i + l;
                        }
                        // (j2,i) = entire read - id, seq, +, qual
                        flag = 1;
                        j = 0;
                        j2 = i + 1;
                    }
                }
            }
            
            if (flag == 0) {
                k = bytes_read - j2;
                memcpy(buffer,buffer+j2,k);
                j = 0;
                j2 = 0;
            }
            
            


        } // while


        if (b1i != 0) {
            fwrite(b1, SIZE_OF_CHAR, b1i, fop);
        }

        
        if (is_stdin == 0) {
            fclose(fip);
        }
        if (is_stdout == 0) {
            fclose(fop);
        }

        free(buffer);
        free(b1);
        
        return 0;
    } // end DROP-SHORT

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
    /*
    NtoA
    */
    if (usage == 11) {

        is_stdin = 0;
        if (strcmp(argv[2],"-")==0) {
            fip = stdin;
            is_stdin = 1;
        } else {
            fip = myfopen(argv[2],"r");
        }
        is_stdout = 0;
        if (strcmp(argv[3],"-")==0) {
            fop = stdout;
            is_stdout = 1;
        } else {
            fop = myfopen(argv[3],"w");
        }
        

        buffer = mymalloc(BUFFER_SIZE * SIZE_OF_CHAR + 1);


        i = 0;
        j = 0;
        l = 0;


        k = 0;

        flag = 0;

        while(1) {

            bytes_read = fread(buffer, SIZE_OF_CHAR, BUFFER_SIZE, fip);

            if (ferror(fip)) {
                    fprintf(stderr, "ERROR: Failed reading the input file.\n");
                    return 2;
            }
            if (bytes_read == 0 && feof(fip)) {
                    break;
            }

            
            if (feof(fip)) {
                if(buffer[bytes_read-1] != '\n') {
                    bytes_read = bytes_read + 1;
                    buffer[bytes_read-1] = '\n';
                } else if (bytes_read > 1 && buffer[bytes_read-2] == '\n' && buffer[bytes_read-1] == '\n') {
                    bytes_read = bytes_read - 1;
                }
            }


            for (i=0;i<bytes_read;i++) {
                if (buffer[i] == '\n') {
                    j = j + 1;
                    if (j==1) {
                        k = i + 1;
                        flag = 1;
                    } else if (j==2) {
                        flag = 0;
                        for (l=k;l<i;l++) {
                            if (buffer[l]=='N') {
                                buffer[l]='A';
                            }
                        }
                    } else if (j==4) {
                        j = 0;
                    }
                }
            }
            
            if (flag == 1) {
                flag = 0;
                i = bytes_read;
                for (l=k;l<i;l++) {
                    if (buffer[l]=='N') {
                        buffer[l]='A';
                    }
                }
                k = 0;
            }

            
            fwrite(buffer, SIZE_OF_CHAR, bytes_read, fop);


        } // while


        
        if (is_stdin == 0) {
            fclose(fip);
        }
        if (is_stdout == 0) {
            fclose(fop);
        }

        free(buffer);

        
        return 0;
    } // end NtoA


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
    /*
    TRIM5
    */
    if (usage == 12) {
        trim = atoi(argv[2]);
        is_stdin = 0;
        if (strcmp(argv[3],"-")==0) {
            fip = stdin;
            is_stdin = 1;
        } else {
            fip = myfopen(argv[3],"r");
        }
        is_stdout = 0;
        if (strcmp(argv[4],"-")==0) {
            fop = stdout;
            is_stdout = 1;
        } else {
            fop = myfopen(argv[4],"w");
        }
        

        buffer = mymalloc(BUFFER_SIZE * SIZE_OF_CHAR + 1);
        b1 = mymalloc(BUFFER_SIZE * SIZE_OF_CHAR + 1);

        i = 0;
        j = 0;


        k = 0;

        j1 = 0;
        j2 = 0;

        
        k1 = 0;
        k2 = 0;
        
        b1i = 0;

        while(1) {

            bytes_read = fread(buffer + k, SIZE_OF_CHAR, BUFFER_SIZE - k, fip);
            bytes_read = k + bytes_read;
            k = 0;
            j2 = 0;

            if (ferror(fip)) {
                    fprintf(stderr, "ERROR: Failed reading the input file.\n");
                    return 2;
            }
            if (bytes_read == 0 && feof(fip)) {
                    break;
            }

            
            if (feof(fip)) {
                if(buffer[bytes_read-1] != '\n') {
                    bytes_read = bytes_read + 1;
                    buffer[bytes_read-1] = '\n';
                } else if (bytes_read > 1 && buffer[bytes_read-2] == '\n' && buffer[bytes_read-1] == '\n') {
                    bytes_read = bytes_read - 1;
                }
            }

            flag = 0;
            for (i=0;i<bytes_read;i++) {
                flag = 0;
                if (buffer[i] == '\n') {
                    j = j + 1;
                    if (j==1) {
                        k1 = i + 1;
                    } else if (j==2) {
                        k2 = i + 1;
                    } else if (j==3) {
                        j1 = i + 1;
                    } else if (j==4) {
                        // j2 = id start
                        // k1 = seq start
                        // k2= plus start
                        // j1 = qual start
                        // i = end of qual
                        // (j2,i) = entire read - id, seq, +, qual

                        l = i - j2 + 1;
                        if (b1i + l > BUFFER_SIZE - 10) {
                            fwrite(b1, SIZE_OF_CHAR, b1i, fop);
                            b1i = 0;
                        }
                        //copy id
                        l = k1 - j2;
                        memcpy(b1+b1i,buffer+j2,l);
                        b1i = b1i + l;

                        //copy seq
                        l = k2 - k1;
                        if (l-1>trim) {
                            k1 = k1 + trim; 
                            l = k2 - k1;
                            memcpy(b1+b1i,buffer+k1,l);
                            b1i = b1i + l;
                        } else {
                            b1[b1i] = 'A';
                            b1[b1i+1] = '\n';
                            b1i = b1i + 2;
                        }

                        //copy +
                        b1[b1i] = '+';
                        b1[b1i+1] = '\n';
                        b1i = b1i + 2;

                        //copy qual
                        l = i - j1 + 1;
                        if (l-1>trim) {
                            j1 = j1 + trim;
                            l = i - j1 + 1;
                            memcpy(b1+b1i,buffer+j1,l);
                            b1i = b1i + l;
                        } else {
                            b1[b1i] = 'I';
                            b1[b1i+1] = '\n';
                            b1i = b1i + 2;
                        }



                        flag = 1;
                        j = 0;
                        j2 = i + 1;
                    }
                }
            }
            
            if (flag == 0) {
                k = bytes_read - j2;
                memcpy(buffer,buffer+j2,k);
                j = 0;
                j2 = 0;
                j1 = 0;
                k1= 0;
                k2 = 0;
            }
            
            


        } // while


        if (b1i != 0) {
            fwrite(b1, SIZE_OF_CHAR, b1i, fop);
        }

        
        if (is_stdin == 0) {
            fclose(fip);
        }
        if (is_stdout == 0) {
            fclose(fop);
        }

        free(buffer);
        free(b1);
        
        return 0;
    } // end TRIM5

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
    /*
    TRIM3
    */
    if (usage == 13) {
        trim = atoi(argv[2]);
        is_stdin = 0;
        if (strcmp(argv[3],"-")==0) {
            fip = stdin;
            is_stdin = 1;
        } else {
            fip = myfopen(argv[3],"r");
        }
        is_stdout = 0;
        if (strcmp(argv[4],"-")==0) {
            fop = stdout;
            is_stdout = 1;
        } else {
            fop = myfopen(argv[4],"w");

        }
        

        buffer = mymalloc(BUFFER_SIZE * SIZE_OF_CHAR + 1);
        b1 = mymalloc(BUFFER_SIZE * SIZE_OF_CHAR + 1);

        i = 0;
        j = 0;


        k = 0;

        j1 = 0;
        j2 = 0;

        
        k1 = 0;
        k2 = 0;
        
        b1i = 0;

        while(1) {

            bytes_read = fread(buffer + k, SIZE_OF_CHAR, BUFFER_SIZE - k, fip);
            bytes_read = k + bytes_read;
            k = 0;
            j2 = 0;

            if (ferror(fip)) {
                    fprintf(stderr, "ERROR: Failed reading the input file.\n");
                    return 2;
            }
            if (bytes_read == 0 && feof(fip)) {
                    break;
            }

            
            if (feof(fip)) {
                if(buffer[bytes_read-1] != '\n') {
                    bytes_read = bytes_read + 1;
                    buffer[bytes_read-1] = '\n';
                } else if (bytes_read > 1 && buffer[bytes_read-2] == '\n' && buffer[bytes_read-1] == '\n') {
                    bytes_read = bytes_read - 1;
                }
            }

            flag = 0;
            for (i=0;i<bytes_read;i++) {
                flag = 0;
                if (buffer[i] == '\n') {
                    j = j + 1;
                    if (j==1) {
                        k1 = i + 1;
                    } else if (j==2) {
                        k2 = i + 1;
                    } else if (j==3) {
                        j1 = i + 1;
                    } else if (j==4) {
                        // j2 = id start
                        // k1 = seq start
                        // k2= plus start
                        // j1 = qual start
                        // i = end of qual
                        // (j2,i) = entire read - id, seq, +, qual

                        l = i - j2 + 1;
                        if (b1i + l > BUFFER_SIZE - 10) {
                            fwrite(b1, SIZE_OF_CHAR, b1i, fop);
                            b1i = 0;
                        }
                        //copy id
                        l = k1 - j2;
                        memcpy(b1+b1i,buffer+j2,l);
                        b1i = b1i + l;

                        //copy seq
                        l = k2 - k1;
                        if (l-1>trim) {
                            k2 = k2 - trim - 1; 
                            l = k2 - k1;
                            memcpy(b1+b1i,buffer+k1,l);
                            b1i = b1i + l;
                            b1[b1i] = '\n';
                            b1i = b1i + 1;
                        } else {
                            b1[b1i] = 'A';
                            b1[b1i+1] = '\n';
                            b1i = b1i + 2;
                        }

                        //copy +
                        b1[b1i] = '+';
                        b1[b1i+1] = '\n';
                        b1i = b1i + 2;

                        //copy qual
                        l = i - j1 + 1;
                        if (l-1>trim) {
                            l = i - j1 - trim;
                            memcpy(b1+b1i,buffer+j1,l);
                            b1i = b1i + l;
                            b1[b1i] = '\n';
                            b1i = b1i + 1;
                        } else {
                            b1[b1i] = 'I';
                            b1[b1i+1] = '\n';
                            b1i = b1i + 2;
                        }



                        flag = 1;
                        j = 0;
                        j2 = i + 1;
                    }
                }
            }
            
            if (flag == 0) {
                k = bytes_read - j2;
                memcpy(buffer,buffer+j2,k);
                j = 0;
                j2 = 0;
                j1 = 0;
                k1= 0;
                k2 = 0;
            }
            
            


        } // while


        if (b1i != 0) {
            fwrite(b1, SIZE_OF_CHAR, b1i, fop);
        }

        
        if (is_stdin == 0) {
            fclose(fip);
        }
        if (is_stdout == 0) {
            fclose(fop);
        }

        free(buffer);
        free(b1);
        
        return 0;
    } // end TRIM5

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
    /*
    RETAIN5
    */
    if (usage == 14) {
        retain = atoi(argv[2]);
        is_stdin = 0;
        if (strcmp(argv[3],"-")==0) {
            fip = stdin;
            is_stdin = 1;
        } else {
            fip = myfopen(argv[3],"r");
        }
        is_stdout = 0;
        if (strcmp(argv[4],"-")==0) {
            fop = stdout;
            is_stdout = 1;
        } else {
            fop = myfopen(argv[4],"w");
        }
        

        buffer = mymalloc(BUFFER_SIZE * SIZE_OF_CHAR + 1);
        b1 = mymalloc(BUFFER_SIZE * SIZE_OF_CHAR + 1);

        i = 0;
        j = 0;


        k = 0;

        j1 = 0;
        j2 = 0;

        
        k1 = 0;
        k2 = 0;
        
        b1i = 0;

        while(1) {

            bytes_read = fread(buffer + k, SIZE_OF_CHAR, BUFFER_SIZE - k, fip);
            bytes_read = k + bytes_read;
            k = 0;
            j2 = 0;

            if (ferror(fip)) {
                    fprintf(stderr, "ERROR: Failed reading the input file.\n");
                    return 2;
            }
            if (bytes_read == 0 && feof(fip)) {
                    break;
            }

            
            if (feof(fip)) {
                if(buffer[bytes_read-1] != '\n') {
                    bytes_read = bytes_read + 1;
                    buffer[bytes_read-1] = '\n';
                } else if (bytes_read > 1 && buffer[bytes_read-2] == '\n' && buffer[bytes_read-1] == '\n') {
                    bytes_read = bytes_read - 1;
                }
            }

            flag = 0;
            for (i=0;i<bytes_read;i++) {
                flag = 0;
                if (buffer[i] == '\n') {
                    j = j + 1;
                    if (j==1) {
                        k1 = i + 1;
                    } else if (j==2) {
                        k2 = i + 1;
                    } else if (j==3) {
                        j1 = i + 1;
                    } else if (j==4) {
                        // j2 = id start
                        // k1 = seq start
                        // k2= plus start
                        // j1 = qual start
                        // i = end of qual
                        // (j2,i) = entire read - id, seq, +, qual

                        l = i - j2 + 1;
                        if (b1i + l > BUFFER_SIZE - 10) {
                            fwrite(b1, SIZE_OF_CHAR, b1i, fop);
                            b1i = 0;
                        }
                        //copy id
                        l = k1 - j2;
                        memcpy(b1+b1i,buffer+j2,l);
                        b1i = b1i + l;

                        //copy seq
                        l = k2 - k1;
                        if (l-1>retain) {
                            l = retain;
                            memcpy(b1+b1i,buffer+k1,l);
                            b1i = b1i + l;
                            b1[b1i] = '\n';
                            b1i = b1i + 1;
                        } else {
                            memcpy(b1+b1i,buffer+k1,l);
                            b1i = b1i + l;
                        }

                        //copy +
                        b1[b1i] = '+';
                        b1[b1i+1] = '\n';
                        b1i = b1i + 2;

                        //copy qual
                        l = i - j1 + 1;
                        if (l-1>retain) {
                            l = retain;
                            memcpy(b1+b1i,buffer+j1,l);
                            b1i = b1i + l;
                            b1[b1i] = '\n';
                            b1i = b1i + 1;
                        } else {
                            memcpy(b1+b1i,buffer+j1,l);
                            b1i = b1i + l;
                        }



                        flag = 1;
                        j = 0;
                        j2 = i + 1;
                    }
                }
            }
            
            if (flag == 0) {
                k = bytes_read - j2;
                memcpy(buffer,buffer+j2,k);
                j = 0;
                j2 = 0;
                j1 = 0;
                k1= 0;
                k2 = 0;
            }
            
            


        } // while


        if (b1i != 0) {
            fwrite(b1, SIZE_OF_CHAR, b1i, fop);
        }

        
        if (is_stdin == 0) {
            fclose(fip);
        }
        if (is_stdout == 0) {
            fclose(fop);
        }

        free(buffer);
        free(b1);
        
        return 0;
    } // end RETAIN5

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
    /*
    RETAIN3
    */
    if (usage == 15) {
        retain = atoi(argv[2]);
        is_stdin = 0;
        if (strcmp(argv[3],"-")==0) {
            fip = stdin;
            is_stdin = 1;
        } else {
            fip = myfopen(argv[3],"r");
        }
        is_stdout = 0;
        if (strcmp(argv[4],"-")==0) {
            fop = stdout;
            is_stdout = 1;
        } else {
            fop = myfopen(argv[4],"w");
        }
        

        buffer = mymalloc(BUFFER_SIZE * SIZE_OF_CHAR + 1);
        b1 = mymalloc(BUFFER_SIZE * SIZE_OF_CHAR + 1);

        i = 0;
        j = 0;


        k = 0;

        j1 = 0;
        j2 = 0;

        
        k1 = 0;
        k2 = 0;
        
        b1i = 0;

        while(1) {

            bytes_read = fread(buffer + k, SIZE_OF_CHAR, BUFFER_SIZE - k, fip);
            bytes_read = k + bytes_read;
            k = 0;
            j2 = 0;

            if (ferror(fip)) {
                    fprintf(stderr, "ERROR: Failed reading the input file.\n");
                    return 2;
            }
            if (bytes_read == 0 && feof(fip)) {
                    break;
            }

            
            if (feof(fip)) {
                if(buffer[bytes_read-1] != '\n') {
                    bytes_read = bytes_read + 1;
                    buffer[bytes_read-1] = '\n';
                } else if (bytes_read > 1 && buffer[bytes_read-2] == '\n' && buffer[bytes_read-1] == '\n') {
                    bytes_read = bytes_read - 1;
                }
            }

            flag = 0;
            for (i=0;i<bytes_read;i++) {
                flag = 0;
                if (buffer[i] == '\n') {
                    j = j + 1;
                    if (j==1) {
                        k1 = i + 1;
                    } else if (j==2) {
                        k2 = i + 1;
                    } else if (j==3) {
                        j1 = i + 1;
                    } else if (j==4) {
                        // j2 = id start
                        // k1 = seq start
                        // k2= plus start
                        // j1 = qual start
                        // i = end of qual
                        // (j2,i) = entire read - id, seq, +, qual

                        l = i - j2 + 1;
                        if (b1i + l > BUFFER_SIZE - 10) {
                            fwrite(b1, SIZE_OF_CHAR, b1i, fop);
                            b1i = 0;
                        }
                        //copy id
                        l = k1 - j2;
                        memcpy(b1+b1i,buffer+j2,l);
                        b1i = b1i + l;

                        //copy seq
                        l = k2 - k1;
                        if (l-1>retain) {
                            k1 = k1 + l - retain - 1;
                            l = retain + 1;
                            memcpy(b1+b1i,buffer+k1,l);
                            b1i = b1i + l;
                        } else {
                            memcpy(b1+b1i,buffer+k1,l);
                            b1i = b1i + l;
                        }

                        //copy +
                        b1[b1i] = '+';
                        b1[b1i+1] = '\n';
                        b1i = b1i + 2;

                        //copy qual
                        l = i - j1 + 1;
                        if (l-1>retain) {
                            j1 = j1 + l - retain - 1;
                            l = retain + 1;
                            memcpy(b1+b1i,buffer+j1,l);
                            b1i = b1i + l;
                        } else {
                            memcpy(b1+b1i,buffer+j1,l);
                            b1i = b1i + l;
                        }



                        flag = 1;
                        j = 0;
                        j2 = i + 1;
                    }
                }
            }
            
            if (flag == 0) {
                k = bytes_read - j2;
                memcpy(buffer,buffer+j2,k);
                j = 0;
                j2 = 0;
                j1 = 0;
                k1= 0;
                k2 = 0;
            }
            
            


        } // while


        if (b1i != 0) {
            fwrite(b1, SIZE_OF_CHAR, b1i, fop);
        }

        
        if (is_stdin == 0) {
            fclose(fip);
        }
        if (is_stdout == 0) {
            fclose(fop);
        }

        free(buffer);
        free(b1);
        
        return 0;
    } // end RETAIN3

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
    /*
    TRIMID
    */
    if (usage == 16) {
        is_stdin = 0;
        if (strcmp(argv[2],"-")==0) {
            fip = stdin;
            is_stdin = 1;
        } else {
            fip = myfopen(argv[2],"r");
        }
        is_stdout = 0;
        if (strcmp(argv[3],"-")==0) {
            fop = stdout;
            is_stdout = 1;
        } else {
            fop = myfopen(argv[3],"w");

        }
        

        buffer = mymalloc(BUFFER_SIZE * SIZE_OF_CHAR + 1);
        b1 = mymalloc(BUFFER_SIZE * SIZE_OF_CHAR + 1);

        i = 0;
        j = 0;


        k = 0;

        j1 = 0;
        j2 = 0;

        
        k1 = 0;
        k2 = 0;
        
        b1i = 0;

        while(1) {

            bytes_read = fread(buffer + k, SIZE_OF_CHAR, BUFFER_SIZE - k, fip);
            bytes_read = k + bytes_read;
            k = 0;
            j2 = 0;

            
            if (ferror(fip)) {
                    fprintf(stderr, "ERROR: Failed reading the input file.\n");
                    return 2;
            }
            if (bytes_read == 0 && feof(fip)) {
                    break;
            }



            if (feof(fip)) {
            
                if(buffer[bytes_read-1] != '\n') {
                    bytes_read = bytes_read + 1;
                    buffer[bytes_read-1] = '\n';
                } else if (bytes_read > 1 && buffer[bytes_read-2] == '\n' && buffer[bytes_read-1] == '\n') {
                    bytes_read = bytes_read - 1;
                }
            }

            
            flag = 0;
            for (i=0;i<bytes_read;i++) {
                flag = 0;
                if (buffer[i] == '\n') {
                    j = j + 1;
                    if (j==1) {
                        k1 = i + 1;

                    } else if (j==2) {

                        k2 = i + 1;
                    } else if (j==3) {

                        j1 = i + 1;
                    } else if (j==4) {

                        // j2 = id start
                        // k1 = seq start
                        // k2= plus start
                        // j1 = qual start
                        // i = end of qual
                        // (j2,i) = entire read - id, seq, +, qual

                        l = i - j2 + 1;
                        if (b1i + l > BUFFER_SIZE - 10) {
                            fwrite(b1, SIZE_OF_CHAR, b1i, fop);
                            b1i = 0;
                        }

                        //copy id
                        l = k1 - j2;
                        for(x=j2+1;x<k1-1;x++) {
                            if(buffer[x]==' ') {
                                buffer[x] = '\n';
                                l = x + 1 - j2;
                                break;
                            }
                        }

                        memcpy(b1+b1i,buffer+j2,l);
                        b1i = b1i + l;


                        //copy seq
                        l = k2 - k1;
                        memcpy(b1+b1i,buffer+k1,l);
                        b1i = b1i + l;


                        //copy +
                        b1[b1i] = '+';
                        b1[b1i+1] = '\n';
                        b1i = b1i + 2;


                        //copy qual
                        l = i - j1 + 1;
                        memcpy(b1+b1i,buffer+j1,l);
                        b1i = b1i + l;


                        flag = 1;
                        j = 0;
                        j2 = i + 1;
                    }
                }
            }
            
            if (flag == 0) {

                k = bytes_read - j2;
                memcpy(buffer,buffer+j2,k);
                j = 0;
                j2 = 0;
                j1 = 0;
                k1= 0;
                k2 = 0;

            }
            
            


        } // while


        if (b1i != 0) {
            fwrite(b1, SIZE_OF_CHAR, b1i, fop);
        }

        
        if (is_stdin == 0) {
            fclose(fip);
        }
        if (is_stdout == 0) {
            fclose(fop);
        }

        free(buffer);
        free(b1);
        
        return 0;
    } // end TRIMID


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
    /*
    TRIMPOLYA & TRIMPOLYT & TRIMPOLYC & TRIMPOLYG & TRIMN
    */
    if (usage == 17) {
    
        char c = 'A';
        char acgt = 0;


        if (strcmp(argv[2],"A")==0) {
            c = 'A';
        } else if (strcmp(argv[2],"C")==0) {
            c = 'C';
        } else if (strcmp(argv[2],"G")==0) {
            c = 'G';
        } else if (strcmp(argv[2],"T")==0) {
            c = 'T';
        } else if (strcmp(argv[2],"N")==0) {
            c = 'N';
        } else if (strcmp(argv[2],"ACGT")==0) {
            c = 'X';
            acgt = 1;
        } else {
            fprintf(stderr,"ERROR: the given nucleotide for poly tails trimming is not supported!\n");
            return 2;
        }


        trim = atoi(argv[3]);
        
        is_stdin = 0;
        if (strcmp(argv[4],"-")==0) {
            fip = stdin;
            is_stdin = 1;
        } else {
            fip = myfopen(argv[4],"r");
        }
        
        is_stdout = 0;
        if (strcmp(argv[5],"-")==0) {
            fop = stdout;
            is_stdout = 1;
        } else {
            fop = myfopen(argv[5],"w");
        }


        buffer = mymalloc(BUFFER_SIZE * SIZE_OF_CHAR + 1);
        b1 = mymalloc(BUFFER_SIZE * SIZE_OF_CHAR + 1);

        i = 0;
        j = 0;


        k = 0;

        j1 = 0;
        j2 = 0;

        
        k1 = 0;
        k2 = 0;
        
        b1i = 0;

        while(1) {

            bytes_read = fread(buffer + k, SIZE_OF_CHAR, BUFFER_SIZE - k, fip);
            bytes_read = k + bytes_read;
            k = 0;
            j2 = 0;

            if (ferror(fip)) {
                    fprintf(stderr, "ERROR: Failed reading the input file.\n");
                    return 2;
            }
            if (bytes_read == 0 && feof(fip)) {
                    break;
            }

            
            if (feof(fip)) {
                if(buffer[bytes_read-1] != '\n') {
                    bytes_read = bytes_read + 1;
                    buffer[bytes_read-1] = '\n';
                } else if (bytes_read > 1 && buffer[bytes_read-2] == '\n' && buffer[bytes_read-1] == '\n') {
                    bytes_read = bytes_read - 1;
                }
            }

            flag = 0;
            for (i=0;i<bytes_read;i++) {
                flag = 0;
                if (buffer[i] == '\n') {
                    j = j + 1;
                    if (j==1) {
                        k1 = i + 1;
                    } else if (j==2) {
                        k2 = i + 1;
                    } else if (j==3) {
                        j1 = i + 1;
                    } else if (j==4) {
                        // j2 = id start
                        // k1 = seq start
                        // k2= plus start
                        // j1 = qual start
                        // i = end of qual
                        // (j2,i) = entire read - id, seq, +, qual
                        l = i - j2 + 1;
                        if (b1i + l > BUFFER_SIZE - 10) {
                            fwrite(b1, SIZE_OF_CHAR, b1i, fop);
                            b1i = 0;
                        }
                        
                        //copy/trim id
                        l = k1 - j2;
                        for(x=j2+1;x<k1-1;x++) {
                            if(buffer[x]==' ') {
                                buffer[x] = '\n';
                                l = x + 1 - j2;
                                break;
                            }
                        }
                        memcpy(b1+b1i,buffer+j2,l);
                        b1i = b1i + l;

                        //copy seq
                        k1old = k1;
                        if (acgt==1) {
                            c = buffer[k1];
                        }
                        for(x=k1;x<k2;x++) { // trim 5
                            if(buffer[x]!=c) {
                                k1 = x;
                                break;
                            }
                        }
                        if (k1-k1old<trim) {
                            k1 = k1old;
                        }
                        k2old = k2;
                        if (acgt==1) {
                            c = buffer[k2-2];
                        }
                        for(x=k2-2;x>k1-2;x--) { // trim3
                            if(buffer[x]!=c) {
                                k2 = x + 2;
                                break;
                            }
                        }

                        if (k2old-k2<trim) {
                            k2 = k2old;
                        } else {
                            buffer[k2-1] = '\n';
                        }

                        l = k2 - k1;
                        if (l==1) {
                            b1[b1i] = 'A';
                            b1[b1i+1] = '\n';
                            b1i = b1i + 2;
                        } else {
                            memcpy(b1+b1i,buffer+k1,l);
                            b1i = b1i + l;
                        }
                        //copy +
                        b1[b1i] = '+';
                        b1[b1i+1] = '\n';
                        b1i = b1i + 2;

                        //copy qual
                        k1old = k1 - k1old;
                        k2old = k2old - k2;
                        if (k1old + k2old == 0) {
                            // no trimming
                            l = i - j1 + 1;
                            memcpy(b1+b1i,buffer+j1,l);
                            b1i = b1i + l;
                        } else {
                            if (l==1) {
                                b1[b1i] = 'I';
                                b1[b1i+1] = '\n';
                                b1i = b1i + 2;
                            } else {
                                j1 = j1 + k1old;
                                l = i - k2old - j1 + 1;
                                buffer[i-k2old] = '\n';
                                memcpy(b1+b1i,buffer+j1,l);
                                b1i = b1i + l;
                            }
                        }

                        flag = 1;
                        j = 0;
                        j2 = i + 1;
                    }
                }
            }
            
            if (flag == 0) {
                k = bytes_read - j2;
                memcpy(buffer,buffer+j2,k);
                j = 0;
                j2 = 0;
                j1 = 0;
                k1= 0;
                k2 = 0;
            }
            
            


        } // while


        if (b1i != 0) {
            fwrite(b1, SIZE_OF_CHAR, b1i, fop);
        }

        
        if (is_stdin == 0) {
            fclose(fip);
        }
        if (is_stdout == 0) {
            fclose(fop);
        }

        free(buffer);
        free(b1);
        
        return 0;
    } // end TRIMPOLY


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
    /*
    COMPRESSID
    */
    if (usage == 19) {

        long long int N = 0;
        long long int N_default = 500000;
        char line[50];
        char interleaved = 0;
        char slash = 0;
        char flag = 0;
        char first = 0;
        char shift = 0;
        
        
        if (argc == 6)  {
            if (strcmp(argv[2],"@@") == 0 || strcmp(argv[2],"/12") == 0 || strcmp(argv[2],"_12") == 0) {
                interleaved = 1;
                flag = 1;
            }
            if (strcmp(argv[2],"/1") == 0 || strcmp(argv[2],"/2") == 0 || strcmp(argv[2],"/12") == 0 || strcmp(argv[2],"_1") == 0 || strcmp(argv[2],"_2") == 0 || strcmp(argv[2],"_12") == 0) {
                slash = 1;
                flag = 1;
            }
            if (strcmp(argv[2],"@") == 0) {
                slash = 0;
                interleaved = 0;
                flag = 1;
            }

            if (slash == 1) {
                EXTRA[0] = argv[2][0];
                EXTRA[1] = argv[2][1];
                EXTRA[2] = 10;
                EXTRA[3] = 0;
            }
            shift = 0;
        } else if (argc==5){
            if (strcmp(argv[2],"@@") == 0 || strcmp(argv[2],"/12") == 0 || strcmp(argv[2],"_12" ) == 0 || strcmp(argv[2],"/1") == 0 || strcmp(argv[2],"/2") == 0 || strcmp(argv[2],"/12") == 0 || strcmp(argv[2],"_1") == 0 || strcmp(argv[2],"_2") == 0 || strcmp(argv[2],"_12") == 0) {
                if (strcmp(argv[2],"@@") == 0 || strcmp(argv[2],"/12") == 0 || strcmp(argv[2],"_12") == 0) {
                    interleaved = 1;
                    flag = 1;
                }
                if (strcmp(argv[2],"/1") == 0 || strcmp(argv[2],"/2") == 0 || strcmp(argv[2],"/12") == 0 || strcmp(argv[2],"_1") == 0 || strcmp(argv[2],"_2") == 0 || strcmp(argv[2],"_12") == 0) {
                    slash = 1;
                    flag = 1;
                }
                if (strcmp(argv[2],"@") == 0) {
                    slash = 0;
                    interleaved = 0;
                    flag = 1;
                }

                if (slash == 1) {
                    EXTRA[0] = argv[2][0];
                    EXTRA[1] = argv[2][1];
                    EXTRA[2] = 10;
                    EXTRA[3] = 0;
                }
                shift = 1;
                N = N_default;
            } else {
                slash = 0;
                interleaved = 0;
                flag = 1;
            }
            shift = 1;
        } else if (argc==4) {
            N = N_default;
            slash = 0;
            interleaved = 0;
            flag = 1;
            shift = 2;
        }


        if (flag == 0) {
            fprintf(stderr, "ERROR: Something wrong with command line options!\n");
            return 2;
        }

        // get counts
        if (N==0) {
            char *p = strchr(argv[3-shift], '.');
            if (p == NULL) {
                // it is not a file
                N = atoi(argv[3-shift]);
            } else {
                // it is a file and then read it from the file
                fip = myfopen(argv[3-shift],"r");
                if (fgets(line, sizeof(line), fip) ){
                    N = atoi(line);
                }
                fclose(fip);
            }
        } 
        if (N<1) {
            fprintf(stderr, "ERROR: Wrong number of reads!\n");
            return 2;
        }
        
        
        DIGITS = (char) (ceil(log10(N+1) / log10(CARS_LEN)));
        if (interleaved == 1) {
            DIGITS = (char) (ceil((log10(N+1) - log10(2))/ log10(CARS_LEN)));
        }
        //fprintf(stderr,"DIGITS %d for N %lld\n",DIGITS,N);

        // input FASTQ
        is_stdin = 0;
        if (strcmp(argv[4-shift],"-")==0) {
            fip = stdin;
            is_stdin = 1;
        } else {
            fip = myfopen(argv[4-shift],"r");
        }
        // output FASTQ
        is_stdout = 0;
        if (strcmp(argv[5-shift],"-")==0) {
            fop = stdout;
            is_stdout = 1;
        } else {
            fop = myfopen(argv[5-shift],"w");

        }
        

        buffer = mymalloc(BUFFER_SIZE * SIZE_OF_CHAR + 1);
        b1 = mymalloc(BUFFER_SIZE * SIZE_OF_CHAR + 1);

        i = 0;
        j = 0;


        k = 0;

        j1 = 0;
        j2 = 0;

        
        k1 = 0;
        k2 = 0;
        
        b1i = 0;
        first = 0;

        while(1) {

            bytes_read = fread(buffer + k, SIZE_OF_CHAR, BUFFER_SIZE - k, fip);
            bytes_read = k + bytes_read;
            k = 0;
            j2 = 0;

            if (ferror(fip)) {
                    fprintf(stderr, "ERROR: Failed reading the input file.\n");
                    return 2;
            }
            if (bytes_read == 0 && feof(fip)) {
                    break;
            }

            
            if (feof(fip)) {
                if(buffer[bytes_read-1] != '\n') {
                    bytes_read = bytes_read + 1;
                    buffer[bytes_read-1] = '\n';
                } else if (bytes_read > 1 && buffer[bytes_read-2] == '\n' && buffer[bytes_read-1] == '\n') {
                    bytes_read = bytes_read - 1;
                }
            }

            flag = 0;
            for (i=0;i<bytes_read;i++) {
                flag = 0;
                if (buffer[i] == '\n') {
                    j = j + 1;
                    if (j==1) {
                        k1 = i + 1;
                    } else if (j==2) {
                        k2 = i + 1;
                    } else if (j==3) {
                        j1 = i + 1;
                    } else if (j==4) {
                        // j2 = id start
                        // k1 = seq start
                        // k2= plus start
                        // j1 = qual start
                        // i = end of qual
                        // (j2,i) = entire read - id, seq, +, qual

                        l = i - j2 + 1;
                        if (b1i + l > BUFFER_SIZE - 10) {
                            fwrite(b1, SIZE_OF_CHAR, b1i, fop);
                            b1i = 0;
                        }
                        
                        
                        //copy id
                        if (interleaved == 0 ) {
                            nextid();
                            l = COUNTER_LEN;
                            memcpy(b1+b1i,COUNTER,l);
                            b1i = b1i + l;
                        } else {
                            if (first == 0) {
                                nextid();
                                first = 1;
                                l = COUNTER_LEN;
                                memcpy(b1+b1i,COUNTER,l);
                                b1i = b1i + l;
                            } else {
                                first = 0;
                                l = COUNTER_LEN;
                                memcpy(b1+b1i,COUNTER,l);
                                b1i = b1i + l;
                                if (slash==1) {
                                    b1[b1i-2] = '2';
                                }
                            }
                        }


                        //copy seq
                        l = k2 - k1;
                        memcpy(b1+b1i,buffer+k1,l);

                        b1i = b1i + l;

                        //copy +
                        b1[b1i] = '+';
                        b1[b1i+1] = '\n';
                        b1i = b1i + 2;

                        //copy qual
                        l = i - j1 + 1;
                        memcpy(b1+b1i,buffer+j1,l);
                        b1i = b1i + l;


                        flag = 1;
                        j = 0;
                        j2 = i + 1;
                    }
                }
            }
            
            if (flag == 0) {
                k = bytes_read - j2;
                memcpy(buffer,buffer+j2,k);
                j = 0;
                j2 = 0;
                j1 = 0;
                k1= 0;
                k2 = 0;
            }
            
            


        } // while


        if (b1i != 0) {
            fwrite(b1, SIZE_OF_CHAR, b1i, fop);
        }

        
        if (is_stdin == 0) {
            fclose(fip);
        }
        if (is_stdout == 0) {
            fclose(fop);
        }

        free(buffer);
        free(b1);
        
        return 0;
    } // end COMPRESSID

///////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
    /*
    DROPSE
    */
    if (usage == 20) {
    
        size_t x1,x2,y,mx;
        char same = 0;
    
    
        is_stdin = 0;
        if (strcmp(argv[2],"-")==0) {
            fip = stdin;
            is_stdin = 1;
        } else {
            fip = myfopen(argv[2],"r");
        }
        is_stdout = 0;
        if (strcmp(argv[3],"-")==0) {
            fop = stdout;
            is_stdout = 1;
        } else {
            fop = myfopen(argv[3],"w");

        }
        

        buffer = mymalloc(BUFFER_SIZE * SIZE_OF_CHAR + 1);
        b1 = mymalloc(BUFFER_SIZE * SIZE_OF_CHAR + 1);

        i = 0;
        j = 0;
        l = 0;


        k = 0;

        j1 = 0;
        j2 = 0;

        
        k1 = 0;
        k2 = 0;
        
        b1i = 0;

        x1 = 0;
        x2 = 0;
        same = 0;
        mx = 0;

        while(1) {

            bytes_read = fread(buffer + k, SIZE_OF_CHAR, BUFFER_SIZE - k, fip);
            bytes_read = k + bytes_read;
            k = 0;
            j2 = 0;

            if (ferror(fip)) {
                    fprintf(stderr, "ERROR: Failed reading the input file.\n");
                    return 2;
            }
            if (bytes_read == 0 && feof(fip)) {
                    break;
            }

            
            if (feof(fip)) {
                if(buffer[bytes_read-1] != '\n') {
                    bytes_read = bytes_read + 1;
                    buffer[bytes_read-1] = '\n';
                } else if (bytes_read > 1 && buffer[bytes_read-2] == '\n' && buffer[bytes_read-1] == '\n') {
                    bytes_read = bytes_read - 1;
                }
            }

            //still it needs work
            flag = 0;
            for (i=0;i<bytes_read;i++) {
                flag = 0;
                if (buffer[i] == '\n') {
                    j = j + 1;
                    if (j==1) {
                        k1 = i + 1;
                        //k2 and j1 removed
                    } else if (j==4) {
                        // j2 = id start
                        // k1 = seq start
                        // i = end of qual
                        // (j2,i) = entire read - id, seq, +, qual


                        same = 0;
                        if (x1 != x2) {
                            //get id
                            l = k1 - j2;
                            for(x=j2+1;x<k1-1;x++) {
                                if(buffer[x]==' ') {
                                    l = x + 1 - j2;
                                    break;
                                }
                            }
                            //compare the previous ID with the current ID
                            same = 1;
                            mx = l;
                            if (x2 - x1 < mx) {
                                mx = x2 - x1;
                            }
                            for(y=0;y<mx;y++) {
                                if(b1[x1+y]==buffer[j2+y]) {
                                    if(b1[x1+y]=='/' || b1[x1+y]=='\n') {
                                        break;
                                    }
                                } else {
                                    same = 0;
                                    break;
                                }
                            }
                            if (same == 0) {
                                b1i = x1;
                            }
                        }

                        //copy id
                        l = k1 - j2;
                        memcpy(b1+b1i,buffer+j2,l);
                        b1i = b1i + l;
                        //save positions of read id in b1
                        x1 = b1i - l;
                        x2 = b1i;

                        //copy seq & + & qual
                        l = i - k1 + 1;
                        memcpy(b1+b1i,buffer+k1,l);
                        b1i = b1i + l;

                        if (same == 1) {
                            x2 = x1;
                            //write the buffer when is full
                            if (b1i + 2000 > BUFFER_SIZE) {
                                fwrite(b1, SIZE_OF_CHAR, b1i, fop);
                                b1i = 0;
                            }
                        }

                        flag = 1;
                        j = 0;
                        j2 = i + 1;
                    }
                }
            }
            
            if (flag == 0) {
                k = bytes_read - j2;
                memcpy(buffer,buffer+j2,k);
                j = 0;
                j2 = 0;
//                j1 = 0;
                k1= 0;
//                k2 = 0;
            }
            
            


        } // while


        if (b1i != 0) {
            if (same == 1) {
                fwrite(b1, SIZE_OF_CHAR, b1i, fop);
            } else {
                fwrite(b1, SIZE_OF_CHAR, x1, fop);
            }
        }

        
        if (is_stdin == 0) {
            fclose(fip);
        }
        if (is_stdout == 0) {
            fclose(fop);
        }

        free(buffer);
        free(b1);
        
        return 0;
    } // end DROPSE


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
    /*
    FQ2FA
    */
    if (usage == 21) {
        is_stdin = 0;
        if (strcmp(argv[2],"-")==0) {
            fip = stdin;
            is_stdin = 1;
        } else {
            fip = myfopen(argv[2],"r");
        }
        is_stdout = 0;
        if (strcmp(argv[3],"-")==0) {
            fop = stdout;
            is_stdout = 1;
        } else {
            fop = myfopen(argv[3],"w");

        }
        

        buffer = mymalloc(BUFFER_SIZE * SIZE_OF_CHAR + 1);
        b1 = mymalloc(BUFFER_SIZE * SIZE_OF_CHAR + 1);

        i = 0;
        j = 0;


        k = 0;

        j1 = 0;
        j2 = 0;

        
        k1 = 0;
        k2 = 0;
        
        b1i = 0;

        while(1) {

            bytes_read = fread(buffer + k, SIZE_OF_CHAR, BUFFER_SIZE - k, fip);
            bytes_read = k + bytes_read;
            k = 0;
            j2 = 0;

            if (ferror(fip)) {
                    fprintf(stderr, "ERROR: Failed reading the input file.\n");
                    return 2;
            }
            if (bytes_read == 0 && feof(fip)) {
                    break;
            }

            
            if (feof(fip)) {
                if(buffer[bytes_read-1] != '\n') {
                    bytes_read = bytes_read + 1;
                    buffer[bytes_read-1] = '\n';
                } else if (bytes_read > 1 && buffer[bytes_read-2] == '\n' && buffer[bytes_read-1] == '\n') {
                    bytes_read = bytes_read - 1;
                }
            }

            flag = 0;
            for (i=0;i<bytes_read;i++) {
                flag = 0;
                if (buffer[i] == '\n') {
                    j = j + 1;
                    if (j==1) {
                        k1 = i + 1;
                    } else if (j==2) {
                        k2 = i + 1;
                    } else if (j==3) {
                        j1 = i + 1;
                    } else if (j==4) {
                        // j2 = id start
                        // k1 = seq start
                        // k2= plus start
                        // j1 = qual start
                        // i = end of qual
                        // (j2,i) = entire read - id, seq, +, qual

                        l = i - j2 + 1;
                        if (b1i + l > BUFFER_SIZE - 10) {
                            fwrite(b1, SIZE_OF_CHAR, b1i, fop);
                            b1i = 0;
                        }
                        
                        //copy id
                        l = k1 - j2;
                        for(x=j2+1;x<k1-1;x++) {
                            if(buffer[x]==' ') {
                                buffer[x] = '\n';
                                l = x + 1 - j2;
                                break;
                            }
                        }
                        memcpy(b1+b1i,buffer+j2,l);
                        b1[b1i] = '>';
                        b1i = b1i + l;


                        //copy seq
                        l = k2 - k1;
                        memcpy(b1+b1i,buffer+k1,l);
                        b1i = b1i + l;


                        flag = 1;
                        j = 0;
                        j2 = i + 1;
                    }
                }
            }
            
            if (flag == 0) {
                k = bytes_read - j2;
                memcpy(buffer,buffer+j2,k);
                j = 0;
                j2 = 0;
                j1 = 0;
                k1= 0;
                k2 = 0;
            }
            
            


        } // while


        if (b1i != 0) {
            fwrite(b1, SIZE_OF_CHAR, b1i, fop);
        }

        
        if (is_stdin == 0) {
            fclose(fip);
        }
        if (is_stdout == 0) {
            fclose(fop);
        }

        free(buffer);
        free(b1);
        
        return 0;
    } // end FQ2FA

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
    /*
    FA2FQ
    */
    if (usage == 22) {
        is_stdin = 0;
        if (strcmp(argv[2],"-")==0) {
            fip = stdin;
            is_stdin = 1;
        } else {
            fip = myfopen(argv[2],"r");
        }
        is_stdout = 0;
        if (strcmp(argv[3],"-")==0) {
            fop = stdout;
            is_stdout = 1;
        } else {
            fop = myfopen(argv[3],"w");

        }
        

        buffer = mymalloc(BUFFER_SIZE * SIZE_OF_CHAR + 1);
        b1 = mymalloc(BUFFER_SIZE * SIZE_OF_CHAR + 1);

        i = 0;
        j = 0;


        k = 0;

        j1 = 0;
        j2 = 0;

        
        k1 = 0;
        k2 = 0;
        
        b1i = 0;

        while(1) {

            bytes_read = fread(buffer + k, SIZE_OF_CHAR, BUFFER_SIZE - k, fip);
            bytes_read = k + bytes_read;
            k = 0;
            j2 = 0;

            if (ferror(fip)) {
                    fprintf(stderr, "ERROR: Failed reading the input file.\n");
                    return 2;
            }
            if (bytes_read == 0 && feof(fip)) {
                    break;
            }

            
            if (feof(fip)) {
                if(buffer[bytes_read-1] != '\n') {
                    bytes_read = bytes_read + 1;
                    buffer[bytes_read-1] = '\n';
                } else if (bytes_read > 1 && buffer[bytes_read-2] == '\n' && buffer[bytes_read-1] == '\n') {
                    bytes_read = bytes_read - 1;
                }
            }

            flag = 0;
            for (i=0;i<bytes_read;i++) {
                flag = 0;
                if (buffer[i] == '\n') {
                    j = j + 1;
                    if (j==1) {
                        k1 = i + 1;
                    } else if (j==2) {
                        // j2 = id start
                        // k1 = seq start
                        // i = end of seq
                        // (j2,i) = entire read - id, seq

                        l = i - j2 + 1;
                        if (b1i + l > BUFFER_SIZE - 10) {
                            fwrite(b1, SIZE_OF_CHAR, b1i, fop);
                            b1i = 0;
                        }
                        
                        //copy id
                        l = k1 - j2;
                        for(x=j2+1;x<k1-1;x++) {
                            if(buffer[x]==' ') {
                                buffer[x] = '\n';
                                l = x + 1 - j2;
                                break;
                            }
                        }
                        memcpy(b1+b1i,buffer+j2,l);
                        b1[b1i] = '@';
                        b1i = b1i + l;

                        //copy seq
                        l = i - k1 + 1;
                        memcpy(b1+b1i,buffer+k1,l);
                        b1i = b1i + l;

                        //copy +
                        b1[b1i] = '+';
                        b1[b1i+1] = '\n';
                        b1i = b1i + 2;

                        //generate qual
                        memset(b1+b1i,'I',l);
                        //memcpy(b1+b1i,buffer+k1,l);
                        b1i = b1i + l;
                        b1[b1i-1] = '\n';


                        flag = 1;
                        j = 0;
                        j2 = i + 1;
                    }
                }
            }
            
            if (flag == 0) {
                k = bytes_read - j2;
                memcpy(buffer,buffer+j2,k);
                j = 0;
                j2 = 0;
                j1 = 0;
                k1= 0;
                k2 = 0;
            }
            
            


        } // while


        if (b1i != 0) {
            fwrite(b1, SIZE_OF_CHAR, b1i, fop);
        }

        
        if (is_stdin == 0) {
            fclose(fip);
        }
        if (is_stdout == 0) {
            fclose(fop);
        }

        free(buffer);
        free(b1);
        
        return 0;
    } // end FA2FQ

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
    /*
    REVCOM
    */
    if (usage == 23) {
    
    
        size_t x,l1,l2;
        char z,q;
        
        is_stdin = 0;
        if (strcmp(argv[2],"-")==0) {
            fip = stdin;
            is_stdin = 1;
        } else {
            fip = myfopen(argv[2],"r");
        }
        is_stdout = 0;
        if (strcmp(argv[3],"-")==0) {
            fop = stdout;
            is_stdout = 1;
        } else {
            fop = myfopen(argv[3],"w");

        }
        

        buffer = mymalloc(BUFFER_SIZE * SIZE_OF_CHAR + 1);
        b1 = mymalloc(BUFFER_SIZE * SIZE_OF_CHAR + 1);

        i = 0;
        j = 0;


        k = 0;

        j1 = 0;
        j2 = 0;

        
        k1 = 0;
        k2 = 0;
        
        b1i = 0;

        while(1) {

            bytes_read = fread(buffer + k, SIZE_OF_CHAR, BUFFER_SIZE - k, fip);
            bytes_read = k + bytes_read;
            k = 0;
            j2 = 0;

            if (ferror(fip)) {
                    fprintf(stderr, "ERROR: Failed reading the input file.\n");
                    return 2;
            }
            if (bytes_read == 0 && feof(fip)) {
                    break;
            }

            
            if (feof(fip)) {
                if(buffer[bytes_read-1] != '\n') {
                    bytes_read = bytes_read + 1;
                    buffer[bytes_read-1] = '\n';
                } else if (bytes_read > 1 && buffer[bytes_read-2] == '\n' && buffer[bytes_read-1] == '\n') {
                    bytes_read = bytes_read - 1;
                }
            }

            flag = 0;
            for (i=0;i<bytes_read;i++) {
                flag = 0;
                if (buffer[i] == '\n') {
                    j = j + 1;
                    if (j==1) {
                        k1 = i + 1;
                    } else if (j==2) {
                        k2 = i + 1;
                    } else if (j==3) {
                        j1 = i + 1;
                    } else if (j==4) {
                        // j2 = id start
                        // k1 = seq start
                        // k2= plus start
                        // j1 = qual start
                        // i = end of qual
                        // (j2,i) = entire read - id, seq, +, qual

                        l = i - j2 + 1;
                        if (b1i + l > BUFFER_SIZE - 10) {
                            fwrite(b1, SIZE_OF_CHAR, b1i, fop);
                            b1i = 0;
                        }
                        
                        //copy id
                        l = k1 - j2;
                        for(x=j2+1;x<k1-1;x++) {
                            if(buffer[x]==' ') {
                                buffer[x] = '\n';
                                l = x + 1 - j2;
                                break;
                            }
                        }
                        memcpy(b1+b1i,buffer+j2,l);
                        b1i = b1i + l;

                        //copy seq
                        l = k2 - k1;
                        memcpy(b1+b1i,buffer+k1,l);
                        //b1i = b1i + l;
                        //now reverse complement
                        l1 = l - 1;
                        l2 = l1 / 2;
                        for(x=0;x<l2;x++){
                            z = b1[b1i+x];
                            if (z=='A') {
                                z = 'T';
                            } else if (z=='C') {
                                z = 'G';
                            } else if (z=='G') {
                                z = 'C';
                            } else if (z=='T') {
                                z = 'A';
                            }
                            
                            //b1[b1i+x] = b1[b1i+l1-1-x];
                            q = b1[b1i+l1-1-x];

                            if (q=='A') {
                                q = 'T';
                            } else if (q=='C') {
                                q = 'G';
                            } else if (q=='G') {
                                q = 'C';
                            } else if (q=='T') {
                                q = 'A';
                            }
                            b1[b1i+x] = q;
                            
                            b1[b1i+l1-1-x] = z;
                        }
                        if (l1%2 == 1) {
                            x = l2;
                            z = b1[b1i+x];
                            if (z=='A') {
                                z = 'T';
                            } else if (z=='C') {
                                z = 'G';
                            } else if (z=='G') {
                                z = 'C';
                            } else if (z=='T') {
                                z = 'A';
                            }
                            b1[b1i+x] = z;
                        }
                        b1i = b1i + l;


                        //copy +
                        b1[b1i] = '+';
                        b1[b1i+1] = '\n';
                        b1i = b1i + 2;

                        //copy qual
                        l = i - j1 + 1;
                        memcpy(b1+b1i,buffer+j1,l);
                        //b1i = b1i + l;
                        //now reverse it
                        l1 = l - 1;
                        l2 = l1 / 2;
                        for(x=0;x<l2;x++){
                            z = b1[b1i+x];
                            b1[b1i+x] = b1[b1i+l1-1-x];
                            b1[b1i+l1-1-x] = z;
                        }
                        b1i = b1i + l;

                        flag = 1;
                        j = 0;
                        j2 = i + 1;
                    }
                }
            }
            
            if (flag == 0) {
                k = bytes_read - j2;
                memcpy(buffer,buffer+j2,k);
                j = 0;
                j2 = 0;
                j1 = 0;
                k1= 0;
                k2 = 0;
            }
            
            


        } // while


        if (b1i != 0) {
            fwrite(b1, SIZE_OF_CHAR, b1i, fop);
        }

        
        if (is_stdin == 0) {
            fclose(fip);
        }
        if (is_stdout == 0) {
            fclose(fop);
        }

        free(buffer);
        free(b1);
        
        return 0;
    } // end REVCOM

   
}









