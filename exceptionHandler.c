#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include "kbcrt.h"
#include "rtx.h"
#include <signal.h>

void exception_handler_process(int result)
{
    // any pre-handling code
    	int setjmp(jmp_buf msg_env);
    	void longjmp (jmp_buf msg_env, int val);
    	PCB* temp;
    	PCB* savePCB = curr_process;
    	switch(result) {
    		case SIGINT: die();
    		break;
    		case SIGALRM: printf("Not Done Yet");//RTX.current_process = RTX.pid_to_PCBptr(TIMER_I_PROC);
    		//tick_handler( );
    		break;
    		case SIGUSR1: kbd_iproc();
    		break;
    		case SIGUSR2: crt_iproc();
    		break;
    	default: printf("Unknown signal!\n");
    	die();
    	exit(1);
    	break;
    }
    //RTX.current_process = savePCB;
    // any post handling code
}
