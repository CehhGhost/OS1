#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>

#define BUFSIZE 5000

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <input_file> <output_file>\n", argv[0]);
        exit(1);
    }
    char* pipe1_name = "fifo1";
    char* pipe2_name = "fifo2";
    if (mkfifo(pipe1_name, 0666) == -1 || mkfifo(pipe2_name, 0666) == -1) {
        perror("mkfifo");
        exit(1);
    }

    pid_t pid1, pid2, pid3;

    pid1 = fork();
    if (pid1 == 0) {

        char buf[BUFSIZE];
        int input_file = open(argv[1], O_RDONLY);
        if (input_file == -1) {
            perror("open input_file");
            exit(1);
        }

        int fd = open(pipe1_name, O_WRONLY);
        if (fd == -1) {
            perror("open pipe");
            exit(1);
        }

        int bytes_read;
        while ((bytes_read = read(input_file, buf, BUFSIZE)) > 0) {
            write(fd, buf, bytes_read);
        }

        close(input_file);
        close(fd);

        exit(0);
    }

    pid2 = fork();
    if (pid2 == 0) {
        if (dup2(open(pipe1_name, O_RDONLY), STDIN_FILENO) == -1 ||
            dup2(open(pipe2_name, O_WRONLY), STDOUT_FILENO) == -1) {
            perror("dup2");
            exit(1);
        }

        execlp("./get_new_line", "get_new_line", NULL);

        exit(0);
    }

    pid3 = fork();
    if (pid3 == 0) {
        char buf[BUFSIZE];
        int output_file = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (output_file == -1) {
            perror("open output_file");
            exit(1);
        }

        int fd = open(pipe2_name, O_RDONLY);
        if (fd == -1) {
            perror("open pipe");
            exit(1);
        }

        int bytes_read;
        while ((bytes_read = read(fd, buf, BUFSIZE)) > 0) {
            write(output_file, buf, bytes_read);
        }

        close(output_file);
        close(fd);

        exit(0);
    }

    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);
    waitpid(pid3, NULL, 0);

    unlink(pipe1_name);
    unlink(pipe2_name);

    return 0;
}
