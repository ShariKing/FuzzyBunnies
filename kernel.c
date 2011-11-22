
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

// ******ATOMIC FUNCTION*********

// check the logic on these, as to whether the static variable holds between the two like this
void atomic_on() { 
    static sigset_t oldmask;
    sigset_t newmask;
    sigemptyset(&newmask);
    sigaddset(&newmask, SIGALRM); //the alarm signal
    sigaddset(&newmask, SIGINT); // the CNTRL-C
    sigaddset(&newmask, SIGUSR1); // the CRT signal
    sigaddset(&newmask, SIGUSR2); // the KB signal
    sigprocmask(SIG_BLOCK, &newmask, &oldmask);
}

void atomic_off() {
     static sigset_t oldmask;
     sigset_t newmask;
     //unblock the signals
     sigprocmask(SIG_SETMASK, &oldmask, NULL);
     }

// *** PCB ENQUEUE ***
int PCB_ENQ(PCB *r, PCB_Q *queue) {

    /// if either the PCB pointer or the queue pointer are NULL don't do anything
    if (!r || !queue)
        return 0;
    
    // if queue is empty
    if (queue->head == NULL) {

        // set head of queue to new PCB
        queue->head = r;

        // set tail of queue to new PCB
        queue->tail = r;

        return 1;
    }

    // If the queue is NOT empty
    // set the 'next' pointer of the PCB at the queue-tail to the newly enqueued PCB
    queue->tail->p = r;

    // set the 'next' pointer of the newly enqueued PCB to NULL (ie. it's the end of the queue)
    r->p = NULL;

    // set the tail of the queue to new PCB
    queue->tail = r;

    return 1;
}


// *** PCB Dequeue ***
PCB *PCB_DEQ(PCB_Q *queue) {

    // if queue is empty
    if (queue->head == NULL) {
        //printf("Queue is empty");
        
        // return a NULL pointer
        return NULL;
    }

    // if the queue is NOT empty
    // create a temp pointer to a PCB
    PCB *t;

    // set temp to point to the head of the queue
    t = queue->head;

    // set the new head of the queue to the 'next' PCB (ie. second in queue) of the old head
    queue->head = queue->head->p;

    //if the new queue only has one PCB, set the tail = head = PCB
    if (queue->head->p == NULL)
        queue->tail = queue->head;

    // return the pointer to the dequeued PCB
    return t;
}


// ***ENVELOPE ENQUEUE***
int env_ENQ(msg_env *e, env_Q *queue) {
   
    if (!e || !queue)
        return 0;
    
    //if queue is empty
    if (queue->head == NULL) {

        // set head of queue to e
        queue->head = e;

        // set tail of queue to e
        queue->tail = e;

        return 1;
    }
      
    // if the queue is NOT empty
    // set the 'next' pointer of the env at the queue-tail to the newly enqueued env
    queue->tail->p = e;
   
    // set the 'next' pointer of the new env to NULL
    e->p = NULL;
    
    // set the tail of the queue to the new env
    queue->tail = e;
    
    return 1;
}


// *** ENVELOPE DEQUEUE ***
msg_env *env_DEQ(env_Q *queue) {
    
    // if the queue doesn't exist
    if (!queue)
        return NULL;
    
    // create a temp env pointer
    msg_env *t = NULL;
        
    // if queue is empty    
    if (queue->head == NULL) { 
        // return a NULL pointer
        return NULL;
    }

    // if the queue is NOT empty
    // point the temp to the head of the queue
    t = queue->head;

    // point the new head of the queue to the 'next' pointer of the old head
    queue->head = queue->head->p;

    // if the queue now only has one env, set the tail = head = sole env
    if (queue->head == NULL) 
        queue->tail = queue->head;
    
    return t;
}


// *** KERNEL SEND MESSAGE ***
int k_send_message(int dest_id, msg_env *e) {
    
    // if the env is NULL
    if (!e)
        return 0;
    
    // if the PCB ID is not valid
    if (dest_id > (NUM_PROC - 1) )  { 
        //printf("Invalid 'Send To' ID, %i, ", dest_id);
        return 0;
    }
    
    // if the PCD ID is valid
    // set the target_id parameter of the env to dest_id
    e->target_id = dest_id;
    e->sender_id = curr_process->pid;

    // create a pointer to the target PCB using the target_id
    PCB *target = convert_PID(dest_id);

    // if the PID converts correctly to a pointer
    if (target){

        /*unblock the target process if necessary*/
        // if the target's receive message queue is empty or the target is not an i-process
        /*if (target->receive_msg_Q->head == NULL || target->priority != -1) { 

            // enqueue the PCB of the process on the appropriate ready queue
            PCB_ENQ(target, convert_priority(target->priority)); //*****not sure if need to put a '&' before convert_priority
            // set the target state to 'ready'
            strcpy(target->state, "READY"); //apparently strcpy is how you write into an array of chars
        }*/

        // enqueue the env on the target's receive queue
        env_ENQ(e, target->receive_msg_Q);

    return 1;
    
    }
    
    // if the PID doesn't convert successfully
    else
        return 0;
     
}

// ***USER SEND MESSAGE***
int send_message(int dest_id, msg_env *e) {
    
    // turn atomicity on
    atomic_on();
    
    // call the kernel send message primitive
    int z = k_send_message(dest_id, e);
    
    // turn atomicity off
    atomic_off();
    
    // return the return value from the k primitive
    return z;
}

// ***KERNEL RECEIVE MESSAGE***
msg_env *k_receive_message() { //Doesn't take the PCB as a parameter. Dealt with using curr_process
    
    // if the receive message queue is empty
    if (curr_process->receive_msg_Q->head == NULL) {
        
        // if the process is an iprocess or should never be blocked, return a NULL pointer (ie. no env)
        if (curr_process->priority == -1)
            return NULL;
        
        // if it's a normal process, block it on receive
        strcpy(curr_process->state, "BLK_ON_RCV"); //*********Doesn't need to be put in a queue, and don't care about process switch now********* FIX THIS
    
        return NULL; // *****TO BE FIXED WITH CONTEXT SWITCHING
    }
    
    // if the message queue is not empty, dequeue the first message in the queue
    else
        return env_DEQ(curr_process->receive_msg_Q);
}

// ***USER RECEIVE MESSAGE***
msg_env *receive_message() {
        // turn atomicity on
        atomic_on();
        
        // call the kernel receive message
        msg_env *temp = k_receive_message();
        
        // turn atomicity off
        atomic_off();
        
        // return the pointer to the message envelope
        return temp;
}

// *** SEND CONSOLE CHARS***
int send_console_chars(msg_env *env) {
    
    // if the env is NULL
    if (!env)
        return 0;
    
    int Z;
    
    // relay the env to the crt i-process using send_message
    Z = send_message(1, env);
    
    // if sending fails
    if (Z == 0){
        //printf("Error with sending");
        return 0;
    }

    return 1;
}


// ***GET CONSOLE CHARS***
int get_console_chars(msg_env * env) {
    
    // if the env is NULL
    if (!env)
        return 0;
    
    int Z;

    // relay the env to the kb i-process using send_message
    Z = send_message(0, env);

    // incorrect return from send_message
    if(Z == 0) {
            //printf("Error with sending");
            return 0;
    }
    
    return 1;
}


// ***GIVE A PROCESS ID AND RETURN IT'S PCB POINTER***
PCB *convert_PID(int PID) {
    
    // if the process ID is invalid
    if (PID > (NUM_PROC - 1) || PID < 0) {
        // printf("invalid ID");
        return NULL;
        }
    
    // if the ID is valid, return the pointer in the PCB array at the value of ID
    return pointer_2_PCB[PID];
}

// ***GET A PROIRITY AND RETURN A POINTER TO THE RPQ***
PCB_Q *convert_priority(int newPri) {
    
    // if the new priority is not valid (1-3)
    if (newPri > 4 || newPri < 0) {           //changed to 5 due to NULL process
        //printf("Invalid priority!!!!!");
        return NULL;
    }
    
    // if the priority is valid, return the pointer to the priority queue from the array
    return pointer_2_RPQ[newPri];
}

// -----------------------------Finish this function---------------------
// ***RELEASE PROCESSOR***
int k_release_processor() {
    strcpy (curr_process->state, "READY");  //Change current process state to "ready"
}
