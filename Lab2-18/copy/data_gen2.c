#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char alphabet[] = "abcdefghijklmnopqrstuvwxyz";

int main(int argc, char *argv[]) {
    if (argc < 4) {
        printf("Использование: %s <имя_файла> <размер_строки> <образец>\n", argv[0]);
        return 1;
    }

    const char *filename = argv[1];
    int size = atoi(argv[2]);
    const char *pattern = argv[3];
    int patlen = strlen(pattern);

    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("Ошибка открытия файла");
        return 1;
    }

    // Период вставки шаблона — каждые примерно size / 10 символов
    int insert_period = size / 10;
    if (insert_period < patlen) insert_period = patlen * 2;

    int inserted = 0;
    int count = 0;

    while (count < size) {
        // Вставляем шаблон через равные промежутки
        if (inserted < 5 && count >= insert_period * inserted && count + patlen < size) {
            fwrite(pattern, 1, patlen, f);
            count += patlen;
            inserted++;
        } else {
            // Просто последовательные буквы без rand()
            char c = alphabet[count % 26];
            fputc(c, f);
            count++;
        }
    }

    fclose(f);

    printf("Создан файл '%s' длиной %d символов с %d вставками шаблона '%s'.\n",
           filename, size, inserted, pattern);
    return 0;
}
