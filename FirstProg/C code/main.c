#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define BUFSIZE 5000

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <input_file> <output_file>\n", argv[0]);
        return 1;
    }

    pid_t pid1, pid2;
    int fd1[2], fd2[2];

    if (pipe(fd1) < 0 || pipe(fd2) < 0) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    if ((pid1 = fork()) < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (pid1 == 0) {
        close(fd1[0]);
        close(fd2[0]);
        close(fd2[1]);

        int fd = open(argv[1], O_RDONLY);
        if (fd < 0) {
            perror("open");
            exit(EXIT_FAILURE);
        }

        char buf[BUFSIZE];
        ssize_t n;

        while ((n = read(fd, buf, BUFSIZE)) > 0) {
            if (write(fd1[1], buf, n) != n) {
                perror("write");
                exit(EXIT_FAILURE);
            }
        }

        if (n < 0) {
            perror("read");
            exit(EXIT_FAILURE);
        }

        close(fd1[1]);

        exit(EXIT_SUCCESS);
    }

    if ((pid2 = fork()) < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (pid2 == 0) {
        close(fd1[1]);
        close(fd2[0]);

        if (dup2(fd1[0], STDIN_FILENO) < 0) {
            perror("dup2");
            exit(EXIT_FAILURE);
        }

        if (dup2(fd2[1], STDOUT_FILENO) < 0) {
            perror("dup2");
            exit(EXIT_FAILURE);
        }

        execlp("./get_new_line", "./get_new_line", NULL);

        close(fd1[0]);
        close(fd2[1]);
    }
	close(fd1[0]);
    close(fd1[1]);
    close(fd2[1]);

    int fd = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fd < 0) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    char buf[BUFSIZE];
    ssize_t n;

    while ((n = read(fd2[0], buf, BUFSIZE)) > 0) {
        if (write(fd, buf, n) != n) {
            perror("write");
            exit(EXIT_FAILURE);
        }
    }

    if (n < 0) {
        perror("read");
        exit(EXIT_FAILURE);
    }

    close(fd);
    close(fd2[0]);

    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);

    return 0;
}
