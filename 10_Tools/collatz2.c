#include <stdio.h>

unsigned int step(unsigned int x)
{
    return (x % 2 == 0) ? x / 2 : 3 * x + 1;
}

unsigned int stepn(unsigned int x0)
{
    unsigned int i = 1, x;

    x = step(x0);
    while (x != 1) {
        x = step(x);
        i++;
    }
    return i;
}

int main(void)
{
    printf("0\tn = 1\n");
    printf("1\tn = 1\n");
    unsigned int n;
    for (int k = 2; k < 5000000; k++) {
        n = stepn(k);
        printf("%d\tn = %d\n", k, n);
    }
    return 0;
}
