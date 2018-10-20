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
char history[HISTORY_SIZE][MAX_LINE];
bool parentwait = true;
bool savetohistory = true;

void print_history() {
    if (strcmp(history[0], "") == 0)
        printf("No commands have been ran yet\n");
    else {
        for (int i = 0; i < HISTORY_SIZE; i++) {
            if (strcmp(history[i], "") != 0)
                printf("%d. %s", (i + 1), history[i]);
        }
    }
}

void add_to_history() {
    if (history[HISTORY_SIZE - 1] != NULL)
        history[HISTORY_SIZE - 1] == NULL;
    for (int i = HISTORY_SIZE - 1; i >= 1; i--) {
        strcpy(history[i], history[i - 1]);
    }
    strcpy(history[0], buffer);
}

void handle_SIGINT() {
    //TODO - Print History
    fflush(stdin);
    *buffer = ""; // clear buffer so it doesn't try to run previous input automatically
    printf("\n");
    print_history();
    savetohistory = false;
}

int main(int argc, char** argv) {

    // Set up Signal Handler, referring to Project 1 PDF
    struct sigaction handler;
    handler.sa_handler = handle_SIGINT;
    sigaction(SIGINT, &handler, NULL);


    char *args[MAX_LINE / 2 + 1]; /* command line (of 80) has max of 40 arguments */
    int should_run = 1;

    while (should_run) {

        printf("osh>");
        fgets(buffer, BUFFER_SIZE, stdin);

        //Add to history
        if (buffer[0] == 'r')
            if (buffer[1] == ' ') {
                int i;
                for (i = 0; i <= HISTORY_SIZE; i++) {
                    if (buffer[2] == history[i][0]) {
                        strcpy(buffer, history[i]);
                        break;
                    }
                }
                if (i > HISTORY_SIZE)
                    printf("No recent commands with that first letter\n");
            } else {
                strcpy(buffer, history[0]);
            }
        if (savetohistory) {
            add_to_history();
        } else {
            savetohistory = true;
        }
        // Break input down into tokens
        char *token;
        token = strtok(buffer, " \n");
        if (token == NULL) {
            continue;
        } else if (strcmp(token, "exit") == 0) {
            should_run = 0;
            break;
        }
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