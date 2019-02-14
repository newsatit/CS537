#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

FILE *fin, *fout;

// file descriptors
int fd[2];

// TODO: size of commands(history)
char *hist[100000];
int hist_count;

// strings for pipe
char *pi_argv[100000];
int pi_argc;
// strings for redirection
char *re_argv[100000];
int re_argc;

char *w_argv[100000];
int w_argc;

char *paths[100000];
int paths_len;

/**
 * Copies the string b to a. Free and allocate new memory for a if necessary
 * @param a destination string
 * @param b source string
 */
void str_copy(char **a, char *b) {
    *a = (char*)malloc(sizeof(b));
    strcpy(*a, b);
}

void error() {
    char error_message[30] = "An error has occurred\n";
    fprintf(stderr, "%s\n", strerror(errno));
    write(STDERR_FILENO, error_message, strlen(error_message));
}

void history() {
    // TODO: deal with non-integer and zero
    int start = 0;
    if (w_argc > 2) {
        error();
    }
    if (w_argc == 2) {
        start = hist_count - atoi(w_argv[1]);
        start = start < 0 ? 0 : start;
    }
    for (int i = start; i < hist_count; i++) {
        fprintf(fout, "%s\n", hist[i]);
        fflush(fout);
    }
}

void cd() {
    if(w_argc != 2) {
        error();
        return;
    }

//    printf("*%s*\n", w_argv[1]);
//    fflush(stdout);
    int rc = chdir(w_argv[1]);
    fflush(fout);
    if(rc == -1) {
        error();
    }
}

void path() {
    for (int i = 0; i < paths_len; i++) {
        free(paths[i]);
    }
    paths_len = w_argc - 1;
    for (int i = 0; i < paths_len; i++) {
        str_copy(&paths[i], w_argv[i + 1]);
        int len = strlen(paths[i]);
        if(paths[i][len - 1] != '/') {
            char *temp = (char*)malloc(sizeof(paths[i]) + 1);
            strcpy(temp, paths[i]);
            temp[len] = '/';
            temp[len + 1] = '\0';
            paths[i] = temp;
        }
    }
//    fprintf(stdout, "no. of paths : %d\n", paths_len);
//    for (int i = 0; i < paths_len; i++) {
//        fprintf(stdout, "%s\n", paths[i]);
//    }
}

void update_history(char *command) {
    hist[hist_count] = (char*)malloc(sizeof(char) * strlen(command));
    strcpy(hist[hist_count++], command);
}

void print_command(char *commands[], int n) {
    for(int i = 0; i < n; i++) {
        fprintf(fout, "*%s*\n", commands[i]);
    }
}

int split(char *command, char *argv[], char *delim) {

    char *full_command = NULL;
    str_copy(&full_command, command);
    char* token = NULL;

    token = strtok(full_command, delim);
    int argc = 0;

    while (token != NULL) {
        str_copy(&argv[argc++], token);
        token = strtok(NULL, delim);
    }
    argv[argc] = NULL;
//    printf("full size %ld\n", sizeof(full_command));
    free(full_command);
//    printf("okay\n");
    return argc;
}

void remove_newline(char **command) {
    // get ride of new line char
    strncpy(*command, *command, strlen(*command) - 1);
    *command[strlen(*command) - 1] = '\0';
}




//
// Created by Dawanit Satitsumpum on 2019-02-12.
//

