#include <stdio.h>
#include "csapp.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

#define LRU_MAX 8192
#define CACHE_ITEMS 10
#define BUF_SIZE (1 << 14)

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";

void doit(int connfd);
void parse_uri(char *uri, char *hostname, char *path, int *port);
void build_http_header(char *http_header, char *hostname, char *path, int port, rio_t *client_rio);
int connect_server(char *hostname, int port, char *http_header);

void *thread(void *vargp);

// Cache Functions
void cache_init();
int cache_find(char *url);
int evict();
void LRU(int idx);
void cache_uri(char *uri,char *buf);
void readLock(int idx);
void readUnlock(int idx);
void writeLock(int idx);
void writeUnlock(int idx);

typedef struct {
    char cache_obj[MAX_OBJECT_SIZE];
    char cache_url[MAXLINE];
    int LRU;
    int isEmpty;

    sem_t wmutex;       // lock access to cache
    int rc;             // read count
    sem_t rcmutex;      // lock read count
    int wc;             // write count
    sem_t wcmutex;      // lock write count
    sem_t queue;
} cache_item;

typedef struct {
    cache_item item[CACHE_ITEMS];  // 10 blocks
    int num; // ?
} Cache;

Cache cache; // Global cache

int main(int argc, char **argv) {
    int listenfd, connfd;
    socklen_t clientlen;
    char hostname[MAXLINE], port[MAXLINE];
    struct sockaddr_storage clientaddr;

    pthread_t tid; // thread id for concurrency

    if(argc != 2){
        fprintf(stderr, "usage :%s <port> \n", argv[0]);
        exit(1);
    }

    cache_init(); // initialize cache

    Signal(SIGPIPE, SIG_IGN); // ignore SIGPIPE

    listenfd = Open_listenfd(argv[1]); // listen

    while(1) {
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA *) &clientaddr, &clientlen); // accept

        Getnameinfo((SA *) &clientaddr, clientlen, hostname, MAXLINE, port, MAXLINE, 0);

        // print connection message
        printf("Accepted connection from (%s %s).\n", hostname, port);

        // create thread
        Pthread_create(&tid, NULL, thread, (void *) (long long) connfd);
    }
    return 0;
}

// Thread routine
void *thread(void *vargp) {
    int connfd = (int) (long long) vargp;
    Pthread_detach(pthread_self()); // detach to avoid memory leak

    doit(connfd); // handle clients

    Close(connfd); // do not forget to close
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

    char url_store[BUF_SIZE + 1];
    strncpy(url_store, uri, BUF_SIZE); // copy original url
    url_store[BUF_SIZE] = '\0';

    // If not GET method, return
    if(strncasecmp(method, "GET", 3) != 0) return;

    // Is the uri in cache?
    int ci; // cache index
    if((ci = cache_find(url_store)) != -1) {
        // cache hit
        readLock(ci);
        Rio_writen(connfd, cache.item[ci].cache_obj, strlen(cache.item[ci].cache_obj));
        readUnlock(ci);
        LRU(ci); // update
        return;
    }

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
    char cbuf[MAX_OBJECT_SIZE]; // buffer to store in cache
    int buf_size = 0;
    while((n = Rio_readlineb(&server_rio, buf, MAXLINE)) != 0) {
        buf_size += n;
        if(buf_size < MAX_OBJECT_SIZE) strncat(cbuf, buf, n); // !
        Rio_writen(connfd, buf, n);
    }

    Close(serverfd); // Must close server fd

    // store in cache
    if(buf_size < MAX_OBJECT_SIZE) cache_uri(url_store, cbuf);
}

void build_http_header(char *http_header, char *hostname, char *path, int port, rio_t *client_rio) {
    char buf[MAXLINE], request_hdr[MAXLINE], other_hdr[MAXLINE], host_hdr[MAXLINE];

    // Print request line to request header
    sprintf(request_hdr, "GET %s HTTP/1.0\r\n", path);

    // Get other request headers
    int n;
    while((n = Rio_readlineb(client_rio, buf, MAXLINE)) > 0) {
        if(!strncmp(buf, "\r\n", 2)) break; // End of file

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
    char *pos = strstr(uri, "//");

    pos = (pos != NULL) ? (pos + 2) : uri;

    // does port exist?
    char *pos2 = strstr(pos, ":");
    if(pos2) {
        *pos2 = '\0';
        sscanf(pos, "%s", hostname); // read hostname
        sscanf(pos2 + 1, "%d%s", port, path); // read port and path
    } else {
        pos2 = strstr(pos, "/"); // path
        if(pos2) {
            *pos2 = '\0';
            sscanf(pos, "%s", hostname); // read hostname
            *pos2 = '/';
            sscanf(pos2, "%s", path); // read path
        } else sscanf(pos, "%s", hostname);
    }
    return;
}

// Implement Cache Function
void cache_init() {
    cache.num = 0;
    int i = 0;
    for(; i < CACHE_ITEMS; ++i) {
        cache_item *item = &cache.item[i];
        item -> LRU = 0;       // not recently used
        item -> isEmpty = 1;   // empty
        item -> rc = 0;        // read count 0
        item -> wc = 0;        // write count 0

        // initialize semaphore
        Sem_init(&item -> wmutex, 0, 1);
        Sem_init(&item -> rcmutex, 0, 1);
        Sem_init(&item -> wcmutex, 0, 1);
        Sem_init(&item -> queue, 0, 1);
    }
}

void readLock(int idx) {
    cache_item *item = &cache.item[idx];

    P(&item -> queue);
    P(&item -> rcmutex);
    item -> rc++;

    if(item -> rc == 1)
        P(&item -> wmutex);

    V(&item -> rcmutex);
    V(&item -> queue);
}

void readUnlock(int idx) {
    cache_item *item = &cache.item[idx];

    P(&item -> rcmutex);
    item -> rc--;

    if(item -> rc == 0)
        V(&item -> wmutex);

    V(&item -> rcmutex);
}

void writeLock(int idx) {
    cache_item *item = &cache.item[idx];

    P(&item -> wcmutex);
    item -> wc++;

    if(item -> wc == 1)
        P(&item -> queue);

    V(&item -> wcmutex);
    P(&item -> wmutex);
}

void writeUnlock(int idx) {
    cache_item *item = &cache.item[idx];

    V(&item -> wmutex);
    P(&item -> wcmutex);
    item -> wc--;

    if(item -> wc == 0)
        V(&item -> queue);

    V(&item -> wcmutex);
}

// find url in cache
int cache_find(char *url) {
    int i = 0;
    for(; i < CACHE_ITEMS; ++i) {
        readLock(i);
        if((cache.item[i].isEmpty == 0) &&
           (strncmp(url, cache.item[i].cache_url, BUF_SIZE) == 0)) break;
        readUnlock(i);
    }

    if(i >= CACHE_ITEMS) return -1; // Cache miss
    return i;
}

// find object to evict
int cache_evict() {
    int min = LRU_MAX;
    int minidx = 0;
    int i = 0;
    for(; i < CACHE_ITEMS; ++i) {
        readLock(i);
        cache_item *item = &cache.item[i];
        if(item -> isEmpty) { // if empty block exists, return it
            minidx = i;
            readUnlock(i);
            break;
        }
        if(item -> LRU < cache.item[minidx].LRU) { // smallest LRU
            minidx = i;
        }
        readUnlock(i);
    }

    return minidx;
}

// update status of LRU
void LRU(int idx) {
    writeLock(idx);
    cache.item[idx].LRU = LRU_MAX;
    writeUnlock(idx);

    int i = 0;
    for(; i < CACHE_ITEMS; ++i) {
        if(i == idx) continue;
        writeLock(i);
        if(!cache.item[i].isEmpty && i != idx) {
            cache.item[i].LRU--;
        }
        writeUnlock(i);
    }
}

// cache uri and content in the cache
void cache_uri(char *uri, char *buf) {
    int idx = cache_evict();

    writeLock(idx);

    cache_item *item = &cache.item[idx];
    strcpy(item -> cache_obj, buf);
    strcpy(item -> cache_url, uri);
    item -> isEmpty = 0;

    writeUnlock(idx);

    LRU(idx);
}
