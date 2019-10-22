#include <stdio.h>
#include "csapp.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

#define LRU_MAGIC_NUMBER 8192
#define CACHE_ITEMS 10
// #define LRU_MAX 8192
#define BUF_SIZE (1 << 14)

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";

void *thread(void *vargp);

void doit(int connfd);
void parse_uri(char *uri, char *hostname, char *path, int *port);
void build_http_header(char *http_header, char *hostname, char *path, int port, rio_t *client_rio);
int connect_endServer(char *hostname, int port, char *http_header);

/*cache function*/
void cache_init();
int cache_find(char *url);
int evict();
void LRU(int index);
void cache_uri(char *uri,char *buf);
void readLock(int i);
void readUnlock(int i);

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
    cache_item item[CACHE_ITEMS];  /*ten cache blocks*/
    int num;
} Cache;

Cache cache;

int main(int argc,char **argv) {
    int listenfd, connfd;
    socklen_t clientlen;
    char hostname[MAXLINE], port[MAXLINE];
    pthread_t tid;
    struct sockaddr_storage clientaddr;

    cache_init(); // initialize cache

    if(argc != 2){
        fprintf(stderr,"usage :%s <port> \n",argv[0]);
        exit(1);
    }
    Signal(SIGPIPE,SIG_IGN);
    listenfd = Open_listenfd(argv[1]);
    while(1){
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd,(SA *)&clientaddr,&clientlen);

        /*print accepted message*/
        Getnameinfo((SA*)&clientaddr,clientlen,hostname,MAXLINE,port,MAXLINE,0);
        printf("Accepted connection from (%s %s).\n",hostname,port);

        /*concurrent request*/
        Pthread_create(&tid,NULL,thread,(void *)connfd);
    }
    return 0;
}

/*thread function*/
void *thread(void *vargp){
    int connfd = (int)vargp;
    Pthread_detach(pthread_self());

    doit(connfd);

    Close(connfd);
}

/*handle the client HTTP transaction*/
void doit(int connfd)
{
    int end_serverfd;/*the end server file descriptor*/

    char buf[MAXLINE],method[MAXLINE],uri[MAXLINE],version[MAXLINE];
    char endserver_http_header [MAXLINE];

    /*store the request line arguments*/
    char hostname[MAXLINE],path[MAXLINE];
    int port;

    rio_t rio,server_rio;/*rio is client's rio,server_rio is endserver's rio*/

    Rio_readinitb(&rio,connfd);
    Rio_readlineb(&rio,buf,MAXLINE);
    sscanf(buf,"%s %s %s",method,uri,version); /*read the client request line*/

    char url_store[BUF_SIZE + 1];
    strncpy(url_store, uri, BUF_SIZE); // copy original url
    url_store[BUF_SIZE] = '\0';

    if(strcasecmp(method,"GET")){
        //printf("Proxy does not implement the method\n");
        return;
    }

    /*the uri is cached ? */
    int cache_index;
    if((cache_index=cache_find(url_store))!=-1){/*in cache then return the cache content*/
         readLock(cache_index);
         Rio_writen(connfd,cache.item[cache_index].cache_obj,strlen(cache.item[cache_index].cache_obj));
         readUnlock(cache_index);
         LRU(cache_index);
         return;
    }

    /*parse the uri to get hostname,file path ,port*/
    parse_uri(uri,hostname,path,&port);

    /*build the http header which will send to the end server*/
    build_http_header(endserver_http_header,hostname,path,port,&rio);

    /*connect to the end server*/
    end_serverfd = connect_endServer(hostname,port,endserver_http_header);
    if(end_serverfd<0){
        printf("connection failed\n");
        return;
    }

    Rio_readinitb(&server_rio,end_serverfd);

    /*write the http header to endserver*/
    Rio_writen(end_serverfd,endserver_http_header,strlen(endserver_http_header));

    /*receive message from end server and send to the client*/
    char cachebuf[MAX_OBJECT_SIZE];
    int sizebuf = 0;
    size_t n;
    while((n=Rio_readlineb(&server_rio,buf,MAXLINE))!=0)
    {
        sizebuf+=n;
        if(sizebuf < MAX_OBJECT_SIZE)  strcat(cachebuf,buf);
        Rio_writen(connfd,buf,n);
    }

    Close(end_serverfd);

    /*store it*/
    if(sizebuf < MAX_OBJECT_SIZE){
        cache_uri(url_store,cachebuf);
    }
}

void build_http_header(char *http_header,char *hostname,char *path,int port,rio_t *client_rio)
{
    char buf[MAXLINE],request_hdr[MAXLINE],other_hdr[MAXLINE],host_hdr[MAXLINE];
    /*request line*/
    sprintf(request_hdr,"GET %s HTTP/1.0\r\n",path);
    /*get other request header for client rio and change it */
    int n;
    while((n = Rio_readlineb(client_rio,buf,MAXLINE))>0)
    {
        if(strncmp(buf,"\r\n", 2)==0) break;/*EOF*/

        if(!strncasecmp(buf,"Host",4))/*Host:*/
        {
            strncpy(host_hdr,buf, n);
            continue;
        }

        if(!strncasecmp(buf,"Connection",10)
                &&!strncasecmp(buf,"Proxy-Connection", 16)
                &&!strncasecmp(buf,"User-Agent",10))
        {
            strncat(other_hdr,buf, n);
        }
    }
    if(strlen(host_hdr)==0)
    {
        sprintf(host_hdr,"Host: %s\r\n",hostname);
    }
    sprintf(http_header,"%s%s%s%s%s%s%s",
            request_hdr,
            host_hdr,
            "Connection: close\r\n",
            "Proxy-Connection: close\r\n",
            user_agent_hdr,
            other_hdr,
            "\r\n");

    return ;
}
/*Connect to the end server*/
inline int connect_endServer(char *hostname,int port,char *http_header){
    char portStr[100];
    sprintf(portStr,"%d",port);
    return Open_clientfd(hostname,portStr);
}

/*parse the uri to get hostname,file path ,port*/
void parse_uri(char *uri,char *hostname,char *path,int *port)
{
    *port = 80;
    char* pos = strstr(uri,"//");

    pos = pos!=NULL? pos+2:uri;

    char*pos2 = strstr(pos,":");
    if(pos2!=NULL)
    {
        *pos2 = '\0';
        sscanf(pos,"%s",hostname);
        sscanf(pos2+1,"%d%s",port,path);
    }
    else
    {
        pos2 = strstr(pos,"/");
        if(pos2!=NULL)
        {
            *pos2 = '\0';
            sscanf(pos,"%s",hostname);
            *pos2 = '/';
            sscanf(pos2,"%s",path);
        }
        else
        {
            sscanf(pos,"%s",hostname);
        }
    }
    return;
}
/**************************************
 * Cache Function
 **************************************/

void cache_init(){
    cache.num = 0;
    int i;
    for(i=0;i<CACHE_ITEMS;i++){
        cache.item[i].LRU = 0;
        cache.item[i].isEmpty = 1;
        Sem_init(&cache.item[i].wmutex,0,1);
        Sem_init(&cache.item[i].rcmutex,0,1);
        cache.item[i].rc = 0;

        cache.item[i].wc = 0;
        Sem_init(&cache.item[i].wcmutex,0,1);
        Sem_init(&cache.item[i].queue,0,1);
    }
}

void readLock(int i){
    P(&cache.item[i].queue);
    P(&cache.item[i].rcmutex);
    cache.item[i].rc++;
    if(cache.item[i].rc==1) P(&cache.item[i].wmutex);
    V(&cache.item[i].rcmutex);
    V(&cache.item[i].queue);
}

void readUnlock(int i){
    P(&cache.item[i].rcmutex);
    cache.item[i].rc--;
    if(cache.item[i].rc==0) V(&cache.item[i].wmutex);
    V(&cache.item[i].rcmutex);

}

void writeLock(int i){
    P(&cache.item[i].wcmutex);
    cache.item[i].wc++;
    if(cache.item[i].wc==1) P(&cache.item[i].queue);
    V(&cache.item[i].wcmutex);
    P(&cache.item[i].wmutex);
}

void writeUnlock(int i){
    V(&cache.item[i].wmutex);
    P(&cache.item[i].wcmutex);
    cache.item[i].wc--;
    if(cache.item[i].wc==0) V(&cache.item[i].queue);
    V(&cache.item[i].wcmutex);
}

/*find url is in the cache or not */
int cache_find(char *url){
    int i;
    for(i=0;i<CACHE_ITEMS;i++){
        readLock(i);
        if((cache.item[i].isEmpty==0) && (strcmp(url,cache.item[i].cache_url)==0)) break;
        readUnlock(i);
    }
    if(i>=CACHE_ITEMS) return -1; /*can not find url in the cache*/
    return i;
}

/*find the empty cacheObj or which cacheObj should be evictioned*/
int cache_evict(){
    int min = LRU_MAGIC_NUMBER;
    int minindex = 0;
    int i;
    for(i=0; i<CACHE_ITEMS; i++)
    {
        readLock(i);
        if(cache.item[i].isEmpty == 1){/*choose if cache block empty */
            minindex = i;
            readUnlock(i);
            break;
        }
        if(cache.item[i].LRU< min){    /*if not empty choose the min LRU*/
            minindex = i;
            readUnlock(i);
            continue;
        }
        readUnlock(i);
    }

    return minindex;
}
/*update the LRU number except the new cache one*/
void LRU(int index){

    writeLock(index);
    cache.item[index].LRU = LRU_MAGIC_NUMBER;
    writeUnlock(index);

    int i;
    for(i=0; i<index; i++)    {
        writeLock(i);
        if(cache.item[i].isEmpty==0 && i!=index){
            cache.item[i].LRU--;
        }
        writeUnlock(i);
    }
    i++;
    for(i; i<CACHE_ITEMS; i++)    {
        writeLock(i);
        if(cache.item[i].isEmpty==0 && i!=index){
            cache.item[i].LRU--;
        }
        writeUnlock(i);
    }
}
/*cache the uri and content in cache*/
void cache_uri(char *uri, char *buf) {
    int idx = cache_evict();

    writeLock(idx);

    // cache_item item = cache.item[idx];
    strcpy(cache.item[idx].cache_obj, buf);
    strcpy(cache.item[idx].cache_url, uri);
    cache.item[idx].isEmpty = 0;

    writeUnlock(idx);

    LRU(idx);
}
