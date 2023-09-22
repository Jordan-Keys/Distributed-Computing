#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#define MAX_THREADS 8

/*
CUSTOM DATA STRUCTURE
 The next block defines a custom data structure SortArgs using a struct. 
 It contains an integer pointer arr, and two size_t variables left and right.
*/  
typedef struct {
    int* arr;
    size_t left;
    size_t right;
} SortArgs;
/*   
MERGE FUNCTION
defines a function named merge that performs the merging step of the merge sort algorithm. 
It takes an array arr and the indices left, mid, and right to specify the subarray to merge.
*/
void merge(int* arr, size_t left, size_t mid, size_t right) {
    int* temp = (int*)malloc((right - left + 1) * sizeof(int));
    size_t i = left;
    size_t j = mid + 1;
    size_t k = 0;

    while (i <= mid && j <= right) {
        if (arr[i] <= arr[j]) {
            temp[k++] = arr[i++];
        } else {
            temp[k++] = arr[j++];
        }
    }

    while (i <= mid) {
        temp[k++] = arr[i++];
    }

    while (j <= right) {
        temp[k++] = arr[j++];
    }

    for (i = left, k = 0; i <= right; i++, k++) {
        arr[i] = temp[k];
    }

    free(temp);
}

pthread_mutex_t mutex; // Declare a mutex

void* merge_sort_thread(void* arg) {
    SortArgs* args = (SortArgs*)arg;

    size_t left = args->left;
    size_t right = args->right;
    int* arr = args->arr;

    if (left < right) {
        size_t mid = left + (right - left) / 2;

        pthread_t* threads = (pthread_t*)malloc(2 * sizeof(pthread_t));
        SortArgs* left_args = (SortArgs*)malloc(sizeof(SortArgs));
        SortArgs* right_args = (SortArgs*)malloc(sizeof(SortArgs));

        left_args->arr = arr;
        left_args->left = left;
        left_args->right = mid;

        right_args->arr = arr;
        right_args->left = mid + 1;
        right_args->right = right;

        pthread_create(&threads[0], NULL, merge_sort_thread, (void*)left_args);
        pthread_create(&threads[1], NULL, merge_sort_thread, (void*)right_args);

        pthread_join(threads[0], NULL);
        pthread_join(threads[1], NULL);

        // Acquire the mutex before merging
        pthread_mutex_lock(&mutex);

        merge(arr, left, mid, right);

        // Release the mutex after merging
        pthread_mutex_unlock(&mutex);

        free(left_args);
        free(right_args);
        free(threads);
    }

    pthread_exit(NULL);
}

void merge_sort(int* arr, size_t size) {
    pthread_t* threads = (pthread_t*)malloc(sizeof(pthread_t));
    SortArgs* args = (SortArgs*)malloc(sizeof(SortArgs));

    args->arr = arr;
    args->left = 0;
    args->right = size - 1;

    pthread_create(&threads[0], NULL, merge_sort_thread, (void*)args);
    pthread_join(threads[0], NULL);

    free(args);
    free(threads);
}

int main() {
    int arr[] = { 8, 27, 43, 3, 9, 82, 10,30 };
    size_t n = sizeof(arr) / sizeof(arr[0]);

    pthread_mutex_init(&mutex, NULL); // Initialize the mutex

    merge_sort(arr, n);

    printf("Sorted array: ");
    for (size_t i = 0; i < n; i++) {
        printf("%d ", arr[i]);
    }
    printf("\n");

    pthread_mutex_destroy(&mutex); // Destroy the mutex

    return 0; 
}
