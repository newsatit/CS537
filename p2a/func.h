#ifndef _FUNC_H
#define _FUNC_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

extern FILE *fin;

extern int fd[2];

// TODO: size of commands(history)
extern char *hist[100000];
extern int hist_count;

// strings for pipe
extern char *pi_argv[100000];
extern int pi_argc;
// strings for redirection
extern char *re_argv[100000];
extern int re_argc;

extern char *w_argv[100000];
extern int w_argc;

extern char *paths[100000];
extern int paths_len;

void str_copy(char **a, char *b);

void error();

void history();

void cd();

void path();

void update_history(char *command);

void print_command(char *commands[], int n);

int split(char *command, char *argv[], char *delim);

void remove_newline(char **command);


// test function


#endif
