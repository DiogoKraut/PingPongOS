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
	SUSPENDED,   // waiting to join
	SLEEP     //  sleep
} Status;

// Estrutura que define um Task Control Block (TCB)
typedef struct task_t
{
	struct task_t *prev, *next ;	// pointers for queue
	int id ;						// task id
	ucontext_t context ;			// task context
	Status status;					// current status
	short static_prio;				// static priority
	short prio;						// current priority
	short is_system_task;			// system task indicator
	unsigned int quantum_size;		// tasks quantum
	unsigned int init_time;			// time of creation
	unsigned int tick_count;		// time spent in the CPU
	unsigned int activation_count;	// times activated
	struct task_t *waitQ;			// queue of tasks waiting to join
	int exitCode;
	unsigned int wake_time;			// time to sleep + systime when sleep is called

	// ... (outros campos serão adicionados mais tarde)
} task_t ;

extern task_t *currentTask, *rdyQ, dispatcherTask;

// estrutura que define um semáforo
typedef struct
{
  int val;
  int lock;
  short destroyed;
  task_t *waitQ;

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

