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
    printf("You're in ProcessA\n");
    // initialize return int, count
    int num = 0;
    PCB* pA_pcb = convert_PID(3);
    // receive the message from the CCI
    msg_env* envA = receive_message();

    // deallocate the message from Proc A's queue in its PCB
    envA= env_DEQ(pA_pcb->receive_msg_Q);
    
    while(1){
        // request an envelope
        msg_env* tempEnv = request_msg_env();
        
        // set the env type and text
        strcpy(tempEnv->msg_type, "count_report\0");
        tempEnv->msg_text[1] = num;
        
        // send to ProcessB
        int result = send_message(4, tempEnv);
        
        num++;
        
        release_processor();        
    }    
}


// *** PROCESS B *** PID 4
void ProcessB(){
    printf("You're in ProcessB\n");
    while(1){
        // receive the message from Process A
        msg_env* envB = receive_message();
        
        // send to ProcessB
        int Z = send_message(5, envB);
        if (Z ==0)
           printf("Sending Failed from Proc B");
     }    
}


// *** PROCESS C *** PID 5
void ProcessC(){
        printf("You're in ProcessC\n");
    // init the local queue
    struct envQ* localQ = (struct envQ *) malloc (sizeof (struct envQ));
    if (!localQ) {
        printf("localQ in Proc C not created properly");
        return;
    } // can we use the queue on the proc pcb instead?

    PCB* pC_pcb = convert_PID(5);
    
    msg_env* envC = request_msg_env();   
    // infinite loop of normal activity  
    while (1){
        int NUM =envC->msg_text[1];
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
        if (strcmp(envC->msg_type,"count_report")==0 && NUM % 20 == 0){
  
            // send the display message
            strcpy(envC->msg_text, "Process C\0");
            int W = send_console_chars(envC); // Returns confirmation of sending
            if (W==1) {
                // if it is the ack message request a delay of 10s, with wakeup code "wakeup10"
                int R = request_delay(10000, "wakeup10", envC); // request_delay returns an int
                if (R==0)
                   printf("Error with request_delay");
                   
                // wait for wakeup message
                envC = receive_message();
    
                // if its not the wakeup message put it on the local Q
                while (envC->msg_type != "wakeup10"){
                    envC = receive_message();
                    int XX = env_ENQ(envC,localQ);
                    if (XX==0)
                       printf("Error with putting message on local Q in Proc C");
                }   
            }
            else
               printf("Error sending 'Process C' to screen in Proc C");
            
        }  
        // deallocate envelopes
        //envC= env_DEQ(pC_pcb->receive_msg_Q);
        int dun = release_msg_env(envC);
        if (dun==0)
           printf("ERROR IN DEALLOCATING ENVELOPE AT END OF Proc C");
        
        // release processor
        release_processor();
            
    }// end of infinite loop
}
