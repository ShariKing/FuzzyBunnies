#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include "rtx.h"

// ***EXCEPTION HANDLER***
void exception_handler_process(int result)
{
    
    // any pre-handling code
    
    int setjmp(jmp_buf msg_env);
    void longjmp (jmp_buf msg_env, int val);
    PCB* temp;
    PCB* savePCB = curr_process;
    
    // based on signal, do something
    switch(result) {
        
        // clean up signal
    	case SIGINT: 
            //die(); - terminate
            break;
        
        // timer signal
        case SIGALRM: 
            printf("Not Done Yet");//RTX.current_process = RTX.pid_to_PCBptr(TIMER_I_PROC);
            //call our clock function
            break;
        
        // kb i-process start signal from kb UART
        case SIGUSR1: 
            kbd_iproc();
            break;
            
        // crt i-process start signal from crt UART
        case SIGUSR2: 
            crt_iproc();
            break;
       
        // signal unknown
        default: 
            printf("Unknown signal!\n");
            //die(); - terminate
            exit(1);
            break;
    }
    
    //RTX.current_process = savePCB;
    // any post handling code
}
