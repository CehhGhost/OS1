#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>

#define BUFSIZE 5000

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <input_file> <output_file>\n", argv[0]);
        exit(1);
    }

    int fd1[2], fd2[2];

    if (pipe(fd1) == -1 || pipe(fd2) == -1) {
        perror("pipe");
        exit(1);
    }

    pid_t pid1, pid2;

    pid1 = fork();
    if (pid1 == 0) {
        close(fd1[0]);
        close(fd2[1]);

        char buf[BUFSIZE];
        int input_fd = open(argv[1], O_RDONLY);
        if (input_fd == -1) {
            perror("open input_file");
            exit(1);
        }

        int n;
        while ((n = read(input_fd, buf, BUFSIZE)) > 0) {
            write(fd1[1], buf, n);
        }

        close(fd1[1]);
        close(input_fd);

        int output_fd = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (output_fd == -1) {
            perror("open output_file");
            exit(1);
        }

        while ((n = read(fd2[0], buf, BUFSIZE)) > 0) {
            write(output_fd, buf, n);
        }

        close(fd2[0]);
        close(output_fd);

        exit(0);
    }

    pid2 = fork();
    if (pid2 == 0) {
        close(fd1[1]);
        close(fd2[0]);

        if (dup2(fd1[0], STDIN_FILENO) == -1 ||
            dup2(fd2[1], STDOUT_FILENO) == -1) {
            perror("dup2");
            exit(1);
        }

        execlp("./get_new_line", "get_new_line", NULL);
        exit(0);
    }

    close(fd1[0]);
    close(fd1[1]);
    close(fd2[0]);
    close(fd2[1]);

    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);

    return 0;
}
