// GRR20166365 Diogo Paris Kraut

#include "queue.h"
#include <stdio.h>

int queue_size(queue_t *queue) {
	if(queue == NULL) // EDGE CASE: empty
		return 0;

	if(queue->next == NULL)
		return 1;

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
		#ifdef DEBUG
		fprintf(stderr, "APPEND: Queue doesn't exist\n");
		#endif
		return -1;
	}

	if(elem->next != NULL) {
		#ifdef DEBUG
		fprintf(stderr, "APPEND:Element belongs to a different queue\n");
		#endif
		return -2;
	}

	if(!queue_size(*queue)) { // EDGE CASE: empty
		*queue = elem;
		(*queue)->next = *queue;
		(*queue)->prev = *queue;
		return 0;
	}

	(*queue)->prev->next = elem;
	elem->prev = (*queue)->prev;
	elem->next = *queue;
	(*queue)->prev = elem;
	return 0;

}

int queue_remove(queue_t **queue, queue_t *elem) {
	if(queue_size(*queue) == 0) {
		#ifdef DEBUG
		fprintf(stderr, "REMOVE:Queue is empty\n");
		#endif
		return -1;
	}

	if(elem == NULL) {
		#ifdef DEBUG
		fprintf(stderr, "REMOVE: Element doesnt exist\n");
		#endif
		return -3;
	}

	if(queue_size(*queue) == 1) { // EDGE CASE: size 1
		if(*queue == elem) {
			*queue = NULL;
			queue = NULL;
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
	fprintf(stderr, "REMOVE: Element belongs to a different queue\n");
	return -2;
}