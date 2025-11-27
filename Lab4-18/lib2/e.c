#include "../contract.h"

float E(int x)
{
    float sum = 1.0f;
    float fact = 1;

    for (int i = 1; i <= x; i++)
    {
        fact *= i;
        sum += 1.0f / fact;
    }

    return sum;
}
