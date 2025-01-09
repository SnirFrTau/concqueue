#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <threads.h>
#include <stdatomic.h>
#include "queue.h"


#define N 230

int SIZE = N + 1;
int nthreads = 10;
atomic_int idx;


int entoqueue(void *numbers) {
    int **nums = (int **)numbers;
    for (int j = 0; j < N / nthreads; j++) {
	enqueue(nums[idx++]);	
    }
    return thrd_success;
}


int detoqueue() {
    printf("Need to complete %d\n", N / nthreads);
    void **out = malloc(sizeof(void *));

    for (int j = 0; j < N / nthreads; ) {
	printf("i dequeue in attempt %d of %d\n", j+1, N / nthreads);
	dequeue();
	    j++;
	// printf("%d\n", *out);
    }
    free(out);
    return thrd_success;
}


int derogueue() {
    void *out;
    out = dequeue();
    printf("HA I GOT YOU %d\n", *(int *)out);
    return thrd_success;
}

// ==================================================


int main(void) {
    int **numbers;
    int i, status, rv;
    // bool succ;
    thrd_t threads[nthreads];
    thrd_t rogue;

    // ==============================================
    // Memory Allocation
    // ==============================================
    
    numbers = malloc(SIZE * sizeof(int *));
    for (i = 0; i < SIZE; i++) {
	numbers[i] = malloc(sizeof(int));
	*(numbers[i]) = i;
    }

    // ==============================================
    // Test Zone
    // ==============================================

    idx = 0;
    initQueue();

    rv = thrd_create(&rogue, derogueue, NULL);
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

    thrd_join(rogue, &status);

    printf("Finished joining!\n");

    for (i = 0; i < nthreads; i++) {
	sleep(10);
	printf("creating %d\n", i);
	rv = thrd_create(&threads[i], detoqueue, NULL);
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
