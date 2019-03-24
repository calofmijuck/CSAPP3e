#include <bits/stdc++.h>
using namespace std;
#define N 8

int A[N][N], B[N][N];

int is_transpose(int z, int x) {
    int i, j;

    for (i = 0; i < z; i++) {
        for (j = 0; j < x; ++j) {
            if (A[i][j] != B[j][i]) {
                printf("%d %d, %d %d", i, j, A[i][j], B[j][i]);
                return 0;
            }
        }
    }
    return 1;
}

int main() {
    for(int i = 0; i < N; ++i) {
        for(int j = 0; j < N; ++j) {
            A[i][j] = N * i + j;
        }
    }


    int a, s, d, f, size = 8, r, c;


    for(r = 0; r < N; r += size) {
        for(c = 0; c < N; c += size) {
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
            //
            //
            //
            // a = A[r + 4][c];
            // A[r + 4][c] = B[c][r + 4];
            // B[c][r + 4] = a;
            //
            // a = A[r + 5][c];
            // A[r + 5][c] = B[c][r + 5];
            // B[c][r + 5] = a;
            //
            // a = A[r + 6][c];
            // A[r + 6][c] = B[c][r + 6];
            // B[c][r + 6] = a;
            //
            // a = A[r + 7][c];
            // A[r + 7][c] = B[c][r + 7];
            // B[c][r + 7] = a;
            //
            // a = A[r + 4][c + 1];
            // A[r + 4][c + 1] = B[c + 1][r + 4];
            // B[c + 1][r + 4] = a;
            //
            // a = A[r + 5][c + 1];
            // A[r + 5][c + 1] = B[c + 1][r + 5];
            // B[c + 1][r + 5] = a;
            //
            // a = A[r + 6][c + 1];
            // A[r + 6][c + 1] = B[c + 1][r + 6];
            // B[c + 1][r + 6] = a;
            //
            // a = A[r + 7][c + 1];
            // A[r + 7][c + 1] = B[c + 1][r + 7];
            // B[c + 1][r + 7] = a;
            //
            // a = A[r + 4][c + 2];
            // A[r + 4][c + 2] = B[c + 2][r + 4];
            // B[c + 2][r + 4] = a;
            //
            // a = A[r + 5][c + 2];
            // A[r + 5][c + 2] = B[c + 2][r + 5];
            // B[c + 2][r + 5] = a;
            //
            // a = A[r + 6][c + 2];
            // A[r + 6][c + 2] = B[c + 2][r + 6];
            // B[c + 2][r + 6] = a;
            //
            // a = A[r + 7][c + 2];
            // A[r + 7][c + 2] = B[c + 2][r + 7];
            // B[c + 2][r + 7] = a;
            //
            // a = A[r + 4][c + 3];
            // A[r + 4][c + 3] = B[c + 3][r + 4];
            // B[c + 3][r + 4] = a;
            //
            // a = A[r + 5][c + 3];
            // A[r + 5][c + 3] = B[c + 3][r + 5];
            // B[c + 3][r + 5] = a;
            //
            // a = A[r + 6][c + 3];
            // A[r + 6][c + 3] = B[c + 3][r + 6];
            // B[c + 3][r + 6] = a;
            //
            // a = A[r + 7][c + 3];
            // A[r + 7][c + 3] = B[c + 3][r + 7];
            // B[c + 3][r + 7] = a;

            // Upper right of B done

            // B[c + 4][r] = A[r + 4][c];
            // B[c + 4][r + 1] = A[r + 4][c + 1];
            // B[c + 4][r + 2] = A[r + 4][c + 2];
            // B[c + 4][r + 3] = A[r + 4][c + 3];
            //
            // B[c + 5][r] = A[r + 5][c];
            // B[c + 5][r + 1] = A[r + 5][c + 1];
            // B[c + 5][r + 2] = A[r + 5][c + 2];
            // B[c + 5][r + 3] = A[r + 5][c + 3];
            //
            // B[c + 6][r] = A[r + 6][c];
            // B[c + 6][r + 1] = A[r + 6][c + 1];
            // B[c + 6][r + 2] = A[r + 6][c + 2];
            // B[c + 6][r + 3] = A[r + 6][c + 3];
            //
            // B[c + 7][r] = A[r + 7][c];
            // B[c + 7][r + 1] = A[r + 7][c + 1];
            // B[c + 7][r + 2] = A[r + 7][c + 2];
            // B[c + 7][r + 3] = A[r + 7][c + 3];

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
        }
    }
    puts("A");
    for(int i = 0; i < N; ++i) {
        for(int j = 0; j < N; ++j) {
            printf("%5d", A[i][j]);
        }
        puts("");
    }

    puts("B");
    for(int i = 0; i < N; ++i) {
        for(int j = 0; j < N; ++j) {
            printf("%5d", B[i][j]);
        }
        puts("");
    }

    printf("%d", is_transpose(N, N));

    return 0;
}
