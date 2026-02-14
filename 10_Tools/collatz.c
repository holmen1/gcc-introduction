#include <stdio.h>

unsigned int step(unsigned int x)
{
    return (x % 2 == 0) ? x / 2 : 3 * x + 1;
}

unsigned int stepn(unsigned int x0)
{
    unsigned int i = 1, x;
    if (x0 == 0 || x0 == 1)
        return i;

    x = step(x0);
    while (x != 1 && x != 0) {
        x = step(x);
        i++;
    }
    return i;
}

int main(void)
{
    unsigned int n;
    for (int k = 0; k < 500; k++) {
        n = stepn(k);
        printf("%d\tn = %d\n", k, n);
    }
    return 0;
}
