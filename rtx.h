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
#include "kbcrt.h"



// *** CONSTANTS ***
#define SIZE 128
#define STACKSIZE 4096 // size of stack pointer
#define TOTAL_NUM_PROC 8 //total number of processes, will change
#define TOTAL_NUM_IPROC 3 //total number of i-processes
#define TIMERIPROCPID 2

// *** STRUCTS ***
struct msgenv {
	struct msgenv *p;	     //pointer to the next env in the queue this env resides in
	int sender_id;
	int target_id;
	char* msg_type;         //type and text are char pointers, which are essentially arrays
	char* msg_text;         //we can call a location of the pointer and it will work like an array
};
typedef struct msgenv msg_env;

struct envQ{
 msg_env* head;
 msg_env* tail;
};
typedef struct envQ env_Q;

struct pcb {
	struct pcb *p;		// pointer to next PCB in the queue that this PCB resides in
	char* state;
	int pid;
	int priority;
	void* PC;			//I'm guessing since it's a counter
	int sleeptime;
	char *SP;		
	env_Q *receive_msg_Q;
	jmp_buf pcb_buf;
};
typedef struct pcb PCB;	//use PCB

struct pcbq{
	PCB* head;
	PCB* tail;
};
typedef struct pcbq PCB_Q;

struct clock{
		int ss;
		int mm;
		int hh;
};
typedef struct clock clk;

struct messageTrace {
       int sender_id;
       int target_id;
       char* msg_type;
       clk timestamp;
};
typedef struct messageTrace msg_trace;

struct init_table {
       int pid;
       int priority;
       int stack_size;
       void* address;
};
typedef struct init_table init_table;

// *** FUNCTION DECLARATIONS ***

void k_terminate(int signal);
void cleanup();

// PROCESSES
void kbd_iproc(int sigval);
void crt_iproc(int sigval);
void timer_iproc(int sigval);

void process_switch();
void context_switch(jmp_buf previous, jmp_buf next);
void atomic_off();
void atomic_on();

// PRIMITIVES
int PCB_ENQ(PCB* r, PCB_Q* queue);
PCB* PCB_DEQ(PCB_Q* queue);
PCB *PCB_REMOVE(PCB_Q *q, int id);

int env_ENQ(msg_env* e, env_Q* queue);
msg_env* env_DEQ(env_Q *queue);
msg_env *env_REMOVE(env_Q *q, int senderid);

int free_env_queue(env_Q* Q);
int free_PCB_queue(PCB_Q* Qu);

int send_message (int dest_id, msg_env* e);
int k_send_message (int dest_id, msg_env* e);

msg_env* receive_message();
msg_env* k_receive_message();

int send_console_chars(msg_env* env);
int k_send_console_chars(msg_env* env);

int get_console_chars(msg_env* env);
int k_get_console_chars(msg_env* env);

void terminate(int signal);
//int k_terminate();

int get_trace_buffers(msg_env* env);
int k_get_trace_buffers(msg_env* env);

int change_priority (int new_priority, int target_process_id);
int k_change_priority (int new_priority, int target_process_id);

msg_env *k_request_msg_env();
msg_env *request_msg_env();

int k_release_msg_env(msg_env *env);
int release_msg_env(msg_env *env);

int request_process_status(msg_env *env);
int k_request_process_status(msg_env *env);

void k_release_processor();
void release_processor();

int request_delay(int time_delay, char* wakeup_code, msg_env *m);
int k_request_delay(int time_delay, char* wakeup_code, msg_env *m);

void ProcessA();
void ProcessB();
void ProcessC();

PCB* convert_PID(int PID);
PCB_Q* convert_priority(int pri);	

void clock_increment(clk* clock, int system_or_wall);
int clock_set(clk* clock, int hours, int minutes, int seconds);
int clock_out(clk* clock, msg_env *e);

void ClockTest(clk *clock);

// INITIALIZATION FUNCTIONS
PCB_Q* create_Q( );
env_Q* create_env_Q( );

int init_clocks();
int init_queues( );
int init_msg_trace();
int init_env( );

int init_processes( );
int init_i_processes( );

void kb_crt_start();

void begin_RTX( );


// *** VARIABLES ***
PCB* pointer_2_PCB[TOTAL_NUM_PROC + TOTAL_NUM_IPROC];	//array of pointers to processes
PCB_Q* pointer_2_RPQ[4];	//array of pointers to ready process queues
env_Q* pointer_2_SQ;	//pointer to sleep queue

PCB* curr_process;		

clk* systemclock;
clk* wallclock;     //Global Clock Variables

msg_trace send_trace[16];                   //went with arrays instead of queues
msg_trace receive_trace[16];

int send_start;                             // indeces for the arrays
int send_end;
int receive_start;
int receive_end;
int send_counter;
int receive_counter;                        //the only way I can think of setting up circular array
int Atom;

// QUEUES
PCB_Q* ready_q_priority0;
PCB_Q* ready_q_priority1;
PCB_Q* ready_q_priority2;
PCB_Q* ready_q_priority3;
PCB_Q* blocked_on_envelope;
env_Q* sleep_Q;
env_Q* envelope_q;

#endif
