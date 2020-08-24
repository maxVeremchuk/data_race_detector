
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "CLocksetAlgorithm.hpp"

int data1;
int data2;

pthread_mutex_t A;
pthread_mutex_t B;

using namespace ImprovedLockset;
CLocksetAlgorithm lockset{10};

void* thread_routine_1(void* arg)
{
    int tmp;

    lockset.lockset_thread_start();

    lockset.lockset_mutex_lock(&A);
    printf("thread 1 : lock (A)\n");

    lockset.lockset_obj_write(&data1);

    lockset.lockset_mutex_unlock(&A);
    printf("thread 1 : unlock (A)\n");

    lockset.lockset_thread_end();

    return 0;
}

void* thread_routine_2(void* arg)
{
    int tmp;

    lockset.lockset_thread_start();

    lockset.lockset_mutex_lock(&B);
    printf("thread 2 : lock (B)\n");

    lockset.lockset_obj_read(&data1);
    lockset.lockset_obj_write(&data1);

    lockset.lockset_mutex_unlock(&B);
    printf("thread 2 : unlock (B)\n");

    lockset.lockset_thread_end();

    return 0;
}

int main()
{
    clock_t t;
    t = clock();

    pthread_t t1, t2;

    lockset.lockset_main_start();

    lockset.lockset_obj_reg(&data1);
    lockset.lockset_obj_reg(&data2);

    lockset.lockset_mutex_init(&A, NULL);
    lockset.lockset_mutex_init(&B, NULL);


    lockset.lockset_thread_create(&t1, 0, thread_routine_1, 0);
    lockset.lockset_thread_create(&t2, 0, thread_routine_2, 0);

    lockset.lockset_thread_join(t1, 0);
    lockset.lockset_thread_join(t2, 0);

    lockset.lockset_mutex_destroy(&A);
    lockset.lockset_mutex_destroy(&B);

    lockset.lockset_main_end();

    t = clock() - t;
    double time_taken = ((double)t) / CLOCKS_PER_SEC; // in seconds s
    printf("fun() took %f seconds to execute \n", time_taken);

    return 0;
}
