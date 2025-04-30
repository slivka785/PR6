#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define THREADS 4
#define MAX_RESULTS 100

typedef struct {
    int* data;
    int target;
    int start;
    int end;
    int* results;
    int* result_count;
    pthread_mutex_t* mutex;
    pthread_barrier_t* barrier;
} thread_args_t;

void* search_all(void* arg) {
    thread_args_t* args = (thread_args_t*)arg;

    for (int i = args->start; i < args->end; ++i) {
        if (args->data[i] == args->target) {
            pthread_mutex_lock(args->mutex);
            args->results[*args->result_count] = i;
            (*args->result_count)++;
            pthread_mutex_unlock(args->mutex);
        }
    }

    pthread_barrier_wait(args->barrier);
    return NULL;
}

int compare_ints(const void* a, const void* b) {
    return (*(int*)a - *(int*)b);
}

int main() {
    int data[] = {1, 3, 5, 3, 9, 3, 3, 5, 3, 1, 0, 3, 8, 11};
    int size = sizeof(data) / sizeof(data[0]);
    int target = 3;

    pthread_t threads[THREADS];
    thread_args_t args[THREADS];

    int results[MAX_RESULTS];
    int result_count = 0;

    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_barrier_t barrier;
    pthread_barrier_init(&barrier, NULL, THREADS);

    int chunk = size / THREADS;

    for (int i = 0; i < THREADS; ++i) {
        args[i].data = data;
        args[i].target = target;
        args[i].start = i * chunk;
        args[i].end = (i == THREADS - 1) ? size : args[i].start + chunk;
        args[i].results = results;
        args[i].result_count = &result_count;
        args[i].mutex = &mutex;
        args[i].barrier = &barrier;
        pthread_create(&threads[i], NULL, search_all, &args[i]);
    }

    for (int i = 0; i < THREADS; ++i) {
        pthread_join(threads[i], NULL);
    }

    pthread_mutex_destroy(&mutex);
    pthread_barrier_destroy(&barrier);

    qsort(results, result_count, sizeof(int), compare_ints);

    printf("Найденные индексы: ");
    for (int i = 0; i < result_count; ++i) {
        printf("%d ", results[i]);
    }
    printf("\n");

    return 0;
} 
