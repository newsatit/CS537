#include <stdio.h>
#include <stdlib.h>
#define BUFFER_SIZE 1000000
int main(int argc, char *argv[]) {
    for(int fi = 1; fi < argc; fi++){
        FILE *fp = fopen(argv[fi], "r");
        char buffer[BUFFER_SIZE];
        
        if(fp == NULL) {
            printf("my-cat: cannot open file\n");
            exit(1);
        }

        while(fgets(buffer, BUFFER_SIZE, fp) != NULL) {
            printf("%s", buffer);
        }
        fclose(fp);
    }
    return 0;
}
