#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include "cachelab.h"

typedef unsigned long long memAddress; // 64-bit data to hold memory addresses

typedef struct {
    int s; // bits needed to represent S = 2 ** s
    int b; // B = 2 ** b
    int E; // Cache lines per set
    int S; // Number of sets
    int B; // Cache line block size in bytes
    int hits;
    int misses;
    int evicts;
} cache_param;

int verbosity; // -v flag

// It will probably be useful to keep track of elements as a struct
// Need valid bit, tag bit, and data. Count accesses for removing policy
typedef struct {
    int valid;
    int access; // counts accesses
    memAddress tag;
    char* block;
} cache_line;

// each set will hold lines
typedef struct {
    cache_line *lines;
} cache_set;

// Think cache as an array of sets
typedef struct {
    cache_set *sets;
} cache;

// create cache
cache make_cache(long long setNum, int lineNum, long long size) {
    cache ret;
    cache_set set;
    cache_line line;

    // Allocate memory
    ret.sets = (cache_set*) malloc(sizeof(cache_set) * setNum);
    for(int s = 0; s < setNum; ++s) {
        // for each set, build lines
        set.lines = (cache_line*) malloc(sizeof(cache_line) * lineNum);
        ret.sets[s] = set;

        // for each line, build blocks
        for(int lidx = 0; lidx < lineNum; ++lidx) {
            line.valid = 0;
            line.access = 0;
            line.tag = 0;
            set.lines[lidx] = line;
        }
    }
    return ret;
}

void clean(cache CACHE, long long setNum, int lineNum, long long size) {
    for(int s = 0; s < setNum; ++s) {
        cache_set set = CACHE.sets[s];
        if(set.lines != NULL) free(set.lines);
    }
    if(CACHE.sets != NULL) free(CACHE.sets);
}

int LRU(cache_set SET, cache_param param, int* used) {
    int ret = 0;
    cache_line line;
    int max = SET.lines[0].access, min = max;
    for(int lidx = 1; lidx < param.E; ++lidx) {
        line = SET.lines[lidx];
        if(line.access < min) {
            ret = lidx;
            min = line.access;
        }
        if(line.access > max) max = line.access;
    }
    used[0] = min;
    used[1] = max;
    return ret;
}

int emptyLine(cache_set SET, cache_param param) {
    cache_line line;
    for(int i = 0; i < param.E; ++i) {
        line = SET.lines[i];
        if(!line.valid) return i;
    }
    return 0; // no empty line
}

// cache simulator
cache_param simulate(cache CACHE, cache_param param, memAddress addr) {
    int tagsize = 64 - param.s - param.b, lineNum = param.E, prev = param.hits;
    int full = 1; // All lines full

    // parse input
    unsigned long long setIdx = (addr << tagsize) >> (tagsize + param.b);
    memAddress inputTag = addr >> (param.s + param.b);

    // look for set
    cache_set search = CACHE.sets[setIdx];

    for(int lidx = 0; lidx < lineNum; ++lidx) {
        cache_line line = search.lines[lidx];
        if(line.valid) {
            if(line.tag == inputTag) { // cache hit
                ++line.access;
                ++param.hits;
                search.lines[lidx] = line;
            }
        } else if(!(line.valid) && full) full = 0;
    }
    if(prev == param.hits) ++param.misses; // cache miss
    else return param; // cache hit - just return
    // cache miss : replacement policy LRU / write to empty line
    int* used = (int*) malloc(sizeof(int) * 2);
    int lruIdx = LRU(search, param, used);

    if(full) { // evict and overwrite
        ++param.evicts;
        search.lines[lruIdx].tag = inputTag;
        search.lines[lruIdx].access = used[1] + 1;
    } else { // write in empty line
        int emptyIdx = emptyLine(search, param);
        search.lines[emptyIdx].tag = inputTag;
        search.lines[emptyIdx].valid = 1;
        search.lines[emptyIdx].access = used[1] + 1;
    }

    free(used); // don't forget to free
    return param;
}


int main(int argc, char** argv) {
    cache_param param;
    cache CACHE;
    malloc(sizeof(cache_param));

    FILE *read_trace;
    char op;
    memAddress addr;
    int size;
    char *trace_file, c;

    // parse arguments and commands
    while((c = getopt(argc, argv, "s:E:b:t:vh")) != -1) {
        switch(c) {
            case 's':
                param.s = atoi(optarg);
                break;
            case 'E':
                param.E = atoi(optarg);
                break;
            case 'b':
                param.b = atoi(optarg);
            case 't':
                trace_file = optarg;
            case 'v':
                verbosity = 1;
                break;
            case 'h':
                printf("You don't get any help. :)\n");
                exit(0);
            default:
                printf("Invalid arguments.\n");
                exit(1);
        }
    }

    // Check for exceptions
    if(!param.s || !param.E || !param.b || !trace_file) {
        printf("Missing required command line arguments.\n");
        exit(1);
    }

    long long sets = 1 << param.s; // number of sets
    long long block = 1 << param.b; // number of blocks
    param.hits = 0;
    param.misses = 0;
    param.evicts = 0;
    // initializations

    CACHE = make_cache(sets, param.E, block);
    read_trace = fopen(trace_file, "r"); // read
    if(read_trace == NULL) return 0;

    // read file and execute
    while(fscanf(read_trace, " %c %llx,%d", &op, &addr, &size) == 3) {
        switch(op) {
            case 'I': // instruction load
                break;
            case 'L': // data load
                param = simulate(CACHE, param, addr);
                break;
            case 'S': // data store
                param = simulate(CACHE, param, addr);
                break;
            case 'M': // data modify
                param = simulate(CACHE, param, addr);
                param = simulate(CACHE, param, addr);
                break;
            default:
                break;
        }
    }

    printSummary(param.hits, param.misses, param.evicts);
    clean(CACHE, sets, param.E, block); // free memory
    fclose(read_trace); // remember to close file

    return 0;
}
