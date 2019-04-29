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

struct stat sbuf;
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

    exit(0);
}

void print_inode(struct dinode dip) {
    printf("file type:%d, ", dip.type);
    printf("nlink: %d, ", dip.nlink);
    printf("size:%d, ", dip.size);
    printf("first_addr:%d\n", dip.addrs[0]);
}

// Each inode is either unallocated or one of the valid types (T_FILE, T_DIR, T_DEV). 
// If not, print ERROR: bad inode.
void test1() {
    for(int i = 0; i < sb->ninodes; i++){
        if(dip[i].type < 0 || dip[i].type > 3){
            fprintf(stderr, "ERROR: bad inode.\n");
            exit(1);
        }
    }
}

void test2() {

}

// Root directory exists, its inode number is 1, and the parent of the root directory is itself.
// If not, print ERROR: root directory does not exist.
void test3() {
    // printf("Ino: %ld \n", sbuf.st_ino);

    uint data_block_addr = dip[1].addrs[0];
    // printf("data_block_addr for / is %d\n", data_block_addr);
    struct xv6_dirent *entry = (struct xv6_dirent *)(img_ptr + data_block_addr * BSIZE);
    // for (int i = 0; i < 5; i++) {
    //     printf("name is %s inum is %d\n", entry[i].name, entry[i].inum);
    // }

    // for(int i = 0; i < dip[1].nlink; i++){
    //     if( (strcmp(entry[i].name,".") == 0 && entry[i].inum != 1) || (strcmp(entry[i].name,"..") == 0 && entry[i].inum != 1) ){
    //         fprintf(stderr, "ERROR: root directory does not exist.");
    //         exit(1);
    //     }
        
    // }

    if (entry[0].inum != 1 || entry[1].inum != 1){
        fprintf(stderr, "ERROR: root directory does not exist.\n");
        exit(1);
    }
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