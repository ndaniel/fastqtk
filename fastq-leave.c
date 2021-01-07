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

char const SIZE_OF_CHAR = sizeof(char);
int const BUFFER_SIZE = 32 * 1024 * 1024;
//unsigned int BUFFER_SIZE = 2 * 1024 * 1024;
int const MAX_LEN = 32*1024 ; //maximum length of reads


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
    

    size_t  bytes_read,bytes_read1,bytes_read2;
    long int i,j,k,b1i,b2i,l;
    long int j1,j2,k1,k2,k1old,k2old;

    FILE *fip, *fr1, *fr2;
    FILE *fop, *fi1p, *fi2p;
    

    usage = 1;
    if (argc > 1) {
        if (strcmp(argv[1],"deinterleave") == 0) {
            if (argc != 5) {
                fprintf(stderr, "\n");
                fprintf(stderr, "Usage:   fastq-leave deinterleave <in.fq> <out1.fq> <out2.fq>\n\n");
                fprintf(stderr, "It splits an interleaved input FASTQ file into two paired-end FASTQ files.\n");
                fprintf(stderr, "For input from STDIN use - instead of <in.fq>.\n\n");
                return 1;
            } else {
                usage = 2;
            }
        } else if (strcmp(argv[1],"interleave") == 0) {
            if (argc != 5) {
                fprintf(stderr, "\n");
                fprintf(stderr, "Usage:   fastq-leave interleave <in1.fq> <in2.fq> <out.fq>\n\n");
                fprintf(stderr, "It interleaves two input paired-end FASTQ files into one output FASTQ file.\n");
                fprintf(stderr, "For redirecting output to STDOUT use - instead of <out.fq>.\n\n");
                return 1;
            } else {
                usage = 3;
            }
        } else if (strcmp(argv[1],"count") == 0) {
            if (argc != 4) {
                fprintf(stderr, "\n");
                fprintf(stderr, "Usage:   fastq-leave count <in.fq> <out.txt>\n\n");
                fprintf(stderr, "It counts the number of reads from an input FASTQ file and outputs it to a text file.\n");
                fprintf(stderr, "For redirecting output to STDOUT/STDIN use - instead of file name.\n\n");
                return 1;
            } else {
                usage = 4;
            }
        } else if (strcmp(argv[1],"lengths") == 0) {
            if (argc != 4) {
                fprintf(stderr, "\n");
                fprintf(stderr, "Usage:   fastq-leave lengths <in.fq> <out.txt>\n\n");
                fprintf(stderr, "It provides a summary statistics regarding the lengths of the reads from an input FASTQ file and outputs it to a text file.\n");
                fprintf(stderr, "The output text file contains the unique lengths of the reads found in the input file, which are sorted in descending order.\n");
                fprintf(stderr, "For redirecting output to STDOUT/STDIN use - instead of file name.\n\n");
                return 1;
            } else {
                usage = 5;
            }
        } else if (strcmp(argv[1],"count-lengths") == 0) {
            if (argc != 5) {
                fprintf(stderr, "\n");
                fprintf(stderr, "Usage:   fastq-leave count-lengths <in.fq> <count.txt> <statistics.txt>\n\n");
                fprintf(stderr, "It provides a summary statistics regarding the lengths of the reads from an input FASTQ file and outputs it to a text file.\n");
                return 1;
            } else {
                usage = 6;
            }
        } else if (strcmp(argv[1],"tab4") == 0) {
            if (argc != 4) {
                fprintf(stderr, "\n");
                fprintf(stderr, "Usage:   fastq-leave tab4 <in.fq> <fastq.txt>\n\n");
                fprintf(stderr, "It converts a FASTQ file into a tab-delimited text file with four columns.\n");
                fprintf(stderr, "For redirecting output to STDOUT/STDIN use - instead of file name.\n\n");
                return 1;
            } else {
                usage = 7;
            }
        } else if (strcmp(argv[1],"tab8") == 0) {
            if (argc != 4) {
                fprintf(stderr, "\n");
                fprintf(stderr, "Usage:   fastq-leave tab8 <in.fq> <fastq.txt>\n\n");
                fprintf(stderr, "It converts an interleaved FASTQ file into a tab-delimited text file with 8 columns.\n");
                fprintf(stderr, "For redirecting output to STDOUT/STDIN use - instead of file name.\n\n");
                return 1;
            } else {
                usage = 8;
            }
        } else if (strcmp(argv[1],"detab") == 0) {
            if (argc != 4) {
                fprintf(stderr, "\n");
                fprintf(stderr, "Usage:   fastq-leave detab <fastq.txt> <out.fq>\n\n");
                fprintf(stderr, "It converts a 4 or 8 columns tab-delimited text file into a FASTQ file.\n");
                fprintf(stderr, "For redirecting output to STDOUT/STDIN use - instead of file name.\n\n");
                return 1;
            } else {
                usage = 9;
            }
        }
    }
    if (usage == 1) {
        fprintf(stderr, "\n");
        fprintf(stderr, "Usage:   fastq-leave <command> <arguments>\n");
        fprintf(stderr, "Version: 0.16\n\n");
        fprintf(stderr, "Command: interleave        interleave two paired-end FASTQ files.\n");
        fprintf(stderr, "         deinterleave      splits one (and already) interleaved FASTQ file.\n");
        fprintf(stderr, "         count             count number of reads from a FASTQ file.\n");
        fprintf(stderr, "         lengths           summary statistics for lengths of reads from a FASTQ file.\n");
        fprintf(stderr, "         count-lengths     count number of reads and provide summary statistics for lengths of reads from a FASTQ file.\n");
        fprintf(stderr, "         tab4              converts a FASTQ file to 4-columns tab-delimited text file.\n");
        fprintf(stderr, "         tab8              converts a (interleaved) FASTQ file to 8-columns tab-delimited text file.\n");
        fprintf(stderr, "         detab             converts a 4/8-columns tab-delimited text file (which was converted using tab4 or tab8) to FASTQ file.\n");
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

        buffer = mymalloc(BUFFER_SIZE * SIZE_OF_CHAR);
        
        b1 = mymalloc(BUFFER_SIZE * SIZE_OF_CHAR);

        b2 = mymalloc(BUFFER_SIZE * SIZE_OF_CHAR);

        
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
            if (bytes_read == 0) { // end of file?
                if (feof(fip)) {
                    break;
                }
            }
            
            if (buffer[bytes_read-1] == '\0') {
                if (bytes_read != 1 && buffer[bytes_read-2] == '\n') {
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


        buffer = mymalloc(BUFFER_SIZE * SIZE_OF_CHAR);
        
        b1 = mymalloc(BUFFER_SIZE * SIZE_OF_CHAR);

        b2 = mymalloc(BUFFER_SIZE * SIZE_OF_CHAR);
        
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
                    if (bytes_read1 == 0) { // end of file
                        if (feof(fi1p)) {
                            end1 = 1;
                            r1 = 1;
                            continue;
                        }
                    }
                    if (b1[bytes_read1-1] == '\0') {
                        if (bytes_read1 != 1 && b1[bytes_read1-2] == '\n') {
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
        

        buffer = mymalloc(BUFFER_SIZE * SIZE_OF_CHAR);
        

        u = 0;
        

        // for(;;)


        while(1) {

            bytes_read = fread(buffer, SIZE_OF_CHAR, BUFFER_SIZE, fip);

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
                if (bytes_read != 1 && buffer[bytes_read-2] == '\n') {
                    bytes_read = bytes_read - 1;
                } else {
                    buffer[bytes_read-1] = '\n';
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
        
        fprintf(fop,"%lld",u);
        
        if (is_stdin == 0) {
            fclose(fip);
        }
        if (is_stdout == 0) {
            fclose(fop);
        }

        free(buffer);

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
        

        buffer = mymalloc(BUFFER_SIZE * SIZE_OF_CHAR);

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
            if (bytes_read == 0) { // end of file?
                if (feof(fip)) {
                    break;
                }
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
                i = bytes_read - 1;
                l = i - k - 1;
            }
            if (k != 0 && k != 1 && buffer[k-1] == '\r') {
                windows = 1;
            }
            
        }
        
        for (i=MAX_LEN-1;-1<i;i--) {
            if (len[i] != 0) {
                fprintf(fop,"%ld\n",i-windows);
            }
        }
        
        if (is_stdin == 0) {
            fclose(fip);
        }
        if (is_stdout == 0) {
            fclose(fop);
        }

        free(buffer);

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

        buffer = mymalloc(BUFFER_SIZE * SIZE_OF_CHAR);

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
            if (bytes_read == 0) { // end of file?
                if (feof(fip)) {
                    break;
                }
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
        fprintf(fr1,"%lld",u);
        
        for (i=MAX_LEN-1;-1<i;i--) {
            if (len[i] != 0) {
                fprintf(fr2,"%ld\n",i-windows);
            }
        }
        
        if (is_stdin == 0) {
            fclose(fip);
        }

        fclose(fr1);
        fclose(fr2);

        free(buffer);

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

        buffer = mymalloc(BUFFER_SIZE * SIZE_OF_CHAR);
        

        

        // for(;;)
        j = 0;


        while(1) {

            bytes_read = fread(buffer, SIZE_OF_CHAR, BUFFER_SIZE, fip);

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
                if (bytes_read != 1 && buffer[bytes_read-2] == '\n') {
                    bytes_read = bytes_read - 1;
                } else {
                    buffer[bytes_read-1] = '\n';
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

        buffer = mymalloc(BUFFER_SIZE * SIZE_OF_CHAR);
        

        

        // for(;;)
        j = 0;


        while(1) {

            bytes_read = fread(buffer, SIZE_OF_CHAR, BUFFER_SIZE, fip);

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
                if (bytes_read != 1 && buffer[bytes_read-2] == '\n') {
                    bytes_read = bytes_read - 1;
                } else {
                    buffer[bytes_read-1] = '\n';
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

    } // end DETAB

    return 0;
}




