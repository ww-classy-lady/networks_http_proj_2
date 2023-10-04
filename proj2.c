// (i) Wendy Wu
// (ii) wxw428
// (iii) proj2.c
// (iv) 9/25/2023
// (v) Description:
// This is the code program containing everything needed to run command, 
// send_receive_socket, store socket data into a file, and printing d, q, r options.
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h> 
#include <strings.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PROTOCOL "tcp" //protocol is TCP
#define BUFLEN 1024 //temporary size 1024 for char []
#define CODE_SIZE 4 //the size of the code string will be 4 (including null terminator)
#define CODE_LENGTH 3 //the length of the code, 200, 301, 404 will be 3 characters long
#define OK 200 //the ok code
#define PORT 80 //already define http port to be 80 used in send_receive_sockets()
//numbers not 0 or 1 is define!
FILE *file; //the file storing binary data
FILE *sp; //sp is the socket descriptor file used for getting the http response header and storing binary data into file
bool uDetected = false; //initially false as u is not detected
bool oDetected = false; //initially false as o is not detected
bool dDetected = false; //initially false as d is not detected
bool qDetected = false; //initially false as q is not detected
bool rDetected = false; //initially false as r is not detected
bool need_to_get_more_response = true; //will be true once all messages are read
char* url = NULL; //the url after -u
bool urlWrong = false; 
char* filename = NULL; //the output file name after -o
char* hostname = NULL; //hostname that will be filled in parseURL
char* web_file = NULL;  //webfile that will be filled in in web_file
char httpForm[] = "http://"; //used in parseURL to get hostname and web_file
char *httpResponseHeader = NULL; //http Response header 
char* rn = "\r\n"; //used for printing and adding INC to r 
char* code = NULL; //used to store the response code and the null terminator
char* modified_header = NULL; //used in printR to build the modified http response header 
bool non200 = false; //initially set non200 to false, if the code is not 200, then set this to true
int sd; //socket descriptor

// void usage: default usage function showing user how to use the program
// modified from sample.c (parseargs, main, and sockets also modified from given code for proj2)
void usage(char *progname) 
{
    fprintf(stderr,"%s ./proj2 -u URL [-d] [-q] [-r] -o filename\n", progname);
    fprintf(stderr,"   -d    DBG: show DBG statements\n");
    fprintf(stderr,"   -q    OUT: Makes the http request message\n");
    fprintf(stderr,"   -r  INC: http response \n");
    exit(1);
}

//void parseargs: parse the command line args that is -u, -o, -d, -q, -r, or unknown args
void parseargs(int argc, char *argv []) 
{
    int opt;
    while((opt = getopt (argc, argv, "u:o:dqr")) != -1) //only options are -u url -o filename and -d, -q, -r 
    {
        switch(opt)
        {
            case 'u':
                uDetected = true; //-u is detected, only if there is a url after it
                url = optarg; //set the url to be parsed in parseURL to the next string after -u
                break;
            case 'o': 
                oDetected = true; //-o is detected, only if there is a filename after it
                filename = optarg; //set the url to be parsed in parseURL to the next string after -u
                break;
            case 'd':
                dDetected = true; //-d is detected
                break;
            case 'q':
                qDetected = true; //-q is detected
                break;
            case 'r':
                rDetected = true; //-r is detected
                break;
            case '?':
                fprintf(stderr, "ERROR: unknown command line arguments entered. Please try again.\n./");
                //anything that is not -u, -o, -d, -q, or -r will prompt program to give an error message and terminate program
                exit(0);
            default:
                usage(argv [0]); 
        }
    }
    if(argv[1] == NULL) //default print message when nothing is entered yet.
    {
        fprintf (stderr,"error: no command line option given\n"); //fprintf is for errors only
        usage (argv [0]); //gives the program usage details in the usage function above
    }
}

// void parseURL: parse the url into hostname and web_file name used for building the http request
void parseURL(char* link)
{
    if(strncasecmp(link, httpForm, strlen(httpForm)) == 0) 
    {
        //check if the link starts with "http://" and if so strncasecmp should return 0 for equality
        char *after_httpForm = link + strlen(httpForm); //set pointer to right after http://, which is the first char of hostname
        char *slash = strchr(after_httpForm, '/'); //set pointer to the slash before web_file 
        if(slash != NULL) //if there exist a web_file after the valid hostname
        {
            size_t hostname_Length = slash - after_httpForm; //finds the length of the hostname
            hostname = (char*)malloc(hostname_Length+1); //allocate space for char* hostname, +1 is for null terminator
            strncpy(hostname, after_httpForm, hostname_Length); //copies hostname to char by copying hostname_Length number of char after http://
            hostname[hostname_Length] = '\0'; //add null terminator
            size_t web_file_Length = strlen(slash);  //length of everything after the hostname including the slash
            web_file = (char*)malloc(web_file_Length + 1); //allocate space for char* web_file
            strcpy(web_file, slash); //copies everything from slash till the end of link to web_file
        } 
        else{
            // No '/' found after "http://", assume the entire string is the hostname
            size_t hostname_length = strlen(after_httpForm); //from the pointer right after http://, we know the rest of the link should be hostname
            hostname = (char*)malloc(hostname_length+1); //allocate space
            strcpy(hostname, after_httpForm); //copies hostname from link to char* hostname 
            hostname[hostname_length] = '\0'; //add null terminator
            size_t webfile_length = strlen("/"); //if no '/' is provided, we will just say web_file is /
            web_file =(char*)malloc(webfile_length + 1); //allocate the length of \ to web_file
            strcpy(web_file, "/");
            web_file[webfile_length] = '\0'; //add null terminator
        }
    } 
    else{
        urlWrong = true; //if link does not begin with http:// print out error message
        fprintf(stderr, "Error: url does not begin with http:// \n");
    }
}

// char* buildRequest: takes hostname and web_file name and compose the http request to send through send_receive_Sockets
char* buildRequest(char *host_name, char *web_file) 
{
    char *result = (char*)malloc(BUFLEN); //first allocate space for the returning result
    snprintf(result, BUFLEN,
            "GET %s HTTP/1.0\r\n"
            "Host: %s\r\n"
            "User-Agent: CWRU CSDS 325 SimpleClient 1.0\r\n"
            "\r\n", web_file, host_name); //store the request with web_file and host_name arguments embedded in the message and save it to result
    return result; //return the completed http request
}

// void send_receive_Sockets: create socket, send socket, use socket descriptor to parse http response header, get the 200/non200 code, and 
// store binary data into file if code is 200.
void send_receive_Sockets(char *host_name, char *web_file, char *filename)
{    //read and write to sockets?
    char *temp_hostname[] = {host_name}; //store the host_name into a temporary string
    struct sockaddr_in sin;
    struct hostent *hinfo;
    struct protoent *protoinfo;
    char *request;
    ssize_t sendReturn;

    /* lookup the hostname */
    hinfo = gethostbyname(temp_hostname [0]); //set the struct hostent to char *pointer type since already have the hostname from parseURL
    if(hinfo == NULL)
        printf("cannot find hostname\n");

    /* set endpoint information */
    memset ((char *)&sin, 0x0, sizeof (sin));
    sin.sin_family = AF_INET;
    sin.sin_port = htons (PORT); //we know port is 80 for http request and response 
    memcpy ((char *)&sin.sin_addr,hinfo->h_addr,hinfo->h_length);

    if((protoinfo = getprotobyname (PROTOCOL)) == NULL)
    {
        printf("cannot find protocol information for %s\n", PROTOCOL);
    }
    /* allocate a socket */
    /*   would be SOCK_DGRAM for UDP */
    sd = socket(PF_INET, SOCK_STREAM, protoinfo->p_proto);
    if(sd < 0)
    {
        printf("cannot create socket\n");
    }
    /* connect the socket */
    if(connect(sd, (struct sockaddr *)&sin, sizeof(sin)) < 0)
    {
        printf("cannot connect\n");
    }
    /* build an http request*/
    request = buildRequest(host_name,web_file); //the char* http request that is built through buildRequst function
    /* send an http request*/
    sendReturn = send(sd, request, strlen(request), 0); //send http request and store the result of the send operation in sendReturn
    if(sendReturn == -1) 
    {
        fprintf(stderr, "Error: send has error\n");
    }
    sp = fdopen(sd, "r"); //open the socket descriptor file in read mode
    size_t totalSize = BUFLEN; //temporarily set the totalSize of the http Response header to BUFLEN
    httpResponseHeader = (char *)malloc(BUFLEN); //allocate size BUFLEN for httpResponseHeader
    httpResponseHeader[0] = '\0'; //null terminate httpResponseHeader
    /* Get the header by getting piece by piece and temporarily store it in store, then transfer to httpResponse Header*/
    if(sp != NULL)  //as long as sp has the response, parse the header annd determine if need to store data or not
    {
        /* parse the header using fgets */
        char store[BUFLEN]; //temporary storage place for pieces of data
        store[0] = '\0'; //null terminate the storage to avoid errors
        while(strcasecmp(fgets(store, sizeof(store), sp), "\r\n") != 0) //keep getting strings as long as we don't reach the last blank line before binary data
        {
            if(strlen(httpResponseHeader) + strlen(store) >= totalSize) //if the existing header plus the new string exceed allocated space, allocate more
            {
                totalSize = totalSize + BUFLEN; //allocate another BUFLEN size of space to store string
                httpResponseHeader = (char *)realloc(httpResponseHeader, totalSize); //realloc more space
            } 
            strcat(httpResponseHeader, store); //store the string temporarily stored to httpResponseHeader
        }
        /* parse the code from string httpResponseHeader */
        code = (char *)malloc(CODE_SIZE); 
        code[0]= '\0';
        char *after_space = strchr(httpResponseHeader, ' ') + 1; //starting at the position after the first space we will have the 3 digit code
        strncpy(code, after_space, CODE_LENGTH); //extract 3 characters from the rest of httpResponseHeader starting at the first code digit
        //copy it over to code
        if(OK != atoi(code)) //convert code to int and see if it is NOT 200
        {
            non200 = true; //not 200 so that we can print error message in main
        }
        else{
            //200 land
            //create the file and put binary data in it
            file = fopen(filename, "w"); //put file in write mode
            if(file == NULL)
            {
                printf("Error with opening file");
            }
            char buffer[BUFLEN]; //temporary buffer to store binary data pieces
            size_t bufferSize = sizeof(buffer);
            size_t bytesRead = 0; //keep track of the bytesRead
            while(!feof(sp)) //as long as we are not at the end of the file for socket descriptor, binary data need to be continuous read and written
            {
                bytesRead = fread(buffer, 1, bufferSize, sp); //read one byte of data from socket descriptor file
                if(bytesRead > 0) //got info to write because bytesRead is greater than 0
                {
                    fwrite(buffer, 1, bytesRead, file); //write 1 byte of data to file
                }
            }
            fclose(file); //All binary data have been written to the file with filename as the name. Close file
        }
    }
    else{
        fprintf(stderr, "ERROR: Socket is NULL."); //if sp == null
    }
    /* free some spaces used in this operation*/
    free(code); //code not used anymore as if non-200 code, we have non200 as the var to print error in main 
    close(sd); //close socket
}

// void printR: -r functionality.
// modify the httpResponseHeader parsed from socket code above and add INC: to the beginning of every line and prints it
void printR(char *httpResponseHeader, char *rn)
{
    //calculate the number of lines in header that ends with \r\n
    int numLines = 0;
    char* header_copy = strdup(httpResponseHeader); //make a copy of the original header
    char *pointer_of_rn = strstr(header_copy, rn); //start to point the first occurrence of rn from the header
    while(pointer_of_rn != NULL) //while we can still find the next \r\n in the current scope of header
    {
        numLines++; //increment the number of lines
        pointer_of_rn += strlen(rn); //start at the next line
        pointer_of_rn = strstr(pointer_of_rn, rn); //find the rn in the rest of the header after moving pointer from previous line to the next line
    }
    size_t new_header_length = strlen(header_copy) + strlen("INC: ") + numLines * strlen("INC: "); //allocate INC: one more time for the last line without \r\n
    modified_header = (char *)malloc(new_header_length + 1); //allocate the total amount of space needed for the response header plus all INC: 
    modified_header[0] = '\0'; //set null terminator in place for the empty string
    char *line_without_rn = strtok(header_copy, rn); //set the first middle of the line to be before rn
    while(line_without_rn !=NULL)
    {
        strcat(modified_header, "INC: "); //concat INC: to header
        strcat(modified_header, line_without_rn); //add middle of the line
        strcat(modified_header, rn); //end with \r\n
        line_without_rn = strtok(NULL, rn); //continue to get the next line without rn substring from httpResponseHeader (same string)
    }
    printf("%s", modified_header); //print the header after appending INC: to the start of everyline
    free(header_copy); //free the original copy of the httpheader
    free(modified_header); //free the modified copy of the http response header
}

// void printD: -d functionality. Prints DBG lines with hostname, web_file, and filename embedded.
void printD(char *copy_hostname, char *copy_web_file, char *copy_filename)
{
    printf("DBG: host: %s\n", copy_hostname); 
    printf("DBG: web_file: %s\n", copy_web_file);
    printf("DBG: output_file: %s\n", copy_filename);
}

//void printQ: -q functionality. Prints OUT: lines with the http request without blank line that terminates the HTTP request
void printQ(char *copy_web_file, char *copy_hostname)
{
    //difference from the helper that creates the actual HTTP request
    //sends to the web server: the blank line that terminates the HTTP request must be excluded from the -q output!
    printf("OUT: GET %s HTTP/1.0\n", copy_web_file);
    printf("OUT: Host: %s\n", copy_hostname);
    printf("OUT: User-Agent: CWRU CSDS 325 SimpleClient 1.0\n");
}

//main: sequentially calls the helper functions to parseargs, parseURL, then -d, send sockets, -q, -r in order.
int main (int argc, char *argv [])
{
    parseargs(argc, argv); //parse the arguments
    if(uDetected && oDetected) //if there exists both valid -u url and -o filename in the command line arguments.
    {
        parseURL(url); //parse url to hostname, web_file
        
        if(urlWrong == true)
        {
            //don't do anything because already printed a message in parseURL but just to keep track?
        }
        else{
            //print d, q, r
            if(dDetected)
            {
                printD(hostname, web_file, filename);
            }
            send_receive_Sockets(hostname, web_file, filename); //send and receive sockets
            if(qDetected)
            {
                printQ(web_file, hostname);
            }
            if(rDetected)
            {
                printR(httpResponseHeader, rn);
            }
            if(non200)
            {
                printf("ERROR: non-200 response code\r\n");
            }
        }
    }
    else if(uDetected)
    {
        //no -o
        //ask to provide -o
        fprintf(stderr, "ERROR: -o is missing from command line arguments. Please try again with -o.\n");
    }
    else if(oDetected)
    {
        //no u
        //ask to provide -u
        fprintf(stderr, "ERROR: -u is missing from command line arguments. Please try again with -u.\n");
    }
    else{
        //honestly nothing should happen in this else but just in case there is an edge case. Here is a line to tell user to try again.
        fprintf(stdout, "This line shouldn't be print out but in case something happens unintentionally, please try again.\n");
        exit(1);
    }
    exit(0);
    
}
