#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
// TODO: size of commands(history)
char *hist[1000];
int hist_count;
void printerr() {
	char error_message[30] = "An error has occurred\n";
	write(STDERR_FILENO, error_message, strlen(error_message));	
} 
void history() {
	for(int i = 0; i < hist_count; i++) {
		printf("%s\n", hist[i]);
	}
}

void add_history(char *command) {
	hist[hist_count] = (char*)malloc(sizeof(char) * strlen(command));	
	strcpy(hist[hist_count++], command);
}
int main(int argc, char* argv[]) {
	FILE *fp;
	
	if (argc > 2) {
		printerr();
		exit(1);
	}
	if (argc == 2) {
		fp = fopen(argv[1], "r");
		if (fp == NULL) {
			printerr();
			exit(1);
		}	
	} else {
		fp = stdin;
	}
	char *command = NULL;
	size_t len = 0;
	while (1) {
		if (argc != 2)
			printf("wish> ");
		if (getline(&command, &len, fp) != -1) {
			// TODO: deal with white space
			// get ride of new line char	
			strncpy(command, command, strlen(command) - 1);
			command[strlen(command) - 1] = '\0';
			// add command to history	
			add_history(command);
			printf("*%s*\n", command);
			
			if (!strcmp(command, "exit")) {
				if (argc == 2) {
					fclose(fp);
				}
				exit(0);
			} else if (!strcmp(command, "history")) {
				history();
			} 
		}
	}
}
