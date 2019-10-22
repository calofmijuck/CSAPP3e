#include <stdio.h>
// void **list = NULL;
int *l0 = 0 , *l1 = 0, *l2 = 0, *l3 = 0, *l4 = 0;
int **list = &l0;
int main() {
    printf("Hello\n");
    printf("&l0 = %lld\n", (long long) (&l0));
    printf("&l1 = %lld\n", (long long) (&l1));
    printf("&l2 = %lld\n", (long long) (&l2));
    printf("&l3 = %lld\n", (long long) (&l3));
    printf("&l4 = %lld\n", (long long) (&l4));
    printf("list = %lld\n", (long long) (list + 1));
    *(list + 1) = 2;
    printf("*l1 = %lld\n", (long long) l1);
    // int a[5] = {1, 2, 3, 4, 5};
    // l1 = &a[1];
    // l2 = &a[2];
    // printf("&l1 = %lld\n", (long long) (&l1));
    // printf("%d\n", **(&l0 + 1));
    // printf("%d\n", &l0 + 1 == &l1);
    // *(&l0+1) = 4;
    // printf("l1: %d\n", l1);
    // **(&l0+2) = 1010;
    // printf("l2: %lld\n", (long long) *l2);
}
