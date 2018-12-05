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

int main(int argc, char** argv) {
    cache_param param;
    malloc(sizeof(param));
    // printSummary(0, 0, 0);

    FILE *read_trace;
    char op;
    memAddress addr;
    int size;
    char *trace_file, c;

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
            // case 'h':
            default:
                exit(1);
        }
    }
    return 0;
}
