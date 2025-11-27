#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include "contract.h"

#define BUF 256

typedef int (*PrimeFunc)(int, int);
typedef float (*EFunc)(int);

void* handle = NULL;
PrimeFunc PrimeCountPtr = NULL;
EFunc EPtr = NULL;
int currentLib = 1;

const char* lib1 = "./lib1/libimpl1.so";
const char* lib2 = "./lib2/libimpl2.so";

/* Загрузка библиотеки */
int loadLibrary(const char* path)
{
    if (handle)
    {
        if (dlclose(handle) != 0)
            printf("Warning: dlclose error: %s\n", dlerror());
    }

    handle = dlopen(path, RTLD_LAZY);
    if (!handle)
    {
        printf("Error loading %s: %s\n", path, dlerror());
        return 0;
    }

    dlerror();

    PrimeCountPtr = (PrimeFunc)dlsym(handle, "PrimeCount");
    char* err = dlerror();
    if (err)
    {
        printf("Error loading PrimeCount: %s\n", err);
        return 0;
    }

    EPtr = (EFunc)dlsym(handle, "E");
    err = dlerror();
    if (err)
    {
        printf("Error loading E: %s\n", err);
        return 0;
    }

    printf("Library loaded: %s\n", path);
    return 1;
}

void switchLibrary()
{
    currentLib = (currentLib == 1 ? 2 : 1);
    const char* path = (currentLib == 1 ? lib1 : lib2);

    if (!loadLibrary(path))
    {
        printf("Fatal error: cannot load library\n");
        exit(1);
    }
}

int main()
{
    char input[BUF];
    int cmd;

    printf("Program2 (dynamic loading mode)\n");

    if (!loadLibrary(lib1))
    {
        printf("Startup failed\n");
        return 1;
    }

    while (1)
    {
        printf("\nCommands:\n");
        printf(" 0     -> switch library\n");
        printf(" 1 A B -> PrimeCount\n");
        printf(" 2 X   -> E\n");
        printf(" -1    -> exit\n> ");

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

        if (cmd == 0)
        {
            switchLibrary();
            continue;
        }

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

            printf("PrimeCount(%d, %d) = %d\n", A, B, PrimeCountPtr(A, B));
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

            printf("E(%d) = %.6f\n", X, EPtr(X));
            continue;
        }

        printf("Unknown command\n");
    }

    if (handle)
    {
        if (dlclose(handle) != 0)
            printf("Warning: dlclose error: %s\n", dlerror());
    }

    printf("Program2 terminated\n");
    return 0;
}
