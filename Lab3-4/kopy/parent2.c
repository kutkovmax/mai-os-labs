#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <ctype.h>

#define FILENAME_MAX_LEN 256

int main() {
    int pipe_fd[2];

    if (pipe(pipe_fd) == -1) {
        perror("pipe");
        exit(1);
    }

    char filename[FILENAME_MAX_LEN];
    printf("Введите имя файла: ");
    scanf("%s", filename);
    getchar(); // съесть '\n' после имени файла

    int file_fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (file_fd == -1) {
        perror("open");
        exit(1);
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(1);
    }

    if (pid == 0) {
        // дочерний процесс
        dup2(pipe_fd[0], STDIN_FILENO);
        close(pipe_fd[0]);
        close(pipe_fd[1]);

        dup2(file_fd, STDOUT_FILENO);
        close(file_fd);

        execl("./child", "child", NULL);
        perror("exec");
        exit(1);
    } else {
        // родительский процесс
        close(pipe_fd[0]);
        close(file_fd);

        printf("Введите числа (через пробел, Enter — завершить ввод): ");

        float num;
        int count = 0;
        int c;
        char buffer[64]; // временный буфер для одного числа
        int i = 0;

        while ((c = getchar()) != '\n' && c != EOF) {
            if (isdigit(c) || c == '.' || c == '-' || c == '+') {
                if (i < (int)sizeof(buffer) - 1)
                    buffer[i++] = (char)c;
            } else if (isspace(c) && i > 0) {
                buffer[i] = '\0';
                num = strtof(buffer, NULL);
                if (write(pipe_fd[1], &num, sizeof(num)) == -1) {
                    perror("write");
                    break;
                }
                count++;
                i = 0;
            }
        }

        // обработка последнего числа перед '\n'
        if (i > 0) {
            buffer[i] = '\0';
            num = strtof(buffer, NULL);
            if (write(pipe_fd[1], &num, sizeof(num)) != -1)
                count++;
        }

        printf("Отправлено чисел: %d\n", count);
        close(pipe_fd[1]);
        wait(NULL);
    }

    return 0;
}
