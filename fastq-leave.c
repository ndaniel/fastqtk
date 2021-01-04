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


#define BUFFER_SIZE (300 * 1024 * 1024)


void errormemory(char *m) {
    if (m == NULL) {
        fprintf(stderr, "MEMORY ERROR!\n");
        exit(2);
    }
}

void errorfile(FILE *f, char *s) {
    if (f == NULL) {
        fprintf(stderr, "FILE ERROR at file '%s'!\n",s);
        exit(2);
    }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
int main(int argc, char * argv[])
{

    unsigned char is_stdin = 0;
    unsigned char is_stdout = 0;
    unsigned char r1 = 0;
    unsigned char size_of_char = sizeof(char);


    char *buffer;
    char *b1;
    char *b2;

    size_t  bytes_read;
    size_t i,j,k,b1i,b2i,l;

    FILE *fip, *fr1, *fr2;
    FILE *fop, *fi1p, *fi2p;
    

    if (argc > 1 && ( strcmp(argv[1],"interleave") == 0 || strcmp(argv[1],"deinterleave") == 0)) {

        if (strcmp(argv[1],"deinterleave") == 0 && argc != 5) {
            fprintf(stderr, "\n");
            fprintf(stderr, "Usage:   fastq-leave deinterleave <in.fq> <ou1.fq> <ou2.fq>\n\n");
            fprintf(stderr, "It splits an interleaved input FASTQ file into two paired-end FASTQ files.\n");
            fprintf(stderr, "For input from STDIN use - instead of <in.fq>.\n\n");
            return 1;
        }
        if (strcmp(argv[1],"interleave") == 0 && argc != 5) {
            fprintf(stderr, "\n");
            fprintf(stderr, "Usage:   fastq-leave interleave <in1.fq> <in2.fq> <ou.fq>\n\n");
            fprintf(stderr, "It interleaves two input paired-end FASTQ files into one output FASTQ file.\n");
            fprintf(stderr, "For redirecting output to STDOUT use - instead of <ou.fq>.\n\n");
            return 1;
        }
        
    } else {
        fprintf(stderr, "\n");
        fprintf(stderr, "Usage:   fastq-leave <command> <arguments>\n");
        fprintf(stderr, "Version: 0.15\n\n");
        fprintf(stderr, "Command: interleave        interleave two paired-end FASTQ files\n");
        fprintf(stderr, "         deinterleave      splits one interleave FASTQ file\n");
        fprintf(stderr, "\n");
        return 1;
    }

    /*
    DEINTERLEAVE
    */
    if (strcmp(argv[1],"deinterleave") == 0 && argc == 5) {
        if (strcmp(argv[2],"-")==0) {
            fip = stdin;
            is_stdin = 1;
        } else {
            fip = fopen(argv[2], "r");
            errorfile(fip,argv[2]);
        }
        
        fr1 = fopen(argv[3],"w");
        errorfile(fr1,argv[3]);
        fclose(fr1); //clear the output file
        fr1 = fopen(argv[3], "a");
        errorfile(fr1,argv[3]);

        fr2 = fopen(argv[4],"w");
        errorfile(fr2,argv[4]);
        fclose(fr2); //clear the output file
        fr2 = fopen(argv[4], "a");
        errorfile(fr2,argv[4]);

        buffer = (char*) malloc(BUFFER_SIZE * size_of_char);
        errormemory(buffer);
        
        b1 = (char*) malloc(BUFFER_SIZE * size_of_char);
        errormemory(b1);

        b2 = (char*) malloc(BUFFER_SIZE * size_of_char);
        errormemory(b2);
        
        i = 0;
        j = 0;
        k = 0;

        r1 = 0;
        b1i = 0;
        b2i = 0;
        // for(;;)
        // while(!feof(stdin)) {


        while(1) {

            bytes_read = fread(buffer, size_of_char, BUFFER_SIZE, fip);


            if (bytes_read == 0) { // end of file
                break;
            }

            
            for (i=0;i<bytes_read;i++) {
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
            
            if (buffer[bytes_read-1] != '\n') {
                i = bytes_read - 1;
                if(r1==0){
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
        
        
        if (is_stdin==0) {
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
    if (strcmp(argv[1],"interleave") == 0 && argc == 5) {
        if (strcmp(argv[4],"-")==0) {
            fop = stdout;
            is_stdout = 1;
        } else {
            fop = fopen(argv[4], "w");
            errorfile(fop,argv[4]);
        }

        fi1p = fopen(argv[2], "r");
        errorfile(fi1p,argv[2]);

        fi2p = fopen(argv[3], "r");
        errorfile(fi2p,argv[3]);

        
        if (is_stdout==0) {
            fclose(fop);
        }


        fclose(fi1p);
        fclose(fi2p);

    }

    return 0;
}




