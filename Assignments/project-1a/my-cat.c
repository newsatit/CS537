#include <stdio.h>
int main(int argc, char *argv[]) {
    int file_counter = 1;
    while(file_counter <= argc) {
        FILE *fp = fopen(argv[file_counter], "r");
        if(fp == NULL) {
            printf("my-cat: cannot open file\n");
            exit(1);
        }
        file_counter++;
    }
    return 0;
}