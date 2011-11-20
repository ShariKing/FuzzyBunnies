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

void CCI() 
{   
      if (env){
          //now enter infinite loop 
          while (1) { 

                get_console_chars(env);   //keyboard input 
                env = receive_message(); //***STOPS HERE TO WAIT FOR INPUT

                while (env == NULL) {
                        usleep(100000);
                        env = receive_message();  
                }
                // send envelope to Process A
                if (env->msg_text[buf_index] == "s") { //if the text in the field is "s"
                      msg_env *env2 = request_msg_env(); //request an envelope
                      int Z =0;
                      Z = send_message(3,env2); // send message to Process A
                      }
                // display process status of all processes
                else if (env->msg_text[buf_index] == "ps") { 
                     }
                // set clock to any valid 24hr time
                else if (env->msg_text[buf_index] == "c" /*NUMBERS */) { 
                     }
                // allows time to be displayed on console
                else if (env->msg_text[buf_index] == "cd") { 
                     }
                // halts the display of clock
                else if (env->msg_text[buf_index] == "ct") { 
                     }
                // b displays past instances of send and receive messages
                else if (env->msg_text[buf_index] == "b") { 
                     }
                // terminates RTX
                else if (env->msg_text[buf_index] == "t") { 
                     terminate();
                     }
                // changes priority
                else if (env->msg_text[buf_index] == "n" /* NUMBERS*/) { 
                     }
                // knock knock joke
                else if (env->msg_text[buf_index] == "knock") { 
                     knock_knock();
                     }
                // time warp
                else if (env->msg_text[buf_index] == "timewarp") { 
                     Time_Warp();
                     }
                // pong
                else if (env->msg_text[buf_index] == "pong") { 
                     Pong();
                     }
                else {
                     printf("Invalid CCI input. Please try again.");                   
                }
        } 
      }
}
