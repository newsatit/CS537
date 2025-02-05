#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>      
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <limits.h>
#include "func.h"

FILE *fin, *fout;

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

// first arguments for piping
char *arg1[100000];
int arg1c = 0;

// second arguments for piping
char *arg2[100000];
int arg2c = 0;


/**
 * Copies the string b to a. Free and allocate new memory for a if necessary
 * @param a destination string
 * @param b source string
 */
void str_copy(char **a, char *b) {
    if(*a != NULL) {
        free(*a);
    }

    int len = strlen(b);
    *a = (char*)malloc(sizeof(char) * (len + 2));
    strcpy(*a, b);
}

int free_args(char *argv[], int argc) {
    for (int i = 0; i < argc; i++) {
        if (argv[i] != NULL) {
            free(argv[i]);
            argv[i] = NULL;            
        }

    }
    return 0;
}

void error() {
    char error_message[30] = "An error has occurred\n";
    //  fprintf(stderr, "%d: %s\n", errno, strerror(errno));
    write(STDERR_FILENO, error_message, strlen(error_message));
}

void history() {
    int start = 0;
    if (w_argc > 2) {
        error();
        return;
    }
    if (w_argc == 2) {

        /**
         * parse the argument 
         **/
        char *endptr, *str;
        long val;
        int base = 10;
        str = w_argv[1];

        val = strtol(str, &endptr, base);
        if ((errno == ERANGE && (val == LONG_MAX || val == LONG_MIN))
                || (errno != 0 && val == 0)) {
            error();
            return;
        }

        if (endptr == str) {
            error();
            return;
        } 

        // round up if it's decimal       
        if (*endptr != '\0') {
            if (*endptr == '.') {
                val++;
            }
        }

        start = hist_count - (int) val;
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
    int rc = chdir(w_argv[1]);
    fflush(fout);
    if(rc == -1) {
        error();
    }
}

void path() {
    paths_len = free_args(paths, paths_len);
    paths_len = w_argc - 1;
    for (int i = 0; i < paths_len; i++) {
        str_copy(&paths[i], w_argv[i + 1]);
        int len = strlen(paths[i]);
        // append '/' for the path that does not end with '/'
        if(paths[i][len - 1] != '/') {
            char *temp = NULL;
            str_copy(&temp, paths[i]);
            temp[len] = '/';
            temp[len + 1] = '\0';
            if (paths[i] != NULL) {
                free(paths[i]);
            }
            paths[i] = temp;
        }
    }
}

void pipe_run() {
    w_argc = free_args(w_argv, w_argc);

    arg1c = split(pi_argv[0], arg1, " ");
    arg2c = split(pi_argv[1], arg2, " ");

    if (arg1c == 0 || arg2c == 0) {
        error(); 
        return;
    }

    char *arg1_0 = NULL;
    char *arg2_0 = NULL;

    int not_found = 1;

    for (int i = 0; i < paths_len; i++) {

        if (arg1_0 != NULL) {
            free(arg1_0);
            arg1_0 = NULL;
        }
        if (arg2_0 != NULL) {
            free(arg2_0);
            arg2_0 = NULL;
        }

        arg1_0 = (char*)malloc(sizeof(char) * (strlen(paths[i]) + strlen(arg1[0]) + 1));
        arg2_0 = (char*)malloc(sizeof(char) * (strlen(paths[i]) + strlen(arg2[0]) + 1));
        strcpy(arg1_0, paths[i]);
        strcpy(arg2_0, paths[i]);
        strcat(arg1_0, arg1[0]);
        strcat(arg2_0, arg2[0]);

        if(access(arg1_0, X_OK) == 0 && access(arg2_0, X_OK) == 0) {
            not_found = 0;
            int fd[2];
            int pc = pipe(fd);
            if (pc == -1) {
                arg1c = free_args(arg1, arg1c);
                arg2c = free_args(arg2, arg2c);
                error();
                break;
            }
            int rc1 = fork();
            if (rc1 < 0) {
                error();
                break;
            } else if (rc1 == 0) {
                dup2(fd[1], STDOUT_FILENO);
                close(fd[0]);
                execv(arg1_0, arg1);
                break; 
            } else {
                int rc2 = fork();
                if (rc2 < 0) {
                    error();
                    break;
                } else if (rc2 == 0) {
                    dup2(fd[0], STDIN_FILENO);
                    close(fd[1]);
                    execv(arg2_0, arg2);
                    break;
                } else {
                    close(fd[1]);
                    close(fd[0]);
                    wait(NULL);
                    wait(NULL);
                    break;
                }
            }
            
        }

    }
    if (not_found) {
        error();
    }
    if (arg1_0 != NULL) {
        free(arg1_0);
        arg1_0 = NULL;
    }
    if (arg2_0 != NULL) {
        free(arg2_0);
        arg2_0 = NULL;
    }
    arg1c = free_args(arg1, arg1c);
    arg2c = free_args(arg2, arg2c);
}

void no_pipe_run() {
    // for storing argument with path
    char *w_argv_0 = NULL;

    if (re_argc == 2) {
        w_argc = free_args(w_argv, w_argc);
        w_argc = split(re_argv[0], w_argv, " ");
        if (w_argc == 0) {
            error();
            return;
        }
        // trim white space for file name
        char* token = NULL;
        char* temp = NULL;
        str_copy(&temp, re_argv[1]);
        char* rest = temp; 
        while ((token = strtok_r(rest, " ", &rest))) {
            str_copy(&re_argv[1], token);
        }
        free(temp);
    }

    int not_found = 1;
    for (int i = 0; i < paths_len; i++) {
        if (w_argv_0 != NULL) {
            free(w_argv_0);
            w_argv_0 = NULL;
        }

        w_argv_0 = (char*)malloc(sizeof(char) * (strlen(paths[i]) + strlen(w_argv[0]) + 1));
        strcpy(w_argv_0, paths[i]);
        strcat(w_argv_0, w_argv[0]);

        if (access(w_argv_0, X_OK) == 0) {
            not_found = 0;
            int rc = fork();
            if (rc == -1) {
                error();
                break;
            } else if (rc == 0) {
                int out = 0;
                if (re_argc == 2) {
                    out = open(re_argv[1], O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);
                    if(out == -1) {
                        error();
                    } else {
                        dup2(out, STDOUT_FILENO);
                        dup2(out, STDERR_FILENO);
                        close(out);
                    }
                }
                if(out != -1) {
                    execv(w_argv_0, w_argv);
                    error();
                }
                if (w_argv_0 != NULL) {
                    free(w_argv_0);
                    w_argv_0 = NULL;
                }
                exit(1);
            } else {
                wait(NULL);
                break;
            }
        } 
    }
    if (not_found) {
        error();
    }
    if (w_argv_0 != NULL) {
        free(w_argv_0);
        w_argv_0 = NULL;
    }
}

void update_history(char *command) {
    if (command[0] == '\0')
        return; 
    str_copy(&hist[hist_count++], command);
}

// for degugging
void print_command(char *commands[], int n) {
    for(int i = 0; i < n; i++) {
        fprintf(fout, "*%s*\n", commands[i]);
    }
}


// parse the arguments from the command
int split(char *command, char *argv[], char *delim) {

    char *full_command = NULL;
    str_copy(&full_command, command);
    char* token = NULL;

    char* rest = full_command;; 
    int argc = 0;

    while ((token = strtok_r(rest, delim, &rest))) {
        str_copy(&argv[argc++], token);
    }
    argv[argc] = NULL;

    if (full_command != NULL) {
        free(full_command);
        full_command = NULL;
    }   
    return argc;
}



