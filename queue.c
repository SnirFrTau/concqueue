#include <stdlib.h>
#include <stdbool.h>
#include <stdatomic.h>
#include <threads.h>

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

bool is_empty() {
    return (queue->first == NULL);
}

// ===========================================================

typedef struct _waitnode {
    struct _waitnode *next;
    cnd_t *cv;
} WaitNode;

typedef struct _waitqueue {
    WaitNode *first;
    WaitNode *last;
} WaitQueue;

WaitQueue *wqueue;
mtx_t thread_mtx;

void wenqueue(cnd_t cv) {
    // Sets up a CV for this thread to sleep on
    cnd_init(&cv);
    WaitNode *wnode = malloc(sizeof(WaitNode));
    *wnode = (WaitNode){NULL, &cv};
    if (wqueue->first) {
	wqueue->last->next = wnode;
    }
    else {
	wqueue->first = wnode;
	wqueue->last = wnode;
    }
    cnd_wait(&cv, &thread_mtx);
}

void wdequeue() {
    // Dequeues the next thread to wake up
    WaitNode *first_wnode;
    cnd_t *cv;
    if (wqueue->first) {
	first_wnode = wqueue->first;
	if (wqueue->first == wqueue->last) {
	    wqueue->last = NULL;
	}
	wqueue->first = wqueue->first->next;
	cv = first_wnode->cv;
	free(first_wnode);
	cnd_signal(cv);
    }
}

// ===========================================================

mtx_t queue_mtx;
cnd_t nonEmpty;
atomic_int count;

// ===========================================================

void initQueue(void) {
    queue = malloc(sizeof(Queue));
    *queue = (Queue){NULL, NULL};
    wqueue = malloc(sizeof(WaitQueue));
    *wqueue = (WaitQueue){NULL, NULL};
    count = 0;
    mtx_init(&queue_mtx, mtx_plain);
    mtx_init(&thread_mtx, mtx_plain);
    cnd_init(&nonEmpty);
}

void destroyQueue(void) {
    mtx_destroy(&queue_mtx);
    mtx_destroy(&thread_mtx);
    cnd_destroy(&nonEmpty);
    empty_queue();
    free(queue);
    free(wqueue);
}

// ===========================================================

void enqueue(void *item) {
    mtx_lock(&thread_mtx);
    cnd_t cv;
    if (wqueue->first) {
	wenqueue(cv);
	cnd_destroy(&cv);
    }
    mtx_lock(&queue_mtx);
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
    cnd_signal(&nonEmpty);
    mtx_unlock(&queue_mtx);
    wdequeue();
    mtx_unlock(&thread_mtx);
}

void *dequeue(void) {
    mtx_lock(&thread_mtx);
    Node *first_node;
    void *item;
    cnd_t cv;
    if (wqueue->first) {
	wenqueue(cv);
	cnd_destroy(&cv);
    }
    mtx_lock(&queue_mtx);
    while (queue->first == NULL) {
	mtx_unlock(&queue_mtx);
	cnd_wait(&nonEmpty, &thread_mtx);
	mtx_lock(&queue_mtx);
    } // Reachable after lock is reacquired
    first_node = queue->first;
    item = first_node->content;
    queue->first = queue->first->next;
    if (queue->first == NULL) {
	queue->last = NULL;
    }
    free(first_node);
    count++;
    mtx_unlock(&queue_mtx);
    wdequeue();
    mtx_unlock(&thread_mtx);
    return item;
}

bool tryDequeue(void **address) {
    Node *first_node;
    int rv = mtx_trylock(&thread_mtx);
    if (rv != thrd_success) {
	return false;
    }
    else {
	rv = mtx_trylock(&queue_mtx);
	if (rv != thrd_success) {
	    return false;
	}	
	else { // Reachable after lock is reacquired
	    if (is_empty()) {
		mtx_unlock(&queue_mtx);
		mtx_unlock(&thread_mtx);
		return false;
	    }
	    first_node = queue->first;
	    *address = first_node->content;
	    queue->first = queue->first->next;
	    free(first_node);
	    count++;
	    mtx_unlock(&queue_mtx);
	    mtx_unlock(&thread_mtx);
	    return true;
	} 
    }
}

// ===========================================================

size_t visited(void) {
    return (size_t)count;
}


