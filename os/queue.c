#include "queue.h"
#include "defs.h"


void init_queue(struct queue *q, int (*cmp)(int, int))
{
	q->front = q->tail = 0;
	q->empty = 1;
	q->cmp = cmp;
}

void push_queue(struct queue *q, int value)
{
	if (!q->empty && q->front == q->tail) {
		panic("queue shouldn't be overflow");
	}
	q->empty = 0;
	q->data[q->tail] = value;
	q->tail = (q->tail + 1) % NPROC;
}


int pop_queue(struct queue *q)
{                                               
	if (q->empty) 
		return -1;
	int idx = q->front; 
	for (int i = (idx + 1) % NPROC; i != q->tail; i = (i + 1) % NPROC) {
		if (q->cmp(q->data[idx], q->data[i])) {
			idx = i;
		}
	}
	int value = q->data[idx];
	q->data[idx] = q->data[q->front];	
	q->front = (q->front + 1) % NPROC;
	if (q->front == q->tail)
		q->empty = 1;
	return value;
}
