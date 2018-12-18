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
    int size, r, c, i, j;
    int a, s, d, f, g, h, k;

    // divide and conquer. divide 32x32 to many block matrices
    if(N == 32) {
        size = 8; // choose size - 8 gives full mark
        for(r = 0; r < N; r += size) {
            for(c = 0; c < M; c += size) {
                for(i = r; i < r + size; ++i) {
                    for(j = c; j < c + size; ++j) {
                        if(i != j) {
                            B[j][i] = A[i][j];
                        }
                    }
                    if(r == c) {
                        B[i][i] = A[i][i];
                    }
                }
            }
        }
    }

    // When using the same method as in N = 32, the driver.py gives 0 score... (checked for size = 8, 16)
    // Suggestion of something similar to loop unrolling?
    // gives 1635 misses (4.2/8)
    else if(N == 64) {
        size = 8; // choose size - choose 8, since block size is 32 bytes

        for(r = 0; r < N; r += size) {
            for(c = 0; c < M; c += size) {
                if(r != c) {
                    B[c][r] = A[r][c];
                    B[c][r + 1] = A[r + 1][c];
                    B[c][r + 2] = A[r + 2][c];
                    B[c][r + 3] = A[r + 3][c];

                    B[c + 1][r] = A[r][c + 1];
                    B[c + 1][r + 1] = A[r + 1][c + 1];
                    B[c + 1][r + 2] = A[r + 2][c + 1];
                    B[c + 1][r + 3] = A[r + 3][c + 1];

                    B[c + 2][r] = A[r][c + 2];
                    B[c + 2][r + 1] = A[r + 1][c + 2];
                    B[c + 2][r + 2] = A[r + 2][c + 2];
                    B[c + 2][r + 3] = A[r + 3][c + 2];

                    B[c + 3][r] = A[r][c + 3];
                    B[c + 3][r + 1] = A[r + 1][c + 3];
                    B[c + 3][r + 2] = A[r + 2][c + 3];
                    B[c + 3][r + 3] = A[r + 3][c + 3];

                    // Upper left of B is done

                    B[c][r + 4] = A[r][c + 4];
                    B[c + 1][r + 4] = A[r][c + 5];
                    B[c + 2][r + 4] = A[r][c + 6];
                    B[c + 3][r + 4] = A[r][c + 7];

                    B[c][r + 5] = A[r + 1][c + 4];
                    B[c + 1][r + 5] = A[r + 1][c + 5];
                    B[c + 2][r + 5] = A[r + 1][c + 6];
                    B[c + 3][r + 5] = A[r + 1][c + 7];

                    B[c][r + 6] = A[r + 2][c + 4];
                    B[c + 1][r + 6] = A[r + 2][c + 5];
                    B[c + 2][r + 6] = A[r + 2][c + 6];
                    B[c + 3][r + 6] = A[r + 2][c + 7];

                    B[c][r + 7] = A[r + 3][c + 4];
                    B[c + 1][r + 7] = A[r + 3][c + 5];
                    B[c + 2][r + 7] = A[r + 3][c + 6];
                    B[c + 3][r + 7] = A[r + 3][c + 7];

                    // Copy!
                    a = B[c][r + 4];
                    s = B[c][r + 5];
                    d = B[c][r + 6];
                    f = B[c][r + 7];

                    B[c][r + 4] = A[r + 4][c];
                    B[c][r + 5] = A[r + 5][c];
                    B[c][r + 6] = A[r + 6][c];
                    B[c][r + 7] = A[r + 7][c];

                    B[c + 4][r] = a;
                    B[c + 4][r + 1] = s;
                    B[c + 4][r + 2] = d;
                    B[c + 4][r + 3] = f;

                    a = B[c + 1][r + 4];
                    s = B[c + 1][r + 5];
                    d = B[c + 1][r + 6];
                    f = B[c + 1][r + 7];

                    B[c + 1][r + 4] = A[r + 4][c + 1];
                    B[c + 1][r + 5] = A[r + 5][c + 1];
                    B[c + 1][r + 6] = A[r + 6][c + 1];
                    B[c + 1][r + 7] = A[r + 7][c + 1];

                    B[c + 5][r] = a;
                    B[c + 5][r + 1] = s;
                    B[c + 5][r + 2] = d;
                    B[c + 5][r + 3] = f;

                    a = B[c + 2][r + 4];
                    s = B[c + 2][r + 5];
                    d = B[c + 2][r + 6];
                    f = B[c + 2][r + 7];

                    B[c + 2][r + 4] = A[r + 4][c + 2];
                    B[c + 2][r + 5] = A[r + 5][c + 2];
                    B[c + 2][r + 6] = A[r + 6][c + 2];
                    B[c + 2][r + 7] = A[r + 7][c + 2];

                    B[c + 6][r] = a;
                    B[c + 6][r + 1] = s;
                    B[c + 6][r + 2] = d;
                    B[c + 6][r + 3] = f;

                    a = B[c + 3][r + 4];
                    s = B[c + 3][r + 5];
                    d = B[c + 3][r + 6];
                    f = B[c + 3][r + 7];

                    B[c + 3][r + 4] = A[r + 4][c + 3];
                    B[c + 3][r + 5] = A[r + 5][c + 3];
                    B[c + 3][r + 6] = A[r + 6][c + 3];
                    B[c + 3][r + 7] = A[r + 7][c + 3];

                    B[c + 7][r] = a;
                    B[c + 7][r + 1] = s;
                    B[c + 7][r + 2] = d;
                    B[c + 7][r + 3] = f;

                    // Lower left is done

                    B[c + 4][r + 4] = A[r + 4][c + 4];
                    B[c + 5][r + 4] = A[r + 4][c + 5];
                    B[c + 6][r + 4] = A[r + 4][c + 6];
                    B[c + 7][r + 4] = A[r + 4][c + 7];

                    B[c + 4][r + 5] = A[r + 5][c + 4];
                    B[c + 5][r + 5] = A[r + 5][c + 5];
                    B[c + 6][r + 5] = A[r + 5][c + 6];
                    B[c + 7][r + 5] = A[r + 5][c + 7];

                    B[c + 4][r + 6] = A[r + 6][c + 4];
                    B[c + 5][r + 6] = A[r + 6][c + 5];
                    B[c + 6][r + 6] = A[r + 6][c + 6];
                    B[c + 7][r + 6] = A[r + 6][c + 7];

                    B[c + 4][r + 7] = A[r + 7][c + 4];
                    B[c + 5][r + 7] = A[r + 7][c + 5];
                    B[c + 6][r + 7] = A[r + 7][c + 6];
                    B[c + 7][r + 7] = A[r + 7][c + 7];

                    // Lower right done

                } else {
                    for(i = 0; i < 4; ++i){
                        a = A[c + i][r + 0];
                        s = A[c + i][r + 1];
                        d = A[c + i][r + 2];
                        f = A[c + i][r + 3];
                        g = A[c + i][r + 4];
                        h = A[c + i][r + 5];
                        j = A[c + i][r + 6];
                        k = A[c + i][r + 7];

                		B[r + 0][c + i + 0] = a;
                		B[r + 0][c + i + 4] = h;
                		B[r + 1][c + i + 0] = s;
                		B[r + 1][c + i + 4] = j;
                		B[r + 2][c + i + 0] = d;
                		B[r + 2][c + i + 4] = k;
                        B[r + 3][c + i + 0] = f;
                		B[r + 3][c + i + 4] = g;
                	}

                	a = A[c + 4][r + 4];
                	s = A[c + 5][r + 4];
                	d = A[c + 6][r + 4];
                	f = A[c + 7][r + 4];
                	g = A[c + 4][r + 3];
                	h = A[c + 5][r + 3];
                	j = A[c + 6][r + 3];
                	k = A[c + 7][r + 3];

                	B[r + 4][c + 0] = B[r + 3][c + 4];
                	B[r + 4][c + 4] = a;
                	B[r + 3][c + 4] = g;
                	B[r + 4][c + 1] = B[r + 3][c + 5];
                	B[r + 4][c + 5] = s;
                	B[r + 3][c + 5] = h;
                	B[r + 4][c + 2] = B[r + 3][c + 6];
                	B[r + 4][c + 6] = d;
                	B[r + 3][c + 6] = j;
                	B[r + 4][c + 3] = B[r + 3][c + 7];
                	B[r + 4][c + 7] = f;
                	B[r + 3][c + 7] = k;

                	for(i = 0; i < 3; ++i) {
                		a = A[c + 4][r + 5 + i];
                		s = A[c + 5][r + 5 + i];
                		d = A[c + 6][r + 5 + i];
                		f = A[c + 7][r + 5 + i];
                		g = A[c + 4][r + i];
                		h = A[c + 5][r + i];
                		j = A[c + 6][r + i];
                		k = A[c + 7][r + i];

                		B[r + 5 + i][c + 0] = B[r + i][c + 4];
                		B[r + 5 + i][c + 4] = a;
                		B[r + i][c + 4] = g;
                		B[r + 5 + i][c + 1] = B[r + i][c + 5];
                		B[r + 5 + i][c + 5] = s;
                		B[r + i][c + 5] = h;
                		B[r + 5 + i][c + 2] = B[r + i][c + 6];
                		B[r + 5 + i][c + 6] = d;
                		B[r + i][c + 6] = j;
                		B[r + 5 + i][c + 3] = B[r + i][c + 7];
                		B[r + 5 + i][c + 7] = f;
                		B[r + i][c + 7] = k;
                    }
                }
            }
        }
    }

    // Transpose matrix for any size
    // Try the same method used in case N = 32
    // size 16 gives 2009 misses (9.9/10)
    // size 20 gives 2030 misses (9.7/10)
    // size 18 gives 1981 misses (10/10)
    else {
        size = 18; // choose size
        for(r = 0; r < N; r += size) {
            for(c = 0; c < M; c += size) {
                for(i = r; i < r + size && i < N; ++i) {
                    for(j = c; j < c + size && j < M; ++j) {
                        if(i != j) {
                            // tmp = ;
                            B[j][i] = A[i][j];
                        }
                    }
                    if(r == c) {
                        // tmp = A[i][i];
                        B[i][i] = A[i][i];
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
