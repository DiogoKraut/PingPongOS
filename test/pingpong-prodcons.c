#include <stdio.h>
#include <stdlib.h>
#include "ppos.h"

#define BUFFER_SIZE 5

typedef struct buffer_t {
	struct buffet_t *prev, *next;
	int val;
} buffer_t;

semaphore_t s_vaga, s_item, s_buffer;
task_t p1, p2, c1, c2, c3;

buffer_t *buffer;

void producer(void *arg) {
	printf("%s inicio\n", (char*) arg);
	while(1) {
		int val = random() %100;

		sem_down(&s_vaga);
		sem_down(&s_buffer);

		buffer_t *item = malloc(sizeof(buffer_t));
		item->val = val;
		item->prev = NULL;
		item->next = NULL;

		queue_append((queue_t **)&buffer, (queue_t *)item);
		printf("%s produziu %d\n", (char *)arg, val);

		sem_up(&s_buffer);
		sem_up(&s_item);
		//task_sleep(1000);
	}
}

void consumer(void *arg) {
	printf("%s inicio\n", (char*) arg);

	while(1) {
		task_sleep(1000);
		sem_down(&s_item);
		sem_down(&s_buffer);

		int val = buffer->val;
		queue_remove((queue_t **)&buffer, (queue_t *)buffer);

		sem_up(&s_buffer);
		sem_up(&s_vaga);

		printf("%s consumiu %d\n", (char *)arg, val);

	}
}

int main(int argc, char const *argv[])
{
	ppos_init();

	sem_create(&s_vaga, BUFFER_SIZE);
	sem_create(&s_item, 0);
	sem_create(&s_buffer, 1);

	task_create(&p1, producer, "p1");
	task_create(&p2, producer, "p2");
	task_create(&c1, consumer, "        c1");
	task_create(&c2, consumer, "        c2");
	task_create(&c3, consumer, "        c3");

	task_join(&p1);
	task_join(&p2);
	task_join(&c1);
	task_join(&c2);
	task_join(&c3);

	return 0;
}