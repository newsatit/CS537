#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>      
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include "func.h"

int main(int argc, char* argv[]) {

	// int rc = chdir("/u/c/s/cs537-1/tests/p2a/history");
    // fflush(stdout);
    // if(rc == -1) {
    //     error();
    // }

	w_argc = 0;
	re_argc = 0;
	pi_argc = 0;

	fout = stdout;
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
		if (argc == 1) {
		    fprintf(fout, "wish> ");
			fflush(fout);
		}

		if (getline(&command, &len, fin) != -1) {
			for (int i = 0; i < w_argc; i++) {
				free(w_argv[i]);
				w_argc = 0;
			}
			for (int i = 0; i < pi_argc; i++) {
				free(pi_argv[i]);
				pi_argc = 0;
			}
			for (int i = 0; i < re_argc; i++) {
				free(re_argv[i]);
				re_argc = 0;
			}
			// remove '\n' from command
            command[strlen(command) - 1] = '\0';
			// add command to history	
			update_history(command);

			// TODO: move w_argc to later
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
				exit(0);
				// TODO: deal with non-integer args
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

			// printf("%d %d\n", re_argc, pi_argc);
			
			if (pi_argc > 2 || re_argc > 2 || (pi_argc > 1 && re_argc > 1)) {
				// printf("here\n");
				error();
				continue;
			}

			// piping
			if (pi_argc == 2) {

			}
			if (re_argc == 2) {
				for (int i = 0; i < w_argc; i++) {
					free(w_argv[i]);
				}
				w_argc = split(re_argv[0], w_argv, " ");
				// TODO: trim white space for file name
			}
			char *w_argv_0 = NULL;
			str_copy(&w_argv_0, w_argv[0]);
			for (int i = 0; i < paths_len; i++) {
				str_copy(&w_argv[0], paths[0]);
				strcat(w_argv[0], w_argv_0);

				int fd[2];
				if (pipe(fd) == -1) {
					error();
					continue;
				}
				if (access(w_argv[0], X_OK) == 0) {
					int rc = fork();
					if (rc == -1) {
						error();
						free(w_argv_0);
						continue;
					} else if (rc == 0) {
						// TODO: error()
						int out = 0;
						// printf("re_argc %d\n", re_argc);
						if (re_argc == 2) {
							// out = open(re_argv[1], O_WRONLY | O_CREAT | O_TRUNC, 0666);
							out = open(re_argv[1], O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);
							if(out == -1) {
								printf("%s\n", re_argv[1]);
								error();
								free(w_argv_0);
								continue;
							} else {
								dup2(out, STDOUT_FILENO);
								dup2(out, STDERR_FILENO);
								close(out);
							}
						}
						if(out != -1) {
							// printf("***\n");
							execv(w_argv[0], w_argv);
							// printf("rro shoud not be here\n");
							fflush(stdout);
							error();
							free(w_argv_0);
						}
						exit(1);
						// TODO: any errors >
						//return 0;
					} else {
						// TODO: error()
						// int *status = NULL;
						// waitpid(rc, status, 0);
						// printf("before\n");
						wait(NULL);
						// printf("after\n");
					}
				}
			}
			free(w_argv_0);
		} else {
			exit(0);
		}
	}
}
