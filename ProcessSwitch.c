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

void null process() {
		while(1) {
			release_processor();
		}
}

process_switch() { 
    if(ready_Q_priority0 != NULL) // If highest priority queue isn't empty
        new_pcb = PCB_DEQ(ready_Q_priority0); //get ptr to highest priority ready process
    else if (ready_Q_priority1 != NULL)
         new_pcb = PCB_DEQ(ready_Q_priority1); //get ptr to highest priority ready process
    else if (ready_Q_priority2 != NULL)
         new_pcb = PCB_DEQ(ready_Q_priority2); //get ptr to highest priority ready process
    else if (ready_Q_priority3 != NULL)
         new_pcb = PCB_DEQ(ready_Q_priority3); //get ptr to highest priority ready process
    else 
         new_pcb = PCB_DEQ(ready_Q_priority4); //only for null process
    strcpy(new_pcb->state, "RUNNING"); //set new proc state to running
    strcpy(old_pcb->state, "READY"); //set old proc state to ready
    old_pcb = curr_process;
    curr_process = new_pcb; //make the next_pcb the current process
    context_switch( old_pcb->jmp_buf, new_pcb->jmp_buf );
}

// context_switch() - performs the context switch between two user processes
// Once we call longjmp(), we resume executing the "next" process
// at the point where its context was last saved with a setjmp(). */

void context_switch(jmp_buf * previous, jmp_buf * next) {
     return_code = setjmp(*previous); 
     if (return_code == 0) {
        longjmp(*next,1);
     }
}




