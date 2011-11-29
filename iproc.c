
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

// ***KEYBOARD I-PROCESS priority 0***
void kbd_iproc(int sigval) {
    printf("You're in kbd_iproc\n");
    // temporary env pointer
    msg_env *temp_k = NULL;

    // only continue if the flag is 1, ie there's stuff in the buffer
    if (*in_mem_p->ok_flag == 1) {
        
        // start at the beginning of the buffer
        buf_index = 0;

        // get a pointer to the kb-iprocess PCB
        PCB * temp = convert_PID(0);

        //dequeue env from Keyboard env queue into temp
        temp_k = env_DEQ(temp->receive_msg_Q);

        // if the dequeued env is not NULL
        if (temp_k == NULL)
            return;
                    
        // read buffer into the env, while we haven't reached the NULL
        do {
            temp_k->msg_text[buf_index] = in_mem_p->indata[buf_index];

            buf_index++;

        } while (in_mem_p->indata[buf_index - 1] != '\0');

       // printf("Keyboard input was: %s\n",in_mem_p->indata); //used for debugging

        // reset flag
        *in_mem_p->ok_flag = 0;

        // set the env message type to 'console input'
        strcpy(temp_k->msg_type, "console_input");

        // send env back to process
        int Z = send_message(4, temp_k); //check location being sent here!

        // if it didn't send like it should have
        if (Z == 0)
            printf("Error with sending\n");
    }
    
    /* if the buffer was empty (ie flag != 1)
    else
        printf("There is no input in the memory to read in!\n"); */
}

// ***CRT I-PROCESS***

void crt_iproc(int sigval) {
printf("You're in crt_iproc\n");
        // temporary env pointer
        msg_env *temp_c = NULL;

        // put stuff in buffer only if the buffer is empty
        if (*out_mem_p->oc_flag == 0) { 
            buf_index = 0;

            // dequeue env into msg_env
            temp_c = env_DEQ(convert_PID(1)->receive_msg_Q); 

            // if the envelope is NULL don't continue
            if (temp_c == NULL)
               return;

             // read env into the buffer
             do {
                 out_mem_p->outdata[buf_index] = temp_c->msg_text[buf_index];

                 buf_index++;

             } while (out_mem_p->outdata[buf_index - 1] != '\0'); 

            // set flag so UART will read memory
            *out_mem_p->oc_flag = 1;
            
            // set env message type to 'display ack'
            strcpy(temp_c->msg_type, "display_ack");
            
            // send env back to process
            int Z = send_message(4, temp_c); //fix this later, sender_id is 31
            
            // if sending is stupid
            if (Z == 0)
                printf("Error with sending\n");
        
        }

        /* if memory is full (ie. flag != 0)
        else{
            printf("fix the function!!!!\n"); */
        
}

void timer_iproc(int sigval) {
     printf("You're in timer_iproc\n");
        static int pulse_counter = 0;     //Dummy Pulse Counter
        msg_env *sleeptraverse;           //Dummy envelope pointer to traverse the Sleep Queue
        msg_env *awakened;                //Dummy envelope pointer for awakened envelopes
        int removeid;                     //id to extract, if extraction necessary
        int sleeptime;                    //Dummy integer to use for string conversion
        
        //printf("\nClock Signal Received.   Incrementing pulse counter from %i ", pulse_counter);
        //increment the pulse counter
  		pulse_counter++;     
        //printf("to %i ...\n", pulse_counter);  		
  		
          //when pulse counter hits ten (one second)
		if(pulse_counter == 10)
        {
            //printf("%p wallclock pointer .... %p system clock pointer\n\n", wallclock, systemclock);
			clock_increment(systemclock, 0);
			clock_increment(wallclock, 1);
                        
                        // if cd has been inputted, print the clock. when ct is input it will reset the wallclockout variable and not get here
                        if (wallClockOut == 1)
                            clock_out(wallclock);
                        
			pulse_counter = 0;
		}
		
//==========CODE TO HANDLE REQUEST DELAY=============		
		msg_env *delayRequest;
		delayRequest = receive_message();              //Receive Message for Delays
		if(delayRequest)
            env_ENQ(delayRequest, sleep_Q);            //Enqueue delay requests onto the sleep Q
        
       	if(sleep_Q->head)     //if the sleep queue is not empty
  		{
             sleeptraverse = sleep_Q->head;        //Point Sleeptraverse at the head to begin
             sleeptime = atoi(sleeptraverse->msg_text);              //Convert the delay time text to an integer
             sleeptime = sleeptime - 100;                                //Decrement the sleeptime
             if(sleeptime <= 0)           //Check if the head env has finished sleeping
             {
                  awakened = sleeptraverse;          //awakened now points to what sleeptraverse did before
                  
                  if(sleeptraverse->p)                   //Move the pointer to the next so we don't lose it after Dequeueing
                       sleeptraverse = sleeptraverse->p;
                  else                                   //If this is the only envelope in the sleep queue, set sleeptraverse to NULL.
                       sleeptraverse = NULL;
                  
                  awakened = env_DEQ(sleep_Q);           //Dequeue awakened from the head
                  send_message(awakened->sender_id, awakened);     //send awakened back to its sender id, which should be blocked_on_receive
             }
             else     //If head process is not awake yet, check for others in the queue and traverse
             {
                  sprintf(sleeptraverse->msg_text, "%d", sleeptime);    //copy the new sleeptime back into a string and put it back in the envelope
                  if(sleeptraverse->p)                            //traverse
                       sleeptraverse = sleeptraverse->p;
                  else
                       sleeptraverse = NULL;
             }
        }
             while(sleeptraverse)    //If there are no others, sleeptraverse will be NULL and skip this. Otherwise, sleeptraverse
             {                       // will be the next, undecremented envelope, and will loop.
                 removeid = sleeptraverse->sender_id;     //Extracting the id to use with env_remove to remove the envelope from the queue
                 
                 sleeptime = atoi(sleeptraverse->msg_text);            //Convert the delay time text to an integer
                 sleeptime = sleeptime - 100;                          //decrement the delay time
                 if(sleeptime <= 0)           //Check if the env has finished sleeping
                 {
                      awakened = sleeptraverse;          //awakened now points to what sleeptraverse did before
                  
                      if(sleeptraverse->p)                   //Move the pointer to the next so we don't lose it after Dequeueing
                           sleeptraverse = sleeptraverse->p;
                      else                                   //If this is the only envelope in the sleep queue, set sleeptraverse to NULL.
                           sleeptraverse = NULL;
                      
                      
                      awakened = env_REMOVE(sleep_Q, removeid);   //Remove awakened from the sleep queue without losing any links
                      send_message(awakened->sender_id, awakened); //send awakened back to its sender id, which should be blocked_on_receive
                 }
                 else     //If current decrementing process is not awake yet, check for others in the queue and traverse
                 {
                      sprintf(sleeptraverse->msg_text, "%d", sleeptime);    //copy the new sleeptime back into a string and put it back in the envelope
                      if(sleeptraverse->p)                                  //traverse
                           sleeptraverse = sleeptraverse->p;
                      else
                           sleeptraverse = NULL;
                 }
             }        
}

//==========    CODE BELOW THIS LINE IS FUNCTIONAL, BUT DOES NOT FIT THE STANDARD DESIGN. LEAVE AS BACKUP     ===================//
/*  		if(sleep_Q->head)     //if the sleep queue is not empty
  		{
             sleeptraverse = sleep_Q->head;
             sleeptraverse->sleeptime = sleeptraverse->sleeptime - 100;      //decrement the sleeptime of the first PCB by 100msec
             if(sleeptraverse->sleeptime <= 0)           //Check if the PCB has finished sleeping
             {
                  awakened = sleeptraverse;          //awakened now points to what sleeptraverse did before
                  removeid = sleeptraverse->pid;     //Extracting the id to remove the PCB
                  
                  if(sleeptraverse->p)                   //Move the pointer to the next so we don't lose it after Dequeueing
                       sleeptraverse = sleeptraverse->p;
                  else                                   //If this is the only PCB in the sleep queue, set sleeptraverse to NULL.
                       sleeptraverse = NULL;
                  
                  strcpy(awakened->state, "READY");      //Set the awakened process state to ready
                  
                  if(awakened->priority == 0)            //Remove the PCB from the queue and enqueue it in it's appropriate ready queue                                         
                       PCB_ENQ(PCB_REMOVE(sleep_Q, removeid), ready_q_priority0);  //This  code may be wrong...I don't understand Yasser's PCB_remove
                  else if(awakened->priority == 1)                                                     
                       PCB_ENQ(PCB_REMOVE(sleep_Q, removeid), ready_q_priority1);     
                  else if(awakened->priority == 2)                                                     
                       PCB_ENQ(PCB_REMOVE(sleep_Q, removeid), ready_q_priority2);
                  else if(awakened->priority == 3)                                                     
                       PCB_ENQ(PCB_REMOVE(sleep_Q, removeid), ready_q_priority3);
                  else
                      printf("Whaaaaaaa?!\n");     //The code should never get here
             }
             else     //If head process is not awake yet, check for others in the queue and traverse
             {
                  if(sleeptraverse->p)
                       sleeptraverse = sleeptraverse->p;
                  else
                       sleeptraverse = NULL;
             }
                  
                       
             while(sleeptraverse)    //If there are no others, sleeptraverse will be NULL and skip this. Otherwise, sleeptraverse
             {                       // will be the next, undecremented PCB, and will loop.
                 sleeptraverse->sleeptime = sleeptraverse->sleeptime - 100; 
                 if(sleeptraverse->sleeptime <= 0)
                 {
                      awakened = sleeptraverse;         //awakened now points to what sleeptraverse did before
                      removeid = sleeptraverse->pid;    //Extracting the id to remove the PCB                    
                      
                      if(sleeptraverse->p)              //Move the pointer to the next so we don't lose it after Dequeueing
                           sleeptraverse = sleeptraverse->p;
                      else                              //If this is the only PCB in the sleep queue, set sleeptraverse to NULL.
                           sleeptraverse = NULL;
                      
                      strcpy(awakened->state, "READY");      //Set the awakened process state to ready
                  
                      if(awakened->priority == 0)      //Remove the PCB from the queue and enqueue it in it's appropriate ready queue                                                 
                           PCB_ENQ(PCB_REMOVE(sleep_Q, removeid), ready_q_priority0);
                      else if(awakened->priority == 1)                                                     
                           PCB_ENQ(PCB_REMOVE(sleep_Q, removeid), ready_q_priority1);     
                      else if(awakened->priority == 2)                                                     
                           PCB_ENQ(PCB_REMOVE(sleep_Q, removeid), ready_q_priority2);
                      else if(awakened->priority == 3)                                                     
                           PCB_ENQ(PCB_REMOVE(sleep_Q, removeid), ready_q_priority3);
                      else
                          printf("Whaaaaaaa?!\n");     //The code should never get here
                 }
                 else                    //If other processes are not awake yet, check for existence of next in the queue and traverse
                 {
                      if(sleeptraverse->p)
                           sleeptraverse = sleeptraverse->p;
                      else
                           sleeptraverse = NULL;            //if no next, kick out of loop
                 }           
              }
        }*/
//==========    CODE ABOVE THIS LINE IS FUNCTIONAL, BUT DOES NOT FIT THE STANDARD DESIGN. LEAVE AS BACKUP     ===================//   

