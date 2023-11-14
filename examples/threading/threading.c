#include "threading.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#define DEBUG_LOG(msg,...)
//#define DEBUG_LOG(msg,...) printf("threading: " msg "\n" , ##__VA_ARGS__)
#define ERROR_LOG(msg,...) printf("threading ERROR: " msg "\n" , ##__VA_ARGS__)

// Function prototype
void* threadfunc(void* thread_param);

void* threadfunc(void* thread_param)
{
    // TODO: wait, obtain mutex, wait, release mutex as described by thread_data structure
    struct thread_data* thread_func_args = (struct thread_data *) thread_param;

    // Wait for a specified time
    usleep(thread_func_args->wait_to_obtain_ms * 1000);

    // Obtain the mutex
    if (pthread_mutex_lock(thread_func_args->mutex) != 0) {
        ERROR_LOG("Failed to obtain mutex");
        thread_func_args->thread_complete_success = false;
        return thread_param;
    }

    // Wait for a specified time
    usleep(thread_func_args->wait_to_release_ms * 1000);

    // Release the mutex
    if (pthread_mutex_unlock(thread_func_args->mutex) != 0) {
        ERROR_LOG("Failed to release mutex");
        thread_func_args->thread_complete_success = false;
        return thread_param;
    }

    // Set the thread completion status to true
    thread_func_args->thread_complete_success = true;

    // Return the thread data structure
    return thread_param;
}

bool start_thread_obtaining_mutex(pthread_t *thread, pthread_mutex_t *mutex, int wait_to_obtain_ms, int wait_to_release_ms)
{
    // Allocate memory for thread_data
    struct thread_data *thread_data_ptr = (struct thread_data*)malloc(sizeof(struct thread_data));
    if (thread_data_ptr == NULL) {
        ERROR_LOG("Failed to allocate memory for thread_data");
        return false;
    }

    // Set up mutex and wait arguments
    thread_data_ptr->mutex = mutex;
    thread_data_ptr->wait_to_obtain_ms = wait_to_obtain_ms;
    thread_data_ptr->wait_to_release_ms = wait_to_release_ms;

    // Pass thread_data to created thread using threadfunc() as the entry point
    if (pthread_create(thread, NULL, threadfunc, (void*)thread_data_ptr) != 0) {
        ERROR_LOG("Failed to create thread");
        free(thread_data_ptr);
        return false;
    }

    // Return true if successful
    return true;
}

