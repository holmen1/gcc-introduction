#include <stdio.h>

double powern(double d, unsigned n)
{
    double x = 1.0;
    for (unsigned i = 1; i <= n; i++)
        x *= d;

    return x;
}

int main(void)
{
    double sum = 0.0;
    for (unsigned i = 1; i <= 1000000000; i++)
        sum += powern(i, i % 5);

    printf("Sum = %g\n", sum);
    return 0;
}
