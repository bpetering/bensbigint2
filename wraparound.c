#include <stdio.h>

int main() {
    unsigned int n;
    unsigned int tmp;
    unsigned int i;

    n = 1 << 30;
    for (i = 0; i < 5; i++) {
        tmp = n;
        n *= 2;
        printf("i = %u\n", i);
        if (n < tmp) {
            printf("wrapround detected!\n");
            printf("-> old n: %u\n", tmp);
            printf("-> new n: %u\n\n", n);
        }
        else {
            printf("old n: %u\n", tmp);
            printf("new n: %u\n\n", n);
        }
    }

}
