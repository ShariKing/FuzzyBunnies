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
    
    if(ready_q_priority0 != NULL) // If highest priority queue isn't empty
        new_pcb = PCB_DEQ(ready_q_priority0); //get ptr to highest priority ready process
    else if (ready_q_priority1 != NULL)
         new_pcb = PCB_DEQ(ready_q_priority1); //get ptr to highest priority ready process
    else if (ready_q_priority2 != NULL)
         new_pcb = PCB_DEQ(ready_q_priority2); //get ptr to highest priority ready process
    else 
         new_pcb = PCB_DEQ(ready_q_priority3); //only for null process
    
    if (old_pcb->state == "RUNNING"){
        strcpy(old_pcb->state, "READY"); //set old proc state to ready
    }
    
    strcpy(new_pcb->state, "RUNNING"); //set new proc state to running
    
    curr_process = new_pcb; //make the next_pcb the current process
    context_switch( old_pcb->pcb_buf, new_pcb->pcb_buf );
    atomic_off();
}

// context_switch() - performs the context switch between two user processes
// Once we call longjmp(), we resume executing the "next" process
// at the point where its context was last saved with a setjmp(). */

void context_switch(jmp_buf previous, jmp_buf next) {
    printf("You're in context_switch\n");
     int return_code = setjmp(previous); 
     if (return_code == 0) {
        longjmp(next,1);
     }
}




