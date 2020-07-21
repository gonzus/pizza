#ifndef THRPOOL_H_
#define THRPOOL_H_

/*
 * ThrPool -- a simple thread pool.
 * Keeps a queue of pending calls to functions (with a single arg).
 */

// Possible errors for a ThrPool.
typedef enum {
    THRPOOL_STATUS_OK,
    THRPOOL_STATUS_INVALID,
    THRPOOL_STATUS_LOCK_UNLOCK,
    THRPOOL_STATUS_QUEUE_FULL,
    THRPOOL_STATUS_SHUTTING_DOWN,
    THRPOOL_STATUS_CANNOT_JOIN,
    THRPOOL_STATUS_CANNOT_STOP,
} ThrPool_error;

// Type of functions that can be added to the pool.
typedef void (*ThrPoolFn)(void*);

// Opaque type of a ThrPool.
typedef struct ThrPool ThrPool;

// Create a thread pool.
ThrPool* thrpool_create(int thread_count, int queue_size);

// Stop and destroy a thread pool.
int thrpool_destroy(ThrPool* pool, int immediate);

// Add a new task in the queue of a thread pool.
int thrpool_add(ThrPool* pool, ThrPoolFn function, void* argument);

#endif
