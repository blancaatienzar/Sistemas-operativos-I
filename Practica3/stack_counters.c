/*
 * Blanca Atienzar Martinez
 * Pau Toni Bibiloni Martinez
 * Unai Pueyo Almodovar
 */

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "my_lib.h"

#define N 1000000
#define NUM_THREADS 10

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

static struct my_stack* stack;

void printStack() {
    struct my_stack_node* node = stack->top;

    if (!node) return;

    while (node->next) {
        int* data = (int*) node->data;
        printf("%d\n", *data);
        node = node->next;
    }

    int* data = (int*) node->data;
    printf("%d\n", *data);
}

void* worker(void* arg) {
    for (int i = 0; i < N; i++) {
        pthread_mutex_lock(&mutex);
        int* data = (int*) my_stack_pop(stack);
        pthread_mutex_unlock(&mutex);

        if (!data) pthread_exit(NULL);

        int* tmp = malloc(sizeof(int));
        *tmp = *data + 1;

        pthread_mutex_lock(&mutex);
        my_stack_push(stack, tmp);
        pthread_mutex_unlock(&mutex);
    }

    pthread_exit(NULL);
}

int main(int args, char** argv) {
    // Handle arguments error
    if (args != 2) {
        printf("Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    // Read stack or create new one
    stack = my_stack_read(argv[1]);
    if (!stack) stack = my_stack_init(sizeof(int));

    // Show info
    int len = my_stack_len(stack);
    printf("Threads: %d, Iterations: %d\nstack->size: %d\ninitial stack length: %d\n",
           NUM_THREADS, N, stack->size, len);

    printf("initial stack content:\n");
    printStack();

    // Fill the stack with if is not size N
    int added = 0;
    for (int i = len; i < NUM_THREADS; ++i) {
        int* tmp = malloc(sizeof(int));
        *tmp = 0;
        my_stack_push(stack, tmp);
        added++;
    }
    if (added) printf("Number of elements added to inital stack: %d\n", added);

    printf("stack content for treatment:\n");
    printStack();

    printf("new stack length: %d\n\n", my_stack_len(stack));

    // Create threads
    pthread_t threads[NUM_THREADS];

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_create(&threads[i], NULL, worker, NULL);
        printf("%d) Thread %lu created\n", i, threads[i]);
    }

    // Wait for threads to finish
    for (int i = 0; i < NUM_THREADS; i++) pthread_join(threads[i], NULL);

    //Print vals
    printf("\nstack content after threads iterations:\n");
    printStack();
    printf("stack length: %d:\n", my_stack_len(stack));

    // Save stack to file and purge
    int readData = my_stack_write(stack, argv[1]);
    int bytes = my_stack_purge(stack);
    printf("\nWritten elements from stack to file: %d\nReleased bytes: %d\nBye from main\n",
           readData , bytes);
    return 0;
}