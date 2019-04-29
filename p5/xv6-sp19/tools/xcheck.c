// gcc -iquote ../include -Wall -Werror -ggdb -o xcheck xcheck.c
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <limits.h>
#include <string.h>

#define stat xv6_stat   // avoid clash with host struct stat
#define dirent xv6_dirent // avoid clash with host struct stat
#include "types.h"
#include "fs.h"
#include "stat.h"
#undef stat
#undef dirent
#define block(i) (img_ptr + BSIZE * i)
#define ROUNDUP(i, A)  (((i)+A-1) & ~(A-1))
#define min(a, b) ( a < b ? a : b)

// xv6 fs img similar to vsfs
// unused | superblock | inode table | unused | bitmap(data) | data blocks

void print_inode(struct dinode dip);

void *img_ptr;
struct  superblock *sb;
struct dinode *dip;

void test1();
void test2();
void test3();
void test4();
void test5();
void test6();
void test7();
void test8();
void test9();
void test10();
void test11();
void test12();

int main(int argc, char *argv[]) {
    int fd;
    // Usage is something like <my prog> <fs.img>
    if (argc == 2) {
        fd = open(argv[1], O_RDONLY);
    } else {
        printf("Usage: program fs.img\n");
        exit(1);
    }

    if (fd < 0) {
        printf("Usage: %s file not found\n", argv[1]);
        exit(1);
    }

    struct stat sbuf;
    fstat(fd, &sbuf);

    img_ptr = mmap(NULL, sbuf.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    sb = (struct superblock *) (img_ptr + BSIZE);
    dip = (struct dinode *) (img_ptr + 2 * BSIZE);


    test1();
    test2();
    test3();
    test4();
    test5();
    test6();
    test7();
    test8();
    test9();
    test10();
    test11();
    test12();

    return 0;
}

void print_inode(struct dinode dip) {
    printf("file type:%d, ", dip.type);
    printf("nlink: %d, ", dip.nlink);
    printf("size:%d, ", dip.size);
    printf("first_addr:%d\n", dip.addrs[0]);
}

void test1() {

}

/*
For in-use inodes, each address that is used by inode is valid (points to a valid datablock address within the image). 
If the direct block is used and is invalid, print ERROR: bad direct address in inode.; 
if the indirect block is in use and is invalid, print ERROR: bad indirect address in inode.
*/
// TODO: indirect in indirect
void test2() {
    for (int i = 0; i < sb->ninodes; i++) {
        if (dip[i].type != 0) {
            // print_inode(dip[i]);
            int num_all_blocks = ROUNDUP(dip[i].size, BSIZE)/BSIZE;
            int num_dblocks = min(num_all_blocks, NDIRECT);
            int num_inblocks = num_all_blocks - NDIRECT;
            
            int first_dblock = 3 + ROUNDUP(sb->ninodes, IPB)/IPB + ROUNDUP(sb->nblocks, BPB)/BPB;
            int last_dblock = first_dblock + sb->nblocks - 1;
            // direct blocks
            for (int j = 0; j < num_dblocks; j++) {
                if (dip[i].addrs[j] < first_dblock || dip[i].addrs[j] > last_dblock) {
                    fprintf(stderr, "ERROR: bad direct address in inode.\n");
                    exit(1);
                }
            }

            // indirect blocks
            uint *ind = (uint*)block(dip[i].addrs[NDIRECT]);
            for (int j = 0; j < num_inblocks; j++) {
                if (ind[j] < first_dblock || ind[j] > last_dblock) {
                    fprintf(stderr, "ERROR: bad indirect address in inode.\n");
                    exit(1);
                }
            }

        }
    }
}

void test3() {

}

void test4() {

}

void test5() {

}

void test6() {

}

void test7() {

}

void test8() {

}

void test9() {

}

void test10() {

}

void test11() {

}

void test12() {

}