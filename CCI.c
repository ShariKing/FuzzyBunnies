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
    printf("You're in CCI\n");
      int HH=0;
      int MM=0;
      int SS=0;
      int pri=0;
      int id=0;
      int U = 0;
      msg_env* env = request_msg_env();
      
          //now enter infinite loop 
         // while (1) { 
                U = get_console_chars(env);   //keyboard input 
                sleep(5);
                // only do this if there was input
                if (U==1){
                    env = receive_message(); //***STOPS HERE TO WAIT FOR INPUT
                    /*
                    while (env == NULL) {
                            usleep(100000);
                            env = receive_message();  
                    }
                    */
                    char* input_txt = env->msg_text;
                    // send envelope to Process A
                    if (input_txt == "s") { //if the text in the field is "s"
                          msg_env *env2 = request_msg_env(); //request an envelope
                          int Z =0;
                          Z = send_message(3,env2); // send message to Process A
                          }
                    // display process status of all processes
                    else if (input_txt == "ps") { 
                         msg_env* env_ps;
                         int J=request_process_status(env_ps);
                         if (J==0)
                            printf("Error with getting Process Status");                  
                         }
                    // set clock to any valid 24hr time
                    else if (input_txt == "c", HH,MM,SS) { 
                         int HH;
                         int MM;
                         int SS;
                         int R = clock_set(wallclock, HH, MM, SS);
                         if (R==0)
                            printf("Error with setting clock in CCI");
                         }
                    // allows time to be displayed on console and halts if getting ct
                    else if (input_txt == "cd") { 
                         /*msg_env* e = request_msg_env();
                         if (input_txt != "ct") {
                             int R = clock_out(wallclock, e);
                             if (R==0)
                                printf("Error with displaying clock in CCI");
                         }
                         else {
                             int L = release_msg_env(e);
                             if (L==0)
                                printf("Message not released in clock displaying in CCI");
                             }*/
                        wallClockOut = 1;
                    }
                    else if (input_txt == "ct")
                        wallClockOut = 0;
                    
                    // b displays past instances of send and receive messages
                    else if (input_txt == "b") { 
                        msg_env* env1=request_msg_env();
                        int U=get_trace_buffers(env1);
                        if (U==0)
                           printf("Error performing call on Trace Buffers");
                         }
                    // terminates RTX
                    else if (input_txt == "t") { 
                         terminate(0);
                         }
                    // changes priority
                    else if (input_txt == "n",pri,id) { 
                         int ID = id;
                         int new_pri = pri;
                         int R = change_priority(new_pri, ID);
                         if (R==0)
                                printf("Error with changing priority in CCI");
                         }
                    /*
                    // knock knock joke
                    else if (input_txt == "knock") { 
                         knock_knock();
                         }
                    // time warp
                    else if (input_txt == "timewarp") { 
                         Time_Warp();
                         }
                    // pong
                    else if (input_txt == "pong") { 
                         Pong();
                         }
                         */
                    else {
                         printf("Invalid CCI input. Please try again.");                   
                    }
                int dun = release_msg_env(env);
                if (dun==0)
                   printf("ERROR IN DEALLOCATING ENVELOPE AT END OF CCI");
                else
                   printf("CCI finished, I think");
              }
          // if there was no input
          release_processor();
        //} 
}

