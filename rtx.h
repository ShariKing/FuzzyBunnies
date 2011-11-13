#ifndef RTX_H
#define RTX_H

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>

//*** CONSTANTS ***
#define SIZE 20
#define NUM_PROC 12		//Number of processes we have. Will change when I know how many.

// *** STRUCTS ***
struct msgenv {
	struct msgenv *p;	//pointer to the next env in the queue this env resides in
	int sender_id;
	int target_id;
	char msg_type[SIZE];
	char msg_text[SIZE];
};
typedef struct msgenv msg_env;

struct envQ{
 msg_env *head;
 msg_env *tail;
};
typedef struct envQ env_Q;

struct pcb {
	struct pcb *p;		// pointer to next PCB in the queue that this PCB resides in
	char state[SIZE];
	int pid;
	int priority;
	int PC;			//I'm guessing since it's a counter
	char *SP;		
	env_Q *receive_msg_Q;
};
typedef struct pcb PCB;	//use PCB

struct pcbq{
	PCB *head;
	PCB *tail;
};
typedef struct pcbq PCB_Q;


// *** FUNCTION DECLARATIONS ***

// PRIMITIVES
int PCB_ENQ(PCB *r, PCB_Q *queue);
PCB *PCB_DEQ(PCB_Q *queue);

int env_ENQ(msg_env *e, env_Q *queue);
msg_env *env_DEQ(env_Q *queue);

int send_message ( int dest_id, msg_env *e);
msg_env *receive_message();

int send_console_chars(msg_env *env);
int get_console_chars(msg_env *env);

void processP();

PCB *convert_PID(int PID);
PCB_Q *convert_priority(int pri);	

// INITIALIZATION FUNCTIONS
PCB_Q* create_Q( );
env_Q* create_env_Q( );

int init_queues( );
int init_env( );

int init_processes( );
int init_i_processes( );

void begin_RTX( );


// *** VARIABLES ***
PCB *pointer_2_PCB[NUM_PROC];	//array of pointers to processes
PCB_Q *pointer_2_RPQ[4];	//array of pointers to ready process queues

PCB *curr_process;		//I'm guessing we need this to track the current process

// QUEUES
PCB_Q* ready_q_priority0;
PCB_Q* ready_q_priority1;
PCB_Q* ready_q_priority2;
PCB_Q* ready_q_priority3;

env_Q* envelope_q;

env_Q* blocked_on_envelope;

#endif
