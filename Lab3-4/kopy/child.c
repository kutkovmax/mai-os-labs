#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <math.h>

#define _R_  "\x1b[31m"
#define _G_  "\x1b[32m"
#define RESET_COLOR "\x1b[0m"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: child <filename>\n");
        exit(1);
    }

    FILE* out = fopen(argv[1], "w");
    if (!out) {
        perror("fopen");
        exit(1);
    }

    bool isFirst = true;
    float res = 0, num;

    while (read(STDIN_FILENO, &num, sizeof(num)) > 0) {
        if (isFirst) {
            res = num;
            isFirst = false;
        } else {
            if (num == 0) { 
                printf(_R_ "Division by zero\n" RESET_COLOR);
                fflush(stdout);
                fclose(out);
                exit(2);
            }
            res /= num;
        }
    }

    if (!isFirst) {
        fprintf(out, "Результат = %f\n", res);
    } else {
        printf("Нет данных для вычисления\n");
    }

    fclose(out);

    printf(_G_ "OK\n" RESET_COLOR);
    fflush(stdout);
    return 0;
}
