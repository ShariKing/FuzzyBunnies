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
    
        //printf("You're in the CCI\n");
        int HH=0;
        int MM=0;
        int SS=0;
        int pri=0;
        int id=0;
        int U = 0;
        int dun = 0;
        
        msg_env* env = request_msg_env();
        msg_env* first = request_msg_env();
    
        while(1)
        {   
            // send the welcome message
/*
            if (first == NULL){
                terminate(1);
            }
            strcpy(first->msg_text, "Enter input for the CCI [s, ps, c [##:##:##], cd, ct, b, n [new_prio] [pid], e... , t]:\0");

            int F = 0;
            F = send_console_chars(first);

            if (F == 0){
                terminate(1);
            }
          
*/
         
            U = get_console_chars(env);   //keyboard input 
            //sleep(5);
            
            // only do this if there was input
            if (U==1)
            {
                env = receive_message();
                while (env->msg_type != CONSOLE_INPUT)
                {                
                        env = receive_message(); //***STOPS HERE TO WAIT FOR INPUT
                }
              
                char* input_txt = env->msg_text;
                
                // send envelope to Process A
                if (strcmp(input_txt,"s")==0) 
                { 
                    // printf("input was s\n");
                     //if the text in the field is "s"
                     env = request_msg_env(); //request an envelope
                     int Z =0;
                     Z = send_message(3,env); // send message to Process A
                }
                
                // display process status of all processes
                else if (strcmp(input_txt,"ps")==0) 
                { 
                    // printf("input was ps\n");
                     env = request_msg_env(); //request an envelope
                     int J=request_process_status(env);
                     if (J==0)
                        printf("Error with getting Process Status\n");                  
                }
                
                // allows time to be displayed on console and halts if getting ct
                else if (strcmp(input_txt,"cd")==0) 
                { 
                    // printf("input was cd\n");
                     
                    wallClockOut = 1;
                }
                else if (strcmp(input_txt,"ct")==0)
                {
                   // printf("input was ct\n");
                     wallClockOut = 0;
                }
    
                // set clock to any valid 24hr time
                else if (strncmp(input_txt,"c", 1)==0) 
                { 
                    // printf("input was c\n");
                     int HH = atoi(input_txt+2);
                     int MM = atoi(input_txt+5);
                     int SS = atoi(input_txt+8);
                     int R = clock_set(wallclock, HH, MM, SS);
                     
                     if (R==0)
                        printf("Error with setting clock in CCI\n");
                }
                
                // b displays past instances of send and receive messages
                else if (strcmp(input_txt,"b")==0) 
                { 
                   // printf("input was b\n");
                    env = request_msg_env(); //request an envelope
                    int U=get_trace_buffers(env);
                    
                    if (U==0)
                       printf("Error performing call on Trace Buffers\n");
               }
                
                // terminates RTX
                else if (strcmp(input_txt,"t")==0)
                { 
                    terminate(0);
                }
                
                // changes priority
                else if (strncmp(input_txt,"n",1)==0) 
                { 
                   //  printf("input was n\n");
                     int new_pri = atoi(input_txt+2);
                     int ID = atoi(input_txt+4);
                     int R = change_priority(new_pri, ID);
                     
                     if (R==0)
                            printf("Error with changing priority in CCI\n");
                }
                
                // echo the input back

                else if (strncmp(input_txt,"e", 1)==0) 
                { 
                   // printf("input was e\n");
                    env = request_msg_env(); //request an envelope
                    input_txt[0] = ' ';
                    strcat(input_txt, "\n\n");
                    strcpy(env->msg_text, "echo is: ");
                    strcpy(env->msg_text, input_txt);
                    int R = send_console_chars(env);
                    
                    if (R==0)
                            printf("Error with echoing to screen\n");
                }
                
                else 
                {
                     env = request_msg_env(); //request an envelope
                     char* temp = (char*) malloc(sizeof(SIZE));
                     sprintf(temp, "'%s' is not valid CCI input. Please try again.\n\n", input_txt);
                     strcpy(env->msg_text, temp);
                     int R = send_console_chars(env);
                    
                     if (R==0)
                         printf("Error with printing invalid input message\n");
                     
                    // printf("'%s' is not valid CCI input. Please try again.\n\n", input_txt);
                }

                
                dun = release_msg_env(env);

                if (dun == 0)
                {
                    env = request_msg_env(); //request an envelope
                    strcpy(env->msg_text, "ERROR IN DEALLOCATING ENVELOPE AT END OF CCI\n");
                    int R = send_console_chars(env);

                    if (R==0)
                          printf("Error with printing error message\n");
                }
                //else
                 //  printf("CCI finished, I think\n");
               
            }
         
        env = request_msg_env();
        
        // if there was no input
        release_processor();
        }
}

