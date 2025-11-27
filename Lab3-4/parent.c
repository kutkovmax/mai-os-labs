#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>

#define SHM_NAME "shared.bin"
#define MAX_NUMS 100

typedef struct {
    int count;
    float nums[MAX_NUMS];
} SharedData;

int main() {
    char filename[256];
    printf("Введите имя файла: ");
    scanf("%s", filename);
    getchar(); // убрать '\n'

    // создаём/очищаем отображаемый файл
    int shm_fd = open(SHM_NAME, O_RDWR | O_CREAT | O_TRUNC, 0666);
    if (shm_fd == -1) {
        perror("open shared file");
        exit(1);
    }

    // задаём размер памяти под структуру
    if (ftruncate(shm_fd, sizeof(SharedData)) == -1) {
        perror("ftruncate");
        close(shm_fd);
        exit(1);
    }

    SharedData *data = mmap(NULL, sizeof(SharedData),
                            PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (data == MAP_FAILED) {
        perror("mmap");
        close(shm_fd);
        exit(1);
    }

    data->count = 0;

    printf("Введите числа (через пробел, Enter — завершить ввод): ");

    float num;
    int c;
    char buffer[64];
    int i = 0;

    while ((c = getchar()) != '\n' && c != EOF) {
        if ((c >= '0' && c <= '9') || c == '.' || c == '-' || c == '+') {
            if (i < (int)sizeof(buffer) - 1)
                buffer[i++] = (char)c;
        } else if ((c == ' ' || c == '\t') && i > 0) {
            buffer[i] = '\0';
            num = strtof(buffer, NULL);
            if (data->count < MAX_NUMS)
                data->nums[data->count++] = num;
            i = 0;
        }
    }
    if (i > 0) { // последнее число перед Enter
        buffer[i] = '\0';
        num = strtof(buffer, NULL);
        if (data->count < MAX_NUMS)
            data->nums[data->count++] = num;
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        munmap(data, sizeof(SharedData));
        close(shm_fd);
        exit(1);
    }

    if (pid == 0) {
        execl("./child", "child", filename, NULL);
        perror("execl");
        exit(1);
    } else {
        if (wait(NULL) == -1) {
            perror("wait");
        }
    }

    if (munmap(data, sizeof(SharedData)) == -1)
        perror("munmap");

    if (close(shm_fd) == -1)
        perror("close");

    if (unlink(SHM_NAME) == -1)
        perror("unlink");

    return 0;
}
