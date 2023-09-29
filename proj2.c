//./proj2 to run
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h> 
#include <string.h>
//./proj2 -u URL [-d] [-q] [-r] -o filename

#define ARG_URL 0x1 //u
#define ARG_DBG 0x2 //d
#define ARG_HTTPReq 0x4 //q
#define ARG_HTTPRes 0x6 //r
#define ARG_CONT 0x8 //o
//General order of methods:
//parseArgs (gets the url pointer(for u, d, q, r and SOCKETS) and the filename for -o)
//parseURL
//SOCKETS(will call a helper method to generate the http request, store that response...and blah blah)
//^ the helper to generate the http request follows:
//GET [url_filename] HTTP/1.0\r\n
//Host: [hostname]\r\n
//User-Agent: CWRU CSDS 325 SimpleClient 1.0\r\n
//\r\n

//use the order char to print out 
//d q or r in the correct order

//URL handling: if the optarg (after -u is not http://...throw error from parsingURL)
//o
//FileName handling: if the optarg (after -o is not a valid file name then error from parsingFilename)
//
//d
//-d to be printed regardless of whether there are errors fetching the web page (sockets).
//-d will NOT be printed if there are errors in the command line options given by the user.
//q
//without networking: will have the OUT: ahead of everything and just prints out the message
//with networking:
//      *generate the http message in the ideal format in a helper method
//      *the socket code probably calls
//r
bool uDetected = false; //initially false as u is not detected
bool oDetected = false; //initially false as o is not detected
bool dDetected = false; //initially false as d is not detected
bool qDetected = false; //initially false as q is not detected
bool rDetected = false; //initially false as r is not detected
char* url = NULL; //the url after -u
bool urlWrong = false;
char* filename = NULL; //the output file name after -o
char order[3] = "";
char* hostname = NULL; //hostname that will be filled in parseURL
char* web_file = NULL;  //webfile that will be filled in in web_file
char httpForm[] = "http://";
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
    /*for(int i = 0; i < argc; i++) 
    {
        printf("argv[%d]: %s\n", i, argv[i]); //for checking the elements in the command given
    }*/
    while((opt = getopt (argc, argv, "u:o:dqr")) != -1)
    {
        switch(opt)
        {
            case 'u':
                uDetected = true;
                url = optarg;
                //strcat(url, optarg);
                //fprintf (stdout, "%s\n", url);
                //printf ("-u detected: call the parseURL method to parse the url into hostname, urlfilename/path\n");
                break;
            case 'o':
                oDetected = true;
                filename = optarg;
                //strcat(filename, optarg);
                //printf ("-o detected: store the response into a file\n");
                //fprintf (stdout, "%s\n", filename);
                break;
            case 'd':
                dDetected = true;
                //printf ("-d detected: should call dbg to make the dbg messages based on info from -u and -o\n");
                strcat(order, "d");
                break;
            case 'q':
                qDetected = true;
                printf ("-q detected: should use a method to  print HTTP request with OUT: ...etc\n");
                strcat(order, "q");
                //put this element in the sequence char to track order
                break;
            case 'r':
                rDetected = true;
                printf ("-r detected: should make a method to print HTTP response\n");
                strcat(order, "r");
                break;
            case '?':
            default:
                usage(argv [0]);
        }
    }
    if(argv[1] == NULL)
    {
        fprintf (stderr,"error: no command line option given\n"); //fprintf is for errors only
        usage (argv [0]);
    }
}

void parseURL(char* link)
{
    //have to handle null link, have to handle if link is -o or anythingelse
    
    //converting link to char []
    if(strncasecmp(link, httpForm, strlen(httpForm)) == 0) 
    {
        //check if the link starts with "http://" and if so strncasecmp should return 0 for equality
        char *after_httpForm = link + strlen(httpForm); //set pointer to right after http://, which is the first char
        //of hostname
        char *slash = strchr(after_httpForm, '/'); //set pointer to the slash before filename
        if(slash != NULL) 
        {
            size_t hostname_Length = slash - after_httpForm; //finds the length of the hostname
            hostname = (char*)malloc(hostname_Length+1);
            strncpy(hostname, after_httpForm, hostname_Length); //copies from after_httpForm the hostname using
            //the hostname's length to hostname string
            hostname[hostname_Length] = '\0'; //add null terminator
            size_t web_file_Length = strlen(slash);
            web_file = (char*)malloc(web_file_Length + 1);
            strcpy(web_file, slash); 
        } 
        else{
            // No '/' found after "http://", assume the entire string is the hostname
            size_t hostname_length = strlen(after_httpForm);
            hostname = (char*)malloc(hostname_length+1);
            strcpy(hostname, after_httpForm);
            hostname[hostname_length] = '\0'; //add null terminator
            size_t webfile_length = strlen("/");
            web_file =(char*)malloc(webfile_length + 1);
            strcpy(web_file, "/");
            web_file[webfile_length] = '\0'; //add null terminator
        }
    } 
    else{
        urlWrong = true;
        fprintf(stderr, "Error: url does not begin with http:// \n");
    }
}
//d operation below
void printD(char *copy_hostname, char *copy_web_file, char *copy_filename)
{
    printf("DBG: host: %s\n", copy_hostname);
    printf("DBG: web_file: %s\n", copy_web_file);
    printf("DBG: output_file: %s\n", copy_filename);
}
void printQ(char *copy_web_file, char *copy_hostname)
{
    //difference from the helper that
    //sends to the web server: the blank line that terminates the HTTP request must be excluded from the -q output!
    printf("OUT: GET %s HTTP/1.0\n", copy_web_file);
    printf("OUT: Host: %s\n", copy_hostname);
    printf("OUT: User-Agent: CWRU CSDS 325 SimpleClient 1.0\n");
}
int main (int argc, char *argv [])
{
    //-o file will be initiated if -u is legit and response is ok 200
    //should do dqr operations last after sending sockets (?)
    //once finished seeing -u and -o then send the request
    parseargs(argc, argv);
    //printf("%s\n", order);
    if(uDetected && oDetected)
    {
        //continue with socket, 
        //then dpr seq printouts (if seq char is greater than 3, too much argument might need to throw error)
        parseURL(url); //parse url to hostname, web_file
        //test -d
        if(dDetected && urlWrong == false)
        {
            printD(hostname, web_file, filename);
        }
        else if(qDetected && urlWrong == false)
        {
            printQ(web_file, hostname);
        }
        else
        {
            printf("d is not detected or url is wrong\n");
        }
        //test -q
        //test -r 
        //after all -d,-q,-r passed/finished, find a way to print them in order
    }
    else if(url == NULL)
    {
        //no url error

    }
    else if(filename == NULL)
    {
        //no file name error
    }
    else{
        fprintf(stdout, "Please provide a valid command line argument.");
    }
    exit (0);
    
}
