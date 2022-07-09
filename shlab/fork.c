# include <sys/types.h>
# include <sys/wait.h>
# include <unistd.h>
# include <stdio.h>
# include <stdlib.h>
# include <errno.h>
# include <string.h>
#include <signal.h>

#define N 2
static size_t rc;
static size_t sec;

void unix_error(char *msg){
    fprintf(stderr, "%s: %s\n", msg, strerror(errno));
    exit(0);
}

size_t snooze(size_t sec){
    rc = sleep(sec);

    printf("Slept for %ld of %ld seconds.\n", sec-rc, sec);
    exit(0);
}

void sigHandler(int sig){
    return;
}

int main(int argc, char* argv[], char* env[]){
    // pid_t pid = fork();
    // if (pid < 0)
    //     unix_error("fork error");
    // else if (pid == 0){
    //     pause();
    //     printf("control should never reach here!\n");
    //     exit(0);
    // }
    // if (kill(pid, 9) < 0)
    //     unix_error("kill error");
    // exit(0);
    // if (argc != 2)
    //     unix_error("format: ./fork sec");
    // sec = *argv[1] - '0';
    // printf("sec: %ld\n", sec);

    // if (signal(SIGINT , sigHandler) == SIG_ERR)
    //     unix_error("signal error");
    // snooze(sec);
    int i;
    char buffer[256];
    printf ("Enter a number: ");
    fgets (buffer, 256, stdin);
    i = atoi (buffer);
    printf ("The value entered is %d.", i);
    //system("pause");
    return 0;
}