#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>


int main(int argc, char *argv[]) {
    /* SHELL PROCESS */
    printf("I'm SHELL process, with PID: %d - Main command is: man ping | grep -A 2 -e '-w' > output.txt\n", getpid());

    int fd[2];

    if (pipe(fd) < 0) {
        fprintf(stderr, "pipe failed\n");
        exit(1);
    }

    pid_t cpid_1;
    cpid_1 = fork();

    if (cpid_1 < 0) {
        fprintf(stderr, "man fork failed\n");
        exit(1);
    }

    else if (cpid_1 == 0) {
        /* CHILD OF SHELL - MAN PROCESS */
        printf("I'm MAN process, with PID: %d - My command is: man ping\n", getpid());

        /* duplicate fd[1] to STDOUT_FILENO */
        if (dup2(fd[1], STDOUT_FILENO) < 0) {
            perror("dup2 pipe write end to STDOUT failed");
            _exit(1);
        }

        /* close both read and write ends */
        close(fd[0]);
        close(fd[1]);

        /* execute man command */
        char* args[] = {"man", "ping", NULL};
        execvp(args[0], args);
    }

    else {
        waitpid(cpid_1, NULL, 0);
        pid_t cpid_2;
        cpid_2 = fork();

        if (cpid_2 < 0) {
            fprintf(stderr, "grep fork failed\n");
            exit(1);
        }

        else if (cpid_2 == 0) {
            /* CHILD OF SHELL - GREP PROCESS */
            printf("I'm GREP process, with PID: %d - My command is: grep -A 2 -e '-w' > output.txt\n", getpid());

            /* duplicate fd[0] to STDIN_FILENO */
            if (dup2(fd[0], STDIN_FILENO) < 0) {
                perror("dup2 pipe read end to STDIN failed");
                _exit(1);
            }

            /* close both write and read ends */
            close(fd[1]);
            close(fd[0]);

            /* create a new file to write the grep command's output */
            int new_fd = open("output.txt", O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
                
            /* duplicate new_fd to STDIN_FILENO */
            if (dup2(new_fd, STDOUT_FILENO) < 0) {
                perror("dup2 new file to STDOUT failed");
                _exit(1);
            }

            close(new_fd);

            /* execute grep command */
            char* args[] = {"grep", "-A 2", "-e", "-w", NULL};
            execvp(args[0], args);
        }

        else {
            /* close both write and read ends */
            close(fd[1]);
            close(fd[0]);

            waitpid(cpid_2, NULL, 0);

            printf("I'm SHELL process, with PID: %d - execution is completed, you can find the results in output.txt\n", getpid());
        }
    }

    return 0;
}
