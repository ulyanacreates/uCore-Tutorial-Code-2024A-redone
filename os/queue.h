#ifndef QUEUE_H
#define QUEUE_H
#define QUEUE_SIZE (1024)

// TODO: change the queue to a priority queue sorted by priority

struct queue {
	int data[QUEUE_SIZE];
	int front;
	int tail;
	int empty;
	// STEP6: add a cmp function to compare the priority of two processes
	int (*cmp)(int, int);
};

void init_queue(struct queue *, int (*cmp)(int, int));
void push_queue(struct queue *, int);
int pop_queue(struct queue *);

#endif // QUEUE_H
