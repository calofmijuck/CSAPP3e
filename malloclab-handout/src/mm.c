/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 *
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)


// #define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

// Macros from code in textbook + My Macros
#define WSIZE 4
#define DSIZE 8
#define INITCHUNKSIZE (1 << 6)
#define CHUNKSIZE (1 << 12)
#define LIST_N 20
#define REALLOC_BUFFER  (1 << 7)

// Pack size and alloc bit into a word
#define PACK(size, alloc) ((size) | (alloc))

// Read from ptr
#define GET(ptr) (*(unsigned int *)(ptr))
// write val to p with tag
#define PUT(p, val) (*(unsigned int *)(p) = (val) | GET_TAG(p))
// write val to p without tag
#define PUT_NOTAG(p, val) (*(unsigned int *)(p) = (val))


// size, alloc bit, tag bit at ptr
#define GET_SIZE(ptr) (GET(ptr) & ~0x7)
#define GET_ALLOC(ptr) (GET(ptr) & 1)
#define GET_TAG(ptr) (GET(ptr) & 2)

// Set tag and remove tag at ptr
#define SET_TAG(ptr) (GET(ptr) |= 2)
#define DEL_TAG(ptr) (GET(ptr) &= ~2)

// Address of block header / footer
#define HDRP(ptr) ((char *)(ptr) - WSIZE)
#define FTRP(ptr) ((char *)(ptr) + GET_SIZE(HDRP(ptr)) - DSIZE)

// next block, prev block
#define NEXT_BLKP(ptr) ((char *)(ptr) + GET_SIZE((char *)(ptr) - WSIZE))
#define PREV_BLKP(ptr) ((char *)(ptr) - GET_SIZE((char *)(ptr) - DSIZE))

// Next, prev free block entry
#define PRED_PTR(ptr) ((char *)(ptr))
#define SUCC_PTR(ptr) ((char *)(ptr) + WSIZE)

// Next, prev of free block on seg list
#define PRED(ptr) (*(char **)(ptr))
#define SUCC(ptr) (*(char **)(SUCC_PTR(ptr)))

// Set Pointer
#define SET_PTR(p, ptr) (*(unsigned int *)(p) = (unsigned int)(ptr))

// First we need to define segregated list data strucutre
// pointer to start of the list
void *l0 = 0, *l1 = 0, *l2 = 0, *l3 = 0, *l4 = 0, *l5 = 0, *l6 = 0, *l7 = 0, *l8 = 0, *l9 = 0, *l10 = 0, *l11 = 0, *l12 = 0, *l13 = 0, *l14 = 0, *l15 = 0, *l16 = 0, *l17 = 0, *l18 = 0, *l19 = 0;
void **seg_list = &l19;

// functions
static void *extend_heap(size_t size);
static void *coalesce(void *ptr);
static void *place(void *ptr, size_t asize);
static void insert(void *ptr, size_t size);
static void delete(void *ptr);


/////////////////// Function implementation //////////
static void *extend_heap(size_t size) {
    void *ptr;
    size_t nsize; // new size

    nsize = ALIGN(size); // alignment

    // fails
    if((ptr = mem_sbrk(nsize)) == (void *) -1) return NULL;

    // Header and footer
    PUT_NOTAG(HDRP(ptr), PACK(nsize, 0));
    PUT_NOTAG(FTRP(ptr), PACK(nsize, 0));
    PUT_NOTAG(HDRP(NEXT_BLKP(ptr)), PACK(0, 1));
    insert(ptr, nsize);

    return coalesce(ptr);
}

static void insert(void *ptr, size_t size) {
    int idx = 0;
    void *search_p = ptr;
    void *insert_p = NULL;

    // Select from segregated_free_listslist
    while((idx < LIST_N - 1) && (size > 1)) {
        size >>= 1;
        idx++;
    }

    // Search
    search_p = seg_list[idx];
    while(search_p != NULL && (size > GET_SIZE(HDRP(search_p)))) {
        insert_p = search_p;
        search_p = PRED(search_p);
    }

    // Set next, prev
    if(search_p) {
        SET_PTR(PRED_PTR(ptr), search_p);
        SET_PTR(SUCC_PTR(search_p), ptr);
        if(insert_p) {
            SET_PTR(SUCC_PTR(ptr), insert_p);
            SET_PTR(PRED_PTR(insert_p), ptr);
        } else {
            SET_PTR(SUCC_PTR(ptr), NULL);
            seg_list[idx] = ptr;
        }
    } else {
        SET_PTR(PRED_PTR(ptr), NULL);
        if(insert_p) {
            SET_PTR(SUCC_PTR(ptr), insert_p);
            SET_PTR(PRED_PTR(insert_p), ptr);
        } else {
            SET_PTR(SUCC_PTR(ptr), NULL);
            seg_list[idx] = ptr;
        }
    }

    return;
}

static void delete(void *ptr) {
    int idx = 0;
    size_t size = GET_SIZE(HDRP(ptr));

    // Select from seg list
    while((idx < LIST_N - 1) && (size > 1)) {
        size >>= 1;
        idx++;
    }

    // delete
    if(PRED(ptr)) {
        if(SUCC(ptr)) {
            SET_PTR(SUCC_PTR(PRED(ptr)), SUCC(ptr));
            SET_PTR(PRED_PTR(SUCC(ptr)), PRED(ptr));
        } else {
            SET_PTR(SUCC_PTR(PRED(ptr)), NULL);
            seg_list[idx] = PRED(ptr);
        }
    } else {
        if(SUCC(ptr)) {
            SET_PTR(PRED_PTR(SUCC(ptr)), NULL);
        } else {
            seg_list[idx] = NULL;
        }
    }

    return;
}

static void *coalesce(void *ptr) {
    // allocation status of prev, next
    size_t prev = GET_ALLOC(HDRP(PREV_BLKP(ptr)));
    size_t next = GET_ALLOC(HDRP(NEXT_BLKP(ptr)));
    size_t size = GET_SIZE(HDRP(ptr));

    // Does the block have reallocation tag?
    if(GET_TAG(HDRP(PREV_BLKP(ptr)))) prev = 1;
    // Case 1
    if(prev && next) return ptr;
    else if(prev && !next) { // Case 2
        delete(ptr);
        delete(NEXT_BLKP(ptr)); // delete next block
        size += GET_SIZE(HDRP(NEXT_BLKP(ptr))); // add size
        PUT(HDRP(ptr), PACK(size, 0)); // update header
        // PUT(FTRP(NEXT_BLKP(ptr)), PACK(size, 0)); // update footer
        PUT(FTRP(ptr), PACK(size, 0));
    } else if(!prev && next) { // Case 3
        delete(ptr);
        delete(PREV_BLKP(ptr)); // delete prev block
        size += GET_SIZE(HDRP(PREV_BLKP(ptr))); // add size
        PUT(FTRP(ptr), PACK(size, 0)); // update footer
        PUT(HDRP(PREV_BLKP(ptr)), PACK(size, 0)); // update header
        ptr = PREV_BLKP(ptr); // set to prev block
    } else { // Case 4
        delete(ptr);
        delete(PREV_BLKP(ptr)); // delete prev block
        delete(NEXT_BLKP(ptr)); // delete next block
        size += GET_SIZE(HDRP(NEXT_BLKP(ptr)));
        size += GET_SIZE(HDRP(PREV_BLKP(ptr))); // add size
        PUT(HDRP(PREV_BLKP(ptr)), PACK(size, 0));
        PUT(FTRP(NEXT_BLKP(ptr)), PACK(size, 0));
        ptr = PREV_BLKP(ptr); // set to prev block
    }
    insert(ptr, size);

    return ptr;
}

/*
 * place - Place block of asize bytes at start of free block bp
 *         and split if remainder would be at least minimum block size
 */
static void *place(void *ptr, size_t asize) {
    size_t csize = GET_SIZE(HDRP(ptr));
    size_t rm = csize - asize;

    delete(ptr);

    if(rm <= 2 * DSIZE) { // remainder too small, do not split
        // update header and footer
        PUT(HDRP(ptr), PACK(csize, 1));
        PUT(FTRP(ptr), PACK(csize, 1));
    } else if(asize >= 100) { // request size is big. split
        // Set rm of memory at ptr
        PUT(HDRP(ptr), PACK(rm, 0));
        PUT(FTRP(ptr), PACK(rm, 0));
        // place asize of memory at next of ptr
        PUT_NOTAG(HDRP(NEXT_BLKP(ptr)), PACK(asize, 1));
        PUT_NOTAG(FTRP(NEXT_BLKP(ptr)), PACK(asize, 1));
        insert(ptr, rm);
        return NEXT_BLKP(ptr);
    } else { // split
        // Set asize memory at ptr
        PUT(HDRP(ptr), PACK(asize, 1));
        PUT(FTRP(ptr), PACK(asize, 1));
        // place rm of memory at next of ptr
        PUT_NOTAG(HDRP(NEXT_BLKP(ptr)), PACK(rm, 0));
        PUT_NOTAG(FTRP(NEXT_BLKP(ptr)), PACK(rm, 0));
        insert(NEXT_BLKP(ptr), rm);
    }
    return ptr;
}

//////////////////////////////////////////////////////
/*
 * mm_init - initialize the malloc package.
 * Initialize everything necessary : allocate heap.. etc.
 */
int mm_init(void) {
    int i;
    char *heap_st; // track the start of the heap

    // Initialize free list
    for(i = 0; i < LIST_N; ++i) {
        seg_list[i] = NULL;
        // printf("%d: %d\n", i, (int) seg_list[i]);
    }

    // printf("%d\n", (int) &l0);
    // printf("%d\n", (int) &l1);
    // printf("%d\n", (int) &l2);
    // printf("%d\n", (int) &l3);
    // printf("%d\n", (int) &l4);
    // printf("%d\n", (int) &l5);
    // printf("%d\n", (int) &l6);
    // printf("%d\n", (int) &l7);
    // printf("%d\n", (int) &l8);
    // printf("%d\n", (int) &l9);
    // printf("%d\n", (int) &l10);
    // printf("%d\n", (int) &l11);
    // printf("%d\n", (int) &l12);
    // printf("%d\n", (int) &l13);
    // printf("%d\n", (int) &l14);
    // printf("%d\n", (int) &l15);
    // printf("%d\n", (int) &l16);
    // printf("%d\n", (int) &l17);
    // printf("%d\n", (int) &l18);
    // printf("%d\n", (int) &l19);

    // Allocate memory on heap
    if((long) (heap_st = mem_sbrk(4 * WSIZE)) == -1) return -1;

    // Initial block
    PUT_NOTAG(heap_st, 0);
    PUT_NOTAG(heap_st + (1 * WSIZE), PACK(DSIZE, 1));
    PUT_NOTAG(heap_st + (2 * WSIZE), PACK(DSIZE, 1));
    PUT_NOTAG(heap_st + (3 * WSIZE), PACK(0, 1));

    // extend heap
    if(extend_heap(INITCHUNKSIZE) == NULL) return -1;

    return 0;
}

/*
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size) {
    size_t asize; // adjust size
    size_t ext_size; // extended size
    void *ptr = NULL;

    // size 0
    if(size == 0) return NULL;
    if(size <= DSIZE) asize = 2 * DSIZE;
    else asize = ALIGN(size + DSIZE);

    int idx = 0;
    size_t ssize = asize; // search size
    // Search for free block in seglist
    while(idx < LIST_N) {
        if(idx == LIST_N - 1 || ((ssize <= 1) && seg_list[idx] != NULL)) {
            ptr = seg_list[idx];
            // ignore blocks with realloc bit
            // ignore small blocks
            while(ptr != NULL && (asize > GET_SIZE(HDRP(ptr)) || (GET_TAG(HDRP(ptr))))) {
                ptr = PRED(ptr);
            }
            if(ptr != NULL) break;
        }
        ssize >>= 1;
        idx++;
    }

    // if not found, extend heap
    if(ptr == NULL) {
        ext_size = asize > CHUNKSIZE ? asize : CHUNKSIZE;
        if((ptr = extend_heap(ext_size)) == NULL) return NULL;
    }

    // Place block, split if necessary
    ptr = place(ptr, asize);
    return ptr;
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr) {
    size_t size = GET_SIZE(HDRP(ptr)); // HDRP(ptr) !!!

    // Remove tag
    DEL_TAG(HDRP(NEXT_BLKP(ptr)));
    // Update header / footer
    PUT(HDRP(ptr), PACK(size, 0));
    PUT(FTRP(ptr), PACK(size, 0));

    // insert to seg_list
    insert(ptr, size);
    coalesce(ptr);

    return;
}

/*
 * mm_realloc - using reallocation tags to maximize utilization
 */
void *mm_realloc(void *ptr, size_t size) {
    void *ret = ptr; // return pointer
    size_t nsize = size; // new size
    int remainder;
    int esize; // extended size
    int block_buffer;

    if(!size) return NULL; // ignore size 0

    // Align block size
    if(nsize <= DSIZE) nsize = 2 * DSIZE;
    else nsize = ALIGN(size + DSIZE);

    // overhead requirements
    nsize += REALLOC_BUFFER;
    block_buffer = GET_SIZE(HDRP(ptr)) - nsize;

    // Allocate more space if overhead falls below the minimum
    if(block_buffer < 0) {
        // Is next block a free block? or the epilogue block?
        if(!GET_ALLOC(HDRP(NEXT_BLKP(ptr))) ||  !GET_SIZE(HDRP(NEXT_BLKP(ptr)))) {
            remainder = GET_SIZE(HDRP(ptr)) + GET_SIZE(HDRP(NEXT_BLKP(ptr)))  - nsize;
            if(remainder < 0) {
                esize = -remainder > CHUNKSIZE ? -remainder : CHUNKSIZE;
                if(extend_heap(esize) == NULL) return NULL;
                remainder += esize;
            }
            delete(NEXT_BLKP(ptr));

            // Do not split block
            PUT_NOTAG(HDRP(ptr), PACK(nsize + remainder, 1));
            PUT_NOTAG(FTRP(ptr), PACK(nsize + remainder, 1));
        } else {
            ret = mm_malloc(nsize - DSIZE);
            memcpy(ret, ptr, size > nsize ? nsize : size);
            mm_free(ptr);
        }
        block_buffer = GET_SIZE(HDRP(ret)) - nsize;
    }

    // Tag the next block if block overhead drops below twice the overhead
    if (block_buffer < 2 * REALLOC_BUFFER)
        SET_TAG(HDRP(NEXT_BLKP(ret)));

    return ret;
}
