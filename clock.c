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
        printf("%d is the seconds %d is the minutes %d is the hours \n", clock->ss, clock->mm, clock->hh);
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
     
     char* temptime = (char *) malloc (sizeof (char)); //temporary holding variable
     char* timewords = (char *) malloc (sizeof (char));  //Temporary character pointer
     
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


//===================TO BE FINISHED, COMPLETED PART UP TO DOUBLE COMMENT================//
//-------NOTE! Sleep queue added in init.c and pointer_2_SQ added to rtx.h!!!----------//


/*int request_delay(int secDelay, int wakeup_code, msg_env *e) {

    int invoketime = systemclock->ss + (systemclock->mm*60) + (systemclock->hh*60);
    int RequestingPID = curr_process->pid;
    strcpy(curr_process->state, "SLEEP");
    //copy processor stack into process_PCB
    //release_processor();                               ///context switching code, to be added later?
    PCB_ENQ(pointer_2_PCB[RequestingPID], pointer_2_SQ);
*//*	set sender_procid, destination_procid (timeout_i_process_pid), and message(wakeup_code) fields in message_envelope
	send_message(timeout_i_process_pid, message_envelope)
	message_envelope = receive_message( )
	if(message_envelope->message = wakeup_code)
		end request_delay, place in ready queue
	else print error message (since all pids are known, the code should never get here. This is for debug purposes only)*//*
}*/

