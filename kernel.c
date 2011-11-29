
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
    printf("You're in atomic_on\n");
    if (Atom == 0){
        static sigset_t oldmask;
        sigset_t newmask;
        sigemptyset(&newmask);
        //sigaddset(&newmask, SIGALRM); //the alarm signal
        //sigaddset(&newmask, SIGINT); // the CNTRL-C
        sigaddset(&newmask, SIGUSR1); // the CRT signal
        sigaddset(&newmask, SIGUSR2); // the KB signal
        //sigaddset(&newmask, SIGTTIN); // random signal that breaks things!!!!!!!!
        sigprocmask(SIG_BLOCK, &newmask, &oldmask);
        Atom ++;
        printf("Atomic on\n");
    }
    else if (Atom == 1)
        printf("Atomic already on, count is %d\n", Atom);
    else
        printf("Atomic error, RUN! (atomicity is %d", Atom);
}

void atomic_off() {
    printf("You're in atomic_off\n");
    if (Atom == 1){
        static sigset_t oldmask;
        sigset_t newmask;
        //unblock the signals
        sigprocmask(SIG_SETMASK, &oldmask, NULL);
        printf("Atomic off\n");
        Atom --;
    }
    else if (Atom == 0)
        printf("Atomic already off, count is %d\n", Atom);
    else
        printf("Atomic error, RUN! (atomicity is %d", Atom);
}


// *** PCB ENQUEUE ***
int PCB_ENQ(PCB *r, PCB_Q *queue) {
printf("You're in PCB_ENQ\n");
    /// if either the PCB pointer or the queue pointer are NULL don't do anything
    if (r ==NULL || queue==NULL)
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


// *** PCB DEQUEUE ***
PCB *PCB_DEQ(PCB_Q *queue) {
printf("You're in PCB_DEQ\n");
    // if queue is empty
    if (queue->head == NULL) {
        //printf("Queue is empty");        
        // return a NULL pointer
        return NULL;
    }

    // if the queue is NOT empty
    // create a temp pointer to a PCB
    PCB* temp;
    PCB* tempNext;

    // set temp to point to the head of the queue
    temp = queue->head;
    tempNext = queue->head->p;
    
    // set the new head of the queue to the 'next' PCB (ie. second in queue) of the old head
    queue->head = tempNext;
    
    // if the new head is not null
    if (queue->head) {    
                        
        //if the new queue only has one PCB, set the tail = head = PCB
        if (queue->head->p == NULL)
            queue->tail = queue->head;
    } 
    else
        queue->tail =NULL;       

    temp->p = NULL;    //set the pointer of the dequeued PCB to NULL 
    // return the pointer to the dequeued PCB
    return temp;
}

//***REMOVE PCB FROM MIDDLE OF QUEUE***
PCB *PCB_REMOVE(PCB_Q *q, int id){
    printf("You're in PCB_REMOVE\n");
    if(q->head == NULL){              //if queue is empty
                   //printf("Queue is empty");
                   return NULL;
    }
    PCB *index = q->head;           //create a pointer that starts at the head
    if(index->pid == id){           //first PCB is the one we're looking for
                  index = PCB_DEQ(q);
                  return index;      //dequeue it and return it
    }
    PCB *prev = index;               //pointer prev trails index
    index = index->p;                //index is leading prev now
    while(index != NULL){            //while index didn't traverse the entire queue
                if(index->pid == id){ //if the PCB is found
                              prev = index->p;
                              index->p = NULL;
                              return index;
                }
                prev = index;
                index = index->p;    //move on to the next PCB
    }
    
    printf("Cannot find PCB in queue\n");
    return NULL;
}

// ***ENVELOPE ENQUEUE***
int env_ENQ(msg_env *e, env_Q *queue) {
   printf("You're in env_ENQ\n");
    if (e==NULL || queue==NULL)
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
    printf("You're in env_DEQ\n");
    // if the queue doesn't exist
    if (queue ==NULL)
        return NULL;
    
    // create a temp env pointer
    msg_env *temp;
    msg_env *tempNext;
        
    // if queue is empty    
    if (queue->head == NULL) { 
        // return a NULL pointer
        return NULL;
    }

    // if the queue is NOT empty
    // point the temp to the head of the queue
    temp = queue->head;
   //printf("%p\n", queue->head);
    tempNext = queue->head->p;
    // point the new head of the queue to the 'next' pointer of the old head
    queue->head = tempNext;

    if(queue->head == NULL) {        
        // if the queue now only has one env, set the tail = head = sole env
            queue->tail = NULL;
    }
    
    temp->p = NULL;    //set the pointer of the dequeued envelope to NULL
    
    return temp;
}


//***REMOVE ENV FROM MIDDLE OF QUEUE***
msg_env *env_REMOVE(env_Q *q, int senderid){
    printf("You're in env_REMOVE\n");
    if(q->head == NULL){              //if queue is empty
                   //printf("Queue is empty");
                   return NULL;
    }
    msg_env *index = q->head;           //create a pointer that starts at the head
    if(index->sender_id == senderid){           //first PCB is the one we're looking for
                  index = env_DEQ(q);
                  return index;      //dequeue it and return it
    }
    msg_env *prev = index;               //pointer prev trails index
    index = index->p;                //index is leading prev now
    while(index != NULL){            //while index didn't traverse the entire queue
                if(index->sender_id == senderid){ //if the PCB is found
                              prev = index->p;
                              index->p = NULL;
                              return index;
                }
                prev = index;
                index = index->p;    //move on to the next PCB
    }
    
    printf("Cannot find env in queue\n");
    return NULL;
}

/*
//*** FREE ENVELOPE QUEUE
int free_env_queue(env_Q* Q){
    msg_env* willy;                       // hehehehe wait for it
    while (Q->head != NULL){     // while the queue is not empty
          willy = env_DEQ(Q);          
          free(willy->msg_text);          // free the msg_text array
          free(willy->msg_type);          // free the msg_type array
          free(willy->p);// Dq from the queue and set it to willy
          free(willy);                    // THERE IT IS!!
    }
         
    free(Q);
    return 1;
}

//*** FREE PCB QUEUE
int free_PCB_queue(PCB_Q* Qu){
    PCB* dom;
    while (Qu->head != NULL){
          dom = PCB_DEQ(Qu);
          free_env_queue(dom->receive_msg_Q);
          free(dom->SP);
          free(dom->state);
          free(dom->p);
          free(dom);
    }
    free(Qu);
}
   */       

// *** KERNEL SEND MESSAGE ***
int k_send_message(int dest_id, msg_env *e) {
    printf("You're in k_send_message\n");
    // if the env is NULL
    if (e==NULL)                             //is this right?
        return 0;
    
    // if the PCB ID is not valid
    if (dest_id > (TOTAL_NUM_PROC - 1 || dest_id < 0) )  { 
        printf("dest ID not in range\n");
        return 0;
    }
    
    // if the PCB ID is valid
    // set the target_id parameter of the env to dest_id
    e->target_id = dest_id;
    e->sender_id = curr_process->pid;

    // create a pointer to the target PCB using the target_id
    PCB *target = convert_PID(dest_id);

    // if the PID converts correctly to a pointer
    if (target){

        /*unblock the target process if necessary*/
        // if the target's blocked on env
        if (target->state == BLK_ON_ENV) { 

            // enqueue the PCB of the process on the appropriate ready queue
            PCB_ENQ(target, convert_priority(target->priority)); //*****not sure if need to put a '&' before convert_priority
            // set the target state to 'ready'
            target->state = READY;
        }

        // enqueue the env on the target's receive queue
        env_ENQ(e, target->receive_msg_Q);
        
        send_counter++;                                          //increment the counter
        send_end = (send_end + 1) % 16;                          //traverse the end index
        if(send_counter > 15 || send_start < 0)                 //if the counter is greater than 15 (when the array  is full) or start index is -1 (ie first send) 
                        send_start = (send_start + 1) % 16;     //traverse the start index 
        
        send_trace[send_end].sender_id = curr_process->pid;                //set the sender_id
        send_trace[send_end].target_id = dest_id;                          //set the target_id
        send_trace[send_end].msg_type = e->msg_type;         //set the msg_type
        send_trace[send_end].timestamp.hh = systemclock->hh;          //set the timestamp
        send_trace[send_end].timestamp.mm = systemclock->mm;
        send_trace[send_end].timestamp.ss = systemclock->ss;
        
        return 1;
    
    }
    
    // if the PID doesn't convert successfully
    else
        return 0;
     
}

// ***USER SEND MESSAGE***
int send_message(int dest_id, msg_env *e) {
    printf("You're in send_message\n");
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
    printf("You're in k_receive_message\n");
    // if the receive message queue is empty
    if (curr_process->receive_msg_Q->head == NULL) {
        
        // if the process is an iprocess or should never be blocked, return a NULL pointer (ie. no env)
        if (curr_process->priority == -1)
            return NULL;
        
        // if it's a normal process, block it on receive
        curr_process->state = BLK_ON_RCV; //*********Doesn't need to be put in a queue, and don't care about process switch now********* FIX THIS
    //printf("in k_rec before proc switch pid %d, %s\n", curr_process->pid, curr_process->state);
        process_switch(); // Fixed it. Used to be return NULL.
    }
    
    // if the message queue is not empty, dequeue the first message in the queue
    else{
        msg_env* env = env_DEQ(curr_process->receive_msg_Q);           //create a pointer and point it to the dequeued envelope
         
        receive_counter++;                                             //increment the counter
        receive_end = (receive_end + 1) % 16;                          //traverse the end index
        if(receive_counter > 15 || receive_start < 0)                  //if the counter is greater than 15 (when the array  is full) or start index is -1 (ie first send) 
                        receive_start = (receive_start + 1) % 16;     //traverse the start index 
        
        receive_trace[receive_end].sender_id = env->sender_id;               //set the sender_id
        receive_trace[receive_end].target_id = curr_process->pid;           //set the target_id
        receive_trace[receive_end].msg_type = env->msg_type;          //set the msg_type
        receive_trace[receive_end].timestamp.hh = systemclock->hh;          //set the timestamp
        receive_trace[receive_end].timestamp.mm = systemclock->mm;
        receive_trace[receive_end].timestamp.ss = systemclock->ss;
        
        return env;
        }
        //printf("in k_rec after proc switch pid %d, %s\n", curr_process->pid, curr_process->state);
}

// ***USER RECEIVE MESSAGE***
msg_env *receive_message() {
    printf("You're in receive_message\n");
        // turn atomicity on
        atomic_on();
        
        // call the kernel receive message
        msg_env *temp = k_receive_message();
        
        // turn atomicity off
        atomic_off();
        
        // return the pointer to the message envelope
        if (temp)
                return temp;
}

// *** KERNEL SEND CONSOLE CHARS***
int k_send_console_chars(msg_env *env) {
    printf("You're in send_console_chars\n");
    // if the env is NULL
    if (env==NULL)
        return 0;
    
    // relay the env to the crt i-process using send_message
    int Z = k_send_message(1, env);       //call the kernel send message
    
    // if sending fails
    if (Z == 0){
        printf("Error with sending\n");
        return 0;
    }

    return 1;
}

// *** USER SEND MESSAGE CHARS***
int send_console_chars(msg_env *env) {
    printf("You're in send_console_chars\n");
    atomic_on();
    int z = k_send_console_chars(env);
    atomic_off();
    return z;
}


// ***KERNEL GET CONSOLE CHARS***
int k_get_console_chars(msg_env * env) {
    printf("You're in k_get_console_chars\n");
    // if the env is NULL
    if (env==NULL)
        return 0;
    
    int Z;

    // relay the env to the kb i-process using send_message
    Z = k_send_message(0, env);                //call the kernel send message

    // incorrect return from send_message
    if(Z == 0) {
            printf("Error with sending\n");
            return 0;
    }
    
    return 1;
}

// ***USER GET CONSOLE CHARS***
int get_console_chars(msg_env * env) {
    printf("You're in get_console_chars\n");
    atomic_on();
    int z = k_get_console_chars(env);
    atomic_off();
    return z;
}
    


// ***GIVE A PROCESS ID AND RETURN IT'S PCB POINTER***
PCB *convert_PID(int PID) {
    printf("You're in convert_PID\n");
    // if the process ID is invalid
    if (PID > (TOTAL_NUM_PROC + TOTAL_NUM_IPROC - 1) || PID < 0) {
        // printf("invalid ID");
        printf("Invalid PID, cannot continue\n");
        terminate(0);
        }
    
    // if the ID is valid, return the pointer in the PCB array at the value of ID
    return pointer_2_PCB[PID];
}

// ***GET A PROIRITY AND RETURN A POINTER TO THE RPQ***
PCB_Q *convert_priority(int pri) {
    printf("You're in convert_priority\n");
    // if the new priority is not valid (1-3)
    if (pri > 3 || pri < 0) {           
        //printf("Invalid priority!!!!!");
        return NULL;
    }
    
    // if the priority is valid, return the pointer to the priority queue from the array
    return pointer_2_RPQ[pri];
}

// ***KERNEL RELEASE PROCESSOR***
void k_release_processor() {                //should be an int but we'll figure it later
    printf("You're in k_release_processor\n");
        //printf ("curr_process:%i\n", curr_process->pid);
    curr_process->state = READY;  //Change current process state to "ready"
    PCB_ENQ(curr_process, convert_priority(curr_process->priority));       //Enqueue PCB into a rpq
    process_switch();                       //Shari is taking care of process switch.
}    

// ***USER RELEASE PROCESSOR***
void release_processor() {
    printf("You're in release_processor\n"); 
    atomic_on();                //turn atomicity on
     k_release_processor();     //call the kernel function
     atomic_off();              //turn atomic off
}

// ***KERNEL GET ENVELOPE***
msg_env *k_request_msg_env() {
        printf("You're in k_request_msg_env\n");
    while (envelope_q->head == NULL){        //while envelope q is empty
              PCB_ENQ(curr_process, blocked_on_envelope);     //enqueue on blocked_on_env q
              curr_process->state = BLK_ON_ENV;         // change state to blocked on env
              process_switch();
        }
        msg_env *temp = env_DEQ(envelope_q);            //make a temp pointer that points to the dequeued envelope from the free env q
        return temp;
}

//***USER GET ENVELOPE***
msg_env *request_msg_env(){
    printf("You're in request_msg_env\n");    
    atomic_on();
        msg_env *tep = k_request_msg_env();
        atomic_off();
        return tep;
}

//***KERNEL RELEASE ENVELOPE***
int k_release_msg_env(msg_env *env){
    printf("You're in k_release_msg_env\n");
    env_ENQ(env, envelope_q);                      //enqueue the envelope to the envelope queue
    if (blocked_on_envelope->head != NULL){        //if there's a blocked process
       PCB *temp = PCB_DEQ(blocked_on_envelope);   //dequeue the first process from the blocked_on_env and make a temp pointer to the PCB
       temp->state = READY;                //set the process state to ready
       PCB_ENQ(temp, convert_priority(temp->priority));      //ENQ PCB in the appropriate rpq
    }
    return 1;
}

//***USER RELEASE ENV***
int release_msg_env(msg_env *env){
   printf("You're in release_msg_env\n");
    atomic_on();
    int z = k_release_msg_env(env);
    atomic_off();
    return z;
}

//***KERNEL CHANGE PRIOIRTY***
int k_change_priority(int new_priority, int target_process_id){
   printf("You're in k_change_priority\n");
    if(new_priority>3 || new_priority<0){
                      printf("invalid priority\n");
                      return 0;
    }
    if(target_process_id > TOTAL_NUM_PROC || target_process_id < TOTAL_NUM_IPROC || target_process_id == 7){
                           printf("Invalid ID\n");
                           return 0;
    }
    PCB *target = convert_PID(target_process_id);              //create a pointer that points to the PCB
    int old_priority = target->priority;                       //need the old one for later
    if(old_priority == new_priority){
                    printf("Old priority same as new one\n");
                    return 1;                                  //success I guess?
    }
    target->priority = new_priority;                                     //change the priority
    if(target->state == READY){
                     PCB_REMOVE(convert_priority(old_priority), target_process_id);      //remove PCB from old rpq
                     PCB_ENQ(target, convert_priority(new_priority));                    //Enqueue it to the new rpq
                     return 1;
    }
}

//***USER CHANGE PRIORITY***
int change_priority (int new_priority, int target_process_id){
    printf("You're in change_priority\n");
    atomic_on();
    int z = k_change_priority(new_priority, target_process_id);
    atomic_off();
    return z;
}

//***KERENEL GET PROCESS STATUS***
int k_request_process_status(msg_env *env){
    printf("You're in k_request_process_status\n");

    if (env==NULL){                                    //if no envelope was passed
              printf("No envelope was passed\n");
              return 0;
    }
    
    char* temp = (char*) malloc(sizeof(SIZE));    //make an char array and allocate memory
    strcpy(env->msg_text, "proc_id    status   priority \n\n");        //write the headers in the env
    
    int i;
    for(i=0; i<TOTAL_NUM_PROC; i++){
             sprintf(temp, "%i      %i        %i \n", pointer_2_PCB[i]->pid,pointer_2_PCB[i]->state, pointer_2_PCB[i]->priority);//write the id status and priority in temp
             strcat(env->msg_text, temp);                                      //cat temp with the envelope
    }
    
    strcat(env->msg_text, "\0");
    
    int worked = send_console_chars(env);
    return worked;
}

//***USER GET PROCESS STATUS***
int request_process_status(msg_env *env){
    printf("You're in request_process_status\n");
    atomic_on();
    int z = k_request_process_status(env);
    atomic_off();
    return z;
} 

//***KERNEL REQUEST DELAY***
int k_request_delay(int time_delay, int wakeup_code, msg_env *m)
{
    printf("You're in k_request_delay\n");
    int RequestingPID = curr_process->pid;         //Temporary PID holder
    
    curr_process->state = SLEEP; 
    
    m->sender_id = RequestingPID;
    m->target_id = TIMERIPROCPID;                  //Set Target ID to the Timer Iproc
    
    m->msg_type = wakeup_code;       //Set the message type to wakeup code and the text to the delay,
    sprintf(m->msg_text, "%d\0", time_delay);        //in order to send both wakeup code and time delay in one envelope.
    
    k_send_message(TIMERIPROCPID, m);              //Send the envelope to the timer iproc
    m = k_receive_message();                       //Invoke receive message, which blocks the invoking process until delay is over
    if(m)
    {
        PCB *RequestingPCB;                            //After unblocking, this code executes:
        RequestingPCB = convert_PID(RequestingPID);    //Create a PCB pointer to manage stuff for enqueueing, then Find the Delayed PCB
        PCB_ENQ(RequestingPCB, convert_priority(RequestingPCB->priority));    //Enqueue the now awakened process
        return 1;              //SUCCESS!
    }
    else
        return 0;              //The code should never get here
}

//***USER REQUEST DELAY***
int request_delay(int time_delay, int wakeup_code, msg_env *m){
    printf("You're in request_delay\n");
    atomic_on();
    int z = k_request_delay(time_delay, wakeup_code, m);
    atomic_off();
    return z;
}
    

//===============    THE CODE BELOW THIS LINE IS WORKING, BUT DOES NOT FIT THE PROTOTYPE. KEEP AS A BACKUP =======================//
/*
int request_delay(int msecDelay) {

    //int invoketime = systemclock->ss + (systemclock->mm*60) + (systemclock->hh*60*60);   //no longer necessary
    int RequestingPID = curr_process->pid;
    strcpy(curr_process->state, "SLEEP");
    curr_process->sleeptime = msecDelay;
    //copy processor stack into process_PCB
    //release_processor();                               //context switching code, to be added later?
    PCB_ENQ(pointer_2_PCB[RequestingPID], pointer_2_SQ);
}
*/


 /*
// ***KERNEL TERMINATE***
int k_terminate(){
   
  free_env_queue(envelope_q);
  free_env_queue(sleep_Q);
  free_PCB_queue(blocked_on_envelope);
  free_PCB_queue(ready_q_priority3);
  free_PCB_queue(ready_q_priority2);
  free_PCB_queue(ready_q_priority1);
  free_PCB_queue(ready_q_priority0);
  
  int i;
  for(i=0; i<4; i++)
          free(pointer_2_RPQ[i]);
  
  free(pointer_2_SQ);
  
  for(i=0; i < TOTAL_NUM_PROC; i++){
           free_env_queue(pointer_2_PCB[i]->receive_msg_Q);
           free(pointer_2_PCB[i]->state);
           free(pointer_2_PCB[i]->SP);
           free(pointer_2_PCB[i]->p);
           free(pointer_2_PCB[i]);
  }
  
  for(i=0; i<16; i++){
           free(send_trace[i].msg_type);
           free(receive_trace[i].msg_type);
  }
  
  free(systemclock);
  free(wallclock);
  
  free_env_queue(curr_process->receive_msg_Q);
  free(curr_process->state);
  free(curr_process->SP);
  free(curr_process->p);
  free(curr_process);
  
  parent_die();
  
  return 1;
}
*/

// ***KERNEL GET TRACE BUFFERS
int k_get_trace_buffers(msg_env* env){
   printf("You're in k_get_trace_buffers\n");

    if(env==NULL){
             printf("No envelope was passed\n");
             return 0;
    }
    
    strcpy(env->msg_text, "send trace buffer (from oldest to newest) \n No.    sender_id    target_id    msg_type    time stamp    \n\n");
    
    int i;
    int j = 1;
    char* temp = (char*) malloc(sizeof(SIZE));
    
    if(send_counter < 0){
         strcat(env->msg_text, "Send trace buffer empty \n");
    }
    else{
         for(i = send_start; i == send_end; i = (i+1)%16){
                             sprintf(temp, "%i    %i    %i    %i:%i:%i    \n", j, send_trace[i].sender_id, send_trace[i].target_id, send_trace[i].msg_type, send_trace[i].timestamp.hh, send_trace[i].timestamp.mm, send_trace[i].timestamp.ss);
                             strcat(env->msg_text, temp);
                             j++;
         }
    }
    
    j = 1;
    
    strcat(env->msg_text, "\n\n receive trace buffer (from oldest to newest) \n No.    sender_id    target_id    msg_type    time stamp    \n\n");
    
    if(receive_counter < 0){
            strcat(env->msg_text, "Receive trace buffer empty \n");
    }
    else{
         for(i = receive_start; i == receive_end; i = (i+1)%16){
            sprintf(temp, "%i    %i    %i    %i:%i:%i    \n", j, receive_trace[i].sender_id, receive_trace[i].target_id, receive_trace[i].msg_type, receive_trace[i].timestamp.hh, receive_trace[i].timestamp.mm, receive_trace[i].timestamp.ss);
            strcat(env->msg_text, temp);
            j++;
         }
    
         strcat(env->msg_text, "\0");
    return 1;
    }
}

// ***USER GET TRACE BUFFERS
int get_trace_buffers(msg_env* env){
    atomic_on();
    int z = k_get_trace_buffers(env);
    atomic_off();
    return z;
}

 
