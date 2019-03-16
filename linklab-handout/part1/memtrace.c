//------------------------------------------------------------------------------
//
// memtrace
//
// trace calls to the dynamic memory manager
//
#define _GNU_SOURCE

#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <memlog.h>
#include <memlist.h>
#include "callinfo.h"

//
// function pointers to stdlib's memory management functions
//
static void *(*mallocp)(size_t size) = NULL;
static void (*freep)(void *ptr) = NULL;
static void *(*callocp)(size_t nmemb, size_t size);
static void *(*reallocp)(void *ptr, size_t size);

//
// statistics & other global variables
//
static unsigned long n_malloc  = 0;
static unsigned long n_calloc  = 0;
static unsigned long n_realloc = 0;
static unsigned long n_allocb  = 0;
static unsigned long n_freeb   = 0;
static item *list = NULL;

void *malloc(size_t size) {
    n_allocb += size;
    n_malloc++;
    char* error;
    assert(size != 0);
    mallocp = dlsym(RTLD_NEXT, "malloc");
    if((error = dlerror()) != NULL) {
        fputs(error, stderr);
        exit(1);
    }
    void *ptr = mallocp(size);
    LOG_MALLOC((int) size, ptr);
    return ptr;
}

void free(void *ptr) {
    char* error;
    assert(ptr != NULL);
    freep = dlsym(RTLD_NEXT, "free");
    if((error = dlerror()) != NULL) {
        fputs(error, stderr);
        exit(1);
    }
    freep(ptr);
    LOG_FREE(ptr);
}

void *calloc(size_t nmemb, size_t size) {
    n_calloc++;
    n_allocb += size;
    char* error;
    assert(size != 0);
    callocp = dlsym(RTLD_NEXT, "calloc");
    if((error = dlerror()) != NULL) {
        fputs(error, stderr);
        exit(1);
    }
    void* ptr = callocp(nmemb, size);
    LOG_CALLOC(nmemb, size, ptr);
    return ptr;
}

void *realloc(void *ptr, size_t size) {
    n_realloc++;
    n_allocb += size;
    char* error;
    assert(ptr != NULL);
    assert(size != 0);
    reallocp = dlsym(RTLD_NEXT, "realloc");
    if((error = dlerror()) != NULL) {
        fputs(error, stderr);
        exit(1);
    }
    void* res = reallocp(ptr, size);
    LOG_REALLOC(ptr, size, res);
    return res;
}

//
// init - this function is called once when the shared library is loaded
//
__attribute__((constructor))
void init(void) {
    char *error;
    LOG_START();
    // initialize a new list to keep track of all memory (de-)allocations
    list = new_list();
}

//
// fini - this function is called once when the shared library is unloaded
//
__attribute__((destructor))
void fini(void) {
    unsigned long cnt = n_malloc + n_calloc + n_realloc;
    LOG_STATISTICS(n_allocb, n_allocb / cnt, n_freeb);
    LOG_STOP();
    free_list(list);
}
