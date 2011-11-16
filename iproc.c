
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

// ***KEYBOARD I-PROCESS***
void kbd_iproc(int sigval) {
    
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
        int Z = send_message(4, temp_k); 

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
