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

void CCI() 
{   
      int HH=0;
      int MM=0;
      int SS=0;
      int pri=0;
      int id=0;
      msg_env* env;
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
                     msg_env* env_ps;
                     int J=request_process_status(env_ps);
                     if (J==0)
                        printf("Error with getting Process Status");                  
                     }
                // set clock to any valid 24hr time
                else if (env->msg_text[buf_index] == "c", HH,MM,SS) { 
                     int HH = hours;
                     int MM = minutes;
                     int SS = seconds;
                     int R = clock_set(clock, hours, minutes, seconds);
                     if (R==0)
                        printf("Error with setting clock in CCI");
                     }
                // allows time to be displayed on console and halts if getting ct
                else if (env->msg_text[buf_index] == "cd") { 
                     msg_env* e = request_msg_env();
                     if (e->msg_text[buf_index] != "ct") {
                         int R = clock_out(clock, e);
                         if (R==0)
                            printf("Error with displaying clock in CCI");
                     }
                     else
                         int L =release_msg_env(e);
                         if (L==0)
                            printf("Message not released in clock displaying in CCI");
                     }
                // b displays past instances of send and receive messages
                else if (env->msg_text[buf_index] == "b") { 
                    msg_env* env1=request_msg_env();
                    int U=get_trace_buffers(env1);
                    if (U==0)
                       printf("Error performing call on Trace Buffers");
                     }
                // terminates RTX
                else if (env->msg_text[buf_index] == "t") { 
                     terminate();
                     }
                // changes priority
                else if (env->msg_text[buf_index] == "n",pri,id) { 
                     int ID = id;
                     int new_pri = pri;
                     int R = change_priority(new_pri, ID);
                     if (R==0)
                            printf("Error with changing priority in CCI");
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
