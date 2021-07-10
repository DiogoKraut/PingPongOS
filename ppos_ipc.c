// GRR20166365 Diogo Paris Kraut

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/time.h>
#include <string.h>
#include "ppos.h"
#include "ppos_data.h"

void enter_cs(int *lock) {
	/* gcc built-in atomic funtion */
	while(__atomic_fetch_or(lock, 1, __ATOMIC_SEQ_CST));
}

void leave_cs(int *lock) {
	*lock = 0;
}

int sem_create(semaphore_t *s, int value) {
	if(!s) return -1;

	s->val = value;
	s->lock = 0;
	s->destroyed = 0;
	s->waitQ = NULL;

	return 0;
}

int sem_down(semaphore_t *s) {
	if(!s || s->destroyed) return -1;

	enter_cs(&s->lock);
	s->val--;
	leave_cs(&s->lock);
	#ifdef DEBUG
	printf("sem_down val: %d taskID: %d\n", s->val, task_id());
	#endif

	/* if semaphore already down, wait */
	if(s->val < 0) {
		currentTask->status = SUSPENDED;
		queue_append((queue_t **)&s->waitQ, (queue_t *)currentTask);
		task_switch(&dispatcherTask); 
	}
	return 0;
}

int sem_up(semaphore_t *s) {
	if(!s || s->destroyed) return -1;

	enter_cs(&s->lock);
	s->val++;
	if(s->waitQ != NULL) {
		task_t *aux = s->waitQ;
		queue_remove((queue_t **)&s->waitQ, (queue_t *)s->waitQ);
		queue_append((queue_t **)&rdyQ, (queue_t *)aux);
	}

	leave_cs(&s->lock);

	#ifdef DEBUG
	printf("sem_up val: %d taskID: %d\n", s->val, task_id());
	#endif
	return 0;
}

int sem_destroy(semaphore_t *s) {
	if(!s || s->destroyed) return -1;

	s->destroyed = 1;
	
	int count = queue_size((queue_t*)s->waitQ);
	task_t *aux, *aux2;
    aux = s->waitQ;

    while(count > 0) {
        aux2 = aux->next;
        queue_remove((queue_t **)&s->waitQ, (queue_t *)aux);
        aux->status = RUNNING;
        queue_append((queue_t **)&rdyQ,   (queue_t *)aux);
        aux = aux2;
        count--;
    }
    return 0;
}

int mqueue_create(mqueue_t *queue, int max_msgs, int msg_size) {
	queue->start = 0;
	queue->end = 0;

	queue->destroyed = 0;
	queue->data = malloc(sizeof(msg_size) * max_msgs);
	queue->max_size = max_msgs;
	queue->elem_size = msg_size;

	sem_create(&queue->sem_send, max_msgs);
	sem_create(&queue->sem_recv, 0);
	sem_create(&queue->sem_buffer, 1);

	return 0;
}

int mqueue_send(mqueue_t *queue, void *msg) {
	if(!queue || queue->destroyed) return -1;

	sem_down(&queue->sem_send);
	sem_down(&queue->sem_buffer);

	memcpy(queue->data + (queue->end * queue->elem_size), msg, queue->elem_size);
	queue->end = (queue->end + 1) % queue->max_size; 

	sem_up(&queue->sem_buffer);
	sem_up(&queue->sem_recv);

	return 0;
}

int mqueue_recv(mqueue_t *queue, void *msg) {
	if(!queue || queue->destroyed) return -1;

	sem_down(&queue->sem_recv);
	sem_down(&queue->sem_buffer);

	memcpy(msg, queue->data + (queue->start * queue->elem_size), queue->elem_size);
	queue->start = (queue->start + 1) % queue->max_size;

	sem_up(&queue->sem_buffer);
	sem_up(&queue->sem_send);

	return 0;
}

int mqueue_destroy(mqueue_t *queue) {
	if(!queue || queue->destroyed) return -1;

	queue->destroyed = 1;

	sem_destroy(&queue->sem_buffer);
	sem_destroy(&queue->sem_send);
	sem_destroy(&queue->sem_recv);

	free(queue->data);

	return 0;
}
