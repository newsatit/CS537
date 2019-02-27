#include "types.h"
#include "stat.h"
#include "user.h"
#include "pstat.h"

int
main(int argc, char *argv[])
{
    struct pstat st;

    if(argc != 2){
        printf(1, "usage: mytest counter");
        exit();
    }

    int i, x, l, j;

    for (i = 0; i < 2; i++) {
        fork();
    }

    printf(1, "tick-tests\n");

    int mypid = getpid();

    for(i = 1; i < atoi(argv[1]); i++){
         x = x + i;
//        x = x + 1;
    }

    getpinfo(&st);
    for (j = 0; j < NPROC; j++) {
        if (st.inuse[j] && st.pid[j] >= 3 && st.pid[j] == mypid) {
        // if (st.pid[j] >= 1) {
            //printf(1, "my pid : %d\n", mypid);
            for (l = 3; l >= 0; l--) {
                printf(1, "level:%d \t ticks-used:%d\n", l, st.ticks[j][l]);
            }
        }
    }
    
    exit();
    return 0;
}