#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define BUFFER_SIZE 1000000
int main(int argc, char *argv[]) {
	char *prev_buffer = (char*)malloc(sizeof(char) * BUFFER_SIZE);
	char *cur_buffer = (char*)malloc(sizeof(char) * BUFFER_SIZE);

	if(argc == 1) {
		strcpy(prev_buffer, "");
		size_t len = 0;
		while(getline(&cur_buffer, &len, stdin) != -1) {
			if(strcmp(prev_buffer, cur_buffer)){
				printf("%s", cur_buffer);	
			}
			strcpy(prev_buffer, cur_buffer);
		}
	} else {
		for(int fi = 1; fi < argc; fi++){
			strcpy(prev_buffer, "");
			FILE *fp = fopen(argv[fi], "r");
			size_t len = 0;

			if(fp == NULL) {
				printf("my-uniq: cannot open file\n");
				exit(1);
			}

			while(getline(&cur_buffer, &len, fp) != -1) {
				if(strcmp(prev_buffer, cur_buffer)){
					printf("%s", cur_buffer);
				}
				strcpy(prev_buffer, cur_buffer);
			}

			fclose(fp);
		}
	}
    return 0;
}
