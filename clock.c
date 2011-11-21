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

void clock_increment(clock* clock, bool system_or_wall) {
		
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
                         clock.hh == 0;
                }
                else
                    clock.hh++
			}
		}
		
        if(clock->ss < 0 || clock->ss > 60 || clock->mm < 0 ||
                     clock->mm > 60 || clock->hh < 0)
             printf("YOU BROKE TIIIIIIIIIME\n");
}

int clock_set(clock* clock, int hours, int minutes, int seconds) {
    
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

int clock_out(clock* clock, msg_env *e){
     
     char* temptime /*<----temporary holding variable*/, timewords;  //Temporary character pointer
     
     itoa(clock->hh, timewords, 10);            //convert and copy the values in hours into the timewords character pointer
     strcat(timewords, ":");
     itoa(clock->mm, temptime, 10);        //convert the values in minutes into characters and put them in a temporary holding variable
     strcat(timewords, temptime);    //concatenate the minutes into the timewords character pointer
     strcat(timewords, ":");
     itoa(clock->ss, temptime, 10);  //convert the values in seconds into characters and write them into the temporary holding variable
     strcat(timewords, temptime);    //concatenate the hours into timewords. Timewords should now contain the properly formatted time.
     
     //If envelope is not empty, output an error message
     if(e->msg_text != NULL)
          printf("Error, clock cannot output due to non-empty envelope!\n");
          
     else
     {
          strcpy(e->msg_text, timewords);    //put the formatted time text into the envelope
          send_console_chars(e);
          return 1; //Success!
     }
     return 0; //Error with clock printing
}
