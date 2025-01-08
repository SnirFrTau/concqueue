#include <stdlib.h>
#include <stdbool.h>
#include <stdatomic.h>
#include <threads.h>

// ===========================================================

typedef struct _node {
    struct node *next;
    void *content;
} Node;


typedef struct _queue {
    Node *first;
    Node *last;
} Queue;

// ===========================================================

Queue *queue;

mtx_t mutex;
cnd_t nonEmpty;
size_t count;

// ===========================================================

void initQueue(void) {
    queue = malloc(sizeof(queue));
    // Assuming malloc(...) succeeded
    *queue = {NULL, NULL};
    count = 0;
    mtx_init(&mutex, mtx_plain);
}


void destroyQueue(void) {
    
    mtx_destroy(&mutex);
}

// ===========================================================

void enqueue(void *item) {
    mtx_lock(&mutex);
    Node *node = malloc(sizeof(Node));
    *node = {NULL, item};
    if (queue->first == NULL) {
	queue->first = node;
	queue->last = node;
    }
    else {
	queue->last->next = node;
	queue->last = node;
    }
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
    free(first_node);
	 
    count++;
    mtx_unlock(&mutex);
}


bool tryDequeue(void **address) {
    Node *first_node;
    int rv = mtx_trylock(&mutex);
    if (rv != thrd_success) {
	return false;
    }
    else { // Reachable after lock is reacquired
	first_node = queue->first;
	*address = first_node->content;
	queue->first = queue->first->next;
	free(first_node);
	
	count++;
	mtx_unlock(&mutex);
    } 
}

// ===========================================================

size_t visited(void) {
    return count;
}


