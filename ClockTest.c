
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

// WallClock user process
int WallClock() 
{   
      printf("You're in Wall Clock\n");
      
      int Printsuccess;
      
      msg_env* clockEnv = request_msg_env();
      if (clockEnv){
         Printsuccess = clock_out(wallclock, clockEnv);
         if(Printsuccess == 1)
              return 1;  //SUCCESS!
         else
         {
             printf("Error with Wall Clock Process!\n");
             return 0;     //Code should never get here
         }
      }
      else
      {
          printf("Request Message Envelope for Wall Clock failed!\n");
          return 0;       //code should never get here either
      }
}
