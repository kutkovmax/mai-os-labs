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
        
        if (close(pipe_fd[0]) == -1) perror("close pipe_fd[0]");
        if (close(pipe_fd[1]) == -1) perror("close pipe_fd[1]");
        exit(1);
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        
        if (close(pipe_fd[0]) == -1) perror("close pipe_fd[0]");
        if (close(pipe_fd[1]) == -1) perror("close pipe_fd[1]");
        if (close(file_fd) == -1) perror("close file_fd");
        exit(1);
    }

    if (pid == 0) {
        // дочерний процесс
        if (dup2(pipe_fd[0], STDIN_FILENO) == -1) { 
            perror("dup2 pipe_fd[0]");
            exit(1);
        }
        if (close(pipe_fd[0]) == -1) { 
            perror("close pipe_fd[0]");
            exit(1);
        }
        if (close(pipe_fd[1]) == -1) { 
            perror("close pipe_fd[1]");
            exit(1);
        }

        if (dup2(file_fd, STDOUT_FILENO) == -1) { 
            perror("dup2 file_fd");
            exit(1);
        }
        if (close(file_fd) == -1) { 
            perror("close file_fd");
            exit(1);
        }

        execl("./child", "child", NULL);
        perror("exec"); // если exec не выполнился
        exit(1);
    } else {
        // родительский процесс
        if (close(pipe_fd[0]) == -1) { 
            perror("close pipe_fd[0]");
            exit(1);
        }
        if (close(file_fd) == -1) { 
            perror("close file_fd");
            exit(1);
        }

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
            if (write(pipe_fd[1], &num, sizeof(num)) == -1) { 
                perror("write (last)");
            } else {
                count++;
            }
        }

        printf("Отправлено чисел: %d\n", count);
        if (close(pipe_fd[1]) == -1) { 
            perror("close pipe_fd[1]");
            exit(1);
        }

        if (wait(NULL) == -1) { 
            perror("wait");
            exit(1);
        }
    }

    return 0;
}
