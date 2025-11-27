#include "../contract.h"

int isPrime(int x)
{
    if (x < 2) return 0;

    for (int i = 2; i < x; i++)
        if (x % i == 0)
            return 0;

    return 1;
}

int PrimeCount(int A, int B)
{
    int count = 0;
    for (int i = A; i <= B; i++)
        if (isPrime(i))
            count++;
    return count;
}
