// using pthreads (POSIX threads) in C to increment a counter value from multiple threads, while ensuring thread safety using a mutex short form for (mutual exclusion)
// NB: Mutex allows multiple threads to take turns accessing a shared resource

// including all necessary header files
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

// Here the ThreadData struct is defined, which contains pointers to the counter and error counter variables, as well as a pointer to the counterLock mutex. This is a custom-defined structure used in the code to pass data to each thread. By encapsulating all these variables it becomes easier to pass data to a thread
typedef struct {
    int* counter;
    int* errorcounter;
    pthread_mutex_t* counterLock;
} ThreadData;

// This function is executed by each thread. It takes a void pointer argument arg that is casted back to a ThreadData pointer. Inside the function, the counter, error count, and counterLock variables are extracted from the ThreadData struct.
void* ThreadFunc(void* arg) {
    ThreadData* threadData = (ThreadData*)arg;
    int* counter = threadData->counter;
    int* errorcount = threadData->errorcounter;
    pthread_mutex_t* counterLock = threadData->counterLock;

    for (int i = 0; i < 10; i++) {
        pthread_mutex_lock(counterLock); // Lock the mutex. This ensures that one thread acesses the variable at a time

        (*counter)++; // Increment counter safely ( by dereferncing the counter pointer and incrementing its value)

        struct timespec sleepTime;
        sleepTime.tv_sec = 0;
        sleepTime.tv_nsec = 100000000; // Sleep for 100 milliseconds to simulate some processing time
 
        nanosleep(&sleepTime, NULL);

        if (*counter > 1) {
            (*errorcount)++; // Increment errorcount safely
        }

        printf("Counter: %d, Error Counter: %d\n", *counter, *errorcount);

        pthread_mutex_unlock(counterLock); // Unlock the mutex to allow other threads to access the shared variables.
    }

    return NULL;
}

int main() {
    const int numThreads = 10; // declaring number of threads 
    
// dynamic memory allocation for the two variables counter and errorcount
    int* counter = (int*)malloc(sizeof(int));
    *counter = 0;

    int* errorcount = (int*)malloc(sizeof(int));
    *errorcount = 0; // dereferncing and setting to zero

    pthread_mutex_t counterlock; // declaring a mutex called counterlock
    pthread_mutex_init(&counterlock, NULL);

    pthread_t threads[numThreads]; // create array for thread ids with size of numThreads
    ThreadData threadData[numThreads]; // Create an array of ThreadData structs with the size of numthreads

    // Create Threads
    /*
    The loop is used to create the threads. In each iteration, the counter, errorcount, and counterlock variables are assigned to the corresponding fields of the ThreadData struct. Then, pthread_create() is called to create a new thread and execute the ThreadFunc function.
    */
    for (int i = 0; i < numThreads; i++) {
        threadData[i].counter = counter;
        threadData[i].errorcounter = errorcount;
        threadData[i].counterLock = &counterlock;

        if (pthread_create(&threads[i], NULL, ThreadFunc, (void*)&threadData[i]) != 0) {
            printf("Failed to create Thread %d\n", i);
            return 1;
        }
    }
    
    
    // This for loop Waits for threads to finish
    for (int i = 0; i < numThreads; i++) {
        pthread_join(threads[i], NULL); //(blocks execution of main thread until the threads finish)
    }
    

    // Destroy the mutex
    pthread_mutex_destroy(&counterlock);

    // Print final counter and error count values
    printf("Final Counter: %d, Final Error Counter: %d\n", *counter, *errorcount);

    // Free up the dynamically allocated memory
    free(counter);
    free(errorcount);

    return 0;
}
