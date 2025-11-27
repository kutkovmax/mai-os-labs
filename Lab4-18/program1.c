#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "contract.h"

#define BUF 256

int main()
{
    char input[BUF];
    int cmd;

    printf("Program1 (static linking)\n");

    while (1)
    {
        printf("\nCommands:\n");
        printf(" -1        -> exit\n");
        printf(" 1 A B     -> PrimeCount\n");
        printf(" 2 X       -> E\n> ");

        if (!fgets(input, BUF, stdin))
        {
            printf("\nEOF received. Exit.\n");
            break;
        }

        if (sscanf(input, "%d", &cmd) != 1)
        {
            printf("Invalid command\n");
            continue;
        }

        if (cmd == -1)
            break;

        if (cmd == 1)
        {
            int A, B;
            if (sscanf(input, "%d %d %d", &cmd, &A, &B) != 3)
            {
                printf("Invalid input. Format: 1 A B\n");
                continue;
            }

            if (A < 1 || B < 1 || A > B)
            {
                printf("Error: A, B must be natural and A <= B\n");
                continue;
            }

            printf("PrimeCount(%d, %d) = %d\n", A, B, PrimeCount(A, B));
            continue;
        }

        if (cmd == 2)
        {
            int X;
            if (sscanf(input, "%d %d", &cmd, &X) != 2)
            {
                printf("Invalid input. Format: 2 X\n");
                continue;
            }

            if (X <= 0)
            {
                printf("Error: X must be > 0\n");
                continue;
            }

            printf("E(%d) = %.6f\n", X, E(X));
            continue;
        }

        printf("Unknown command\n");
    }

    printf("Program1 terminated\n");
    return 0;
}
