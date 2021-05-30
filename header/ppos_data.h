// GRR20166365 Diogo Paris Kraut

// PingPongOS - PingPong Operating System
// Prof. Carlos A. Maziero, DINF UFPR
// Versão 1.1 -- Julho de 2016

// Estruturas de dados internas do sistema operacional

#ifndef __PPOS_DATA__
#define __PPOS_DATA__

#include <ucontext.h>		// biblioteca POSIX de trocas de contexto
#include "queue.h"		    // biblioteca de filas genéricas

#define STACKSIZE 64*1024	/* tamanho de pilha das threads */
#define QUANTUM_SIZE 20		// default quantum size in ticks
#define TRUE 1
#define FALSE 0

typedef enum {
	READY,   // rdy to run
	RUNNING, // currently running
	STOPPED, // reached end of execution
	SLEEP,   // waiting for IO
	IDLE     // interruptable sleep
} Status;

// Estrutura que define um Task Control Block (TCB)
typedef struct task_t
{
	struct task_t *prev, *next ;	// ponteiros para usar em filas
	int id ;						// identificador da tarefa
	ucontext_t context ;			// contexto armazenado da tarefa
	Status status;				// current status
	char static_prio;			// static priority
	char prio;					// current priority
	char is_system_task;
	short quantum_size;
	// ... (outros campos serão adicionados mais tarde)
} task_t ;

// estrutura que define um semáforo
typedef struct
{
  // preencher quando necessário
} semaphore_t ;

// estrutura que define um mutex
typedef struct
{
  // preencher quando necessário
} mutex_t ;

// estrutura que define uma barreira
typedef struct
{
  // preencher quando necessário
} barrier_t ;

// estrutura que define uma fila de mensagens
typedef struct
{
  // preencher quando necessário
} mqueue_t ;

#endif

