/**
 *  Jiazi Yi
 *
 * LIX, Ecole Polytechnique
 * jiazi.yi@polytechnique.edu
 *
 * Updated by Pierre Pfister
 *
 * Cisco Systems
 * ppfister@cisco.com
 *
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

/* extra includes */
#include <fcntl.h>
#include <netinet/tcp.h>

#include "url.h"
#include "wgetX.h"

#define MAXRCVLEN 10*1000*1000 + CHUNK_SIZE // 1MB
#define MAX_URL_LEN 500
#define CHUNK_SIZE 512

int main(int argc, char* argv[]) {
    url_info info;
    const char * file_name = "received_page";
    if (argc < 2) {
	    fprintf(stderr, "Missing argument. Please enter URL.\n");
	    return 1;
    }

    char *url = argv[1];

    // Get optional file name
    if (argc > 2) {
	    file_name = argv[2];
    }

    // First parse the URL
    int ret = parse_url(url, &info);
    if (ret) {
	    fprintf(stderr, "Could not parse URL '%s': %s\n", url, parse_url_errstr[ret]);
	    return 2;
    }

    //If needed for debug
    printf("=========================\n");
    print_url_info(&info);
    printf("=========================\n");

    // Download the page
    struct http_reply reply;

    ret = download_page(&info, &reply);
    if (ret) {
	    return 3;
    }
    
    /* debug lines */
    // printf("After downloading page!\n");

    // Now parse the responses
    char *response = read_http_reply(&reply, 0);
    if (response == NULL) {
	    fprintf(stderr, "Could not parse http reply\n");
	    return 4;
    }

    // Write response to a file
    write_data(file_name, response, reply.reply_buffer + reply.reply_buffer_length - response);

    // Free allocated memory
    free(reply.reply_buffer);

    // Just tell the user where is the file
    fprintf(stderr, "the file is saved in %s. \n", file_name);
    return 0;
}

int download_page(url_info *info, http_reply *reply) {

    /*
     * To be completed:
     *   You will first need to resolve the hostname into an IP address.
     *
     *   Option 1: Simplistic
     *     Use gethostbyname function.
     *
     *   Option 2: Challenge
     *     Use getaddrinfo and implement a function that works for both IPv4 and IPv6.
     *
     */

    /* Option 1 - saw that gethostbyname is obsolete */
    struct hostent *hp = gethostbyname(info->host);

    // Testing that the function succeeded
    if (hp == NULL) {
        printf("Error in gethostbyname(%s) \n", info->host);
        return 1;
    }

    /* for debug */
    printf("h_name = %s\n", hp->h_name);


    /*
     * To be completed:
     *   Next, you will need to send the HTTP request.
     *   Use the http_get_request function given to you below.
     *   It uses malloc to allocate memory, and snprintf to format the request as a string.
     *
     *   Use 'write' function to send the request into the socket.
     *
     *   Note: You do not need to send the end-of-string \0 character.
     *   Note2: It is good practice to test if the function returned an error or not.
     *   Note3: Call the shutdown function with SHUT_WR flag after sending the request
     *          to inform the server you have nothing left to send.
     *   Note4: Free the request buffer returned by http_get_request by calling the 'free' function.
     *
     */


    /* Part of the code taken from
        https://forums.codeguru.com/showthread.php?517963-sending-an-http-GET-request-in-C
    */

    struct hostent *server = hp;
    struct sockaddr_in serveraddr;
    int tcpSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (tcpSocket < 0){
        printf("Error opening socket\n");
        return 1;
    }
    else{
        printf("Successfully opened socket\n");
    }

    // printf("server name = %s \n", server->h_name);
    unsigned int j = 0;
    while (server->h_addr_list[j] != NULL)
    {
        /* numbers-and-dots notation into binary form (in network byte order) */
        inet_ntoa(*(struct in_addr *)(server->h_addr_list[j]));
        j++;
    }

    /* overwrites sizeof(serveraddr) bytes to 0x00 */
    // bzero((char *)&serveraddr, sizeof(serveraddr));
    memset((char *) &serveraddr, 0, sizeof(serveraddr));

    serveraddr.sin_family = AF_INET;

    /* copies server->length bytes from server->addr_list[0] to &serveraddr.sin_addr.s_addr*/
    // bcopy((char *)server->h_addr_list[0], (char *)&serveraddr.sin_addr.s_addr, server->h_length);
    memcpy((char *)&serveraddr.sin_addr.s_addr, (char *)server->h_addr_list[0], server->h_length);

    serveraddr.sin_port = htons(info->port);

    if (connect(tcpSocket, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0){
        printf("Error Connecting\n");
        return 1;
    } else {
        printf("Successfully Connected\n");
    }
    /* end of code taken */

    char *request_buffer = http_get_request(info);

    if (write(tcpSocket, request_buffer, strlen(request_buffer)) < 0)
    {
        perror("write");
        free(request_buffer);

        return 1;
    }

    free(request_buffer);

    if (shutdown(tcpSocket, SHUT_WR) < 0) {
        perror("shutdown");
        return 1;
    }

    /*
     * To be completed:
     *   Now you will need to read the response from the server.
     *   The response must be stored in a buffer allocated with malloc, and its address must be save in reply->reply_buffer.
     *   The length of the reply (not the length of the buffer), must be saved in reply->reply_buffer_length.
     *
     *   Important: calling recv only once might only give you a fragment of the response.
     *              in order to support large file transfers, you have to keep calling 'recv' until it returns 0.
     *
     *   Option 1: Simplistic
     *     Only call recv once and give up on receiving large files.
     *     BUT: Your program must still be able to store the beginning of the file and
     *          display an error message stating the response was truncated, if it was.
     *
     *   Option 2: Challenge
     *     Do it the proper way by calling recv multiple times.
     *     Whenever the allocated reply->reply_buffer is not large enough, use realloc to increase its size:
     *        reply->reply_buffer = realloc(reply->reply_buffer, new_size);
     *
     *
     */

    /* Option 1 */
    
    // char *response_buffer = malloc(MAXRCVLEN * sizeof(char)); // allocates 1kB
    // ssize_t size_response;
    // if ((size_response = recv(tcpSocket, response_buffer, MAXRCVLEN, 0)) < 0) {
    //     perror("recv");
    //     return 1;
    // }

    /* Option 2 */
    char *response_buffer = malloc(CHUNK_SIZE * sizeof(char));
    ssize_t size_buffer = CHUNK_SIZE;

    ssize_t size_last_block = -1;
    ssize_t size_response = 0;
    
    while (size_last_block != 0) {
        if ((size_last_block = recv(tcpSocket, response_buffer + size_response, CHUNK_SIZE, 0)) < 0) {
            perror("recv");
            free(response_buffer);

            return 1;
        } 
        else if (size_last_block > 0) {
            if (size_buffer+CHUNK_SIZE > MAXRCVLEN) {
                printf("FILE is too big. Download aborted \n");
                free(response_buffer);

                return 1;
            }

            size_response += size_last_block;
            size_buffer += CHUNK_SIZE;
            response_buffer = (char *) realloc(response_buffer, size_buffer * sizeof(char));
        }
    }

    close(tcpSocket);

    /* debug lines */
    printf("Size of response: %d\n", (int) size_response);
    // printf("%s", response_buffer);

    reply->reply_buffer = response_buffer;
    reply->reply_buffer_length = size_response;

    return 0;
}

void write_data(const char *path, const char * data, int len) {
    /*
     * To be completed:
     *   Use fopen, fwrite and fclose functions.
     */

    /* Idea taken from
        https://www.tutorialspoint.com/c_standard_library/c_function_fopen.htm
        https://www.tutorialspoint.com/c_standard_library/c_function_fwrite.htm
    */

    FILE *fp;

    fp = fopen(path, "w");
    fwrite(data, 1, len, fp);
    fclose(fp);
}

char* http_get_request(url_info *info) {
    char * request_buffer = (char *) malloc(100 + strlen(info->path) + strlen(info->host));
    snprintf(request_buffer, 1024, "GET /%s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n",
	    info->path, info->host);
    return request_buffer;
}

char *next_line(char *buff, int len) {
    if (len == 0) {
	    return NULL;
    }

    char *last = buff + len - 1;
    while (buff != last) {
        if (*buff == '\r' && *(buff+1) == '\n') {
            return buff;
        }
        buff++;
    }
    return NULL;
}

char *read_http_reply(struct http_reply *reply, int depth) {

    if (depth >= 11) {
        /* too many redirects, give up */
        return NULL; 
    }

    // Let's first isolate the first line of the reply
    char *status_line = next_line(reply->reply_buffer, reply->reply_buffer_length);
    if (status_line == NULL) {
	    fprintf(stderr, "Could not find status\n");
	    return NULL;
    }
    *status_line = '\0'; // Make the first line is a null-terminated string

    // Now let's read the status (parsing the first line)
    int status;
    double http_version;
    int rv = sscanf(reply->reply_buffer, "HTTP/%lf %d", &http_version, &status);
    if (rv != 2) {
	    fprintf(stderr, "Could not parse http response first line (rv=%d, %s)\n", rv, reply->reply_buffer);
	    return NULL;
    }


    printf("Status: %d\n", status);
    if (status != 200) {
        if (300 <= status && status <= 399) {
            printf("==========================\n");
            printf("Redirecting connection\n");
            /* redirect */
            char *new_url = malloc(MAX_URL_LEN * sizeof(char));

            char *redirect_line = status_line + 2;
            int remaining_len = reply->reply_buffer_length - (redirect_line - reply->reply_buffer);

            while (1) {
                /* Isolates the line LOCATION: <url> */
                char *temp;
                if ((temp = next_line(redirect_line, remaining_len)) != NULL) {
                    *temp = '\0';
                } else {
                    printf("Error: could not redirect \n");
                    return NULL;
                }

                if (sscanf(redirect_line, "Location: %s", new_url) == 1) {
                    break;
                }
                
                /* url of redirect was not in this line */
                /* move to the next header line */
                redirect_line = temp + 2;
                remaining_len = reply->reply_buffer_length - (redirect_line - reply->reply_buffer);
            }

            url_info info;
            if (parse_url(new_url, &info)) {
                fprintf(stderr, "In redirect: could not parse url: %s\n", new_url);
                free(new_url);

                return NULL;
            }

            printf("=========================\n");
            print_url_info(&info);
            printf("=========================\n");

            free(reply->reply_buffer);
            if (download_page(&info, reply)) {
                fprintf(stderr, "Error when downloading page in redirect\n");
                free(new_url);

                return NULL;
            }

            free(new_url);

            return read_http_reply(reply, depth+1);
        }

	    fprintf(stderr, "Server returned status %d (should be 200)\n", status);
	    return NULL;
    }

    char *buf = status_line + 2;
    int len = reply->reply_buffer_length - (buf - reply->reply_buffer);

    // printf("len = %d\n", len);
    while (buf != NULL && !((*buf == '\r') && (*(buf+1) == '\n'))) {
        char *new_buf = next_line(buf, len) + 2;
        len -= (new_buf - buf);
        buf = new_buf;
    }

    if (buf == NULL) {
        printf("Error excluding the header lines\n");
        return NULL;
    }

    buf += 2;
    return buf;

    /*
     * To be completed:
     *   The previous code only detects and parses the first line of the reply.
     *   But servers typically send additional header lines:
     *     Date: Mon, 05 Aug 2019 12:54:36 GMT<CR><LF>
     *     Content-type: text/css<CR><LF>
     *     Content-Length: 684<CR><LF>
     *     Last-Modified: Mon, 03 Jun 2019 22:46:31 GMT<CR><LF>
     *     <CR><LF>
     *
     *   Keep calling next_line until you read an empty line, and return only what remains (without the empty line).
     *
     *   Difficult challenge:
     *     If you feel like having a real challenge, go on and implement HTTP redirect support for your client.
     *
     */
}

/* download binary image */