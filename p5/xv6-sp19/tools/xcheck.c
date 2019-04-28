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
    // printf("Image that i read is %ld in size\n", sbuf.st_size);

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

}

void print_inode(struct dinode dip) {
    printf("file type:%d, ", dip.type);
    printf("nlink: %d, ", dip.nlink);
    printf("size:%d, ", dip.size);
    printf("first_addr:%d\n", dip.addrs[0]);
}

void test1() {

}

void test2() {

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