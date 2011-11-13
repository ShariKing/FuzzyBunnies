
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

// pseudocode for kbd input -> crt output passing process 
void processP() 
{ 
      int tWait = 500000;          //rcv loop wait time in usec, approx value 
      struct msgenv* env = (struct msgenv *) malloc (sizeof (struct msgenv));
      
      printf("IT'S ALIIIIIIIVEEEEE!!!! MWAHAHAHAHA!\n");
          
      //now enter infinite loop 
      while (1) { 
                get_console_chars(env);   //keyboard input 
                env = receive_message(); 
      
                while (env == NULL) {    
                        usleep(tWait); 
                        env = receive_message(); 
                }
                
                send_console_chars(env);   //CRT output, wait for ack 
                env = receive_message(); 
      
                while (env == NULL) { 
                        usleep (tWait); 
                        env = receive_message(); 
                }   
     } 
}
