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

int main(int argc, char* argv[]) {

	w_argc = 0;
	re_argc = 0;
	pi_argc = 0;

	fout = stdout;
	if (argc > 2) {
		error();
        exit(1);
	}
	if (argc == 2) {
		fin = fopen(argv[1], "r");
		if (fin == NULL) {
			error();
            exit(1);
		}	
	} else {
		fin = stdin;
	}
	char *command = NULL;
	size_t len = 0;

	str_copy(&paths[0], "/bin/");
	paths_len = 1;

	while (1) {
		if (argc == 1) {
		    fprintf(fout, "wish> ");
			fflush(fout);
		}
        if (command != NULL) {
            free(command);
            command = NULL;
        }
		if (getline(&command, &len, fin) != -1) {

            w_argc = free_args(w_argv, w_argc);
            pi_argc = free_args(pi_argv, pi_argc);
            re_argc = free_args(re_argv, re_argc);
			// remove '\n' from command
            command[strlen(command) - 1] = '\0';
			// add command to history	
			update_history(command);

			w_argc = split(command, w_argv, " ");
			if (w_argc == 0) {
				continue;
			}
			// for (int i = 0; i < w_argc; i++) {
			// 	printf("command: %s\n", w_argv[i]);
			// }
			if (!strcmp(w_argv[0], "exit")) {
				if (argc != 1) {
					fclose(fin);
				}
				if (w_argc > 1) {
					error();
					continue;
				}
                if (command != NULL) {
                    free(command);
                    command = NULL;
                }
                w_argc = free_args(w_argv, w_argc);
                pi_argc = free_args(pi_argv, pi_argc);
                re_argc = free_args(re_argv, re_argc);
                paths_len = free_args(paths, paths_len);
                hist_count = free_args(hist, hist_count);
				exit(0);
			} else if (!strcmp(w_argv[0], "history")) {
				history();
				continue;
			} else if (!strcmp(w_argv[0], "cd")) { ;
				cd();
				continue;
			} else if (!strcmp(w_argv[0], "path")) {
				path();
				continue;
			}

			re_argc = split(command, re_argv, ">");
			pi_argc = split(command, pi_argv, "|");
		
			if (pi_argc > 2 || re_argc > 2 || (pi_argc > 1 && re_argc > 1)) {
				error();
				continue;
			}
			
			// TODO: piping
            if (pi_argc == 2) {
                pipe_run();
            } else {
                /**
                 * redirecting and normal command call 
                 **/
                no_pipe_run();                
            }
		} else {
            if (command != NULL) {
                free(command);
                command = NULL;
            }
            w_argc = free_args(w_argv, w_argc);
            pi_argc = free_args(pi_argv, pi_argc);
            re_argc = free_args(re_argv, re_argc);
            paths_len = free_args(paths, paths_len);
            hist_count = free_args(hist, hist_count);
            exit(0);
		}
	}
 }



