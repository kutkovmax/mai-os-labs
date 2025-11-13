#include "stdio.h"
#include "stdlib.h"
#include "unistd.h"
#include "sys/wait.h"

#define _R_            "\x1b[31m"
#define _G_            "\x1b[32m"
#define RESET_COLOR    "\x1b[0m"

int main() {
    int pipe1_fd[2];
    int pipe2_fd[2];

    if (pipe(pipe1_fd) == -1 || pipe(pipe2_fd) == -1) {
        perror("pipe");
        exit(1);
    }

    char filename[256];
    printf("Введите имя файла: ");
    scanf("%s", filename);

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(1);
    }

    if (pid == 0) {  
        dup2(pipe1_fd[0], STDIN_FILENO);
        close(pipe1_fd[0]);
        close(pipe1_fd[1]);

        dup2(pipe2_fd[1], STDOUT_FILENO);
        close(pipe2_fd[0]);
        close(pipe2_fd[1]);

        execl("./child", "child", filename, NULL);
        perror("exec");
        exit(1);
    } else {  
        close(pipe1_fd[0]);
        close(pipe2_fd[1]);

        float num;
        int status;

        while (scanf("%f", &num) != EOF) {
            if (waitpid(pid, &status, WNOHANG) > 0) {
                break;
            }

            if (write(pipe1_fd[1], &num, sizeof(num)) == -1) {
                break;
            }
        }
        close(pipe1_fd[1]);

        char buf[128];
        int n;
        while ((n = read(pipe2_fd[0], buf, sizeof(buf)-1)) > 0) {
            buf[n] = '\0';
            printf("Msg frm child: %s\n", buf);
        }
        close(pipe2_fd[0]);
    }

    printf(RESET_COLOR);
    return 0;
}
