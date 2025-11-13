#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <math.h>

#define _R_  "\x1b[31m"
#define _G_  "\x1b[32m"
#define RESET_COLOR "\x1b[0m"

int main(int argc, char *argv[]) {
    bool isFirst = true;
    float res = 0, num;
    int count = 0; // Для отладки: считаем числа

    while (read(STDIN_FILENO, &num, sizeof(num)) > 0) {
        fprintf(stderr, "Получено число %d: %f\n", ++count, num); // Отладочный вывод
        if (isFirst) {
            res = num;
            isFirst = false;
        } else {
            if (fabs(num) < 1e-6) {
                fprintf(stderr, _R_ "Division by zero\n" RESET_COLOR);
                fflush(stderr);
                exit(2);
            }
            res /= num;
        }
    }
    if (read(STDIN_FILENO, &num, sizeof(num)) == -1) {
        perror("read");
    }

    if (!isFirst) {
        printf("Результат = %f\n", res);
    } else {
        printf("Нет данных для вычисления\n");
    }

    fprintf(stderr, _G_ "OK\n" RESET_COLOR); // Изменено: выводим в stderr
    fflush(stderr);
    return 0;
}