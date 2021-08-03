#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include "ppos_disk.h"
#include "disk.h"
#include "ppos.h"

struct sigaction sig;
disk_t disk;
task_t managerTask;


void diskDriverBody (void * args) {
    while (1) {
        #ifdef DEBUG
        printf("Disk Driver call\n");
        #endif
        // obtém o semáforo de acesso ao disco
        sem_down(&disk.disk_s);
        // se foi acordado devido a um sinal do disco
        if (disk.sig_sent) {
            #ifdef DEBUG
            printf("Sig sent\n");
            #endif
        // acorda a tarefa cujo pedido foi atendido
            queue_remove((queue_t **)&suspendedQ, (queue_t *)disk.currentReq->requester);
            if(queue_append((queue_t **)&rdyQ, (queue_t *)disk.currentReq->requester) == 0){
                #ifdef DEBUG
                printf("Task %d was serviced\n", disk.currentReq->requester->id);
                #endif
            }
            free(disk.currentReq);
            disk.sig_sent = 0;
        }

        // se o disco estiver livre e houver pedidos de E/S na fila
        if (disk_cmd(DISK_CMD_STATUS, 0, 0) == DISK_STATUS_IDLE && disk.reqQ != NULL) {
        // escolhe na fila o pedido a ser atendido, usando FCFS
            disk.currentReq = disk.reqQ;
            queue_remove((queue_t **)&disk.reqQ, (queue_t *)disk.reqQ);
        // solicita ao disco a operação de E/S, usando disk_cmd()
            if(disk.currentReq->type == READ) {
                #ifdef DEBUG
                printf("READING BLOCK %d\n", disk.currentReq->block);
                #endif
                disk_cmd(DISK_CMD_READ, disk.currentReq->block, disk.currentReq->buffer);
            } else {
                #ifdef DEBUG
                printf("WRITING BLOCK %d\n", disk.currentReq->block);
                #endif
                disk_cmd(DISK_CMD_WRITE, disk.currentReq->block, disk.currentReq->buffer);
            }
        }

        // libera o semáforo de acesso ao disco
        sem_up(&disk.disk_s);
        // suspende a tarefa corrente (retorna ao dispatcher)
        task_yield();
    }
}

void signal_handler() {
    #ifdef DEBUG
    printf("SIG HANDLER: sig\n");
    #endif
    // sem_down(&disk.disk_s);
    disk.sig_sent = 1;
    
    if(managerTask.next == NULL)
        queue_append((queue_t **)rdyQ, (queue_t *)&managerTask);

    // sem_up(&disk.disk_s);

}

int disk_mgr_init (int *num_blocks, int *block_size) {
    if(disk_cmd(DISK_CMD_INIT, 0, 0) == -1)
        return -1;

    *num_blocks = disk_cmd(DISK_CMD_DISKSIZE, 0, 0);
    *block_size = disk_cmd(DISK_CMD_BLOCKSIZE, 0, 0);

    sig.sa_handler = signal_handler;
    sigemptyset (&sig.sa_mask);
    sig.sa_flags = 0;
    if (sigaction (SIGUSR1, &sig, 0) < 0) {
        perror ("SIGACTION ERR: ");
        exit (1);
    }

    sem_create(&disk.disk_s, 1);
    task_create(&managerTask, (void *)diskDriverBody, NULL);
    taskCount--;
    managerTask.is_system_task = TRUE;

    return 0;
}

disk_request_t *disk_request_init(int block, void *buffer) {
    disk_request_t *dr = malloc(sizeof(disk_request_t));
    dr->next = NULL;
    dr->prev = NULL;
    dr->requester = currentTask;
    dr->block = block;
    dr->buffer = buffer;



    return dr;
}

int disk_block_write (int block, void *buffer) {
    #ifdef DEBUG
    printf("Write request for block %d\n", block);
    #endif
    // obtém o semáforo de acesso ao disco
    sem_down(&disk.disk_s);
    // inclui o pedido na fila_disco
    disk_request_t *req = disk_request_init(block, buffer);
    req->type = WRITE;
    if(queue_append((queue_t **)&disk.reqQ, (queue_t *)req) == 0) {
        #ifdef DEBUG
        printf("@Write block %d added to reqQ\n", block);
        #endif
    }

    // acorda o gerente de disco (põe ele na fila de prontas)
    // if append isnt successful, manager was awake
    if(managerTask.next == NULL)
        queue_append((queue_t **)rdyQ, (queue_t *)&managerTask);
 
    // libera semáforo de acesso ao disco
    sem_up(&disk.disk_s);
    // suspende a tarefa corrente (retorna ao dispatcher)
    currentTask->status = SUSPENDED;
    queue_append((queue_t **)suspendedQ, (queue_t *)currentTask);
    task_switch(&dispatcherTask);
    return 0;
}

int disk_block_read (int block, void *buffer) {
    #ifdef DEBUG
    printf("Read request of block %d\n", block);
    #endif
    // obtém o semáforo de acesso ao disco
    sem_down(&disk.disk_s);
    // inclui o pedido na fila_disco
    disk_request_t *req = disk_request_init(block, buffer);
    req->type = READ;
    if(queue_append((queue_t **)&disk.reqQ, (queue_t *)req) == 0) {
        #ifdef DEBUG
        printf("@Read block %d added to reqQ\n", block);
        #endif
    }

    // acorda o gerente de disco (põe ele na fila de prontas)
    // if append isnt successful, manager is waiting
    if(managerTask.next == NULL)
        queue_append((queue_t **)rdyQ, (queue_t *)&managerTask);
 
    // libera semáforo de acesso ao disco
    sem_up(&disk.disk_s);
    // suspende a tarefa corrente (retorna ao dispatcher)
    currentTask->status = SUSPENDED;
    queue_append((queue_t **)&suspendedQ, (queue_t *)currentTask);
    task_switch(&dispatcherTask);

    return 0;
}