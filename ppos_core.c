// GRR20166365 Diogo Paris Kraut

#include <stdio.h>
#include <stdlib.h>
#include "ppos.h"
#include "ppos_data.h"

int id; 			 // id tracker to guarantee unique task IDs
int taskCount;
task_t contextMain;  // main() task descriptor
task_t *currentTask;
task_t *rdyQ;
task_t dispatcherTask;

void dispatcher();
task_t *scheduler();

void ppos_init() {
	#ifdef DEBUG
	printf("INIT: PPOS intialized\n");
	#endif
	/* desativa o buffer da saida padrao (stdout), usado pela função printf */
	setvbuf (stdout, 0, _IONBF, 0);

	id = 0;
	taskCount = 0;
	contextMain.id = id; // set main id to 0
	currentTask = &contextMain; // main is initial task

	task_create(&dispatcherTask, dispatcher, NULL);

}

task_t *scheduler() {
	if(queue_size((queue_t *)rdyQ) == 0)
		return NULL;

	task_t *aux = rdyQ->next;
	task_t *highestPrio = rdyQ;
	#ifdef DEBUG
	printf("SCHED: %02d\n", rdyQ->prio);
	#endif
	while(aux != rdyQ) {
		#ifdef DEBUG
		printf("SCHED: %02d\n", aux->prio);
		#endif
		if(aux->prio <= highestPrio->prio) // smaller prio value == higher priority
			highestPrio = aux;

		aux = aux->next;
	}

	aux = highestPrio->next;
	while(aux != highestPrio) {
		aux->prio--;
		aux = aux->next;
	}

	highestPrio->prio = task_getprio(highestPrio);
	// #ifdef DEBUG
	// printf("SCHED: Selected task %02d with prio %02d\n", highestPrio->id, highestPrio->prio);
	// #endif
	return highestPrio;
}

void dispatcher() {
	task_t *next;
	while(taskCount > 0) {
		next = scheduler();

		if(next != NULL) {
			#ifdef DEBUG
			task_t *aux = rdyQ->next;
			printf("RDY QUEUE: [%d", rdyQ->id);

			while(aux != rdyQ) {
				printf(" %d", aux->id);
				aux = aux->next;
			}
			printf("]\n");
			#endif


			queue_remove((queue_t **)&rdyQ, (queue_t *)next);
			next->status = RUNNING;

			task_switch(next);

			switch(next->status) {
				case READY:
					queue_append((queue_t**)&rdyQ, (queue_t *)next);
					break;

				case RUNNING:
					/* Something went terribly wrong*/
					break;

				case STOPPED:
					free(next->context.uc_stack.ss_sp);
					break;

				case SLEEP:
					/* add to sleep Q */
					break;
				case IDLE:
					/* add to idle Q */
					break;
			}
		} 
	}

	#ifdef DEBUG
	printf("DISPATCHER: switching to task %02d\n", next->id);
	#endif
	task_switch(&contextMain);
}

void task_setprio(task_t *task, int prio) {
	if(prio > 20)
		prio = 20;
	else if(prio < -20)
		prio = -20;

	if(task) {
		task->static_prio = prio;
		task->prio = prio;
	}
	else {
		currentTask->static_prio = prio;
		currentTask->prio = prio;
	}

}

int task_getprio(task_t *task) {
	if(task)
		return task->static_prio;
	return currentTask->static_prio;
}

void task_yield() {

	#ifdef DEBUG
	printf("YIELD: task %02d yielded\n", currentTask->id);
	#endif

	currentTask->status = READY;
	task_switch(&dispatcherTask);
}


int task_create(task_t *task, void (*start_routine)(void *),  void *arg) {
	char *stack;

	getcontext(&task->context);

	stack = malloc (STACKSIZE);
	if(stack) {
		task->context.uc_stack.ss_sp = stack;
		task->context.uc_stack.ss_size = STACKSIZE;
		task->context.uc_stack.ss_flags = 0;
		task->context.uc_link = 0;

		task->prev = NULL;
		task->next = NULL;
		task->id = ++id;
		task->prio = 0;
		task->static_prio = 0;

		makecontext(&task->context, (void *)start_routine, 1, arg);

		#ifdef DEBUG
		printf("CREATE: Task %02d created \n", task->id);
		#endif

		if(task != &dispatcherTask) { // dispatcher cant go on ready Q
			taskCount++;
			task->status = READY;
			queue_append((queue_t **)&rdyQ, (queue_t *)task);
		} else
			task->status = RUNNING;

		return task->id;
	}
	
	perror("ERR: Failed creating stack");

	return -1;
}

int task_switch(task_t *task) {
	/* Update current task */
	task_t *aux = currentTask;
	currentTask = task;

	/* Swap context */
	#ifdef DEBUG
	printf("SWITCH: swapping context : %02d to %02d\n", aux->id, task->id);
	#endif
	if(swapcontext(&aux->context, &task->context) >= 0) {
		return 0;
	}

	perror("ERR: Failed to swap contexts");
	return -1;
}

void task_exit(int exitCode) {
	#ifdef DEBUG
	printf("EXIT: Exiting task %02d\n", currentTask->id);
	#endif

	if(currentTask != &dispatcherTask) {
		taskCount--;
		currentTask->status = STOPPED;
		task_switch(&dispatcherTask);
	} else {
		free(currentTask->context.uc_stack.ss_sp);
		task_switch(&contextMain);
	}

}

int task_id() {
	return currentTask->id;
}