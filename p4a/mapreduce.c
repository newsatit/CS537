#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdio.h>
#include "mapreduce.h"
#include <stdlib.h>
#include <string.h>
typedef struct __pair_t{
    char *key;
    char *value;
} pair_t;

typedef struct __key_pos_t{
    char *key; 
    int cur;
} key_pos_t;

typedef struct __p_lock_t{
    sem_t mutex;
    pair_t *pairs;
    int max_size;
    int cur_size;
    key_pos_t *key_pos;
    int key_count;
} p_lock_t;

typedef struct __lock_t{
    sem_t mutex;
    int f;
} lock_t;

typedef struct __maparg_t{
    int num_files;
    Mapper map;
    char **files;
} maparg_t;

typedef struct __redarg_t{
    int partition_number;
} redarg_t;

Partitioner part_func;
Reducer reduce_func;
Mapper map_func;
int num_partitions;
p_lock_t **partitions;

lock_t *lock;

unsigned long MR_DefaultHashPartition(char *key, int num_partitions) {
    unsigned long hash = 5381;
    int c;
    while ((c = *key++) != '\0')
        hash = hash * 33 + c;
    return hash % num_partitions;
}


void MR_Emit(char *key, char *value) {
    // Implement MR_Emit here
    // you need to have some data structure that is going to store intermediate values
    // You can assume that intermediate data fits into memory
    // -- how multiple map threads are going to insert data into this data structure
    // How do we get all the values for a particular key to the same reducer
    //
    // num_partitions == num_reducers
    // -- you want to have a data structure per-partition and put in values correspondingly

    // Do the reducers need to wait for mappers
    // -- Getter function needs to return all values for a key in the reducer
    // -- When you start a reducer it needs to see all keys in partition in sorted order
    // -- For a particular key the values are also in sorted order

    int part_num = part_func(key, num_partitions);
    sem_wait(&partitions[part_num]->mutex);
    if(partitions[part_num]->cur_size >= partitions[part_num]->max_size){
        //expand array
        partitions[part_num]->max_size*= 2;
        partitions[part_num]->pairs = (pair_t*)realloc(partitions[part_num]->pairs, sizeof(pair_t) * partitions[part_num]->max_size);
    }
    partitions[part_num]->pairs[partitions[part_num]->cur_size].key = (char *)malloc(sizeof(char) * (strlen(key) + 1));
    strcpy(partitions[part_num]->pairs[partitions[part_num]->cur_size].key,key);
    partitions[part_num]->pairs[partitions[part_num]->cur_size].value = (char *)malloc(sizeof(char) * (strlen(value)+1));
    strcpy(partitions[part_num]->pairs[partitions[part_num]->cur_size].value, value);
    partitions[part_num]->cur_size++;

    sem_post(&partitions[part_num]->mutex);
}
/*
    char *files[];
    int num_files;
    Mapper map;
*/
void map_thread(void *arg) {
    int fi;
    // get available file
    while(1) {
        maparg_t *m = (maparg_t *) arg;
        
        sem_wait(&lock->mutex);
        fi = lock->f++;
        sem_post(&lock->mutex);
        
        if (fi > m->num_files) break; 
        
        // Map file
        map_func(m->files[fi]);
    }
}

int comparator(const void *pv1, const void *pv2) {
    pair_t p1 = *(pair_t *)(pv1);
    pair_t p2 = *(pair_t *)(pv2);
    if (strcmp(p1.key, p2.key) == 0) {
        
        return strcmp(p1.value, p2.value);
    } else {
        return strcmp(p1.key, p2.key);
    }
}

 char* get_next(char *key, int partition_number) {
     p_lock_t* part = partitions[partition_number];

    int l = 0;
    int r = part->key_count - 1;

    while (l <= r) {
        int i = (l + r)/2;
        
        int cmp_result = strcmp(key, part->key_pos[i].key);
        if (cmp_result == 0) {
            int index = part->key_pos[i].cur;  
            //if (part->pairs[index] == NULL) {
            //    return NULL;
            //}
            if (index >= part->cur_size)
                return NULL;
            if (!strcmp(part->pairs[index].key, key)) {
                char *ret_val = part->pairs[index].value;
                part->key_pos[i].cur++;
                return ret_val;;
            } else {
                return NULL;
            }
        } else if (cmp_result < 0) {
            r = i - 1;
        } else {
            l = i + 1;
        }

    }
    return NULL;


    //  for (i = 0; i < part->key_count; i++) {
    //      if(!strcmp(key, part->key_pos[i].key)) {
    //         int index = part->key_pos[i].cur;  
    //         if (part->pairs[index] == NULL) {
    //             return NULL;
    //         }
    //         if (!strcmp(part->pairs[index]->key, key)) {
    //             char *ret_val = part->pairs[index]->value;
    //             part->key_pos[i].cur++;
    //             return ret_val;;
    //         } else {
    //             return NULL;
    //         }
    //      }
    //  }
    // return NULL;
     
 }



void reduce_thread(void *arg) {
    redarg_t *r = (redarg_t*) arg;
    int part_num = r->partition_number;
    // printf("partition number : %d\n", part_num);
    p_lock_t* part = partitions[part_num];  
    // for (int i = 0; i < part->cur_size; i++) {
    //     printf("(%d)reducing key: %s, value: %s\n", part_num, part->pairs[i]->key, part->pairs[i]->value);
    // }
    // printf("\n\n");
    qsort((void*)part->pairs, part->cur_size, sizeof(pair_t), comparator);
    // for (int i = 0; i < part->cur_size; i++) {
    //     printf("(%d)reducing key: %s, value: %s\n", part_num, part->pairs[i]->key, part->pairs[i]->value);
    // }
    part->key_count = 0;
    if (part->cur_size > 0) {
        int num_keys = 1;
        for (int i = 0; i < part->cur_size - 1; i++) {
            if (strcmp(part->pairs[i].key, part->pairs[i + 1].key) != 0) {
                num_keys++;
            }
        }

        part->key_pos = (key_pos_t*)malloc(sizeof(key_pos_t) * num_keys);
        part->key_pos[0].cur = 0;
        part->key_pos[0].key = part->pairs[0].key;

        part->key_count = 1;

        for (int i = 1; i < part->cur_size ; i++) {
            if (strcmp(part->pairs[i - 1].key, part->pairs[i].key) != 0) {
                part->key_pos[part->key_count].cur = i;
                part->key_pos[part->key_count].key = part->pairs[i].key;
                part->key_count++;
            }
        }
        for (int i = 0; i < part->key_count; i++) {
            reduce_func(part->key_pos[i].key, get_next, part_num);
        }

        if (part->key_pos != NULL) {
            free(part->key_pos);
        }
    }
}



// arvgv here
void MR_Run(int argc, char *argv[], Mapper map, int num_mappers, Reducer reduce, int num_reducers, Partitioner partition) {
    // Use multiple threads
    //
    // You want to create many threads as num_mappers and lanch a Map function inside each of them
    // int pthread_
    // start_routine -- need to run Mapper within start_routine
    // pass in arguments using to it using the arg pointer    
    // You can have more input keys than than there are mappers
    // 100 files that you are mapping over but you only have 10 mappers       
    int num_files = argc - 1;
    lock = (lock_t*)malloc(sizeof(lock_t));
    sem_init(&lock->mutex, 0, 1);
    lock->f = 1;
    part_func = partition;
    reduce_func = reduce;
    map_func = map;
    num_partitions = num_reducers;
    partitions = (p_lock_t**)malloc(sizeof(p_lock_t*) * num_reducers);
    for(int i = 0; i < num_partitions; i++){
        partitions[i] = (p_lock_t*)malloc(sizeof(p_lock_t));
        partitions[i]->max_size = 1;
        partitions[i]->cur_size = 0;
        partitions[i]->pairs = (pair_t*)malloc(sizeof(pair_t)*partitions[i]->max_size);
        sem_init(&partitions[i]->mutex, 0, 1);
    }

    //Map
    pthread_t* mappers; // Allocate this to make sure there are enough threads!
    mappers = (pthread_t*)malloc(sizeof(pthread_t) * num_mappers);
    maparg_t* argsm = (maparg_t*)malloc(sizeof(maparg_t) * num_mappers);
    for (int i = 0; i < num_mappers; i++) {
        argsm[i].files = argv;
        argsm[i].num_files = num_files;
        pthread_create(&mappers[i], NULL, (void*)&map_thread, &argsm[i]);
    }

    for (int i = 0; i < num_mappers; i++) {
        pthread_join(mappers[i], NULL);
    }    
    if (argsm != NULL) 
        free(argsm);
    if (mappers != NULL) 
        free(mappers);

    //reduce
    pthread_t* reducers;
    // TODO: change args to array of argument (malloc)
    reducers = (pthread_t*)malloc(sizeof(pthread_t) * num_reducers);
    redarg_t* argsr = (redarg_t*)malloc(sizeof(redarg_t)* num_reducers);
    for (int i = 0; i < num_reducers; i++) {
        argsr[i].partition_number = i;
        // printf("ppp %d\n", argsr[i].partition_number);
        pthread_create(&reducers[i], NULL, (void*)&reduce_thread, &argsr[i]);
    }
    for (int i = 0; i < num_reducers; i++) {
        pthread_join(reducers[i], NULL);
    }    
    if (argsr != NULL)
        free(argsr);
    if (reducers != NULL)
        free(reducers);


    for(int i = 0; i < num_partitions; i++){
        for (int j = 0; j < partitions[i]->cur_size; j++) {
            free(partitions[i]->pairs[j].key);
            free(partitions[i]->pairs[j].value);   
        }
        free(partitions[i]->pairs);
        free(partitions[i]);
    }    
    if (partitions != NULL)
        free(partitions);
    sem_destroy(&lock->mutex);
    free(lock);
}


// gcc -o wordcount2.c wordcount.c mapreduce.c -Wall -Werror -lpthread
// valgrind --leak-check=full ./wordcount2
// valgrind --tool=helgrind ./wordcount2

