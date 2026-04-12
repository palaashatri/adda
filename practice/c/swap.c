#include<stdio.h>

// using pointers, swap two numbers

int main() {
    int a, b;
    printf("Enter two numbers: ");
    scanf("%d %d", &a, &b);

    // swap using pointers
    int *p1 = &a;
    int *p2 = &b;

    int temp = *p1;
    *p1 = *p2;
    *p2 = temp;

    printf("After swapping: %d %d\n", a, b);
    return 0;
}