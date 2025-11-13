#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>

typedef struct {
    const char *text;
    int text_len;
    const char *pattern;
    int pat_len;
    int start_idx;
    int end_idx;
    int *matches;
    int matches_count;
} ThreadData;

// Наивный алгоритм поиска
void *naive_search_thread(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    data->matches_count = 0;
    
    for (int i = data->start_idx; i < data->end_idx; i++) {
        if (i + data->pat_len > data->text_len) break;
        
        int j = 0;
        while (j < data->pat_len && data->text[i + j] == data->pattern[j]) {
            j++;
        }
        if (j == data->pat_len) {
            data->matches[data->matches_count++] = i;
        }
    }
    
    return NULL;
}

double get_time_diff(struct timespec start, struct timespec end) {
    return (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
}

// Функция для чтения всей строки из stdin
char* read_input_string() {
    int capacity = 1024;
    int size = 0;
    char *str = malloc(capacity);
    int ch;
    
    while ((ch = getchar()) != '\n' && ch != EOF) {
        if (size >= capacity - 1) {
            capacity *= 2;
            str = realloc(str, capacity);
        }
        str[size++] = ch;
    }
    str[size] = '\0';
    
    return str;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Использование: %s <количество_потоков>\n", argv[0]);
        printf("Пример: %s 4\n", argv[0]);
        return 1;
    }

    char *text;
    int text_len;
    char pattern[256];
    int num_threads;
    int pat_len;
    struct timespec start_time, end_time;
    // Чтение строки из стандартного ввода
    text = read_input_string();
    text_len = strlen(text);

    if (text_len == 0) {
        printf("Ошибка: строка не может быть пустой!\n");
        free(text);
        return 1;
    }

    // Чтение образца для поиска
    if (scanf("%255s", pattern) != 1) {
        printf("Ошибка чтения образца!\n");
        free(text);
        return 1;
    }

    num_threads = atoi(argv[1]);

    if (num_threads <= 0) {
        printf("Ошибка: количество потоков должно быть положительным числом!\n");
        free(text);
        return 1;
    }

    pat_len = strlen(pattern);

    if (pat_len == 0) {
        printf("Ошибка: образец не может быть пустым!\n");
        free(text);
        return 1;
    }

    if (pat_len > text_len) {
        printf("Ошибка: образец длиннее строки!\n");
        free(text);
        return 1;
    }

    // Создание структур для потоков
    pthread_t *threads = malloc(num_threads * sizeof(pthread_t));
    ThreadData *tdata = malloc(num_threads * sizeof(ThreadData));
    int **all_matches = malloc(num_threads * sizeof(int*));
    
    // Выделяем память для результатов
    int max_matches_per_thread = text_len / num_threads + pat_len;
    for (int i = 0; i < num_threads; i++) {
        all_matches[i] = malloc(max_matches_per_thread * sizeof(int));
    }

    int chunk_size = text_len / num_threads;


    // Начинаем замер времени ТОЛЬКО для ожидания завершения работы потоков
    clock_gettime(CLOCK_MONOTONIC, &start_time);

    // Создание потоков (НЕ учитывается во времени)
    for (int i = 0; i < num_threads; i++) {
        tdata[i].text = text;
        tdata[i].text_len = text_len;
        tdata[i].pattern = pattern;
        tdata[i].pat_len = pat_len;
        tdata[i].start_idx = i * chunk_size;
        tdata[i].end_idx = (i == num_threads - 1) ? text_len : (i + 1) * chunk_size;
        tdata[i].matches = all_matches[i];
        tdata[i].matches_count = 0;
        
        if (pthread_create(&threads[i], NULL, naive_search_thread, &tdata[i]) != 0) {
            printf("Ошибка создания потока %d\n", i);
            return 1;
        }
    }

    
    

    // Ожидание завершения потоков и сбор результатов
    int total_matches = 0;
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
        total_matches += tdata[i].matches_count;
    }

    clock_gettime(CLOCK_MONOTONIC, &end_time);
    double elapsed = get_time_diff(start_time, end_time);

    // Вывод ТОЛЬКО CSV результата
    printf("RESULTS_CSV: %d,%d,%d,%.6f,%d\n", 
           text_len, pat_len, num_threads, elapsed, total_matches);

    // Освобождение памяти
    free(text);
    free(threads);
    free(tdata);
    for (int i = 0; i < num_threads; i++) {
        free(all_matches[i]);
    }
    free(all_matches);

    return 0;
}