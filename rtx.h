#ifndef RTX_H
#define RTX_H

#define SIZE 20
#define NO_PROC 12		//Number of processes we have. Will change when I know how many.

struct msgenv {
	struct msgenv *p;	//kernel pointer
	int sender_id;
	int target_id;
	char msg_type[SIZE];
	char msg_text[SIZE];
};
typedef struct msgenv msg_env;	//use msg_env

typedef struct{
	msg_env *head;
	msg_env *tail;
} env_Q;

struct pcb {
	struct pcb *p;		//kernel pointer
	char state[SIZE];
	int pid;
	int priority;
	int PC;			//I'm guessing since it's a counter
	char *SP;		//checked the pointer type with Saad and Kraemer
	env_Q receive_msg_Q;
};
typedef struct pcb PCB;	//use PCB

typedef struct{
	PCB *head;
	PCB *tail;
} PCB_Q;

PCB *pointer_2_PCB[NO_PROC];	//array of pointers to processes
PCB_Q *pointer_2_RPQ[4];	//array of pointers to ready process queues
PCB *curr_process;		//I'm guessing we need this to track the current process

int PCB_ENQ(PCB *r, PCB_Q queue);
PCB *PCB_DEQ(PCB_Q queue);
int env_ENQ(msg_env *e, env_Q queue);
msg_env *env_DEQ(env_Q queue);
int send_message ( int dest_id, msg_env *e);
msg_env *receive_message();
PCB *convert_PID(int PID);
PCB_Q *convert_priority(int pri);	
PCB_Q* create_Q( );
env_Q* create_env_Q( );
void init_queues( );
void init_processes( );
void init_env( );
void init_i_processes( );
void begin_RTX( );

#endif
