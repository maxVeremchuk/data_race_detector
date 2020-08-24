/////////////////////////////////////////////////////////
//

#pragma once

#include <cstdint>
#include <pthread.h>

namespace ImprovedLockset {

using LOCKS_CANDIDATE = uint64_t;
using LOCKS_HELD = uint64_t;
using WRITE_LOCKS_HELD = uint64_t;

enum class State
{
    Virgin,
    Initializing,
    Exclusive,
    Shared,
    Clean,
    Empty,
};

struct Data
{
    int *data; // memory location of each shared variable
    LOCKS_CANDIDATE locks_candidate; // of each shared variable
    pthread_t thread_id; // last thread that access each shared variable
    State state; // state of each shared variable
};

struct Thread
{
    pthread_t thread; // pthread_t object
    LOCKS_HELD locks_held; // locks held by each thread
    WRITE_LOCKS_HELD write_locks_held; // write locks held by each thread
};

} // namespace ImprovedLockset