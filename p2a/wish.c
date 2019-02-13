#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include "func.h"

int main(int argc, char* argv[]) {

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



			if (pi_argc > 2 || re_argc > 2 || (pi_argc > 1 && re_argc > 1)) {
				error();
			}

			// piping
			if (pi_argc == 2) {

			}

			int exec_rc = 0;

			// redirection
			if (re_argc == 2) {
				w_argc = split(re_argv[0], w_argv, " ");
				char *w_argv_0 = NULL;
				// save original first wish argument
				str_copy(&w_argv_0, w_argv[0]);
				str_copy(&w_argv[0], paths[0]);
				strcat(w_argv[0], w_argv_0);

				// TODO: open
				int rc = fork();
				if(rc == -1) {
					error();
				}
				if (rc == 0) {
					int out = open(re_argv[1], O_WRONLY | O_CREAT | O_TRUNC, 0666);
					if(out == -1) {
						error();
						continue;
					}
					dup2(out, STDOUT_FILENO);
					dup2(out, STDERR_FILENO);
					close(out);
					exec_rc = execv(w_argv[0], w_argv);
				} else {
					wait(NULL);
					if(exec_rc == 0)
						continue;
				}
			} else {
				char *w_argv_0 = NULL;
				// save original first wish argument
				str_copy(&w_argv_0, w_argv[0]);
				str_copy(&w_argv[0], paths[0]);
				strcat(w_argv[0], w_argv_0);

				if (!access(w_argv[0], X_OK)) {
					int rc = fork();
					if(rc == -1) {
						error();
						continue;
					}
					if (rc == 0) {
						exec_rc = execv(w_argv[0], w_argv);
					} else {
						wait(NULL);
						// TODO: what is this?
						if(exec_rc == 0)
							continue;
					}
				}

				str_copy(&w_argv[0], w_argv_0);
				if (!strcmp(w_argv[0], "exit")) {
					fprintf(stdout, "exit\n");
					if (argc == 2) {
						fclose(fin);
					}
					if(w_argc > 1) {
						error();
						continue;
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
			}

		} else {
			exit(0);
		}
	}
}
