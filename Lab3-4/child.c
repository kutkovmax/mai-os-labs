#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <math.h>
#include <errno.h>
#include <string.h>
#include <signal.h>


#define SHM_NAME "shared.bin"
#define MAX_NUMS 100

typedef struct {
    int count;
    float nums[MAX_NUMS];
} SharedData;

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <output_file>\n", argv[0]);
        exit(1);
    }

    int shm_fd = open(SHM_NAME, O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("open shared file");
        exit(1);
    }

    SharedData *data = mmap(NULL, sizeof(SharedData),
                            PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (data == MAP_FAILED) {
        perror("mmap");
        close(shm_fd);
        exit(1);
    }

    if (data->count == 0) {
        printf("Нет данных для вычисления\n");
        munmap(data, sizeof(SharedData));
        close(shm_fd);
        return 0;
    }

    float res = data->nums[0];
    for (int i = 1; i < data->count; i++) {
        if (fabs(data->nums[i]) < 1e-6) {
            fprintf(stderr, "Ошибка: деление на 0\n");
            munmap(data, sizeof(SharedData));
            close(shm_fd);
            kill(getppid(), SIGTERM);
            exit(2);
        }
        res /= data->nums[i];
    }

    FILE *out = fopen(argv[1], "w");
    if (!out) {
        perror("fopen");
        munmap(data, sizeof(SharedData));
        close(shm_fd);
        exit(1);
    }

    fprintf(out, "Результат = %f\n", res);
    fclose(out);

    if (munmap(data, sizeof(SharedData)) == -1)
        perror("munmap");
    if (close(shm_fd) == -1)
        perror("close");

    return 0;
}
