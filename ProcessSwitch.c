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
    printf("You're in null_process\n");
		while(1) {
			release_processor();
		}
}

void process_switch() {
    atomic_on();
    printf("You're in process_switch\n");
    PCB* new_pcb;
    PCB* old_pcb = curr_process;
    //printf("%d\n",curr_process->pid);
    if(ready_q_priority0->head != NULL) {// If highest priority queue isn't empty
        new_pcb = PCB_DEQ(ready_q_priority0); //get ptr to highest priority ready process
        //printf("q0");
        }
    else if (ready_q_priority1->head != NULL){
         new_pcb = PCB_DEQ(ready_q_priority1); //get ptr to highest priority ready process
        // printf("q1");
        }
    else if (ready_q_priority2->head != NULL){
         new_pcb = PCB_DEQ(ready_q_priority2); //get ptr to highest priority ready process
        // printf("q2");
        }
    else if (ready_q_priority3->head != NULL){
         new_pcb = PCB_DEQ(ready_q_priority3); //only for null process
         //printf("q3");
        }
    else {
        printf("EMPTY\n");
        return;
        }
    if (old_pcb->state == "RUNNING"){
        strcpy(old_pcb->state, "READY"); //set old proc state to ready
    }
    
    strcpy(new_pcb->state, "RUNNING"); //set new proc state to running
    //printf("%d\n",curr_process->pid);   
    curr_process = new_pcb; //make the next_pcb the current process
    printf("%d\n",old_pcb->pcb_buf);
    printf("%d\n",new_pcb->pcb_buf);
    context_switch( old_pcb->pcb_buf, new_pcb->pcb_buf );
    //printf("%d\n",curr_process->pid);
    atomic_off();
}

// context_switch() - performs the context switch between two user processes
// Once we call longjmp(), we resume executing the "next" process
// at the point where its context was last saved with a setjmp(). */

void context_switch(jmp_buf previous, jmp_buf next) {
    printf("You're in context_switch\n");
    // int return_code = setjmp(previous);
     if (setjmp(previous) == 0) {
        printf("MAYBE HERE?????");
        longjmp(next,1);
     }
     else{ 
         printf("WHATS HERE?");                                   
        // curr_process = new_pcb; // sets the new pcb to be the current process
         void (*fpTmp)();
         (fpTmp) = (void *)curr_process->PC; //gets address of process code
         fpTmp(); 
         }
}




