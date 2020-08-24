/////////////////////////////////////////////////////////
//

#pragma once

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

#include "LocksetDataUtils.hpp"

namespace ImprovedLockset {

namespace {

const int max_size = 100;
}

class CLocksetAlgorithm
{
public: // methods
    CLocksetAlgorithm()
        : mutex_array(max_size)
        , shared_var_array(max_size)
        , thread_array(max_size)
        , barrier_threads_numbers(max_size)
    {
    }

    CLocksetAlgorithm(int max_vector_size)
        : mutex_array(max_vector_size)
        , shared_var_array(max_vector_size)
        , thread_array(max_vector_size)
        , barrier_threads_numbers(max_vector_size)
    {
    }

    ///
    /// @brief initialize the thread
    ///
    void lockset_thread_start();

    ///
    /// @brief termination of thread
    ///
    void lockset_thread_end();

    ///
    /// @brief init in main
    ///
    void lockset_main_start();

    ///
    /// @brief termination in main
    ///
    void lockset_main_end();

    ///
    /// @brief registrate var
    ///
    void lockset_obj_reg(int *data);

    ///
    /// @brief call when read happens
    ///
    void lockset_obj_read(int *data);

    ///
    /// @brief call when write happens
    ///
    void lockset_obj_write(int *data);

    ///
    /// @brief init of mutex
    ///
    int lockset_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr);

    ///
    /// @brief destruction of mutex
    ///
    int lockset_mutex_destroy(pthread_mutex_t *mutex);

    ///
    /// @brief locks mutex
    ///
    int lockset_mutex_lock(pthread_mutex_t *mutex);

    ///
    /// @brief unlocks mutex
    ///
    int lockset_mutex_unlock(pthread_mutex_t *mutex);

    ///
    /// @brief init of mutex
    ///
    int lockset_thread_create(
        pthread_t *thread,
        const pthread_attr_t *attr,
        void *(*start_routine)(void *),
        void *arg);

    ///
    /// @brief thread join
    ///
    int lockset_thread_join(pthread_t thread, void **value_ptr);

    ///
    /// @brief find mutex in mutex_array
    ///
    int lockset_find_mutex(pthread_mutex_t *mutex);

    ///
    /// @brief find thread in thread_array
    ///
    int lockset_find_thread(pthread_t thread);

    ///
    /// @brief find var in shared_var_array
    ///
    int lockset_find_var(int *data);

    ///
    /// @brief call barrier
    ///
    void lockset_barrier();

    ///
    /// @brief init barrier
    ///
    int lockset_barrier_init(
        pthread_barrier_t *barrier,
        const pthread_barrierattr_t *attr,
        int count);

private: // fields
    std::vector<Data> shared_var_array;
    int shared_var_index;

    std::vector<pthread_mutex_t *> mutex_array;
    int mutex_index;

    std::vector<Thread> thread_array;
    int thread_index;
    pthread_mutex_t this_thread;

    pthread_barrier_t lockset_barrier_t;
    std::vector<pthread_t> barrier_threads_numbers;
    int barrier_index;
    int barrier_count;

    bool data_race_detected;
};

} // namespace ImprovedLockset
