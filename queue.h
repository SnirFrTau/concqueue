#include <stdlib.h>
#include <stdbool.h>


void initQueue(void);
void destroyQueue(void);
void enqueue(void *);
void *dequeue(void);
bool tryDequeue(void **);
size_t visited(void);
bool is_empty(void);
