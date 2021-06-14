// GRR20166365 Diogo Paris Kraut

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/time.h>
#include "ppos.h"
#include "ppos_data.h"

unsigned int sys_ticks;
int id;              // id tracker to guarantee unique task IDs
int taskCount;
task_t mainTask;  // main() task descriptor
task_t *currentTask;
task_t *rdyQ;
task_t dispatcherTask;

// structures to assist with preemtion handling
struct sigaction action ;
struct itimerval timer ;

void dispatcher();
task_t *scheduler();
void alarm_handler(int signum);
unsigned int systime();

void ppos_init() {
    #ifdef DEBUG
    printf("INIT: PPOS intialized\n");
    #endif
    /* desativa o buffer da saida padrao (stdout), usado pela função printf */
    setvbuf (stdout, 0, _IONBF, 0);

    /* Alarm signal setup */
    action.sa_handler = alarm_handler;
    sigemptyset (&action.sa_mask);
    action.sa_flags = 0;
    if (sigaction (SIGALRM, &action, 0) < 0) {
        perror ("SIGACTION ERR: ");
        exit (1);
    }

    id = 0;
    taskCount = 0;

    task_create(&mainTask, NULL, NULL);

    // mainTask.id = id; // set main id to 0
    currentTask = &mainTask; // main is initial task

    task_create(&dispatcherTask, dispatcher, NULL);


    /* Timer setup */
    sys_ticks = 0; // set tick counter
    // ajusta valores do temporizador
    timer.it_value.tv_usec = 1000;    // primeiro disparo, em micro-segundos
    timer.it_value.tv_sec  = 0;       // primeiro disparo, em segundos
    timer.it_interval.tv_usec = 1000; // disparos subsequentes, em micro-segundos
    timer.it_interval.tv_sec  = 0;    // disparos subsequentes, em segundos

    // arma o temporizador ITIMER_REAL (vide man setitimer)
    if (setitimer (ITIMER_REAL, &timer, 0) < 0) {
        perror ("SETITIMER ERR: ");
        exit (1);
    }

    task_switch(&dispatcherTask);


}

unsigned int systime() {
    return sys_ticks;
}

task_t *scheduler() {
    if(queue_size((queue_t *)rdyQ) == 0)
        return NULL;

    task_t *aux = rdyQ->next;
    task_t *highestPrio = rdyQ;
    #ifdef DEBUG
    // printf("SCHED: %02d\n", rdyQ->prio);
    #endif
    while(aux != rdyQ) {
        #ifdef DEBUG
        // printf("SCHED: %02d\n", aux->prio);
        #endif
        if(aux->prio < highestPrio->prio) // smaller prio value == higher priority
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

void alarm_handler(int signum) {
    sys_ticks++;
    currentTask->tick_count++;

    if(currentTask->is_system_task)
        return;

    if(currentTask->quantum_size == 0) {
        // #ifdef DEBUG
        // printf("ALARM: task %02d yielded\n", currentTask->id);
        // #endif
        task_yield();
    } else {
        currentTask->quantum_size--;
    }
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
            next->quantum_size = QUANTUM_SIZE-1; // interval of size QUANTUM_SIZE is [0..QUANTUM_SIZE-1]

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
    task_exit(0);
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

    // #ifdef DEBUG
    // printf("YIELD: task %02d yielded\n", currentTask->id);
    // #endif

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
        task->id = id++;
        task->prio = 0;
        task->static_prio = 0;
        task->init_time = systime();
        task->tick_count = 0;
        task->activation_count = 0;

        makecontext(&task->context, (void *)start_routine, 1, arg);

        #ifdef DEBUG
        printf("CREATE: Task %02d created \n", task->id);
        #endif

        if(task != &dispatcherTask) { // dispatcher cant go on ready Q
            taskCount++;
            task->status = READY;
            task->is_system_task = FALSE;
            queue_append((queue_t **)&rdyQ, (queue_t *)task);
        } else {
            task->status = RUNNING;
            task->is_system_task = TRUE;
        }


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

    task->activation_count++;
    if(swapcontext(&aux->context, &task->context) >= 0) {
        return 0;
    }

    perror("ERR: Failed to swap contexts");
    return -1;
}

void task_exit(int exitCode) {
    // #ifdef DEBUG
    // printf("EXIT: Exiting task %02d\n", currentTask->id);
    // #endif

    printf("Task %d exit: execution time %5u ms, processor time %4u ms, %3u activations\n", 
    currentTask->id, systime() - currentTask->init_time, currentTask->tick_count, currentTask->activation_count);

    if(currentTask != &dispatcherTask) {
        taskCount--;
        currentTask->status = STOPPED;
        task_switch(&dispatcherTask);
    } else {
        free(currentTask->context.uc_stack.ss_sp);
        task_switch(&mainTask);
    }

}

int task_id() {
    return currentTask->id;
}