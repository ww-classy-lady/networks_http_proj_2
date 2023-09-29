//./proj2 to run
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

//./proj2 -u URL [-d] [-q] [-r] -o filename

#define ARG_URL 0x1 //u
#define ARG_DBG 0x2 //d
#define ARG_HTTPReq 0x4 //q
#define ARG_HTTPRes 0x6 //r
#define ARG_CONT 0x8 //o
//u
//URL handling: if the optarg (after -u is not http://...throw error from parsingURL)
//o
//FileName handling: if the optarg (after -o is not a valid file name then error from parsingFilename)
//
//d
//q
//r
unsigned short cmd_line_flags = 0;
char *url = NULL; //the url after -u
char *filename = NULL; //the output file name after -o
char *seq [];
void usage (char *progname)
{
    fprintf (stderr,"%s ./proj2 -u URL [-d] [-q] [-r] -o filename\n", progname);
    fprintf (stderr,"   -d    show DBG statements\n");
    fprintf (stderr,"   -q    Makes the http request message\n");
    fprintf (stderr,"   -r  send and get response? \'X\'\n");
    exit (1);
}
void parseargs(int argc, char *argv [])
{
    int opt;
    for (int i = 0; i < argc; i++) {
        printf("argv[%d]: %s\n", i, argv[i]); //for checking the elements in the command given
    }
    while((opt = getopt (argc, argv, "u:o:dqr")) != -1)
    {
        switch(opt)
        {
            case 'u':
                cmd_line_flags = ARG_URL;
                url = optarg;
                fprintf (stdout, "%s\n", url);
                printf ("-u detected: call the parseURL method to parse the url into hostname, urlfilename/path\n");
                break;
            case 'o':
                cmd_line_flags = ARG_CONT;
                filename = optarg;
                printf ("-o detected: store the response into a file\n");
                fprintf (stdout, "%s\n", filename);
                break;
            case 'd':
                cmd_line_flags = ARG_DBG;
                printf ("-d detected: should call dbg to make the dbg messages based on info from -u and -o\n");
                break;
            case 'q':
                cmd_line_flags = ARG_HTTPReq;
                printf ("-q detected: should use a method to  print HTTP request with OUT: ...etc\n");
                break;
            case 'r':
                cmd_line_flags = ARG_HTTPRes;
                printf ("-r detected: should make a method to print HTTP response\n");
                break;
            case '?':
            default:
                usage(argv [0]);
        }
    }
    if (cmd_line_flags == 0)
    {
        fprintf (stderr,"error: no command line option given\n"); //fprintf is for errors only
        usage (argv [0]);
    }
}
int main (int argc, char *argv [])
{
    //-o file will be initiated if -u is legit and response is ok 200
    //should do dqr operations last after sending sockets (?)
    //once finished seeing -u and -o then send the request
    parseargs(argc, argv);
    
    exit (0);
    
}
