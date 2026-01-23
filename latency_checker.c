#define _GNU_SOURCE
#include <stdio.h>
#include <stdint.h>
#include <x86intrin.h>
#include <stdlib.h>
#include <pthread.h>
#include <sched.h>
#include <sys/mman.h>
#include <string.h>

#define RAM_CHECK_ARRAY_SIZE 4 *1024 * 1024 
#define L1_CHECK_ARRAY_SIZE 16 * 1024
#define L2_CHECK_ARRAY_SIZE 256 * 1024
#define L3_CHECK_ARRAY_SIZE 2 * 1024 * 1024



#define NUMBER_OF_CORES 16
#define ITERATIONS_NUMBER 100000000


static inline uint64_t get_current_tsc(){
    return __rdtsc();
}

void pin_to_core(int core_number){
    cpu_set_t set;
    CPU_ZERO(&set);
    CPU_SET(core_number, &set);

    pthread_setaffinity_np(pthread_self(), sizeof(set), &set); 
}



void shuffle_indexes(uint32_t* arr,size_t array_size){
    for(size_t i = 0; i < array_size;i++){
        arr[i] = i;
    }
    for(size_t i = 1; i < array_size;i++){
        size_t temp_val = arr[i];
        long rand_index = rand() % (i);
        arr[i] = arr[rand_index];
        arr[rand_index] = temp_val;
    }
}




double get_latency(uint32_t* arr,size_t size){

    shuffle_indexes(arr,size);

    uint64_t start,end;

    int array_index = 0;
    start = get_current_tsc();
    for(int i = 0; i < ITERATIONS_NUMBER;i++){
        array_index = arr[array_index];
    }
    end = get_current_tsc();
    return (double)(end - start) / ITERATIONS_NUMBER;
}






int main(int argc,char* argv[]){




    pin_to_core(rand() % NUMBER_OF_CORES-1); //cpu affinity

    struct sched_param p = { .sched_priority = 99 };
    sched_setscheduler(0, SCHED_FIFO, &p);
    //prevents from context switching in the middle of the calculation




    printf("calculating...\n");
    uint32_t* l1_array = malloc(sizeof(uint32_t) * L1_CHECK_ARRAY_SIZE);
    double l1_latency = get_latency(l1_array,L1_CHECK_ARRAY_SIZE);



    uint32_t* l2_array = malloc(sizeof(uint32_t) * L2_CHECK_ARRAY_SIZE);
    double l2_latency = get_latency(l2_array,L2_CHECK_ARRAY_SIZE);


    uint32_t* l3_array = malloc(sizeof(uint32_t) * L3_CHECK_ARRAY_SIZE);
    double l3_latency = get_latency(l3_array,L3_CHECK_ARRAY_SIZE);


    uint32_t* ram_array = malloc(sizeof(uint32_t) * RAM_CHECK_ARRAY_SIZE);
    double ram_latency = get_latency(ram_array,RAM_CHECK_ARRAY_SIZE);

    printf("l1 latency (in cycles) : %f\n"
        "l2 latency (in cycles) : %f\n"
        "l3 latency (in cycles) : %f\n"
        "ram latency (in cycles) : %f\n",l1_latency,l2_latency,l3_latency,ram_latency);


    free(l1_array);
    free(l2_array);
    free(l3_array);
    free(ram_array);
}





