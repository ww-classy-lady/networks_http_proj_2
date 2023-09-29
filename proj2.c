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
//u
//URL handling: if the optarg (after -u is not http://...throw error from parsingURL)
//o
//FileName handling: if the optarg (after -o is not a valid file name then error from parsingFilename)
//
//d
//q
//r
bool uDetected = false; //initially false as u is not detected
bool oDetected = false; //initially false as o is not detected
bool dDetected = false; //initially false as d is not detected
bool qDetected = false; //initially false as q is not detected
bool rDetected = false; //initially false as r is not detected
char *url = NULL; //the url after -u
char *filename = NULL; //the output file name after -o
char order[3] = "";
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
                uDetected = true;
                url = optarg;
                //strcat(url, optarg);
                fprintf (stdout, "%s\n", url);
                printf ("-u detected: call the parseURL method to parse the url into hostname, urlfilename/path\n");
                break;
            case 'o':
                oDetected = true;
                filename = optarg;
                //strcat(filename, optarg);
                printf ("-o detected: store the response into a file\n");
                fprintf (stdout, "%s\n", filename);
                break;
            case 'd':
                dDetected = true;
                printf ("-d detected: should call dbg to make the dbg messages based on info from -u and -o\n");
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
    if (argv[1] == NULL)
    {
        fprintf (stderr,"error: no command line option given\n"); //fprintf is for errors only
        usage (argv [0]);
    }
}
void parseURL(char* link)
{
    //have to handle null link, have to handle if link is -o or anythingelse
    size_t linkLen = strlen(link);
    char urlLink[linkLen + 1];
    strcpy(urlLink, link);
    //"http://www.example.org/index/readme.html";
    char hostname[strlen(urlLink)/2]; //give a length for the hostname as half of the url
    char web_file[strlen(urlLink)/2];  //give a length for the web_file as half of the url
    char httpForm[] = "http://";
    if (strncasecmp(urlLink, httpForm, 7) == 0) 
    {
        //check if the link starts with "http://" and if so strncasecmp should return 0 for equality
        char *after_httpForm = urlLink + 7; //set pointer to the 7th element, which is the first char
        //of hostname
        char *slash = strchr(after_httpForm, '/'); //set pointer to the slash before filename
        if (slash != NULL) {
            size_t hostname_Length = slash - after_httpForm; //finds the length of the hostname
            strncpy(hostname, after_httpForm, hostname_Length); //copies from after_httpForm the hostname using
            //the hostname's length to hostname string
            hostname[hostname_Length] = '\0'; //add null terminator
            strcpy(web_file, slash); 
        } else {
            // No '/' found after "http://", assume the entire string is the hostname
            strcpy(hostname, after_httpForm);
            strcpy(web_file, "/");
        }

        // Print the extracted hostname and web file path
        printf("Hostname: %s\n", hostname);
        printf("Web File: %s\n", web_file);
    } 
    else{
        fprintf(stderr, "Error: url does not begin with https:// \n");
    }
}
int main (int argc, char *argv [])
{
    //-o file will be initiated if -u is legit and response is ok 200
    //should do dqr operations last after sending sockets (?)
    //once finished seeing -u and -o then send the request
    parseargs(argc, argv);
    printf("%s\n", order);
    if(uDetected && oDetected)
    {
        //continue with socket, 
        //then dpr seq printouts (if seq char is greater than 3, too much argument might need to throw error)
        parseURL(url);
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
