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
    mallocp = dlsym(RTLD_NEXT, "malloc");
    if((error = dlerror()) != NULL) {
        fputs(error, stderr);
        exit(1);
    }
    void *ptr = mallocp(size);
    alloc(list, ptr, size); // add item to list
    LOG_MALLOC((int) size, ptr);
    return ptr;
}

void free(void *ptr) {
    LOG_FREE(ptr);
    if(ptr == NULL) return;
    item* block = find(list, ptr); // find item in list
    if(block == NULL) {
        LOG_ILL_FREE();
        return;
    }
    if(block -> cnt <= 0) {
        LOG_DOUBLE_FREE();
        return;
    }
    n_freeb += (block -> size); // add freed byte
    char* error;
    freep = dlsym(RTLD_NEXT, "free");
    if((error = dlerror()) != NULL) {
        fputs(error, stderr);
        exit(1);
    }
    freep(ptr);
    dealloc(list, ptr); // delete item in list
}

void *calloc(size_t nmemb, size_t size) {
    n_calloc++;
    n_allocb += size * nmemb;
    char* error;
    callocp = dlsym(RTLD_NEXT, "calloc");
    if((error = dlerror()) != NULL) {
        fputs(error, stderr);
        exit(1);
    }
    void* ptr = callocp(nmemb, size);
    alloc(list, ptr, size * nmemb); // add item in list
    LOG_CALLOC(nmemb, size, ptr);
    return ptr;
}

void *realloc(void *ptr, size_t size) {
    char* error;
    reallocp = dlsym(RTLD_NEXT, "realloc");
    if((error = dlerror()) != NULL) {
        fputs(error, stderr);
        exit(1);
    }
    void* res = reallocp(ptr, size); // first, reallocate
    LOG_REALLOC(ptr, size, res); // LOG it
    n_realloc++; // increment count
    // n_allocb += size;

    if(res == ptr) { // pointer didn't change resizing happened
        item* block = find(list, ptr); // find block
        // this block cannot be null
        if(block -> size > size) { // decreased size
            n_freeb += block -> size - size;
        } else { // increased size
            n_allocb += size - block -> size;
        }
        // update totals
        dealloc(list, block); // deallocate
        block -> cnt --;
        alloc(list, res, size); // allocate item in list
    } else { // realloc freed memory in ptr and allocated somewhere else
        item* block = find(list, ptr); // try to find ptr in list
        /*
            There is a chance block might be NULL
            Case 1. ptr is NULL
                In this case, realloc(NULL, size) was called.
                C standard says that this is equal to malloc(size)
            Case 2. ptr is not NULL but not found in the list
                In this case, realloc to some other location was called.
                The processor will still try to allocate memory.
                This is undefined behavior.
            In both cases, free doesn't happen
        */
        if(ptr == NULL || block == NULL) {
            alloc(list, res, size); // allocate item in list
            n_allocb += size; // add
        } else { // block was found
            n_freeb += block -> size; // original place was freed
            n_allocb += size; // add
            dealloc(list, block); // deallocate
            block -> cnt --;
            alloc(list, res, size); // add item in list
        }
    }
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
    // (not needed for part 1)
    list = new_list();

    // ...
}

//
// fini - this function is called once when the shared library is unloaded
//
__attribute__((destructor))
void fini(void) {
    // ...
    unsigned long cnt = n_malloc + n_calloc + n_realloc;

    LOG_STATISTICS(n_allocb, n_allocb / cnt, n_freeb);
    if(n_allocb != n_freeb) LOG_NONFREED_START();
    while(list -> next) {
        item *curr = list -> next;
        if(curr -> cnt >= 1) {
            LOG_BLOCK(curr -> ptr, curr -> size, curr -> cnt, curr -> fname, curr -> ofs);
        }
        list = list -> next;
    }
    LOG_STOP();

    // free list (not needed for part 1)
    free_list(list);
}

// ...

