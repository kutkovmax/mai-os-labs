#include <stdio.h>
#include <stdlib.h>
#include <time.h>
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

    srand(time(NULL));
    for (int i = 0; i < size; i++) {
        fputc(alphabet[rand() % 26], f);
    }

    // Вставим шаблон несколько раз случайно
    for (int i = 0; i < 5; i++) {
        int pos = rand() % (size - patlen);
        fseek(f, pos, SEEK_SET);
        fwrite(pattern, 1, patlen, f);
    }

    fclose(f);
    printf("Создан файл '%s' размером %d символов с вставками шаблона '%s'.\n",
           filename, size, pattern);
    return 0;
}
