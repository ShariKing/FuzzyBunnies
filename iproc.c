
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
void kbd_iproc() {

    // temporary env pointer
    msg_env *temp_k = NULL;

    //check flag
    if (in_mem_p->ok_flag == 1) { 
        buf_index = 0;

        // get a pointer to the iprocess PCB
        PCB * temp = convert_PID(0);
        
        //dequeue env from Keyboard env queue into temp
        temp_k = env_DEQ(temp->receive_msg_Q);
        
        // read buffer into an env
        do {
            if (temp_k == NULL)
                printf("No envelope!!!");
            
            //read characters from buffer into msg_env
            temp_k->msg_text[ in_mem_p->indata[buf_index] ] = in_mem_p->indata[buf_index];

            buf_index++;
        } while (in_mem_p->indata[buf_index - 1] = '\0');
        
        printf("Keyboard input was: %s\n",in_mem_p->indata);
        
        // reset flag
        in_mem_p->ok_flag = 0;
        
        // set acknowledgement message type to 'console input'
        strcpy(temp_k->msg_type, "console_input");

        //send env back to process
        int Z = send_message(temp_k->sender_id, temp_k);  
        if (Z == 0)
            printf("Error with sending");
    }
    
    // if the buffer was empty (ie flag != 1)
    else
        printf("There is no input in the memory to read in!");
}

// ***CRT I-PROCESS***

void crt_iproc() {
    
    // temporary env pointer
    msg_env *temp_c = NULL;

    //check flag
    if (out_mem_p->oc_flag == 0) { 
        buf_index = 0;

        // read env into the buffer
        do {
            
            //dequeue env into msg_env
            temp_c = env_DEQ(convert_PID(0)->receive_msg_Q); 

            if (temp_c == NULL)
                printf("No envelope!!!");

            //send characters from msg_env into buffer
            out_mem_p->outdata[buf_index] = temp_c->msg_text[ out_mem_p->outdata[buf_index] ];

            buf_index++;
        } while (out_mem_p->outdata[buf_index - 1] = '\0');

        // set flag so UART will read memory
        out_mem_p->oc_flag = 1;
        
        // set acknowledgement message type to 'display ack'
        strcpy(temp_c->msg_type, "display_ack");

        //send env back to process 
        int Z = send_message(temp_c->sender_id, temp_c); 
        if (Z == 0)
            printf("Error with sending");
    }
    
    // if memory is full (ie. flag != 0)
    else
        printf("fix the functionn!!!!");
}
