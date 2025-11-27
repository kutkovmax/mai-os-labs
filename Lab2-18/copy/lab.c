#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>

#define CHUNK_SIZE (1024 * 1024) // 1 МБ

typedef struct {
    const char *filename;
    long start;
    long end;
    const char *pattern;
    int pat_len;
    int found;
} ThreadData;

int count_occurrences(const char *buf, size_t len, const char *pattern, int pat_len) {
    int count = 0;
    for (size_t i = 0; i + pat_len <= len; i++) {
        int j = 0;
        while (j < pat_len && buf[i + j] == pattern[j]) j++;
        if (j == pat_len) count++;
    }
    return count;
}

void *thread_search(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    FILE *f = fopen(data->filename, "rb"); // бинарный режим
    if (!f) {
        perror("Ошибка открытия файла");
        data->found = 0;
        return NULL;
    }

    char *buffer = malloc(CHUNK_SIZE + data->pat_len);
    if (!buffer) {
        perror("Ошибка выделения памяти");
        fclose(f);
        data->found = 0;
        return NULL;
    }

    data->found = 0;
    long offset = data->start;

    while (offset < data->end) {
        size_t to_read = CHUNK_SIZE;
        if (offset + to_read > data->end)
            to_read = data->end - offset;

        fseek(f, offset, SEEK_SET);
        size_t read_bytes = fread(buffer, 1, to_read, f);
        if (read_bytes == 0 || read_bytes <= data->pat_len - 1) break;

        size_t search_len = read_bytes;
        if (offset + read_bytes < data->end)
            search_len += data->pat_len - 1;

        data->found += count_occurrences(buffer, search_len, data->pattern, data->pat_len);
        offset += read_bytes - (data->pat_len - 1);
    }

    free(buffer);
    fclose(f);
    return NULL;
}

double get_time_diff(struct timespec start, struct timespec end) {
    return (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
}

int main(int argc, char *argv[]) {
    if (argc < 4) {
        printf("Использование: %s <файл> <образец> <кол-во_потоков>\n", argv[0]);
        return 1;
    }

    const char *filename = argv[1];
    const char *pattern = argv[2];
    int num_threads = atoi(argv[3]);
    int pat_len = strlen(pattern);

    FILE *f = fopen(filename, "rb"); // бинарный режим
    if (!f) {
        perror("Ошибка открытия файла");
        return 1;
    }
    fseek(f, 0, SEEK_END);
    long file_size = ftell(f);
    fclose(f);

    pthread_t *threads = malloc(num_threads * sizeof(pthread_t));
    ThreadData *tdata = malloc(num_threads * sizeof(ThreadData));

    long chunk_size = file_size / num_threads;
    for (int i = 0; i < num_threads; i++) {
        tdata[i].filename = filename;
        tdata[i].pattern = pattern;
        tdata[i].pat_len = pat_len;
        tdata[i].start = i * chunk_size;
        tdata[i].end = (i == num_threads - 1) ? file_size : (i + 1) * chunk_size + pat_len - 1;
        tdata[i].found = 0;
    }

    struct timespec start_time, end_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);

    for (int i = 0; i < num_threads; i++)
        pthread_create(&threads[i], NULL, thread_search, &tdata[i]);

    int total_found = 0;
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
        total_found += tdata[i].found;
    }

    clock_gettime(CLOCK_MONOTONIC, &end_time);
    double elapsed = get_time_diff(start_time, end_time);

    printf("file: %s\n", filename);
    printf("%s\n", pattern);
    printf("threads: %d\n", num_threads);
    printf("count: %d\n", total_found);
    printf("time: %.6f s\n", elapsed);

    free(threads);
    free(tdata);
    return 0;
}
