#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
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

int main(int argc, char** argv) {
    cache_param param;
    cache CACHE;
    memset(&param, 0, sizeof(param));
    // malloc(sizeof(param));
    // printSummary(0, 0, 0);

    FILE *read_trace;
    char op;
    memAddress addr;
    int size;
    char *trace_file, c;

    // parse arguments and commands
    while((c = getopt(argc, argv, "s:E:b:t:vh")) != -1) {
        switch (c) {
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
    if(!param.s || !param.E || !param.b || trace_file == NULL) {
        printf("Missing critical arguments.\n");
        exit(1);
    }

    long long sets = 1 << param.S; // number of sets
    long long block = 1 << param.b; // number of blocks
    param.hits = 0;
    param.misses = 0;
    param.evicts = 0;
    // initializations

    CACHE = make_cache(sets, param.E, block);
    read_trace = fopen(trace_file, "r"); // read

    // read file and execute
    // TODO

    return 0;
}
