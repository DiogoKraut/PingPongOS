// GRR20166365 Diogo Paris Kraut

#include "queue.h"
#include <stdio.h>

int queue_size(queue_t *queue) {
	if(queue == NULL) // EDGE CASE: empty
		return 0;

	if(queue->next == NULL) // EDGE CASE: lone element (not a queue, cant have size)
		return 0;

	queue_t *aux = queue;
	int count = 1;

	while(aux->next != queue) {
		count++;
		aux = aux->next;
	}

	return count;
}

void queue_print(char *name, queue_t *queue, void print_elem(void*) ) {
	printf("%s\n", name);

	if(!queue_size(queue)) // EDGE CASE: empty
		return;

	queue_t *aux = queue;

	do {
		print_elem(aux);
		aux = aux->next;
	} while(aux != NULL && aux != queue);
}

int queue_append(queue_t **queue, queue_t *elem) {
	if(queue == NULL) {
		fprintf(stderr, "Queue doesn't exist\n");
		return -1;
	}

	if(elem->next != NULL) {
		fprintf(stderr, "Element belongs to a different queue\n");
		return -2;
	}

	if(!queue_size(*queue)) { // EDGE CASE: empty
		*queue = elem;
		(*queue)->next = *queue;
		(*queue)->prev = *queue;
		return 0;
	}

	if(queue_size(*queue) == 1) { // EDGE CASE: size 1
		(*queue)->next = elem;
		(*queue)->prev = elem;
		elem->next = *queue;
		elem->prev = *queue;
		return 0;
	}

	(*queue)->prev->next = elem;
	elem->prev = (*queue)->prev;
	elem->next = *queue;
	(*queue)->prev = elem;
	return 0;

}

int queue_remove(queue_t **queue, queue_t *elem) {
	if(queue == NULL) {
		fprintf(stderr, "Queue doesn't exist\n");
		return -1;
	}

	if(elem == NULL) {
		fprintf(stderr, "Element doesnt exist\n");
		return -3;
	}

	if(!queue_size(*queue)) {
		fprintf(stderr, "Queue must not be empty\n");
		return -4;
	}

	if(queue_size(*queue) == 1) { // EDGE CASE: size 1
		if(*queue == elem) {
			*queue = NULL;
			elem->next = NULL;
			elem->prev = NULL;
			return 0;
		}
	}

	queue_t *aux = *queue;

	do {
		if(aux == elem) {
			aux->next->prev = aux->prev;
			aux->prev->next = aux->next;

			if(aux == *queue) // if removing first element, update queue to point to second element
				*queue = (*queue)->next;

			elem->next = NULL;
			elem->prev = NULL;
			return 0;
		}
		aux = aux->next;
	} while(aux != *queue);

	// Didnt find element in queue
	fprintf(stderr, "Element belongs to a different queue\n");
	return -2;
}