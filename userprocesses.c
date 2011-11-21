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
    // initialize return int, count
    int num = 0;
    
    // receive the message from the CCI
    envA = receive_message();

    // deallocate the message from Proc A's queue in its PCB
    // FIX       deallocate_message(envA);
    
    while(1){
        // request an envelope
        tempEnv = request_msg_env();
        
        // set the env type and text
        strcpy(tempEnv->msg_type, "count_report");
        tempEnv->msg_text[1] = num;
        
        // send to ProcessB
        int result = send_message(4, tempEnv);
        
        num++;
        
        release_processor();        
    }    
}


// *** PROCESS B *** PID 4
void ProcessB(){
    while(1){
        // receive the message from Process A
        envB = receive_message();
        
        // send to ProcessB
        int Z = send_message(5, envB);
        if (Z ==0)
           printf("Sending Failed from Proc B");
     }    
}


// *** PROCESS C *** PID 5
void ProcessC(){

    // init the local queue
    struct envQ* localQ = (struct envQ *) malloc (sizeof (struct envQ));
    if (!localQ) {
        printf("localQ in Proc C not created properly");
        return;
    } // can we use the queue on the proc pcb instead?
            
    // init a PCB and a couple env
    struct pcb* CPCB = (struct pcb *) malloc(sizeof (struct pcb));
    struct msgenv* envC = (struct msgenv *) malloc (sizeof (struct msgenv));
    struct msgenv* envSend = (struct msgenv *) malloc (sizeof (struct msgenv));

    CPCB = convert_PID(5);
    envSend = request_msg_env();    
        
    // infinite loop of normal activity  
    while (1){
        
        // if theres nothing the localQ, receive a message and enqueue it
        if (localQ->head == NULL){
            envC = receive_message();     
            int H = env_ENQ(envC,localQ);
            if (H ==0)
               printf("Cannot enqueue on localQ");
        }
        
        // if there is something on the localQ, dequeue it
        else        
            envC = env_DEQ(localQ);
            
        // if the message type is count report, and the value in msg_text is evenly divisible by 20, display "Process C"
        if (envC->msg_type == "count_report" && envC->msg_text % 20 == 0){
  
            // send the display message
            strcpy(envC->msg_text, "Process C");
            int W = send_console_chars(envSend); // Returns confirmation of sending
            if (W==1) {
                // if it is the ack message request a delay of 10s, with wakeup code "wakeup10"
                int R = request_delay(10, "wakeup10", envC); // request_delay returns an int
                if (R==0)
                   printf("Error with request_delay");
                   
                // wait for wakeup message
                envC = receive_message();
    
                // if its not the wakeup message put it on the local Q
                while (envC->msg_type != "wakeup10"){
                    envC = receive_message();
                    envC = env_ENQ(localQ);
                }   
            }
            else
               printf("Error sending 'Process C' to screen in Proc C");
            
        }  
        // deallocate envelopes
        deallocate_message(envC);
        
        // release processor
        int R =release_processor();
            
    }// end of infinite loop
}
