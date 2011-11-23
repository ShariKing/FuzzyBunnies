
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

// Clock Test Process
void ClockTest(clk *clock) 
{   
      struct msgenv* env = (struct msgenv *) malloc (sizeof (struct msgenv));
      env->msg_type = (char*)malloc (sizeof (SIZE));
      env->msg_text = (char*)malloc (sizeof (SIZE));

      if (env){

          //now enter infinite loop 
          while (1) { 
                clock_out(clock, env);
                usleep(100000);
                /*get_console_chars(env);   //keyboard input 

                env = receive_message(); //***STOPS HERE TO WAIT FOR INPUT

                while (env == NULL) {
                        usleep(100000);
                        env = receive_message();  
                }

                send_console_chars(env);   //CRT output, wait for ack 

                env = receive_message(); 

                while (env == NULL) { 
                        usleep (100000); 
                        env = receive_message(); 
                }*/
                
                
        } 
      }
}
