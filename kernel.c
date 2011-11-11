#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include "kbcrt.h"
#include "rtx.h"
#define CRT_iproc_pid

int PCB_ENQ(PCB *r, PCB_Q queue){
	if (queue->head==NULL){								//if queue is empty
		queue->head = queue->tail = r;
		return 0;
	}
	queue->tail->p = r;
	r->p = NULL;
	queue->tail = r; 	
	return 0;
}

PCB *PCB_DEQ(PCB_Q queue){
	if (queue->head==NULL){								//if queue is empty
		printf("Queue is empty");
		return NULL;
	}
	PCB *t;
	t = queue->head;
	queue->head = t->p;
	if (t->p==NULL)									//if only one PCB in queue 
		queue->tail = NULL;
	return t;
}

int env_ENQ(msg_env *e, env_Q queue){
	if (queue->head==NULL){								//if queue is empty
		queue->head = queue->tail = e;
		return 0;
	}
	queue->tail->p = e;
	e->p = NULL;
	queue->tail = e; 	
	return 0;
}

msg_env *env_DEQ(env_Q queue){
	if (queue->head==NULL){								//if queue is empty
		printf("Queue is empty");
		return NULL;
	}
	msg_env *t;
	t = queue->head;
	queue->head = t->p;
	if (t->p==NULL)									//if only one PCB in queue 
		queue->tail = NULL;
	return t;
}

int send_message ( int dest_id, msg_env *e){
	if(dest_id>NO_PROC-1){								//PCB id is not found
		printf("Invalid ID");
		return 0;
	}
	e->target_id = dest->id;
	PCB *target = convert_PID(dest_id);
	if(target->receive_msg_Q->head==NULL && target->priority !=-1){			//if queue is empty and not an i-process
		PCB_ENQ(target, convert_priority(target->priority));			//not sure if need to put & before convert_priority
		strcopy(target->state,"READY");						//apparently strcopy is how you write into an array of chars
	env_ENQ(e,target->receive_msg_Q);
	return 0;
}

msg_env *receive_message(){								//Doesn't take the PCB as a parameter. Dealt with using curr_process
	if(curr_process->receive_msg_Q->head==NULL){					//if receive_msg_Q is empty{
		if(curr_prcoess->priority==-1 || curr_process->state=="NO_BLK_RCV")	//if it's i-process or has the wierd state
			return NULL;
		strcopy(curr_process->state,"BLK_ON_RCV")				//Doesn't need to be put in a queue, and don't care about process switch now
	}
	return env_DEQ(curr_process->receive_msg_Q);
}

int send_console_chars(msg_env *env) {
	
	int Z;
	Z = send_message(env->target_id, env);						//write string to env
	if(Z=1)										// incorrect return from send_msg
		printf('Error with sending');
	kill(0,SIGUSR2);
	return 0;
}

PCB *convert_PID(int PID){
	if(PID>NO_PROC-1 || PID<0){
		printf("invalid ID");
		return NULL;
	}
	return pointer_2_PCB[PID];
}

PCB_Q *convert_priority(int pri){							//used pri cuz priority is defined in PCB struct
	if(pri>3 || pri<0){
		printf("Invalid priority");
		return NULL;
	}
	return pointer_2_RPQ[pri];
}

