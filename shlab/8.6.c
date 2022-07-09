# include <sys/types.h>
# include <sys/wait.h>
# include <unistd.h>
# include <stdio.h>
# include <stdlib.h>
# include <errno.h>
# include <string.h>

#define N 2

void unix_error(char *msg){
    fprintf(stderr, "%s: %s\n", msg, strerror(errno));
    exit(0);
}

int main(int argc, char* argv[], char* env[]){
    for(int i = 0; i < argc; ++i){
        printf("argv[%d]: %s\n", i, argv[i]);
    }
    int count  = 0;
    while(env[count] != NULL){
        printf("envp[%d]: %s\n", count, env[count]);
        ++count;
    }
}