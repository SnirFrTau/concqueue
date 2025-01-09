#include <stdlib.h>
#include <stdbool.h>
#include <stdatomic.h>
#include <threads.h>

// Remove later
#include <stdio.h>

// ===========================================================

typedef struct _node {
    struct _node *next;
    void *content;
} Node;


typedef struct _queue {
    Node *first;
    Node *last;
} Queue;

Queue *queue;

void empty_queue() {
    Node *next;
    Node *first;
    if (!(queue->first)) return;
    else {
	while (queue->first != queue->last) {
	    next = queue->first->next;
	    first = queue->first;
	    queue->first = next;
	    free(first);
	}
	free(queue->first);
    }
}

// ===========================================================

mtx_t mutex;
cnd_t nonEmpty;
atomic_int count;

// ===========================================================

void initQueue(void) {
    queue = malloc(sizeof(Queue));
    // Assuming malloc(...) succeeded
    *queue = (Queue){NULL, NULL};
    count = 0;
    mtx_init(&mutex, mtx_plain);
    cnd_init(&nonEmpty);
}


void destroyQueue(void) {
    mtx_destroy(&mutex);
    cnd_destroy(&nonEmpty);
    printf("cond %d\n", queue->first == queue->last);
    empty_queue();
    free(queue);
}

// ===========================================================

void enqueue(void *item) {
    mtx_lock(&mutex);
    Node *node = malloc(sizeof(Node));
    *node = (Node){NULL, item};
    if (queue->first == NULL) {
	queue->first = node;
	queue->last = node;
    }
    else {
	queue->last->next = node;
	queue->last = node;
    }
    printf("eltesto\n");
    printf("enqueued %d\n", *(int *)item);
    cnd_signal(&nonEmpty);
    mtx_unlock(&mutex);
}


void *dequeue(void) {
    Node *first_node;
    void *item;
    mtx_lock(&mutex);
    while (queue->first == NULL) {
	cnd_wait(&nonEmpty, &mutex);
    } // Reachable after lock is reacquired
    first_node = queue->first;
    item = first_node->content;
    queue->first = queue->first->next;
    if (queue->first == NULL) {
	queue->last = NULL;
    }
    free(first_node);
    printf("%d\n", *((int *)item));
    count++;
    mtx_unlock(&mutex);

    return item;
}


bool tryDequeue(void **address) {
    Node *first_node;
    int rv = mtx_trylock(&mutex);
    if (rv != thrd_success) {
	printf("Failed!\n");
	return false;
    }
    else { // Reachable after lock is reacquired
	first_node = queue->first;
	*address = first_node->content;
	queue->first = queue->first->next;
	free(first_node);
	printf("%d\n", **((int **)address));
	count++;
	mtx_unlock(&mutex);
	return true;
    } 
}

// ===========================================================

size_t visited(void) {
    return (size_t)count;
}


