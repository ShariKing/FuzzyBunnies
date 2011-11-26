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

void clock_increment(clk* clock, int system_or_wall) {
		
        //printf("%p is the pointer %d", clock, system_or_wall);
        //printf("%d is the seconds %d is the minutes %d is the hours \n", clock->ss, clock->mm, clock->hh);
        if (clock->ss < 60)
			clock->ss++;
		else
        {
			clock->ss = 0;
			if (clock->mm < 60)
				clock->mm++;
			else
            {
				clock->mm = 0;
				if(system_or_wall == 1)
                {
					if(clock->hh < 24 || clock->hh > 0)
                         clock->hh++;
                    else
                         clock->hh == 0;
                }
                else
                    clock->hh++;
			}
		}
		
        if(clock->ss < 0 || clock->ss > 60 || clock->mm < 0 ||
                     clock->mm > 60 || clock->hh < 0)
             printf("YOU BROKE TIIIIIIIIIME\n");
}

int clock_set(clk* clock, int hours, int minutes, int seconds) {
    
        if(seconds < 60 && seconds >= 0)	{
             if(minutes < 60 && minutes >= 0){
                  if(hours < 24 && hours >= 0){
                       clock->ss = seconds;
                       clock->mm = minutes;
                       clock->hh = hours;
                       return 1; //Success! ends function before hitting error message
                       }
             }
        }
        printf("Entered time invalid or in incorrect format.\n");
        return 0; //Return a zero if there is a problem with setting the clock
}

int clock_out(clk* clock, msg_env *e){
     
     char* temptime = (char *) malloc (sizeof (SIZE)); //temporary holding variable
     char* timewords = (char *) malloc (sizeof (SIZE));  //Temporary character pointer
     
     if(!temptime || !timewords)
          return 0; //Error with clock printing
          
     
     sprintf(timewords, "%d", clock->hh);
//     itoa(clock->hh, timewords, 10);            //convert and copy the values in hours into the timewords character pointer
     strcat(timewords, ":");
     sprintf(temptime, "%d", clock->mm);
//     itoa(clock->mm, temptime, 10);        //convert the values in minutes into characters and put them in a temporary holding variable
     strcat(timewords, temptime);    //concatenate the minutes into the timewords character pointer
     strcat(timewords, ":");
     sprintf(temptime, "%d", clock->ss);
//     itoa(clock->ss, temptime, 10);  //convert the values in seconds into characters and write them into the temporary holding variable
     strcat(timewords, temptime);    //concatenate the hours into timewords. Timewords should now contain the properly formatted time.
     
     //If envelope is not empty, output an error message
     //if(e->msg_text != NULL)
          //printf("Error, clock cannot output due to non-empty envelope!\n");
     //     printf("Message contains %s EOM\n", e->msg_text);
     //else
     //{
          strcpy(e->msg_text, timewords);    //put the formatted time text into the envelope
          send_console_chars(e);
          return 1; //Success!
     //}
}

int request_delay(int time_delay, int wakeup_code, msg_env *m)
{
    int RequestingPID = curr_process->pid;         //Temporary PID holder
    strcpy(curr_process->state, "SLEEP");          
    m->sender_id = RequestingPID;
    m->target_id = TIMERIPROCPID;                  //Set Target ID to the Timer Iproc
    sprintf(m->msg_type, "%d", wakeup_code);       //Set the message type to wakeup code and the text to the delay,
    sprintf(m->msg_text, "%d", time_delay);        //in order to send both wakeup code and time delay in one envelope.
    k_send_message(TIMERIPROCPID, m);              //Send the envelope to the timer iproc
    m = k_receive_message();                       //Invoke receive message, which blocks the invoking process until delay is over
    if(m)
    {
        PCB *RequestingPCB;                            //After unblocking, this code executes:
        RequestingPCB = convert_PID(RequestingPID);    //Create a PCB pointer to manage stuff for enqueueing, then Find the Delayed PCB
        PCB_ENQ(RequestingPCB, convert_priority(RequestingPCB->priority));    //Enqueue the now awakened process
        return 1;              //SUCCESS!
    }
    else
        return 0;              //The code should never get here
}
    

//===============    THE CODE BELOW THIS LINE IS WORKING, BUT DOES NOT FIT THE PROTOTYPE. KEEP AS A BACKUP =======================//
/*
int request_delay(int msecDelay) {

    //int invoketime = systemclock->ss + (systemclock->mm*60) + (systemclock->hh*60*60);   //no longer necessary
    int RequestingPID = curr_process->pid;
    strcpy(curr_process->state, "SLEEP");
    curr_process->sleeptime = msecDelay;
    //copy processor stack into process_PCB
    //release_processor();                               //context switching code, to be added later?
    PCB_ENQ(pointer_2_PCB[RequestingPID], pointer_2_SQ);
}
*/
