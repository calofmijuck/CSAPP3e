#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
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

int main(int argc, char** argv) {
    cache_param param;
    memset(&param, 0, sizeof(param));
    printSummary(0, 0, 0);
    return 0;
}
