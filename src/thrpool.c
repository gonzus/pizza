#include <pthread.h>
#include <stdlib.h>
#include "pizza/memory.h"
#include "pizza/thrpool.h"

// Keep in mind that each pthread needs stack space
#define MAX_THREAD_COUNT     64

// TODO: make queue grow dynamically
#define MAX_QUEUE_SIZE    65536

// Flags for the thrpool
#define THRPOOL_FLAG_SHUTTING_DOWN      (1 <<  0)
#define THRPOOL_FLAG_SHUTDOWN_IMMEDIATE (1 <<  1)

// A task enqueued in the thrpool
typedef struct ThrPoolTask {
    ThrPoolFn function;        // Function to be called
    void* argument;            // Argument to pass to function
} ThrPoolTask;

// The pool data
struct PoolData {
    pthread_t* threads;        // Array containing worker thread ids
    int size;                  // Number of threads
    int started;               // Number of started threads
};

// The queue data
struct QueueData {
    ThrPoolTask* tasks;        // Array containing tasks in the queue
    int size;                  // Size of the tasks array
    int head;                  // Index of the first element
    int tail;                  // Index of the next element
    int pending;               // Number of pending tasks
};

// This is the ThrPool itself
struct ThrPool {
    pthread_mutex_t lock;      // Internal pool lock
    pthread_cond_t notify;     // Condition variable to notify worker threads
    struct PoolData pool;      // Pool data
    struct QueueData queue;    // Queue data
    int flags;                 // Flags for thread pool
};

static int thrpool_stop(ThrPool* tp, int immediate);
static int thrpool_free(ThrPool* tp);
static void* thrpool_worker(void* thrpool);

ThrPool* thrpool_create(int thread_count, int queue_size) {
    ThrPool* tp = 0;
    int mutex_inited = 0;
    int cond_inited = 0;
    int pool_created = 0;

    do {
        if (thread_count <= 0 || thread_count > MAX_THREAD_COUNT) {
            break;
        }

        if (queue_size <= 0 || queue_size > MAX_QUEUE_SIZE) {
            break;
        }

        size_t pool_bytes = sizeof(ThrPool);
        tp = (ThrPool*) memory_realloc(0, pool_bytes);
        if (!tp) {
            break;
        }

        // Initialize
        memset(tp, 0, pool_bytes);
        tp->queue.size = queue_size;

        // Allocate array for threads
        size_t thread_bytes = thread_count * sizeof(pthread_t);
        tp->pool.threads = (pthread_t*) memory_realloc(0, thread_bytes);
        if (!tp->pool.threads) {
            break;
        }
        memset(tp->pool.threads, 0, thread_bytes);

        // Allocate array for queue
        size_t queue_bytes = queue_size * sizeof(ThrPoolTask);
        tp->queue.tasks = (ThrPoolTask*) memory_realloc(0, queue_bytes);
        if (!tp->queue.tasks) {
            break;
        }
        memset(tp->queue.tasks, 0, queue_bytes);

        // Initialize pool mutex
        if (pthread_mutex_init(&tp->lock, 0) != 0) {
            break;
        }
        mutex_inited = 1;

        // Initialize pool conditional variable
        if (pthread_cond_init(&tp->notify, 0) != 0) {
            break;
        }
        cond_inited = 1;

        // Start worker threads
        for (int j = 0; j < thread_count; j++) {
            if (pthread_create(&tp->pool.threads[j], 0, thrpool_worker, tp) != 0) {
                break;
            }
            tp->pool.size++;
            tp->pool.started++;
        }
        pool_created = 1;

        if (tp->pool.size != thread_count) {
            break;
        }

        // SUCCESS!
        return tp;
    } while (0);

    // ERROR CASES

    if (pool_created) {
        thrpool_stop(tp, 1);
        pool_created = 0;
    }

    if (cond_inited) {
        pthread_cond_destroy(&tp->notify);
        cond_inited = 0;
    }

    if (mutex_inited) {
        pthread_mutex_destroy(&tp->lock);
        mutex_inited = 0;
    }

    if (tp && tp->queue.tasks) {
        free((void*) tp->queue.tasks);
        tp->queue.tasks = 0;
    }

    if (tp && tp->pool.threads) {
        free((void*) tp->pool.threads);
        tp->pool.threads = 0;
    }

    if (tp) {
        free((void*) tp);
        tp = 0;
    }

    return tp;
}

int thrpool_destroy(ThrPool* tp, int immediate) {
    int err = THRPOOL_STATUS_OK;

    do {
        if (!tp) {
            err = THRPOOL_STATUS_INVALID;
            break;
        }

        err = thrpool_stop(tp, immediate);
        if (err) {
            break;
        }

        if (tp->pool.started > 0) {
            err = THRPOOL_STATUS_CANNOT_STOP;
            break;
        }

        err = thrpool_free(tp);
        if (err) {
            break;
        }
    } while (0);

    return err;
}

int thrpool_add(ThrPool* tp, ThrPoolFn function, void* argument) {
    int mutex_acquired = 0;
    int err = THRPOOL_STATUS_OK;

    do {
        if (!function) {
            err = THRPOOL_STATUS_INVALID;
            break;
        }

        if (!tp) {
            err = THRPOOL_STATUS_INVALID;
            break;
        }


        if (pthread_mutex_lock(&tp->lock) != 0) {
            err = THRPOOL_STATUS_LOCK_UNLOCK;
            break;
        }
        mutex_acquired = 1;

        // Is the queue full?
        if (tp->queue.pending == tp->queue.size) {
            err = THRPOOL_STATUS_QUEUE_FULL;
            break;
        }

        // Are we shutting down?
        if (tp->flags & THRPOOL_FLAG_SHUTTING_DOWN) {
            err = THRPOOL_STATUS_SHUTTING_DOWN;
            break;
        }

        // Add task to queue
        tp->queue.tasks[tp->queue.tail] = (ThrPoolTask) { function, argument };
        tp->queue.tail = (tp->queue.tail + 1) % tp->queue.size;
        ++tp->queue.pending;

        // Wake up at least one worker thread
        if (pthread_cond_signal(&tp->notify) != 0) {
            err = THRPOOL_STATUS_LOCK_UNLOCK;
            break;
        }

    } while (0);

    if (mutex_acquired) {
        if (pthread_mutex_unlock(&tp->lock) != 0) {
            err = THRPOOL_STATUS_LOCK_UNLOCK;
        }
        mutex_acquired = 0;
    }

    return err;
}

// Must be called with mutex released
// TODO make things be reentrant, so a pool can be reused after stopping
static int thrpool_stop(ThrPool* tp, int immediate) {
    int mutex_acquired = 0;
    int err = THRPOOL_STATUS_OK;

    do {
        if (pthread_mutex_lock(&tp->lock) != 0) {
            err = THRPOOL_STATUS_LOCK_UNLOCK;
            break;
        }
        mutex_acquired = 1;

        tp->flags |= THRPOOL_FLAG_SHUTTING_DOWN;
        if (immediate) {
            tp->flags |= THRPOOL_FLAG_SHUTDOWN_IMMEDIATE;
        }

        // Wake up all worker threads
        if (pthread_cond_broadcast(&tp->notify) != 0) {
            err = THRPOOL_STATUS_LOCK_UNLOCK;
            break;
        }

        // Release pool lock
        if (pthread_mutex_unlock(&tp->lock) != 0) {
            err = THRPOOL_STATUS_LOCK_UNLOCK;
            break;
        }
        mutex_acquired = 0;

        // Join all worker threads -- keep a count
        int count = 0;
        for (int j = 0; j < tp->pool.size; j++) {
            if (pthread_join(tp->pool.threads[j], 0) != 0) {
                err = THRPOOL_STATUS_CANNOT_JOIN;
                continue;
            }
            ++count;
        }
        if (count != tp->pool.size) {
            // err was already set in loop for this case
            break;
        }
    } while (0);

    if (mutex_acquired) {
        if (pthread_mutex_unlock(&tp->lock) != 0) {
            err = THRPOOL_STATUS_LOCK_UNLOCK;
        }
        mutex_acquired = 0;
    }

    tp->flags = 0;
    tp->pool.size = 0;
    tp->pool.started = 0;
    return err;
}

// Must be called with mutex released
static int thrpool_free(ThrPool* tp) {
    pthread_cond_destroy(&tp->notify);
    pthread_mutex_destroy(&tp->lock);
    free((void*) tp->queue.tasks);
    free((void*) tp->pool.threads);
    free((void*) tp);
    return THRPOOL_STATUS_OK;
}

// Must be called with mutex released
static void* thrpool_worker(void* arg) {
    ThrPool* tp = (ThrPool*)arg;

    // TODO check for errors
    while (1) {
        // Lock must be taken to wait on conditional variable
        pthread_mutex_lock(&tp->lock);

        // Wait  for notification
        // There could be spurious wakeups
        // This will release and reacquire the lock
        while (tp->queue.pending == 0 && !(tp->flags & THRPOOL_FLAG_SHUTTING_DOWN)) {
            pthread_cond_wait(&tp->notify, &tp->lock);
        }

        if (tp->flags & THRPOOL_FLAG_SHUTDOWN_IMMEDIATE) {
            // done even if there is pending work
            break;
        }

        if ((tp->flags & THRPOOL_FLAG_SHUTTING_DOWN) && tp->queue.pending == 0) {
            // done, there is no pending work
            break;
        }

        // Grab our task
        ThrPoolTask task = {
            tp->queue.tasks[tp->queue.head].function,
            tp->queue.tasks[tp->queue.head].argument,
        };
        tp->queue.head = (tp->queue.head + 1) % tp->queue.size;
        tp->queue.pending -= 1;

        // Unlock mutex
        pthread_mutex_unlock(&tp->lock);

        // do the work, baby!
        (*task.function)(task.argument);
    }

    // Thread done
    tp->pool.started--;
    pthread_mutex_unlock(&tp->lock);
    return 0;
}
