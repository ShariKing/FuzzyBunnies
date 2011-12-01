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
#include "rtx.h"

void null_process() {
    //printf("You're in null_process\n");
		while(1) {
            usleep(100000);
            //printf("IN NULLLLLLLLLL!");      
			release_processor();
		}
}

void process_switch() {
    atomic(ON);
    //printf("You're in process_switch\n");
    //printf("proc switch one, curr proc = %d\n", curr_process->pid); 
    PCB* new_pcb;
    //printf("curr proc %p %d %d\n", curr_process, curr_process->state, curr_process->priority);
    //printf("point 2 pcb %p\n", pointer_2_PCB);
    PCB* old_pcb;
    old_pcb = pointer_2_PCB[curr_process->pid];
    //printf("proc switch two old = %d\n", old_pcb->pid); 
    //printf("%d\n",curr_process->pid);
    if(ready_q_priority0->head != NULL) {// If highest priority queue isn't empty
        new_pcb = PCB_DEQ(ready_q_priority0); //get ptr to highest priority ready process
        //printf("q0\n");
        }
    else if (ready_q_priority1->head != NULL){
         new_pcb = PCB_DEQ(ready_q_priority1); //get ptr to highest priority ready process
         //printf("q1\n");
        }
    else if (ready_q_priority2->head != NULL){
         new_pcb = PCB_DEQ(ready_q_priority2); //get ptr to highest priority ready process
         //printf("q2\n");
        }
    else if (ready_q_priority3->head != NULL){
         new_pcb = PCB_DEQ(ready_q_priority3); //only for null process
         //printf("q3\n");
        }
    else {
        //printf("All queues are empty\n");
        return;
        }
    if (old_pcb->state == RUNNING){
        old_pcb->state = READY; //set old proc state to ready
        //printf("proc switch three old = %d, new = %d\n", old_pcb->pid, new_pcb->pid); 
    
    }
    
    new_pcb->state = RUNNING; //set new proc state to running  
    curr_process = pointer_2_PCB[new_pcb->pid]; //make the next_pcb the current process
    atomic(OFF);
    //printf("proc switch seven old = %d, new = %d\n", old_pcb->pid, new_pcb->pid); 
    //printf("old uc %d new uc %p\n", &old_pcb->uc, &new_pcb->uc);
    
    swapcontext(&old_pcb->uc, &new_pcb->uc);
    //context_switch( old_pcb->pcb_buf, new_pcb->pcb_buf );
    //printf("after context curr is %d\n",curr_process->pid);
    
    atomic(OFF);
}

// context_switch() - performs the context switch between two user processes
// Once we call longjmp(), we resume executing the "next" process
// at the point where its context was last saved with a setjmp(). */

void context_switch(jmp_buf previous, jmp_buf next) {
    //printf("You're in context_switch\n");
    // int return_code = setjmp(previous);
    printf("cont switch one\n");
     if (setjmp(previous) == 0) {
         printf("cont switch two\n");
        //printf("Context of prev saved, switching to next\n");
        longjmp(next,1);
        printf("cont switch three\n");
     }

         
}




