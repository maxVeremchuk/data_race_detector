/////////////////////////////////////////////////////////
//

#include "CLocksetAlgorithm.hpp"

namespace ImprovedLockset {

void CLocksetAlgorithm::lockset_thread_start()
{
    pthread_mutex_lock(&this_thread);

    thread_array[thread_index].thread = pthread_self();
    thread_array[thread_index].locks_held = 0;
    thread_array[thread_index].write_locks_held = 0;
    ++thread_index;

    pthread_mutex_unlock(&this_thread);
}

void CLocksetAlgorithm::lockset_thread_end()
{
    pthread_mutex_lock(&this_thread);

    std::cout << pthread_self() << " thread stoped execution." << std::endl;

    pthread_mutex_unlock(&this_thread);
}

void CLocksetAlgorithm::lockset_main_start()
{
    data_race_detected = false;
    shared_var_index = 0;
    mutex_index = 0;
    thread_index = 0;

    pthread_mutex_init(&this_thread, NULL);
}

void CLocksetAlgorithm::lockset_main_end()
{
    if (data_race_detected)
    {
        std::cout << "Warning: data race detected!" << std::endl;
    }
    else
    {
        std::cout << "Data race not detected!" << std::endl;
    }
}

void CLocksetAlgorithm::lockset_obj_reg(int *data)
{
    shared_var_array[shared_var_index].data = data;
    shared_var_array[shared_var_index].locks_candidate = 0;
    shared_var_array[shared_var_index].thread_id = 0;
    shared_var_array[shared_var_index].state = State::Virgin;

    ++shared_var_index;
}

void CLocksetAlgorithm::lockset_obj_read(int *data)
{
    pthread_mutex_lock(&this_thread);

    int tmp_var_index, tmp_thread_index;
    tmp_var_index = lockset_find_var(data); // find specified shared variable and
    tmp_thread_index =
        lockset_find_thread(pthread_self()); // find specified thread and return

    if (shared_var_array[tmp_var_index].thread_id == pthread_self())
    {
        shared_var_array[tmp_var_index].locks_candidate =
            thread_array[tmp_thread_index].locks_held; // C(X)
        pthread_mutex_unlock(&this_thread);
        return;
    }

    if (tmp_var_index != -1 && tmp_thread_index != -1)
    {
        shared_var_array[tmp_var_index].locks_candidate &=
            thread_array[tmp_thread_index].locks_held;
        switch (shared_var_array[tmp_var_index].state)
        {
        case State::Virgin:
            std::cout << "Virgin State. var: " << tmp_var_index << std::endl;
            shared_var_array[tmp_var_index].thread_id = pthread_self();
            break;

        case State::Initializing:
            std::cout << "Initializing State. var: " << tmp_var_index << std::endl;
            if (shared_var_array[tmp_var_index].thread_id != pthread_self())
            {
                shared_var_array[tmp_var_index].state = State::Shared;
            }
            break;

        case State::Exclusive:
            std::cout << "Exclusive State. var: " << tmp_var_index << std::endl;
            if (shared_var_array[tmp_var_index].thread_id != pthread_self())
            {
                if ((shared_var_array[tmp_var_index].locks_candidate &
                     thread_array[tmp_thread_index].locks_held) == 0)
                {
                    data_race_detected = true;
                    shared_var_array[tmp_var_index].state = State::Empty;
                }
                else
                {
                    shared_var_array[tmp_var_index].state = State::Shared;
                }
            }
            shared_var_array[tmp_var_index].locks_candidate =
                thread_array[tmp_thread_index].locks_held; // C(X)
            break;

        case State::Shared:
            std::cout << "Shared State. var: " << tmp_var_index << std::endl;
            if ((shared_var_array[tmp_var_index].locks_candidate &
                 thread_array[tmp_thread_index].locks_held) == 0)
            {
                data_race_detected = true;
                shared_var_array[tmp_var_index].state = State::Empty;
            }

            shared_var_array[tmp_var_index].locks_candidate =
                thread_array[tmp_thread_index].locks_held; // C(X)
            break;

        case State::Empty:
            std::cout << "Empty State. Data race warning" << std::endl;
            break;

        case State::Clean:
            std::cout << "Clean State. var: " << tmp_var_index << std::endl;
            bool is_thread_in_barrier = false;
            for (int t = 0; t < barrier_count; t++)
            {
                if (barrier_threads_numbers[t] ==
                    shared_var_array[tmp_var_index].thread_id)
                {
                    is_thread_in_barrier = true;
                    break;
                }
            }
            if (!is_thread_in_barrier &&
                (shared_var_array[tmp_var_index].locks_candidate &
                 thread_array[tmp_thread_index].locks_held) == 0)

            {
                data_race_detected = true;
            }
            shared_var_array[tmp_var_index].state = State::Exclusive;

            break;
        }
    }
    pthread_mutex_unlock(&this_thread);
}

void CLocksetAlgorithm::lockset_obj_write(int *data)
{
    pthread_mutex_lock(&this_thread);

    int tmp_var_index, tmp_thread_index;
    tmp_thread_index =
        lockset_find_thread(pthread_self()); // find specified thread and return

    if (tmp_thread_index != -1)
        thread_array[tmp_thread_index].write_locks_held =
            thread_array[tmp_thread_index].locks_held;

    tmp_var_index = lockset_find_var(data); // find specified shared variable and

    if (shared_var_array[tmp_var_index].thread_id == pthread_self() &&
        shared_var_array[tmp_var_index].state != State::Virgin)
    {
        shared_var_array[tmp_var_index].locks_candidate =
            thread_array[tmp_thread_index].locks_held; // C(X)
        return;
    }
    if (tmp_var_index != -1 && tmp_thread_index != -1)
    {
        switch (shared_var_array[tmp_var_index].state)
        {
        case State::Virgin:
            std::cout << "Virgin write State. var: " << tmp_var_index << std::endl;
            shared_var_array[tmp_var_index].state = State::Initializing;

            shared_var_array[tmp_var_index].thread_id = pthread_self();

            shared_var_array[tmp_var_index].locks_candidate =
                thread_array[tmp_thread_index].locks_held; // C(X)
            break;

        case State::Initializing:
            std::cout << "Initializing write State. var: " << tmp_var_index
                      << std::endl;
            if (shared_var_array[tmp_var_index].thread_id != pthread_self())
            {
                shared_var_array[tmp_var_index].state = State::Shared;
            }
            break;

        case State::Exclusive:
            std::cout << "Exclusive write State. var: " << tmp_var_index
                      << std::endl;
            if (shared_var_array[tmp_var_index].thread_id != pthread_self())
            {
                if ((shared_var_array[tmp_var_index].locks_candidate &
                     thread_array[tmp_thread_index].locks_held) == 0)
                {
                    data_race_detected = true;
                    shared_var_array[tmp_var_index].state = State::Empty;
                }
                else
                {
                    shared_var_array[tmp_var_index].state = State::Shared;
                }
            }
            shared_var_array[tmp_var_index].locks_candidate =
                thread_array[tmp_thread_index].locks_held; // C(X)
            break;

        case State::Shared:
            std::cout << "Shared write State. var: " << tmp_var_index << std::endl;

            if ((shared_var_array[tmp_var_index].locks_candidate &
                 thread_array[tmp_thread_index].locks_held) == 0)
            {
                data_race_detected = true;
                shared_var_array[tmp_var_index].state = State::Empty;
            }
            shared_var_array[tmp_var_index].locks_candidate =
                thread_array[tmp_thread_index].locks_held; // C(X)
            break;

        case State::Empty:
            std::cout << "Empty write State. Data race warning" << std::endl;
            break;

        case State::Clean:
            std::cout << "Clean write State. var: " << tmp_var_index << std::endl;
            bool is_thread_in_barrier = false;
            for (int t = 0; t < barrier_count; t++)
            {
                std::cout << barrier_threads_numbers[t] << " "
                          << shared_var_array[tmp_var_index].thread_id
                          << std::endl;
                if (barrier_threads_numbers[t] ==
                    shared_var_array[tmp_var_index].thread_id)
                {
                    is_thread_in_barrier = true;
                    break;
                }
            }

            if (!is_thread_in_barrier &&
                shared_var_array[tmp_var_index].locks_candidate == 0)
            {
                data_race_detected = true;
            }
            shared_var_array[tmp_var_index].state = State::Exclusive;

            break;
        }
    }
    pthread_mutex_unlock(&this_thread);
}

void CLocksetAlgorithm::lockset_barrier()
{
    int tmp_var_index, tmp_thread_index;
    tmp_thread_index =
        lockset_find_thread(pthread_self()); // find specified thread and return

    barrier_threads_numbers[barrier_index++] =
        thread_array[tmp_thread_index].thread;

    pthread_barrier_wait(&lockset_barrier_t);

    for (int i = 0; i < shared_var_index; ++i)
    {
        shared_var_array[i].state = State::Clean;
    }
}

int CLocksetAlgorithm::lockset_barrier_init(
    pthread_barrier_t *barrier,
    const pthread_barrierattr_t *attr,
    int count)
{
    pthread_barrier_init(barrier, attr, count);
    lockset_barrier_t = *barrier;
    barrier_count = count;
}

int CLocksetAlgorithm::lockset_mutex_init(
    pthread_mutex_t *mutex,
    const pthread_mutexattr_t *attr)
{
    mutex_array[mutex_index++] = mutex;
    pthread_mutex_init(mutex, attr);
}

int CLocksetAlgorithm::lockset_mutex_destroy(pthread_mutex_t *mutex)
{
    pthread_mutex_destroy(mutex); // destroy used lock (pthread_mutex_t object)
}

int CLocksetAlgorithm::lockset_mutex_lock(pthread_mutex_t *mutex)
{
    pthread_mutex_lock(mutex); // call pthread_mutex_lock

    pthread_mutex_lock(&this_thread);

    int tmp_mutex_index, tmp_thread_index;
    tmp_mutex_index = lockset_find_mutex(mutex);
    tmp_thread_index = lockset_find_thread(pthread_self());

    if (tmp_mutex_index != -1 && tmp_thread_index != -1)
    {
        thread_array[tmp_thread_index].locks_held |= 1 << tmp_mutex_index;
    }

    pthread_mutex_unlock(&this_thread);
}

int CLocksetAlgorithm::lockset_mutex_unlock(pthread_mutex_t *mutex)
{
    pthread_mutex_lock(&this_thread);

    int tmp_mutex_index, tmp_thread_index;
    tmp_mutex_index = lockset_find_mutex(mutex);

    tmp_thread_index = lockset_find_thread(pthread_self());

    if (tmp_mutex_index != -1 && tmp_thread_index != -1)
    {
        thread_array[tmp_thread_index].locks_held &=
            ~(1 << tmp_mutex_index); // update locks held by current thread
        thread_array[tmp_thread_index].write_locks_held &=
            ~(1 << tmp_mutex_index); // update write locks held by current thread
    }

    pthread_mutex_unlock(&this_thread);

    pthread_mutex_unlock(mutex);
}

int CLocksetAlgorithm::lockset_thread_create(
    pthread_t *thread,
    const pthread_attr_t *attr,
    void *(*start_routine)(void *),
    void *arg)
{
    pthread_create(thread, attr, start_routine, arg); // create thread
}

int CLocksetAlgorithm::lockset_thread_join(pthread_t thread, void **value_ptr)
{
    pthread_join(thread, value_ptr); // join thread
}

int CLocksetAlgorithm::lockset_find_mutex(pthread_mutex_t *mutex)
{
    for (int i = 0; i < mutex_index; ++i)
    {
        if (mutex_array[i] == mutex)
        {
            return i;
        }
    }

    return -1;
}

int CLocksetAlgorithm::lockset_find_thread(pthread_t thread)
{
    for (int i = 0; i < thread_index; ++i)
    {
        if (thread_array[i].thread == thread)
        {
            return i;
        }
    }
    return -1;
}

int CLocksetAlgorithm::lockset_find_var(int *data)
{
    for (int i = 0; i < shared_var_index; ++i)
    {
        if (shared_var_array[i].data == data)
        {
            return i;
        }
    }
    return -1;
}

} // namespace ImprovedLockset
