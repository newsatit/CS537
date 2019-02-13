#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define BUFFER_SIZE 1000000

char ret_line[BUFFER_SIZE];

char* replace(char *line, char *old, char *new) {
	int old_len = strlen(old);
	int new_len = strlen(new);
	char *ret_s = ret_line;
	char *s = line;
	while(*s) {
		if(strstr(s, old) == s) {
			strcpy(ret_s, new);
			ret_s +=  new_len;
			s +=  old_len;
		} else {
			*ret_s = *s;
			ret_s++;
			s++;
		}
	}
	*ret_s = '\0';
	return ret_line;
}

int main(int argc, char *argv[]) {
	if(argc < 3 ) {
		printf("my-sed: find_term replace_term [file ...]\n");
		exit(1);
	}	

	if(argc == 3) {
		char *buffer = NULL;	
		size_t len = 0;
		while(getline(&buffer, &len, stdin) != -1) {
			char *str = replace(buffer, argv[1], argv[2]);
			printf("%s", str);
		}
	} else {
        for(int fi = 3; fi < argc; fi++){
            FILE *fp = fopen(argv[fi], "r");
            char *buffer = NULL;
            size_t len = 0;

            if(fp == NULL) {
                printf("my-sed: cannot open file\n");
                exit(1);
            }

            while(getline(&buffer, &len, fp) != -1) {
				char *str = replace(buffer, argv[1], argv[2]);
				printf("%s", str);
            }
            fclose(fp);
        }
	}
    return 0;
}
