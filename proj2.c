//./proj2 to run
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h> 
#include <strings.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
//./proj2 -u URL [-d] [-q] [-r] -o filename

#define ARG_URL 0x1 //u
#define ARG_DBG 0x2 //d
#define ARG_HTTPReq 0x4 //q
#define ARG_HTTPRes 0x6 //r
#define ARG_CONT 0x8 //o
#define PROTOCOL "tcp"
#define BUFLEN 1024
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
//TO DO: free unneeded temp varis
bool uDetected = false; //initially false as u is not detected
bool oDetected = false; //initially false as o is not detected
bool dDetected = false; //initially false as d is not detected
bool qDetected = false; //initially false as q is not detected
bool rDetected = false; //initially false as r is not detected
bool need_to_get_more_response = true; //will be true once all messages are read
char* url = NULL; //the url after -u
bool urlWrong = false;
char* filename = NULL; //the output file name after -o
char order[3] = "";
char* hostname = NULL; //hostname that will be filled in parseURL
char* web_file = NULL;  //webfile that will be filled in in web_file
char httpForm[] = "http://";
int port = 80; //already define http port to be 80 used in send_receive_sockets()
char buffer[BUFLEN]; //buffer string used to store http response and then used to parse the response
char* httpResponseHeader = NULL; //http Response header 
char* responseBody = NULL; //data
char rnrn[] = "\r\n\r\n";
char* rn = "\r\n"; //used for printing and adding INC to r 
char* code = (char*)malloc(4); //used to store the response code and the null terminator
int codeLength = 3;
char* modified_header = NULL;
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
char* buildRequest(char *host_name, char *web_file)
{
    //build the pointer string http request for socket send and receive
    //char *getFirst = "GET ";
    //char *getLater = " HTTP/1.0\r\n";
    //char *hostFirst = "Host: ";
    //char *hostLater = "\r\n";
    //char *result =  "User-Agent: CWRU CSDS 325 SimpleClient 1.0\r\n";
    //size_t total = strlen(getFirst) + strlen(web_file) + strlen(getLater) 
    //+ strlen(hostFirst) + strlen(host_name) + strlen(hostLater) 
    //+ strlen(result) + strlen(hostLater);
    /*char *request = (char *)malloc(total+1);
    strcpy(request, getFirst);
    strcat(request, web_file);
    strcat(request, getLater);
    strcat(request, hostFirst);
    strcat(request, host_name);
    strcat(request, hostLater);
    strcat(request, result);
    strcat(request, hostLater); */
    char *result = (char*)malloc(BUFLEN);
    snprintf(result, BUFLEN,
            "GET %s HTTP/1.0\r\n"
            "Host: %s\r\n"
            "User-Agent: CWRU CSDS 325 SimpleClient 1.0\r\n"
            "\r\n", web_file, host_name);
    return result;

}
void send_receive_Sockets(char *host_name, char *web_file)
{
    char *temp_hostname[] = {host_name};
    struct sockaddr_in sin;
    struct hostent *hinfo;
    struct protoent *protoinfo;
    char *request;
    int sd, ret;
    ssize_t sendReturn;
    size_t bufferSize;
    //usage, not really relevant right now
    /*if (argc != REQUIRED_ARGC)
        usage (argv [0]);*/

    /* lookup the hostname */
    hinfo = gethostbyname(temp_hostname [0]); //set the struct hostent to char *pointer type since already have the hostname from parseURL
    if (hinfo == NULL)
        printf("cannot find hostname\n");

    /* set endpoint information */
    memset ((char *)&sin, 0x0, sizeof (sin));
    sin.sin_family = AF_INET;
    sin.sin_port = htons (port); //we know port is 80 as an int for http request and response 
    memcpy ((char *)&sin.sin_addr,hinfo->h_addr,hinfo->h_length);

    if ((protoinfo = getprotobyname (PROTOCOL)) == NULL)
        printf("cannot find protocol information for %s\n", PROTOCOL);

    /* allocate a socket */
    /*   would be SOCK_DGRAM for UDP */
    sd = socket(PF_INET, SOCK_STREAM, protoinfo->p_proto);
    if(sd < 0)
        printf("cannot create socket\n");

    /* connect the socket */
    if(connect(sd, (struct sockaddr *)&sin, sizeof(sin)) < 0)
        printf("cannot connect\n");
    /* build an http request*/
    request = buildRequest(host_name,web_file);
    /* send an http request*/
    sendReturn = send(sd, request, strlen(request), 0);
    if(sendReturn == -1)
    {
        fprintf(stderr, "Error: send has error\n");
    }
    /* snarf whatever server provides and print it */

    memset(buffer,0x0,BUFLEN);
    ret = read(sd, buffer, BUFLEN -1);
    
    if(ret < 0)
    {
        printf("reading error\n");
    }
    //after all data have been read and stored, close the connection
    
    //fprintf (stdout,"%s\n", buffer);
    //httpResponseHeader = strstr(buffer, "\r\n\r\n");
    //httpResponseHeader = strstr(buffer, "\r\n\r\n");
    //httpResponseHeader ='\0'; //null terminate header
    //strcpy(resHeader, httpResponseHeader);
    /*bufferSize = sizeof(buffer) - 1;
    while(need_to_get_more_response)
    {
        ret = read(sd, buffer, bufferSize);
        if(ret < 0)
        {
            printf("reading error\n");
            break;
        }
        else if(ret == 0)
        {
            //finished connection and all response received
            break;
        }
        bufferSize+= ret; //ad the size received by ret to the total bufferSize in order to reallocate more space for buffer
        buffer = (char *)realloc(buffer, bufferSize);

    }*/
    /* parse the http response */
    //Find the header
    char *lastofHeader= strstr(buffer, rnrn);
    size_t headLength = lastofHeader - buffer; //gets the length of the header that is between the start of the buffer string and the start of \r\n\r\n
    httpResponseHeader = (char*)malloc(headLength+1);
    strncpy(httpResponseHeader, buffer, headLength); //copies header (based on its length) from buffer to httpResponseHeader
    httpResponseHeader[headLength] = '\0'; //null terminate
    /* process the response body */
    char *after_rnrn = lastofHeader + strlen(rnrn);
    size_t bodyLength = strlen(after_rnrn); //calculate the string's length starting at the pointer located right after \r\n\r\n
    responseBody = (char*)malloc(bodyLength+1);
    strcpy(responseBody, after_rnrn);
    responseBody[bodyLength] = '\0';
    /* process the http response code from the header*/       
    char *after_space = strchr(httpResponseHeader, ' ') + 1; //starting at the position after the first space
    strncpy(code, after_space, codeLength); //extract 3 characters
    code[sizeof(code)] = '\0';

    /* close & exit */
    close (sd);
}
void printR(char *httpResponseHeader, char *rn)
{
    //calculate the number of lines in header that ends with \r\n
    int numLines = 0;
    char* header_copy = strdup(httpResponseHeader); //make a copy of header
    char *pointer_of_rn = strstr(header_copy, rn);
    while(pointer_of_rn != NULL)
    {
        numLines++;
        pointer_of_rn += strlen(rn); //start at the next line
        pointer_of_rn = strstr(pointer_of_rn, rn); //find  the rn in the rest of the head after moving pointer from prev ;ine to the next line
    }
    size_t new_header_length = strlen(header_copy) + strlen("INC: ") + numLines * strlen("INC: "); //allocate INC: one more time for the last line without \r\n
    modified_header = (char *)malloc(new_header_length + 1);
    modified_header[0] = '\0';
    char *line_without_rn = strtok(header_copy, rn);
    while(line_without_rn !=NULL)
    {
        strcat(modified_header, "INC: ");
        strcat(modified_header, line_without_rn);
        strcat(modified_header, rn);
        line_without_rn = strtok(NULL, rn); //cobtinue to get the next line without rn substring from httpResponseHeader (same string)
    }
    printf("%s", modified_header);
    free(modified_header);

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
    //should do dqr operations last after sending sockets (?) yea
    //once finished seeing -u and -o then send the request
    parseargs(argc, argv);
    //printf("%s\n", order);
    if(uDetected && oDetected)
    {
        //continue with socket, 
        //then dpr seq printouts (if seq char is greater than 3, too much argument might need to throw error)
        parseURL(url); //parse url to hostname, web_file
        //send and receive sockets
        send_receive_Sockets(hostname, web_file);
        //fprintf (stdout,"%s\n",buffer); //works, response saved to a limited size buffer
        //test -d
        //free(buffer);
        printR(httpResponseHeader, rn);
        if(dDetected && urlWrong == false)
        {
            //printD(hostname, web_file, filename);
        }
        else if(qDetected && urlWrong == false)
        {
            //printQ(web_file, hostname);
        }
        else
        {
            //printf("d is not detected or url is wrong\n");
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
