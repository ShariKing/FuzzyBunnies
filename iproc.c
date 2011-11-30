
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
    printf ("flag:%i\n", *in_mem_p->ok_flag);
    printf ("queue:%p\n", pointer_2_PCB[0]->receive_msg_Q->head);
    
    PCB* interrupted_proc = curr_process;
    curr_process = convert_PID (0);
    
    msg_env *temp_k = receive_message();
printf ("temp_k:%p\n", temp_k);
    // if the dequeued env is NULL, ie there is nothing for the kb iproc to do
    if (temp_k == NULL){
        *in_mem_p->ok_flag = 0;   
        curr_process = interrupted_proc;
        return;
    }
    
    // only continue if the flag is 1, ie there's stuff in the buffer
    if (*in_mem_p->ok_flag == 1) {
        
        // start at the beginning of the buffer
        buf_index = 0;

        //dequeue env from Keyboard env queue into temp
        //temp_k = env_DEQ(temp->receive_msg_Q);
                    
        // read buffer into the env, while we haven't reached the NULL
        do {
            temp_k->msg_text[buf_index] = in_mem_p->indata[buf_index];

            buf_index++;

        } while (in_mem_p->indata[buf_index - 1] != '\0');

       // printf("Keyboard input was: %s\n",in_mem_p->indata); //used for debugging

        // reset flag
        *in_mem_p->ok_flag = 0;

        // set the env message type to 'console input'
        temp_k->msg_type = CONSOLE_INPUT;

        // send env back to process
        int Z = send_message(temp_k->sender_id, temp_k); //check location being sent here!

        // if it didn't send like it should have
        if (Z == 0)
            printf("Error with sending\n");
    }
    
    curr_process = interrupted_proc;
    /* if the buffer was empty (ie flag != 1)
    else
        printf("There is no input in the memory to read in!\n"); */
}

// ***CRT I-PROCESS***

void crt_iproc(int sigval) {
//printf("You're in crt_iproc\n");
        // temporary env pointer
        
        PCB* interrupted_proc = curr_process;
        curr_process = convert_PID (1);
        
        msg_env *temp_c = receive_message();
        
        // if the envelope is NULL don't continue
        if (temp_c == NULL){
           *out_mem_p->oc_flag = 0;
           curr_process = interrupted_proc;
           return;
        }

        // put stuff in buffer only if the buffer is empty
        if (*out_mem_p->oc_flag == 0) { 
            buf_index = 0;

             // read env into the buffer
             do {
                 out_mem_p->outdata[buf_index] = temp_c->msg_text[buf_index];

                 buf_index++;

             } while (out_mem_p->outdata[buf_index - 1] != '\0'); 

            // set flag so UART will read memory
            *out_mem_p->oc_flag = 1;
            
            // set env message type to 'display ack'
            temp_c->msg_type = 1;
            
            // send env back to process
            int Z = send_message(temp_c->sender_id, temp_c); //fix this later, sender_id is 31
            
            // if sending is stupid
            if (Z == 0)
                printf("Error with sending\n");
        
        }

        curr_process = interrupted_proc;
        /* if memory is full (ie. flag != 0)
        else{
            printf("fix the function!!!!\n"); */
        
}

void timer_iproc(int sigval) {
    //printf("You're in timer_iproc\n");
     
    PCB* interrupted_proc = curr_process;
    curr_process = convert_PID (2);

    //increment the pulse counter
    pulse_counter++;     
    
    //when pulse counter hits ten (one second)
    if(pulse_counter == 10)
    {
            clock_increment(systemclock, 0);
            clock_increment(wallclock, 1);

            // if cd has been inputted, print the clock. when ct is input it will reset the wallclockout variable and not get here
            if (wallClockOut == 1)
                clock_out(wallclock);

            pulse_counter = 0;
    }
		
//==========CODE TO HANDLE REQUEST DELAY=============	
    
    msg_env *sleeptraverse;           //Dummy envelope pointer to traverse the Sleep Queue
    msg_env *awakened;                //Dummy envelope pointer for awakened envelopes
    msg_env *delayRequest;
    int removeid;                     //id to extract, if extraction necessary
    int sleeptime;                    //Dummy integer to use for string conversion
    
    delayRequest = receive_message();              //Receive Message for Delays
    if(delayRequest != NULL && delayRequest->msg_type == 4)
         env_ENQ(delayRequest, sleep_Q);            //Enqueue delay requests onto the sleep Q

    if(sleep_Q->head != NULL) {    //if the sleep queue is not empty
        
         sleeptraverse = sleep_Q->head;        //Point Sleeptraverse at the head to begin
         sleeptime = atoi(sleeptraverse->msg_text);              //Convert the delay time text to an integer
         sleeptime = sleeptime - 100;                                //Decrement the sleeptime
         
         /*if(sleeptime <= 0)           //Check if the head env has finished sleeping
         {
              awakened = sleeptraverse;          //awakened now points to what sleeptraverse did before

              if(sleeptraverse->p != NULL)                   //Move the pointer to the next so we don't lose it after Dequeueing
                   sleeptraverse = sleeptraverse->p;
              else                                   //If this is the only envelope in the sleep queue, set sleeptraverse to NULL.
                   sleeptraverse = NULL;
              
              awakened = env_DEQ(sleep_Q);           //Dequeue awakened from the head
              send_message(awakened->sender_id, awakened);     //send awakened back to its sender id, which should be blocked_on_receive
         }
         else     //If head process is not awake yet, check for others in the queue and traverse
         {
              sprintf(sleeptraverse->msg_text, "%d\0", sleeptime);    //copy the new sleeptime back into a string and put it back in the envelope
              if(sleeptraverse->p != NULL)                            //traverse
                   sleeptraverse = sleeptraverse->p;
              else
                   sleeptraverse = NULL;
         }
    }*/
         while(sleeptraverse != NULL)    //If there are no others, sleeptraverse will be NULL and skip this. Otherwise, sleeptraverse
         {                       // will be the next, undecremented envelope, and will loop.
             removeid = sleeptraverse->sender_id;     //Extracting the id to use with env_remove to remove the envelope from the queue

             sleeptime = atoi(sleeptraverse->msg_text);            //Convert the delay time text to an integer
             sleeptime = sleeptime - 100;                          //decrement the delay time
             
             if(sleeptime <= 0)           //Check if the env has finished sleeping
             {
                  awakened = sleeptraverse;          //awakened now points to what sleeptraverse did before

                  if(sleeptraverse->p != NULL)                   //Move the pointer to the next so we don't lose it after Dequeueing
                       sleeptraverse = sleeptraverse->p;
                  else                                   //If this is the only envelope in the sleep queue, set sleeptraverse to NULL.
                       sleeptraverse = NULL;


                  awakened = env_REMOVE(sleep_Q, removeid);   //Remove awakened from the sleep queue without losing any links
                  send_message(awakened->sender_id, awakened); //send awakened back to its sender id, which should be blocked_on_receive
             }
             
             else     //If current decrementing process is not awake yet, check for others in the queue and traverse
             {
                  sprintf(sleeptraverse->msg_text, "%d\0", sleeptime);    //copy the new sleeptime back into a string and put it back in the envelope
                  
                  if(sleeptraverse->p != NULL)                                  //traverse
                       sleeptraverse = sleeptraverse->p;
                  else
                       sleeptraverse = NULL;
             }
         }
    }
    
    curr_process = interrupted_proc;
}
