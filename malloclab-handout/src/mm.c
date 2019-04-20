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


#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

// Macros from code in textbook + My Macros
#define WSIZE 4
#define DSIZE 8
#define INITCHUNKSIZE (1 << 6)

#define LIST_N 20

// Pack size and alloc bit into a word
#define PACK(size, alloc) ((size) | (alloc))

// Read from ptr
#define GET(ptr) (*(unsigned int *) (p))
// write val to p with tag
#define PUT(p, val) (*(unsigned int *) (p))
// write val to p without tag
#define PUT_NOTAG(p, val) (*(unsigned int *) (p) = (val))


// size, alloc bit, tag bit at ptr
#define GET_SIZE(ptr) (GET(ptr) & ~0x7)
#define GET_ALLOC(ptr) (GET(ptr) & 1)
#define GET_TAG(ptr) (GET(ptr) & 2)

// Set tag and remove tag at ptr
#define SET_TAG(ptr) (GET(ptr) |= 2)
#define DEL_TAG(ptr) (GET(ptr) &= ~2);

// Address of block header / footer
#define HDRP(ptr) ((char *) (ptr) - WSIZE)
#define FTRP(ptr) ((char *) (ptr) + GET_SIZE(HDRP(ptr)) - DSIZE)

// next block, prev block
#define NEXT_BLKP(bp) ((char *) (bp) + GET_SIZE(((char *) (bp) - WSIZE)))
#define PREV_BLKP(bp) ((char *) (bp) - GET_SIZE(((char *) (bp) - DSIZE)))

// Next, prev free block entry
#define PRED_PTR(ptr) ((char *) (ptr))
#define SUCC_PTR(ptr) ((char *) + WSIZE)

// Next, prev of free block on seg list
#define PRED(ptr) (*(char **) (ptr))
#define SUCC(ptr) (*(char **) (SUCC_PTR(ptr)))

// Set Pointer
#define SET_PTR(p, ptr) (*(unsigned int *) (p) = (unsigned int) (ptr))

// First we need to define segregated list data strucutre
// pointer to start of the list
void **seg_list;

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
    if(ptr = mem_sbrk(nsize)) == (void *) -1) return NULL;

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
    while(search_p && (size > GET_SIZE(HDRP(search_p)))) {
        insert_p = search_p;
        search_p = PRED(search_ptr);
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
    if(GET_TAG(HDRP(PREV_BLCK(ptr)))) prev = 1;
    // Case 1
    if(prev && next) return ptr;
    else if(prev && !next) { // Case 2
        delete(ptr);
        delete(NEXT_BLKP(ptr)); // delete next block
        size += GET_SIZE(NEXT_BLKP(ptr)); // add size
        PUT(HDRP(ptr), PACK(size, 0)); // update header
        PUT(FTRP(NEXT_BLKP(ptr)), PACK(size, 0)); // update footer
    } else if(!prev && next) { // Case 3
        delete(ptr);
        delete(PREV_BLKP(ptr)); // delete prev block
        size += GET_SIZE(PREV_BLKP(ptr)); // add size
        PUT(HDRP(PREV_BLKP(ptr)), PACK(size, 0)); // update header
        PUT(FTRP(ptr), PACK(size, 0)); // update footer
        ptr = PREV_BLKP(ptr); // set to prev block
    } else { // Case 4
        delete(ptr);
        delete(NEXT_BLKP(ptr)); // delete next block
        delete(PREV_BLKP(ptr)); // delete prev block
        size += GET_SIZE(NEXT_BLKP(ptr));
        size += GET_SIZE(PREV_BLKP(ptr)); // add size
        PUT(HDRP(PREV_BLOCK(ptr)), PACK(size, 0));
        PUT(FTRP(NEXT_BLOCK(ptr)), PACK(size, 0));
        ptr = PREV_BLKK(ptr); // set to prev block
    }
    insert(ptr, size);

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
    }

    // Allocate memory on heap
    if((long) (heap_st = mem_sbrk(4 * WSIZE)) == -1) return -1;

    // Initial block
    PUT_NOTAG(heap_st, 0);
    PUT_NOTAG(heap_st + (1 * WSIZE), PACK(DSIZE, 1));
    PUT_NOTAG(heap_st + (2 * WSIZE), PACK(DSIZE, 1));
    PUT_NOTAG(heap_st + (3 * WSIZE), PACK(0, 1));

    // extend heap
    if(!extend_heap(INITCHUNKSIZE)) return -1;

    return 0;
}

/*
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    int newsize = ALIGN(size + SIZE_T_SIZE);
    void *p = mem_sbrk(newsize);
    if (p == (void *)-1)
	return NULL;
    else {
        *(size_t *)p = size;
        return (void *)((char *)p + SIZE_T_SIZE);
    }
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    void *oldptr = ptr;
    void *newptr;
    size_t copySize;

    newptr = mm_malloc(size);
    if (newptr == NULL)
      return NULL;
    copySize = *(size_t *)((char *)oldptr - SIZE_T_SIZE);
    if (size < copySize)
      copySize = size;
    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    return newptr;
}
