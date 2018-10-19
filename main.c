#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>

#define BUFFER_SIZE 50
char buffer[BUFFER_SIZE];
#define MAX_LINE  80 /* 80 chars per line, per command */
#define HISTORY_SIZE 10
int historyPointer = 0;
char history[HISTORY_SIZE][MAX_LINE];
bool parentwait = true;
bool sighand = false;

void print_history() {
    for(int i = 0; i < historyPointer; i++) {
        printf("%s", history[i]);
    }
}
void handle_SIGINT() {
    //TODO - Print History
    fflush(stdin);
    *buffer = ""; // clear buffer so it doesn't try to run previous input automatically
    printf("\n");
    print_history();
    sighand = true;
}

int main(int argc, char** argv) {

    // Set up Signal Handler, referring to Project 1 PDF
    struct sigaction handler;
    handler.sa_handler = handle_SIGINT;
    sigaction(SIGINT, &handler, NULL);


    char *args[MAX_LINE / 2 + 1]; /* command line (of 80) has max of 40 arguments */
    int should_run = 1;

    int i;

    while (should_run) {

        printf("osh>");
        fgets(buffer, BUFFER_SIZE, stdin);
        
        //Add to history
        if(!sighand) {
            strcpy(history[historyPointer], buffer);
        historyPointer++;
        } else {
            sighand = false;
        }
        
        // Break input down into tokens
        char *token;
        token = strtok(buffer, " \n");
        int i = 0; // For iterating through args
        while (token != NULL) {
            args[i++] = token;
            token = strtok(NULL, " \n");
        }
        // Set to null and tell program to not wait on child
        if (*args[i - 1] == '&') {
            args[i - 1] = NULL;
            parentwait = false;
        } else {
            args[i] = NULL; //args needs to end with a null character
        }

        pid_t pid = fork();
        if (pid < 0) {
            perror("Forking error");
            _exit(2);
        }
        if (pid == 0) {
            execvp(args[0], args);
            _exit(0);
        } else if (pid > 0) {
            int status;
            if (parentwait)
                wait(&status); // gets status of child    
            else
                parentwait = true; // Parent doesn't wait
        }              
    }
    return 0;
}