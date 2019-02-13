#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include "func.h"

int main(int argc, char* argv[]) {

	FILE *fin;
	
	if (argc > 2) {
		error();
	}
	if (argc == 2) {
		fin = fopen(argv[1], "r");
		if (fin == NULL) {
			error();
		}	
	} else {
		fin = stdin;
	}
	char *command = NULL;
	size_t len = 0;


	str_copy(&paths[0], "/bin/");
	paths_len = 1;

	while (1) {
		if (argc != 2) {
		    printf("wish> ");
		}

		if (getline(&command, &len, fin) != -1) {
			// remove '\n' from command
            command[strlen(command) - 1] = '\0';
			// add command to history	
			update_history(command);

			// TODO: move w_argc to later
			w_argc = split(command, w_argv, " ");
			re_argc = split(command, re_argv, ">");
			pi_argc = split(command, pi_argv, "|");

            char *w_argv_0 = NULL;
            // save original first wish argument
			str_copy(&w_argv_0, w_argv[0]);
			str_copy(&w_argv[0], paths[0]);
			strcat(w_argv[0], w_argv_0);

			if(pi_argc > 2 || re_argc > 2 || (pi_argc > 1 && re_argc > 1)) {
				error();
			}

			int exec_rc = 0;

			if (!access(w_argv[0], X_OK)) {
				int rc = fork();
				if (rc == 0) {
					exec_rc = execv(w_argv[0], w_argv);
				} else if (rc == -1) {
					error();
				} else {
				    wait(NULL);
					if(exec_rc == 0)
						continue;
				}
			}

			str_copy(&w_argv[0], w_argv_0);
			if (!strcmp(w_argv[0], "exit")) {
				printf("exit\n");
				if (argc == 2) {
					fclose(fin);
				}
				if(w_argc > 1) {
					error();
				}
				exit(0);
				// TODO: deal with non-integer args
			} else if (!strcmp(w_argv[0], "history")) {
				history();
			} else if (!strcmp(w_argv[0], "cd")) {
				cd();
			} else if (!strcmp(w_argv[0], "path")) {
				path();
			} else {
				error();
			}
		} else {
			exit(0);
		}
	}
}
