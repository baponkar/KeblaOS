/*
Implementing a simple fork system call to create a new process.
The parent process waits for the child process to finish.
*/

#include <stdio.h>  // printf
#include <stdlib.h> // exit system call
#include <unistd.h> // fork system call
#include <sys/wait.h> // wait system call
#include <string.h> // string functions

int main(int argc, char *argv[]) {
    printf("hello world (pid:%d)\n", (int) getpid());
    int rc = fork(); // creating a new process
    if (rc < 0) {
        fprintf(stderr, "fork failed\n");
        exit(1);
    } else if (rc == 0) {
        printf("hello, I am child (pid:%d)\n", (int) getpid());
        char *myargs[3];
        myargs[0] = strdup("wc"); // program: "wc" (word count)
        myargs[1] = strdup("process_check.c"); // argument: file to count
        myargs[2] = NULL; // marks end of array
        execvp(myargs[0], myargs); // runs word count
        printf("this shouldn’t print out");
    } else {    // parent goes down this path (main)
        int rc_wait = wait(NULL);
        printf("hello, I am parent of %d (pid:%d)\n", rc, (int) getpid());
    }
    return 0;
}