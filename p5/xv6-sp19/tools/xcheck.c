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

struct stat sbuf;
void *img_ptr;
struct  superblock *sb;
struct dinode *dip;
int first_dblock;
int last_dblock;
char* barray;

void test1();
void test2();
void test3();
void test4();
void test56();
void test78();
void test910();
void test1112();
void extra1();

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
        fprintf(stderr, "image not found.\n");
        exit(1);
    }

    fstat(fd, &sbuf);

    img_ptr = mmap(NULL, sbuf.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    sb = (struct superblock *) (img_ptr + BSIZE);
    dip = (struct dinode *) (img_ptr + 2 * BSIZE);

    first_dblock = BBLOCK((sb->nblocks-1), sb->ninodes) + 1;
    last_dblock = first_dblock + sb->nblocks - 1;

    barray = calloc(sb->nblocks,sizeof(char));

    test1();
    test2();
    test4();
    test3();
    test78();
    test56();
    test910();
    test1112();

    free(barray);

    return 0;
}

void print_inode(struct dinode dip) {
    printf("file type:%d, ", dip.type);
    printf("nlink: %d, ", dip.nlink);
    printf("size:%d, ", dip.size);
    printf("first_addr:%x\n", dip.addrs[0]);
}

// Each inode is either unallocated or one of the valid types (T_FILE, T_DIR, T_DEV). 
// If not, print ERROR: bad inode.
void test1() {
    for(int i = 1; i < sb->ninodes + 1; i++){
        if(dip[i].type < 0 || dip[i].type > 3){
            fprintf(stderr, "ERROR: bad inode.\n");
            exit(1);
        }
    }
}

/*
For in-use inodes, each address that is used by inode is valid (points to a valid datablock address within the image). 
If the direct block is used and is invalid, print ERROR: bad direct address in inode.; 
if the indirect block is in use and is invalid, print ERROR: bad indirect address in inode.
*/
void test2() {
    for (int i = 1; i < sb->ninodes + 1; i++) {
        if (dip[i].type != 0) {
            // print_inode(dip[i]);
            int num_all_blocks = ROUNDUP(dip[i].size, BSIZE)/BSIZE;
            int num_dblocks = min(num_all_blocks, NDIRECT);
            int num_inblocks = num_all_blocks - NDIRECT;
            
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

// Root directory exists, its inode number is 1, and the parent of the root directory is itself.
// If not, print ERROR: root directory does not exist.
void test3() {
    int num_all_blocks = ROUNDUP(dip[1].size, BSIZE)/BSIZE;
    int num_dblocks = min(num_all_blocks, NDIRECT);
    int num_inblocks = num_all_blocks - NDIRECT;

    if(dip[1].type != T_DIR){
        fprintf(stderr, "ERROR: root directory does not exist.\n");
        exit(1);
    }

    // direct blocks
    for (int j = 0; j < num_dblocks; j++) {
        struct xv6_dirent *entry = (struct xv6_dirent *)(img_ptr + dip[1].addrs[j] * BSIZE);
        for(int k = 0; k < BSIZE/sizeof(struct xv6_dirent); k++){
            if( (!strcmp(entry[k].name,".") || !strcmp(entry[k].name,"..")) && entry[k].inum != 1 ){
                fprintf(stderr, "ERROR: root directory does not exist.\n");
                exit(1);
            }
        }
    }

    // indirect blocks
    uint *ind = (uint*)block(dip[1].addrs[NDIRECT]);
    for (int j = 0; j < num_inblocks; j++) {
        struct xv6_dirent *entry = (struct xv6_dirent *)(img_ptr + ind[j] * BSIZE);
        for(int k = 0; k < BSIZE/sizeof(struct xv6_dirent); k++){
            if( (!strcmp(entry[k].name,".") || !strcmp(entry[k].name,"..")) && entry[k].inum != 1 ){
                fprintf(stderr, "ERROR: root directory does not exist.\n");
                exit(1);
            }
        }
    }
}


/*
Each directory contains . and .. entries, and the . entry points to the directory itself. 
If not, print ERROR: directory not properly formatted.
*/
void test4() {
    for (int i = 1; i < sb->ninodes + 1; i++) {
        if (dip[i].type == T_DIR) {
            // has current directory "."
            int cur_dir = 0;
            // has parent directory ".."
            int parent_dir = 0;
            int num_all_blocks = ROUNDUP(dip[i].size, BSIZE)/BSIZE;
            int num_dblocks = min(num_all_blocks, NDIRECT);
            int num_inblocks = num_all_blocks - NDIRECT;
            
           // direct blocks
            for (int j = 0; j < num_dblocks; j++) {
                uint data_block_addr = dip[i].addrs[j];
                struct xv6_dirent *entry = (struct xv6_dirent *)(img_ptr + data_block_addr * BSIZE);
                for (int k = 0; k < DIRSIZ; k++) {
                    if (!strcmp(entry[k].name, ".")) {
                        cur_dir = 1;
                        if (entry[k].inum != i) {
                            fprintf(stderr, "ERROR: directory not properly formatted.\n");
                            exit(1);
                        }
                    } else if (!strcmp(entry[k].name, "..")) {
                        parent_dir = 1;
                    }
                }
            } 

            // indirect blocks
            uint *ind = (uint*)block(dip[i].addrs[NDIRECT]);
            for (int j = 0; j < num_inblocks; j++) {
                uint data_block_addr = ind[j];
                struct xv6_dirent *entry = (struct xv6_dirent *)(img_ptr + data_block_addr * BSIZE);
                for (int k = 0; k < DIRSIZ; k++) {
                    if (!strcmp(entry[k].name, ".")) {
                        cur_dir = 1;
                        if (entry[k].inum != i) {
                            fprintf(stderr, "ERROR: directory not properly formatted.\n");
                            exit(1);
                        }
                    } else if (!strcmp(entry[k].name, "..")) {
                        parent_dir = 1;
                    }
                }
            }
        
            if (cur_dir == 0|| parent_dir == 0) { 
                fprintf(stderr, "ERROR: directory not properly formatted.\n");
                exit(1);
            }
        }
    }
}

// For in-use inodes, each direct address in use is only used once. 
// If not, print ERROR: direct address used more than once.
// For in-use inodes, each indirect address in use is only used once.
// If not, print ERROR: indirect address used more than once.
void test78() {
    
    // printf("inodes = %d\n", sb->ninodes);
    //iterate through all inodes
    for(int i = 1; i < sb->ninodes + 1; i++){
        // printf("inode %d\n", i);
        if (dip[i].type != 0) {
            // print_inode(dip[i]);
            // int num_all_blocks = ROUNDUP(dip[i].size, BSIZE)/BSIZE;
            int num_all_blocks = dip[i].size/BSIZE;
            if (dip[i].size % BSIZE > 0) {
                num_all_blocks++;
            }
            int num_dblocks = min(num_all_blocks, NDIRECT);
            int num_inblocks = num_all_blocks - NDIRECT;
                
            // direct blocks
            for (int j = 0; j < num_dblocks; j++) {
                // printf("direct block %d\n", dip[i].addrs[j] - first_dblock);
                if (barray[dip[i].addrs[j] - first_dblock] != 0) {
                    fprintf(stderr, "ERROR: direct address used more than once.\n");
                    exit(1);
                }else{
                    barray[dip[i].addrs[j] - first_dblock] = 1;
                }
            }

            if(num_inblocks > 0){
                // printf("immediate block %d\n", dip[i].addrs[NDIRECT] - first_dblock);
                if (barray[dip[i].addrs[NDIRECT] - first_dblock] != 0) {
                    fprintf(stderr, "ERROR: direct address used more than once.\n");
                    exit(1);
                }else{
                    barray[dip[i].addrs[NDIRECT] - first_dblock] = 1;
                }
            }


            // indirect blocks
            uint *ind = (uint*)block(dip[i].addrs[NDIRECT]);
            for (int j = 0; j < num_inblocks; j++) {
                // printf("indirect block %d\n", ind[j] - first_dblock);
                if (barray[ind[j] - first_dblock] != 0) {
                    fprintf(stderr, "ERROR: indirect address used more than once.\n");
                    exit(1);
                }else{
                    barray[ind[j] - first_dblock] = 2;
                }
            }
        }
    }    
}

void test56() {
    for (int j = 0; j < sb->nblocks; j++) {
        char* bitblock = (char*)( img_ptr + BBLOCK((first_dblock + j),sb->ninodes) * BSIZE);
        // printf("bitblock %lx\n", BBLOCK((first_dblock + j),sb->ninodes));
        int bit_index = (first_dblock + j) % BPB;
        int byte_offset = bit_index / 8;
        int bit_offset = bit_index % 8;

        char b = bitblock[byte_offset];
        
        // check test 5
        if ((b & (1 << bit_offset)) != 0) {
            if (barray[j] == 0) {
                fprintf(stderr, "ERROR: bitmap marks block in use but it is not in use.\n");
                exit(1);                        
            }
        }    

        // check test 6
        if (barray[j] != 0) {
            if  ((b & (1 << bit_offset)) == 0){
                fprintf(stderr, "ERROR: address used by inode but marked free in bitmap.\n");
                exit(1);                        
            }
        }  
    }
}

void test910() {
    // get lists of inode referred by a directory
    // 1: reference by some inode 0: not reference by some inode
    char* inode_array = calloc((sb->ninodes) + 1,sizeof(char));    
    for(int i = 1; i < sb->ninodes + 1; i++){
        if(dip[i].type == T_DIR){
            // print_inode(dip[i]);
            int num_all_blocks = ROUNDUP(dip[i].size, BSIZE)/BSIZE;
            int num_dblocks = min(num_all_blocks, NDIRECT);
            int num_inblocks = num_all_blocks - NDIRECT;

            // direct blocks
            for (int j = 0; j < num_dblocks; j++) {
                struct xv6_dirent *entry = (struct xv6_dirent *)(img_ptr + dip[i].addrs[j] * BSIZE);
                for(int k = 0; k < BSIZE/(int)sizeof(struct xv6_dirent); k++){
                    inode_array[entry[k].inum] = 1;
                }
            }

            // indirect blocks
            uint *ind = (uint*)block(dip[i].addrs[NDIRECT]);
            for (int j = 0; j < num_inblocks; j++) {
                struct xv6_dirent *entry = (struct xv6_dirent *)(img_ptr + ind[j] * BSIZE);
                for(int k = 0; k < BSIZE/ (int)sizeof(struct xv6_dirent); k++){
                    inode_array[entry[k].inum] = 1;
                }
            }
        }
    }

    for(int i = 1; i < sb->ninodes + 1; i++){
    //    printf("%d %d\n",entry[k].name, entry[k].inum);
        if(dip[i].type != 0) {
            if(inode_array[i] == 0){
                fprintf(stderr,"ERROR: inode marked use but not found in a directory.\n");
                exit(1);
            }
        }

        if(inode_array[i] != 0){
            if(dip[i].type == 0){
                fprintf(stderr,"ERROR: inode referred to in directory but marked free.\n");
                exit(1);
            }
        }
    }

}

void test1112() {
    char* links_count = calloc((sb->ninodes) + 1,sizeof(char));    
    for(int i = 1; i < sb->ninodes + 1; i++){
        if(dip[i].type == T_DIR){
            // print_inode(dip[i]);
            int num_all_blocks = ROUNDUP(dip[i].size, BSIZE)/BSIZE;
            int num_dblocks = min(num_all_blocks, NDIRECT);
            int num_inblocks = num_all_blocks - NDIRECT;

            // direct blocks
            for (int j = 0; j < num_dblocks; j++) {
                struct xv6_dirent *entry = (struct xv6_dirent *)(img_ptr + dip[i].addrs[j] * BSIZE);
                for(int k = 0; k < BSIZE/(int)sizeof(struct xv6_dirent); k++){
                    if (strcmp(entry[k].name, ".") && strcmp(entry[k].name, ".."))
                        links_count[entry[k].inum]++;
                }
            }

            // indirect blocks
            uint *ind = (uint*)block(dip[i].addrs[NDIRECT]);
            for (int j = 0; j < num_inblocks; j++) {
                struct xv6_dirent *entry = (struct xv6_dirent *)(img_ptr + ind[j] * BSIZE);
                for(int k = 0; k < BSIZE/ (int)sizeof(struct xv6_dirent); k++){
                    if (strcmp(entry[k].name, ".") && strcmp(entry[k].name, ".."))
                        links_count[entry[k].inum]++;
                }
            }
        }
    }

    for(int i = 1; i < sb->ninodes + 1; i++){
        // test 11
        if (dip[i].type == T_FILE) {
            if (dip[i].nlink != links_count[i]) {
                fprintf(stderr, "ERROR: bad reference count for file.\n");
                exit(1);
            }
        // test 12
        } else if (dip[i].type == T_DIR) {
            if (links_count[i] > 1) {
                fprintf(stderr, "ERROR: directory appears more than once in file system.\n");
                exit(1);
            }
        }
   }

}

short extra1helper(ushort parent_inum, ushort child_inum){
    short found = 0;
    int num_all_blocks = ROUNDUP(dip[parent_inum].size, BSIZE)/BSIZE;
    int num_dblocks = min(num_all_blocks, NDIRECT);
    int num_inblocks = num_all_blocks - NDIRECT;
    int k;
    // direct blocks
    for (int j = 0; j < num_dblocks; j++) {
        struct xv6_dirent *entry = (struct xv6_dirent *)(img_ptr + dip[parent_inum].addrs[j] * BSIZE);
        k = 0;
        while(k < BSIZE/(int)sizeof(struct xv6_dirent) && found != 1){
            if (entry[k].inum == child_inum){
                found = 1;
            }
            k++;
        }
    }
    // indirect blocks
    uint *ind = (uint*)block(dip[parent_inum].addrs[NDIRECT]);
    for (int j = 0; j < num_inblocks; j++) {
        struct xv6_dirent *entry = (struct xv6_dirent *)(img_ptr + ind[j] * BSIZE);
        k = 0;
        while(k < BSIZE/ (int)sizeof(struct xv6_dirent) && found != 1){
            if (entry[k].inum == child_inum){
                found = 1;
            }
            k++;
        }
    }
    if(found != 1){
        fprintf(stderr, "ERROR: parent directory mismatch.\n");
        exit(1);
    }
    return 1;
}

void extra1(){
    short found;
    for(int i = 2; i < sb->ninodes + 1; i++){
        if(dip[i].type == T_DIR){
            int k;
            int num_all_blocks = ROUNDUP(dip[i].size, BSIZE)/BSIZE;
            int num_dblocks = min(num_all_blocks, NDIRECT);
            int num_inblocks = num_all_blocks - NDIRECT;
            // direct blocks
            for (int j = 0; j < num_dblocks; j++) {
                struct xv6_dirent *entry = (struct xv6_dirent *)(img_ptr + dip[i].addrs[j] * BSIZE);
                k = 0;
                while(k < BSIZE/(int)sizeof(struct xv6_dirent) && found != 1){
                    if (!strcmp(entry[k].name, "..")){
                        found = extra1helper(entry[k].inum, k);
                    }
                    k++;
                }
            }
            // indirect blocks
            uint *ind = (uint*)block(dip[i].addrs[NDIRECT]);
            for (int j = 0; j < num_inblocks; j++) {
                struct xv6_dirent *entry = (struct xv6_dirent *)(img_ptr + ind[j] * BSIZE);
                k = 0;
                while(k < BSIZE/(int)sizeof(struct xv6_dirent) && found != 1){
                    if (!strcmp(entry[k].name, "..")){
                        found = extra1helper(entry[k].inum, k);
                    }
                    k++;
                }
            }
        }
    }   
}