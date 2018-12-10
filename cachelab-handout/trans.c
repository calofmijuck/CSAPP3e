/*
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */
#include <stdio.h>
#include "cachelab.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

/*
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded.
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N]) {
    int size, r, c, i, j, tmp;

    // divide and conquer. divide 32x32 to many block matrices
    if(N == 32) {
        size = 8; // choose size - 8 gives full mark
        for(r = 0; r < N; r += size) {
            for(c = 0; c < M; c += size) {
                for(i = r; i < r + size; ++i) {
                    for(j = c; j < c + size; ++j) {
                        if(i != j) {
                            tmp = A[i][j];
                            B[j][i] = tmp;
                        }
                    }
                    if(r == c) {
                        tmp = A[i][i];
                        B[i][i] = tmp;
                    }
                }
            }
        }
    }

    // When using the same method as in N = 32, the driver.py gives 0 score... (checked for size = 8, 16)
    // Suggestion of something similar to loop unrolling?
    else if(N == 64) {

    }

    // Transpose matrix for any size
    // Try the same method used in case N = 32
    // Size 16 gives 2009 misses (9.9/10)
    else {
        size = 16; // choose size
        for(r = 0; r < N; r += size) {
            for(c = 0; c < M; c += size) {
                for(i = r; i < r + size && i < N; ++i) {
                    for(j = c; j < c + size && j < M; ++j) {
                        if(i != j) {
                            tmp = A[i][j];
                            B[j][i] = tmp;
                        }
                    }
                    if(r == c) {
                        tmp = A[i][i];
                        B[i][i] = tmp;
                    }
                }
            }
        }
    }
}

/*
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started.
 */

/*
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }

}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc);

    /* Register any additional transpose functions */
    // registerTransFunction(trans, trans_desc);

}

/*
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}
