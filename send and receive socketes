void send_receive_Sockets(char *host_name, char *web_file, char *filename)
{
    char *temp_hostname[] = {host_name};
    struct sockaddr_in sin;
    struct hostent *hinfo;
    struct protoent *protoinfo;
    char *request;
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
    size_t current_buffer_size = BUFLEN;
    size_t current_total_size = 0;
    ret = 1; //initialized
    while(ret>0)
    {
        if(ret < 0)
        {
            fprintf(stderr, "reading error\n");
        }
        ret = read(sd, buffer + current_total_size, current_buffer_size - current_total_size -1);
        current_total_size += ret;
        if(current_total_size >= current_buffer_size - 1)
        {
            current_buffer_size *=2;
            char *bigger_buffer = (char *)realloc(buffer, current_buffer_size);

            if(bigger_buffer == NULL)
            {
                fprintf(stderr, "ERROR: cannot realloc more spaces");
                free(buffer);
            }
            buffer = bigger_buffer;
        }
    }
    /* parse the http response */

    //Find the header
    char *lastofHeader= strstr(buffer, rnrn);
    size_t headLength = lastofHeader - buffer; //gets the length of the header that is between the start of the buffer string and the start of \r\n\r\n
    httpResponseHeader = (char*)malloc(headLength+1);
    strncpy(httpResponseHeader, buffer, headLength); //copies header (based on its length) from buffer to httpResponseHeader
    httpResponseHeader[headLength] = '\0'; //null terminate

    /* process the http response code from the header*/      
    char *after_space = strchr(httpResponseHeader, ' ') + 1; //starting at the position after the first space
    strncpy(code, after_space, codeLength); //extract 3 characters
    code[sizeof(code)] = '\0'; 
    if(ok != atoi(code))
    {
        non200 = true;
    }
    else
    {
        /*FILE *sp = fdopen(sd, "rb");
        char tBuffer [BUFLEN];
        size_t res_bytes_read;
        file = fopen(filename, "wb");
        while((res_bytes_read = fread(tBuffer, 1, BUFLEN, sp)) > 0)
        {
            size_t bytes_written = fwrite(tBuffer, 1, res_bytes_read, file);

        }*/
        /* process the response body */
        /*char *after_rnrn = lastofHeader + strlen(rnrn);
        size_t bodyLength = strlen(after_rnrn); //calculate the string's length starting at the pointer located right after \r\n\r\n
        responseBody = (char *)malloc(bodyLength+1);
        strcpy(responseBody, after_rnrn);
        responseBody[bodyLength] = '\0';*/
    }
    /* close & exit */
    close (sd);
}
