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


// *** PROCESS A *** PID 3
void ProcessA(){
    
     // initialize return int, count and temporary envelopes
    int result, num = 0;
    struct msgenv* envA = (struct msgenv *) malloc (sizeof (struct msgenv));
    struct msgenv* tempEnv = (struct msgenv *) malloc (sizeof (struct msgenv));
   
    // receive the message from the CCI
    envA = receive_message();

    // deallocate the message
    deallocate_message(envA);
    
    // if the temp is NULL
    if (!tempEnv)
        return;
    
    while(1){
        
        // request an envelope
        tempEnv = request_msg_env();
        
        // set the env type and text
        strcpy(tempEnv->msg_type, "count_report");
        tempEnv->msg_text[1] = num;
        
        // send to ProcessB
        result = send_message(4, tempEnv);
        
        num++;
        
        release_processor();        
    }    
}


// *** PROCESS B *** PID 4
void ProcessB(){
    
    // create an env to use
    struct msgenv* envB = (struct msgenv *) malloc (sizeof (struct msgenv));
    
    // if the env is NULL
    if (!envB)
        return;
    
    while(1){

        // receive the message from Process A
        envB = receive_message();
        
        // send to ProcessB
        envB = send_message(5, envB);
 
    }    
}


// *** PROCESS C *** PID 5
void ProcessC(){

    // init the local queue
    envQ localQ = (envQ) malloc (sizeof envQ);
    if (!localQ)
        return
            
    // init a PCB and a couple env
    struct pcb* CPCB = (struct pcb *) malloc(sizeof (struct pcb));
    struct msgenv* envC = (struct msgenv *) malloc (sizeof (struct msgenv));
    struct msgenv* envSend = (struct msgenv *) malloc (sizeof (struct msgenv));

    CPCB = convert_PID(5);
    envSend = request_msg_env();    
        
    // infinite loop of normal activity  
    while (1){
        
        // if theres nothing the localQ, receive a message and enqueue it
        if (!localQ->head){
            envC = receive_message;     
            envC = env_ENQ(localQ);
        }
        
        // if there is something on the localQ, dequeue it
        else        
            envC = env_DEQ(localQ);
            
        // if the message type is count report, and the value in msg_text is evenly divisible by 20, display "Process C"
        if (envC->msg_type == "count_report" && envC->msg_type % 20 == 0){
  
            // send the display message
            strcpy(envSend->msg_text, "Process C");
            send_console_chars(envSend);

            // wait for the ack message
            envC = receive_message();

            // if it's not the ack message, put it on the local Q
            while (envC->msg_type != "display_ack"){
                envC = receive_message();
                envC = env_ENQ(localQ);
            }

            // if it is the ack message request a delay of 10s, with wakeup code "wakeup10"
            request_delay(10, "wakeup10");

            // wait for wakeup message
            envC = receive_message();

            // if its not the wakeup message put it on the local Q
            while (envC->msg_type != "wakeup10"){
                envC = receive_message();
                envC = env_ENQ(localQ);
            }

        }  
        
        // deallocate envelopes
        deallocate_message(envC);
        deallocate_message(envSend);
        
        // release processor
        release_processor();
            
    }// end of infinite loop
}