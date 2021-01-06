/* The MIT License

   Copyright (c) 2010-2021 Daniel Nicorici

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

unsigned char size_of_char = sizeof(char);
unsigned int BUFFER_SIZE = 32 * 1024 * 1024;
//unsigned int BUFFER_SIZE = 2 * 1024 * 1024;


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
    char *ptr = (char *)malloc(size_of_char * n);
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

    unsigned char is_stdin = 0;
    unsigned char is_stdout = 0;
    unsigned char r1 = 0;
    unsigned char end1 = 0;
    unsigned char end2 = 0;



    char *buffer;
    char *b1;
    char *b2;
    char flag = 0;
    char usage = 0;
    

    size_t  bytes_read,bytes_read1,bytes_read2;
    size_t i,j,k,b1i,b2i,l;
    size_t j1,j2,k1,k2,k1old,k2old;

    FILE *fip, *fr1, *fr2;
    FILE *fop, *fi1p, *fi2p;
    

    usage = 1;
    if (argc > 1) {
        if (strcmp(argv[1],"deinterleave") == 0) {
            if (argc != 5) {
                fprintf(stderr, "\n");
                fprintf(stderr, "Usage:   fastq-leave deinterleave <in.fq> <ou1.fq> <ou2.fq>\n\n");
                fprintf(stderr, "It splits an interleaved input FASTQ file into two paired-end FASTQ files.\n");
                fprintf(stderr, "For input from STDIN use - instead of <in.fq>.\n\n");
                return 1;
            } else {
                usage = 2;
            }
        } else if (strcmp(argv[1],"interleave") == 0) {
            if (argc != 5) {
                fprintf(stderr, "\n");
                fprintf(stderr, "Usage:   fastq-leave interleave <in1.fq> <in2.fq> <ou.fq>\n\n");
                fprintf(stderr, "It interleaves two input paired-end FASTQ files into one output FASTQ file.\n");
                fprintf(stderr, "For redirecting output to STDOUT use - instead of <ou.fq>.\n\n");
                return 1;
            } else {
                usage = 3;
            }
        } else {
            usage = 1;
        }
    }
    if (usage == 1) {
        fprintf(stderr, "\n");
        fprintf(stderr, "Usage:   fastq-leave <command> <arguments>\n");
        fprintf(stderr, "Version: 0.15\n\n");
        fprintf(stderr, "Command: interleave        interleave two paired-end FASTQ files.\n");
        fprintf(stderr, "         deinterleave      splits one (and already) interleaved FASTQ file.\n");
        //fprintf(stderr, "         tab4              converts  FASTQ file to 4-columns tab-delimited text file.\n");
        //fprintf(stderr, "         tab8              converts (interleaved) FASTQ file to 8-columns tab-delimited text file.\n");
        fprintf(stderr, "\n");
        return 1;
    }

    /*
    DEINTERLEAVE
    */
    if (usage == 2) {
        if (strcmp(argv[2],"-")==0) {
            fip = stdin;
            is_stdin = 1;
        } else {
            fip = myfopen(argv[2],"r");
        }
        
        fr1 = myfopen(argv[3],"w");
        
        fr2 = myfopen(argv[4],"w");

        buffer = mymalloc(BUFFER_SIZE * size_of_char);
        
        b1 = mymalloc(BUFFER_SIZE * size_of_char);

        b2 = mymalloc(BUFFER_SIZE * size_of_char);

        
        i = 0;
        j = 0;
        k = 0;

        r1 = 0;
        b1i = 0;
        b2i = 0;
        // for(;;)


        while(1) {

            bytes_read = fread(buffer, size_of_char, BUFFER_SIZE, fip);

            if (ferror(fip)) {
                    fprintf(stderr, "ERROR: Failed reading the input file.\n");
                    return 2;
            }
            if (bytes_read == 0) { // end of file?
                if (feof(fip)) {
                    break;
                }
            }
            
            if (buffer[bytes_read-1] == '\0') {
                if (bytes_read >1 && buffer[bytes_read-2] == '\n') {
                    bytes_read = bytes_read - 1;
                } else {
                    buffer[bytes_read-1] = '\n';
                }
            }

            flag = 0;
            for (i=0;i<bytes_read;i++) {
                flag = 0;
                if (buffer[i] == '\n') {
                    j = j + 1;
                    if (j==4) {
                        flag = 1;
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
            

            fwrite(b1, size_of_char, b1i, fr1);
            fwrite(b2, size_of_char, b2i, fr2);
            
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


    }

    /*
    INTERLEAVE
    */
    if (usage == 3) {
        
        if (strcmp(argv[4],"-")==0) {
            fop = stdout;
            is_stdout = 1;
        } else {
            fop = myfopen(argv[4],"w");
        }

        fi1p = myfopen(argv[2],"r");

        fi2p = myfopen(argv[3],"r");


        buffer = mymalloc(BUFFER_SIZE * size_of_char);
        
        b1 = mymalloc(BUFFER_SIZE * size_of_char);

        b2 = mymalloc(BUFFER_SIZE * size_of_char);
        
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
                    bytes_read1 = fread(b1, size_of_char, BUFFER_SIZE, fi1p);
                    if (ferror(fi1p)) {
                        fprintf(stderr, "ERROR: Failed reading the input file.\n");
                        return 2;
                    }
                    if (bytes_read1 == 0) { // end of file
                        if (feof(fi1p)) {
                            end1 = 1;
                            r1 = 1;
                            continue;
                        }
                    }
                    if (b1[bytes_read1-1] == '\0') {
                        if (bytes_read1 > 1 && b1[bytes_read1-2] == '\n') {
                            bytes_read1 = bytes_read1 - 1; 
                        } else {
                            b1[bytes_read1-1] = '\n';
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
                    fwrite(buffer, size_of_char, k, fop);
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
                    bytes_read2 = fread(b2, size_of_char, BUFFER_SIZE, fi2p);
                    if (ferror(fi2p)) {
                        fprintf(stderr, "ERROR: Failed reading the input file.\n");
                        return 2;
                    }
                    if (bytes_read2 == 0) { // end of file
                        if (feof(fi2p)) {
                            end2 = 1;
                            r1 = 0;
                            continue;
                        }
                    }
                    if (b2[bytes_read2-1] == '\0') {
                        if (bytes_read2 > 1 && b2[bytes_read2-2] == '\n') {
                            bytes_read2 = bytes_read2 - 1;
                        } else {
                            b2[bytes_read2-1] = '\n';
                        }
                    }
                }
                //printf("-->2: r1=1\n");
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
                    fwrite(buffer, size_of_char, k, fop);
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
            fwrite(buffer, size_of_char, k, fop);
        }

        if (is_stdout==0) {
            fclose(fop);
        }

        fclose(fi1p);
        fclose(fi2p);

        free(buffer);
        free(b1);
        free(b2);

    }

    return 0;
}
