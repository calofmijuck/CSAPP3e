#include <stdio.h>
#include "csapp.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";

void doit(int connfd);
void parse_uri(char *uri, char *hostname, char *path, int *port);
void build_http_header(char *http_header, char *hostname, char *path, int port, rio_t *client_rio);
int connect_server(char *hostname, int port, char *http_header);

int main(int argc,char **argv) {
    int listenfd, connfd;
    socklen_t clientlen;
    char hostname[MAXLINE], port[MAXLINE];
    struct sockaddr_storage clientaddr;

    if(argc != 2){
        fprintf(stderr,"usage :%s <port> \n", argv[0]);
        exit(1);
    }

    Signal(SIGPIPE, SIG_IGN); // ignore SIGPIPE

    listenfd = Open_listenfd(argv[1]); // listen

    while(1) {
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA*) &clientaddr, &clientlen); // accept

        Getnameinfo((SA*) &clientaddr, clientlen, hostname, MAXLINE, port, MAXLINE, 0);

        // print connection message
        printf("Accepted connection from (%s %s).\n", hostname, port);

        doit(connfd); // handle clients sequentially

        Close(connfd); // do not forget to close
    }
    return 0;
}

// Services the client
void doit(int connfd) {
    int serverfd; // server file descriptor

    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
    char server_http_hdr[MAXLINE];

    // Arguments for requests
    char hostname[MAXLINE], path[MAXLINE];
    int port;

    // Use robust io
    rio_t rio, server_rio;

    Rio_readinitb(&rio, connfd);
    Rio_readlineb(&rio, buf, MAXLINE);

    // Read in client's request line
    sscanf(buf, "%s %s %s", method, uri, version);

    // If not GET method, return
    if(strncasecmp(method, "GET", 3) != 0) return;

    // Parse uri and get hostname, path, and port
    parse_uri(uri, hostname, path, &port);

    // Create http header
    build_http_header(server_http_hdr, hostname, path, port, &rio);

    // Connect to server
    serverfd = connect_server(hostname, port, server_http_hdr);
    if(serverfd < 0) {
        puts("Connection Failed");
        return;
    }

    Rio_readinitb(&server_rio, serverfd);

    // Write http header to server
    Rio_writen(serverfd, server_http_hdr, strlen(server_http_hdr));

    // Receive message from server and send to client
    size_t n;
    while((n = Rio_readlineb(&server_rio, buf, MAXLINE)) != 0) {
        Rio_writen(connfd, buf, n);
    }

    Close(serverfd); // Must close server fd
}

void build_http_header(char *http_header, char *hostname, char *path, int port, rio_t *client_rio) {
    char buf[MAXLINE], request_hdr[MAXLINE], other_hdr[MAXLINE], host_hdr[MAXLINE];

    // Print request line to request header
    sprintf(request_hdr,"GET %s HTTP/1.0\r\n", path);

    // Get other request headers
    int n;
    while((n = Rio_readlineb(client_rio, buf, MAXLINE)) > 0) {
        if(!strncmp(buf,"\r\n", 2)) break; // End of file

        // Host
        if(!strncasecmp(buf, "Host", 4)) {
            strncpy(host_hdr, buf, n);
            continue;
        }

        // Other Headers
        if(!strncasecmp(buf, "Connection", 10)
            && !strncasecmp(buf, "Proxy-Connection", 16)
            && !strncasecmp(buf, "User-Agent", 10)) {
            strncat(other_hdr, buf, n);
        }
    }

    if(!strlen(host_hdr)) {
        sprintf(host_hdr, "Host: %s\r\n", hostname);
    }

    sprintf(http_header, "%s%s%s%s%s%s%s", request_hdr,
            host_hdr, "Connection: close\r\n", "Proxy-Connection: close\r\n",
            user_agent_hdr, other_hdr, "\r\n");

    return;
}

// Connect to server
int connect_server(char *hostname, int port, char *http_header) {
    char portStr[100]; // port as str
    sprintf(portStr, "%d", port);
    return Open_clientfd(hostname, portStr);
}

// parse uri
void parse_uri(char *uri, char *hostname, char *path, int *port) {
    *port = 80; // default port is 80

    // find hostname
    char* pos = strstr(uri, "//");

    pos = (pos != NULL) ? (pos + 2) : uri;

    // does port exist?
    char* pos2 = strstr(pos, ":");
    if(pos2) {
        *pos2 = '\0';
        sscanf(pos2 + 1, "%d%s", port, path); // read port and path
    } else {
        pos2 = strstr(pos, "/"); // path
        if(pos2) {
            *pos2 = '/';
            sscanf(pos2, "%s", path); // read path
        }
    }
    sscanf(pos, "%s", hostname); // read hostname
    return;
}
