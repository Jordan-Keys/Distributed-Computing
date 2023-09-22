//Certainly! Let's go through each block of code in the downloader program and explain its purpose:


#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define NUM_THREADS 4
#define NUM_FILES 8

typedef struct {
    int file_id;
    char filename[50];
} DownloadTask;
/*
The program starts by including necessary header files (stdio.h, stdlib.h, and pthread.h) for input/output, memory allocation, and thread operations, respectively.

Next, we define two constants: NUM_THREADS represents the number of threads that will be created to download files concurrently, and NUM_FILES represents the total number of files to be downloaded.

Then, we define a structure called DownloadTask to encapsulate information about a file download task. It contains an int field for the file ID and a character array filename to store the name of the file being downloaded.
*/


void* download_file(void* arg) {
    DownloadTask* task = (DownloadTask*)arg;
    printf("Downloading file %s...\n", task->filename);
    // Simulating file download
    sleep(3);
    printf("File %s downloaded successfully.\n", task->filename);
    pthread_exit(NULL);
}
/*
The download_file function represents the work done by each thread. It takes a void pointer arg as an argument, which is expected to point to a DownloadTask structure.

Inside the function, the arg is cast to a DownloadTask pointer and stored in the task variable. It then prints a message indicating the file being downloaded. In this simplified example, a call to sleep(3) simulates the file download process. After that, it prints a success message and frees the memory allocated for the task.

Finally, it calls pthread_exit(NULL) to indicate that the thread has finished executing.
*/

int main() {
    pthread_t threads[NUM_THREADS];
    int i;

    // Create tasks
    DownloadTask* tasks[NUM_FILES];
    for (i = 0; i < NUM_FILES; i++) {
        tasks[i] = (DownloadTask*)malloc(sizeof(DownloadTask));
        tasks[i]->file_id = i;
        sprintf(tasks[i]->filename, "file%d.txt", i);
    }
/*
The main function is where the program execution begins. It first declares an array of pthread_t type, named threads, to store thread identifiers.

Next, an array of DownloadTask pointers, named tasks, is declared. It is used to store the tasks for downloading files.

The for loop initializes the tasks array by dynamically allocating memory for each DownloadTask structure using malloc. It sets the file_id field to the corresponding file ID, and uses sprintf to generate the filename based on the file ID. The filename format used here is "fileX.txt", where X is the file ID.
*/

    // Create threads
    for (i = 0; i < NUM_THREADS; i++) {
        if (pthread_create(&threads[i], NULL, download_file, tasks[i % NUM_FILES]) != 0) {
            fprintf(stderr, "Error creating thread.\n");
            exit(1);
        }
    }
/*
In this block, a for loop is used to create the desired number of threads (NUM_THREADS). The pthread_create function is called to create a thread, passing the address of threads[i] to store the thread identifier.

If pthread_create returns a non-zero value, it means an error occurred while creating the thread. In that case, an error message is printed to the standard error stream using fprintf, and the program is exited with an error code of 1.

The fourth argument to pthread_create is the arg parameter passed to the download_file function, which is a task from the tasks array. The % operator is used to cycle through the available tasks when the number of threads is greater than the number of files. This way, each thread gets a task to work on.
*/

    // Wait for threads to finish
    for (i = 0; i < NUM_THREADS; i++) {
        if (pthread_join(threads[i], NULL) != 0) {
            fprintf(stderr, "Error joining thread.\n");
            exit(1);
        }
    }
/*
After creating the threads, another for loop is used to wait for each thread to finish its execution. The pthread_join function is called with threads[i] as the thread identifier.

If pthread_join returns a non-zero value, an error occurred while joining the thread. In that case, an error message is printed, and the program is exited with an error code of 1.
*/

    // Cleanup
for (i = 0; i < NUM_FILES; i++) {
    free(tasks[i]);
}


    return 0;
}
/*
The last block of code performs cleanup by freeing the memory allocated for each task in the tasks array using the free function.

Finally, the main function returns 0 to indicate successful program execution.

Keep in mind that this example focuses on demonstrating the basics of distributed computing and lock-free synchronization. In a real-world scenario, additional considerations such as error handling, load balancing, fault tolerance, and more sophisticated synchronization mechanisms might be required.*/




/*
During the execution of the program, the threads are responsible for downloading files concurrently. Here is the exact behavior of the threads:

    Thread Creation:
        The program creates an array of pthread_t type called threads to store thread identifiers.
        It also creates an array of DownloadTask pointers called tasks to store the tasks for downloading files.

    Task Initialization:
        The program uses a for loop to initialize the tasks array.
        For each DownloadTask structure, memory is dynamically allocated using malloc.
        The file_id field is set to the corresponding file ID, and the filename field is generated using sprintf based on the file ID.

    Thread Creation and Execution:
        Another for loop is used to create the desired number of threads (specified by NUM_THREADS).
        The pthread_create function is called to create a thread.
        The address of threads[i] is passed as an argument to store the thread identifier.
        The fourth argument to pthread_create is the arg parameter, which is a task from the tasks array. The % operator is used to cycle through the available tasks when the number of threads is greater than the number of files. This way, each thread gets a task to work on.
        If an error occurs during thread creation, an error message is printed, and the program exits with an error code of 1.

    File Download Simulation:
        Each thread executes the download_file function.
        Inside download_file, the task is cast from void* to a DownloadTask* pointer.
        A message is printed indicating the file being downloaded.
        The sleep function is called to simulate the file download process, causing the thread to pause for 3 seconds.
        After the simulated download, a success message is printed.
        Note that the free(task) line has been removed from the download_file function as per the previous explanation.

    Thread Joining:
        After creating the threads, another for loop is used to wait for each thread to finish its execution using the pthread_join function.
        The pthread_join function is called with threads[i] as the thread identifier.
        If an error occurs while joining the thread, an error message is printed, and the program exits with an error code of 1.

    Cleanup:
        Once all threads have finished executing, a final for loop is used to free the memory allocated for each task in the tasks array using the free function.

    Program Completion:
        The main function returns 0 to indicate successful program execution.

In summary, the program creates multiple threads that download files concurrently. Each thread is assigned a task from the tasks array and simulates the file download process. After all threads have completed their execution, the program cleans up by freeing the dynamically allocated memory and then exits.
*/