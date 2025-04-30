
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define NUM_WORKERS 4

typedef struct {
    int* array;
    int value;
    int from;
    int to;
    int* is_found;
    int* index_result;
    pthread_mutex_t* lock;
    pthread_barrier_t* sync_point;
} worker_input_t;

void* threaded_search(void* input) {
    worker_input_t* params = (worker_input_t*)input;

    for (int pos = params->from; pos < params->to; ++pos) {
        pthread_mutex_lock(params->lock);
        if (*(params->is_found)) {
            pthread_mutex_unlock(params->lock);
            break;
        }
        if (params->array[pos] == params->value) {
            *(params->is_found) = 1;
            *(params->index_result) = pos;
            pthread_mutex_unlock(params->lock);
            break;
        }
        pthread_mutex_unlock(params->lock);
    }

    pthread_barrier_wait(params->sync_point);
    return NULL;
}

int main() {
    int sequence[] = {4, 2, 6, 8, 2, 10, 2, 7, 2, 0, -1, 2, 11};
    int length = sizeof(sequence) / sizeof(sequence[0]);
    int search_val = 2;

    pthread_t worker_threads[NUM_WORKERS];
    worker_input_t tasks[NUM_WORKERS];
    pthread_mutex_t access_mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_barrier_t barrier;
    pthread_barrier_init(&barrier, NULL, NUM_WORKERS);

    int match_found = 0;
    int match_index = -1;
    int segment_size = length / NUM_WORKERS;

    for (int t = 0; t < NUM_WORKERS; ++t) {
        tasks[t].array = sequence;
        tasks[t].value = search_val;
        tasks[t].from = t * segment_size;
        tasks[t].to = (t == NUM_WORKERS - 1) ? length : (t + 1) * segment_size;
        tasks[t].is_found = &match_found;
        tasks[t].index_result = &match_index;
        tasks[t].lock = &access_mutex;
        tasks[t].sync_point = &barrier;

        pthread_create(&worker_threads[t], NULL, threaded_search, &tasks[t]);
    }

    for (int t = 0; t < NUM_WORKERS; ++t) {
        pthread_join(worker_threads[t], NULL);
    }

    if (match_index >= 0) {
        printf("Найдено первое совпадение по индексу: %d\n", match_index);
    } else {
        printf("Совпадений не найдено.\n");
    }

    pthread_mutex_destroy(&access_mutex);
    pthread_barrier_destroy(&barrier);
    return 0;
} 
