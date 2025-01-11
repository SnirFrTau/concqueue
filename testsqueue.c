#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <threads.h>
#include <stdatomic.h>
#include "queue.h"

// ==================================================

#define N 230

int SIZE = N + 1;
int nthreads = 10;
atomic_int idx;

// ==================================================

int entoqueue(void *numbers) {
    int **nums = (int **)numbers;
    for (int j = 0; j < N / nthreads; j++) {
	enqueue(nums[idx++]);	
    }
    return thrd_success;
}

int detoqueue(void *index) {
    printf("Need to complete %d\n", N / nthreads);
    void **out = malloc(sizeof(void *));

    for (int j = 0; j < N / nthreads; ) {
	//printf("i (%d) dequeue in attempt %d of %d\n", *((int *)index), j+1, N / nthreads);
	dequeue();
	j++;
	// printf("%d\n", *out);
    }
    free(out);
    return thrd_success;
}

int detryqueue(void *index) {
    printf("Need to complete %d\n", N / nthreads);
    void **out = malloc(sizeof(void *));

    for (int j = 0; j < N / nthreads; ) {
	printf("i (%d) dequeue in attempt %d of %d\n", *((int *)index), j+1, N / nthreads);
	if (tryDequeue(out))
	    j++;
	// printf("%d\n", *out);
    }
    free(out);
    return thrd_success;
}

int derogueue() {
    void *out;
    out = dequeue();
    printf("'Tis I, the rogue! And I captured %d!\n", *(int *)out);
    return thrd_success;
}

// ==================================================

void enqueue_numbers(int **numbers) {
    int i, status, rv;
    thrd_t threads[nthreads];
    thrd_t rogue;
    
    idx = 0;
    
    // Create rogue thread
    rv = thrd_create(&rogue, derogueue, NULL);

    // Enqueue all numbers
    for (i = 0; i < nthreads; i++) {
      	rv = thrd_create(&threads[i], entoqueue, (void *)numbers);
	if (rv != thrd_success) {
	    fprintf(stderr, "Oopsie in thrd_create\n");
	    exit(-1);
	}
    }
    for (i = 0; i < nthreads; i++) {
	rv = thrd_join(threads[i], &status);
	if (rv != thrd_success) {
	    fprintf(stderr, "Oopsie in thrd_join\n");
	    exit(-1);
	}
    }
    enqueue(numbers[SIZE - 1]);
    thrd_join(rogue, &status);
    
    printf("Joined in the end of enqueuing.\n");
}

// ==================================================

int main(void) {
    int **numbers;
    int **out;
    int i, status, rv;
    // bool succ;
    thrd_t threads[nthreads];

    // ==============================================
    // Memory Allocation
    // ==============================================
    
    numbers = malloc(SIZE * sizeof(int *));
    for (i = 0; i < SIZE; i++) {
	numbers[i] = malloc(sizeof(int));
	*(numbers[i]) = i*2;
    }
    out = malloc(sizeof(int *));

    // ==============================================
    // Test Zone
    // ==============================================

    initQueue();
    printf("Not to be too smart, first try got %d", tryDequeue((void **)out));
    
    enqueue_numbers(numbers);
    
    // Safe dequeuing
    for (i = 1; i < SIZE; i++) {
	*out = dequeue();
	printf("%d", **out == *(numbers[i]));
    }
    //printf("\nIn the end, the queue has is_empty=%d\n", is_empty());

    // Safe enqueuing
    
    for (i = 0; i < SIZE; i++) {
	// printf("%ld\n", numbers[i]);
	enqueue(numbers[i]);
    }
    derogueue();
    
    for (i = 0; i < nthreads; i++) {
        int indices[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
	printf("creating %d\n", i);
	if (i % 2){
	    rv = thrd_create(&threads[i], detoqueue, &(indices[i]));
	}
	else {
	    rv = thrd_create(&threads[i], detryqueue, &(indices[i]));
	}
	if (rv != thrd_success) {
	    fprintf(stderr, "Oopsie in thrd_create\n");
	    exit(-1);
	}
    }

    for (i = 0; i < nthreads; i++) {
	printf("joining %d\n", i);
	rv = thrd_join(threads[i], &status);
	if (rv != thrd_success) {
	    fprintf(stderr, "Oopsie in thrd_join\n");
	    exit(-1);
	}
    }

    printf("Allegedly visited %ld\n", visited());
    destroyQueue();

    printf("Testing over\n");

    // ==============================================

    // Important: memory deallocation
    for (i = 0; i < SIZE; i++) {
	free(numbers[i]);
    }
    free(numbers);
    
    return 0;
}
