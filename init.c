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
#include "kbcrt.h"

init_table itable[TOTAL_NUM_PROC + TOTAL_NUM_IPROC];

// *** FUNCTION TO CLEAN UP PARENT PROCESSES***
void terminate(int signal)
{
    printf("You're in terminate\n");
    cleanup();
    printf("\n\nSignal %i Received.   Leaving RTOS ...\n", signal);
    exit(0);
}

// ***CLEAN UP ROUTINE***
// routine to clean up things before terminating main program
// This stuff must be cleaned up or we have child processes and shared
// memory hanging around after the main process terminates
void cleanup() {
    printf("You're in cleanup\n");
    // terminate child process(es)
    kill(in_pid, SIGINT);
    kill(out_pid, SIGINT);

    //******* KEYBOARD ********
    // remove shared memory segment and do some standard error checks
    k_status = munmap(k_mmap_ptr, k_bufsize);
    if (k_status == -1) {
        printf("Bad Keyboard munmap during cleanup\n");
    }
    
    // close the temporary mmap file 
    k_status = close(k_fid);
    if (k_status == -1) {
        printf("Bad close of temporary Keyboard mmap file during cleanup\n");
    };
    
    // unlink (i.e. delete) the temporary mmap file
    k_status = unlink(k_sfilename);
    if (k_status == -1) {
        printf("Bad unlink during Keyboard clean up\n");
    }
    
    //******* CRT ********
    // remove shared memory segment and do some standard error checks
    c_status = munmap(c_mmap_ptr, c_bufsize);
    if (c_status == -1) {
        printf("Bad CRT munmap during clean up\n");
    }
    
    // close the temporary mmap file 
    c_status = close(c_fid);
    if (c_status == -1) {
        printf("Bad close of temporary CRT mmap file during cleanup\n");
    };
    
    // unlink (i.e. delete) the temporary mmap file
    c_status = unlink(c_sfilename);
    if (c_status == -1) {
        printf("Bad unlink during CRT cleanup.\n");
    }
}

// *** CREATE PCB QUEUE FUNCTION ***
PCB_Q* create_Q( )
{
      // create the queue pointer
      PCB_Q *p;
      p = (struct pcbq *) malloc(sizeof (struct pcbq));
      
      // if the pointer is not NULL
      if (p){
        p->head = NULL;
        p->tail = NULL;
      }
        return p;     
}


// *** CREATE ENV QUEUE FUNCTION ***
env_Q* create_env_Q( )
{
      // create the queue pointer
      env_Q *bob;
      bob = (struct envQ *) malloc(sizeof (struct envQ));
      
      // if the pointer is not NULL
      if (bob){
        bob->head = NULL;
        bob->tail = NULL;
      }
        return bob;
}

// *** INITIALIZE QUEUES ***
int init_queues( )
{
    // ready queue: Prio = 0
     ready_q_priority0 = create_Q();
     
     if(ready_q_priority0){
          printf("Priority Queue 0 Created\n");
          pointer_2_RPQ[0] = ready_q_priority0;
     }
     
     else {
          printf("Error Creating Priority Queue 0\n");
          return 0;
     }
          
     // ready queue: Prio = 1
     ready_q_priority1 = create_Q();
     
     if(ready_q_priority1){
          printf("Priority Queue 1 Created\n");
          pointer_2_RPQ[1] = ready_q_priority1;
     }
     
     else {
          printf("Error Creating Priority Queue 1\n");
          return 0;
     }
          
     // ready queue: Prio = 2
     ready_q_priority2 = create_Q();
     
     if(ready_q_priority2){
          printf("Priority Queue 2 Created\n");
          pointer_2_RPQ[2] = ready_q_priority2;
     }
     
     else {
          printf("Error Creating Priority Queue 2\n");
          return 0;
     }
          
     // ready queue: Prio = 3
     ready_q_priority3 = create_Q();
     
     if(ready_q_priority3){
          printf("Priority Queue 3 Created\n");
          pointer_2_RPQ[3] = ready_q_priority3;
     }
     
     else {
          printf("Error Creating Priority Queue 3\n");
          return 0;
     }
     
          // Sleep Queue
     env_Q* sleep_Q = create_env_Q();
     
     if(sleep_Q){
          printf("Sleep Queue Created\n");
          //pointer_2_SQ = sleep_Q;
     }
     
     else {
          printf("Error Creating Sleep Queue \n");
          return 0;
     }
     
     // Blocked On Receive queue
     PCB_Q* blocked_on_receive = create_Q(); //This is not necessary
     
     if(blocked_on_receive)
          printf("Blocked on Receive Queue Created\n");
     
     else {
          printf("Error Creating Blocked on Envelope Queue\n");
          return 0;
     }
          
     // Blocked On Envelope queue
     blocked_on_envelope = create_Q();               //PCB queue
     
     if(blocked_on_envelope)
          printf("Blocked on Envelope Queue Created\n");
     
     else {
          printf("Error Creating Blocked on Envelope Queue\n");
          return 0;
     }
          
     // Free Envelope Queue
     envelope_q = create_env_Q();
     
     if(envelope_q)
          printf("Free Envelope Queue Created\n");
     
     else {
          printf("Error Creating Envelope Queue\n");
          return 0;
     }
          
     return 1;
}

int init_msg_trace(){
    int i;
    for(i=0; i<16; i++){
        
             send_trace[i].msg_type = 3;
             send_trace[i].sender_id = -1;
             send_trace[i].target_id = -1;
             send_trace[i].timestamp.hh = -1;
             send_trace[i].timestamp.mm = -1;
             send_trace[i].timestamp.ss = -1;
             
             receive_trace[i].msg_type = 3;
             receive_trace[i].sender_id = -1;
             receive_trace[i].target_id = -1;
             receive_trace[i].timestamp.hh = -1;
             receive_trace[i].timestamp.mm = -1;
             receive_trace[i].timestamp.ss = -1;
    }
    
    send_counter = -1;                //so that the first send/receive will set it to 0;
    send_start = -1;
    send_end = -1;
    receive_counter = -1;
    receive_start = -1;
    receive_end = -1;
}


int init_env()
{
     int i;
     for(i = 0; i < 128; i++)
     {
        // create temp env to be enqueued
        struct msgenv* new_env = (struct msgenv *) malloc (sizeof (struct msgenv));
        
        // if the new_env is not created properly
        if (new_env==NULL)
            return 0;
        
        // initialize env parameters
        new_env->p = NULL;
        new_env->sender_id = -1; //setting the id to an int of -1 just for initialize
        new_env->target_id = -1; //setting the id to an int of -1 just for initialize
        new_env->msg_type = 3;

        
        char* tempMsgText = (char *) malloc (sizeof (SIZE)); //initialize the character array pointer

        // if the msg_text pointer is not created properly
        if (tempMsgText==NULL)
            return 0;
            
        strcpy(tempMsgText, "no_text\0");
        new_env->msg_text = tempMsgText;
        
        // enqueue the new env on the free env queue
        env_ENQ(new_env, envelope_q);
      }
     
      // if everything worked okay
      return 1;
}

// *** INITIALIZE Non-I PROCESSES ****
int init_processes ( )
{
    jmp_buf kernel_buf;
       itable[3].pid = 3;
       itable[3].priority = 2;
       itable[3].stack_size =STACKSIZE;
       itable[3].process_type = 1; //user_process is type 1
       itable[3].address = &ProcessA;

       itable[4].pid = 4;
       itable[4].priority = 2;
       itable[4].stack_size =STACKSIZE;
       itable[4].process_type = 1;
       itable[4].address = &ProcessB;
       
       itable[5].pid = 5;
       itable[5].priority = 1;
       itable[5].stack_size =STACKSIZE;
       itable[5].process_type = 1;
       itable[5].address = &ProcessC;
        
       itable[6].pid = 6;
       itable[6].priority = 0;
       itable[6].stack_size =STACKSIZE;
       itable[6].process_type = 1;  
       itable[6].address = &CCI;

       itable[7].pid = 7; 
       itable[7].priority = 3;
       itable[7].stack_size =STACKSIZE;
       itable[7].process_type = 2; // null proc is type 2 
       itable[7].address = &null_process;

               
    //curr_process = NULL;          //Initialize the current process to be null
    /*
    // read in file for itable
    FILE* itablefile;
    itablefile = fopen("itable.txt", "r");

    // initialize variables used for reading from table
    int itable[TOTAL_NUM_PROC][2] = {0};
    int pid = 3;
    int priority = 3;
    int proc_count = 3;
    int i = 0;
   
    
    // setting up PIDs and Prioritys
    for (i = 0; i < TOTAL_NUM_PROC; i++) {
        
        // reading PID
        if (fscanf(itablefile, "%d", &pid)){
        
            itable[i][0] = pid;
        }
        else {
            printf("Error, initialization table missing PID for process ", i, "\n");
            return 0;
        }


        // reading Priority
        if (fscanf(itablefile, "%d", &priority))
            itable[i][1] = priority; //ALL PRIORITIES ARE SET TO ZERO FOR NOW. FIX THIS.
        
        else {
            printf("Error, initialization table missing Priority for process \n", i, "\n");
            return 0;
        }

   }
   
    
    // close file for itable
    fclose(itablefile);
*/
    
    int j;
    for (j = TOTAL_NUM_IPROC; j < (TOTAL_NUM_IPROC + TOTAL_NUM_PROC); j++) {
        
        // create temp pcb struct to put on appropriate queue
        struct pcb* new_pcb = (struct pcb *) malloc(sizeof (struct pcb));
        
        if (new_pcb){
        
            //create tempState and malloc size of char array
           char* tempStack = (char*) malloc(sizeof (itable[j].stack_size));
           
           //return 0 if it didn't malloc right
           if (tempStack == NULL) 
              return 0;
                          
            // initialize the 'next' pointer (for queues) to NULL
            new_pcb->p = NULL;

            // set all processes to the READY state
            new_pcb->state = READY;
                        
            // set the PID for the appropriate process from the table
            new_pcb->pid = itable[j].pid;
            
            // set the Priority for the appropriate process from the table
            new_pcb->priority = itable[j].priority;
            
            // set process counter for the appropriate process from the table
            
            new_pcb->PC = itable[j].address; 
          
            new_pcb->sleeptime = -2;
            
            new_pcb->SP = tempStack; 
            
            // initialize the process' receiving queue
            new_pcb->receive_msg_Q = create_env_Q();
            
            // create a pointer to the pcb, based on its PID, and save it in the array
            pointer_2_PCB[j] = new_pcb;


            //-------- From initialization pdf on Ace-----
            //-------- Initializing context of pcbs---------

            if (setjmp(kernel_buf)==0){ // used for first time of initializing context

                char* jmpsp = new_pcb->SP + STACKSIZE;

               //#ifdef __i386__
               __asm__ ("movl %0,%%esp" :"=m" (jmpsp)); // if Linux i386 target
               //#endif // line 2

               if ( setjmp(new_pcb->pcb_buf ) == 0) // if first time
                  longjmp(kernel_buf,1); 
               
               else{                                  
                // curr_process = new_pcb; // sets the new pcb to be the current process
                 void (*fpTmp)();
                 (fpTmp) = (void *)curr_process->PC; //gets address of process code
                 fpTmp(); 
                 }
                 
              }
              
            // enqueue the process on the appropriate ready queue
            if (new_pcb->priority == 0)
                PCB_ENQ(new_pcb, ready_q_priority0);
            
            else if (new_pcb->priority == 1)
                PCB_ENQ(new_pcb, ready_q_priority1);
            
            else if (new_pcb->priority == 2)
                PCB_ENQ(new_pcb, ready_q_priority2);
            
            else if (new_pcb->priority == 3)
                PCB_ENQ(new_pcb, ready_q_priority3);
            
            else {
                printf("Error, invalid priority for process %d w/ priority %d \n", j, new_pcb->priority);
                return 0;
            }
        }
        
        // if the PCB pointer is created as NULL
        else{
            return 0;
        }
        
    }
   
   // if we get here, success!
   return 1;
}

int init_i_processes()
{
     int k;
     jmp_buf kernel_buf;
     
       itable[0].pid = 0;
       itable[0].priority = -1;
       itable[0].stack_size =STACKSIZE;
       itable[0].process_type = 0; //i process type is 0
       itable[0].address = &kbd_iproc;
       
       itable[1].pid = 1;
       itable[1].priority = -1;
       itable[1].stack_size =STACKSIZE;
       itable[1].process_type = 0;
       itable[1].address = &crt_iproc;
       
       itable[2].pid = 2;
       itable[2].priority = -1;
       itable[2].stack_size =STACKSIZE;
       itable[2].process_type = 0;
       itable[2].address = &timer_iproc;
       
     for (k = 0; k < TOTAL_NUM_IPROC; k++)
     {
         struct pcb* new_pcb = (struct pcb *) malloc (sizeof (struct pcb));
         char* tempStack = (char*) malloc(sizeof (STACKSIZE));
         
         // if the PCB pointer is cool
         if (new_pcb && tempStack){
             
             new_pcb->p = NULL;
             
             new_pcb->state = READY;
                          
             new_pcb->pid = itable[k].pid;
             
             new_pcb->priority = itable[k].priority;
             
             new_pcb->PC = itable[k].address;
             
             new_pcb->sleeptime = -2;
             
             new_pcb->SP = tempStack; //FOR CONTEXT SWITCHING. TO BE CHANGED LATER.
             
             new_pcb->receive_msg_Q = create_env_Q();
             
             pointer_2_PCB[k] = new_pcb;//This code creates a pointer to the pcb, based on its pid
             
             if (setjmp(kernel_buf)==0){ // used for first time of initializing context

                 char* jmpsp = new_pcb->SP +STACKSIZE;
    
                 //#ifdef __i386__
                 __asm__ ("movl %0,%%esp" :"=m" (jmpsp)); // if Linux i386 target
                 //#endif // line 2
    
                 if ( setjmp(new_pcb->pcb_buf ) == 0) // if first time
                    longjmp(kernel_buf,1); 
    
                 else{ 
                    printf("MADE IT HERE");                                 
                    // curr_process = new_pcb; // sets the new pcb to be the current process
                    void (*fpTmp)();
                    (fpTmp) = (void *)curr_process->PC; //gets address of process code
                    fpTmp(); 
                 }
                 
            }
         }
         
         // if the PCB pointer needs to go back to school
         else
             return 0;
     }
     
     // if everything jives
     return 1;
}
        
void kb_crt_start(){
    /* FORKING THE KEYBOARD  */
  
    /* Create a new mmap file for read/write access with permissions restricted
   to owner rwx access only */
    k_fid = open(k_sfilename, O_RDWR | O_CREAT | O_EXCL, (mode_t) 0755);
    if (k_fid < 0) {
        printf("Bad open of mmap file <%s> %i\n", k_sfilename, k_fid);
        terminate(0);
    };

    // make the file the same size as the buffer 
    k_status = ftruncate(k_fid, k_bufsize);
    if (k_status) {
        printf("Failed to ftruncate the file <%s>, status = %d\n", k_sfilename, k_status);
        terminate(0);
    }
   
    // pass parent's process id and the file id to child
    char k_childarg1[20], k_childarg2[20]; // arguments to pass to child process(es)
    int k_mypid = getpid(); // get current process pid
    sprintf(k_childarg1, "%d", k_mypid); // convert to string to pass to child
    sprintf(k_childarg2, "%d", k_fid);   // convert the file identifier
    
    // create the keyboard reader process
    // fork() creates a second process identical to the current process,
    // except that the "parent" process has in_pid = new process's ID,
    // while the new (child) process has in_pid = 0.
    // After fork(), we do execl() to start the actual child program.
    // (see the fork and execl man pages for more info)
    
    in_pid = fork();
    if (in_pid == 0)	// is this the child process ?
    {
            execl("./kbd_child", "keyboard", k_childarg1, k_childarg2, (char *)0);
            // should never reach here
            fprintf(stderr,"Can't exec keyboard, errno %d\n",errno);
            cleanup();
            terminate(0);
    };
    // the parent process continues executing here

    // sleep for a second to give the child process time to start
    sleep(1);
    
    // allocate a shared memory region using mmap 
    // the child process also uses this region

    k_mmap_ptr = mmap((caddr_t)0,   // Memory location, 0 lets O/S choose
		    k_bufsize + 1,              // How many bytes to mmap -> size + 1 cause flag is at the end
		    PROT_READ | PROT_WRITE, // Read and write permissions
		    MAP_SHARED,    // Accessible by another process
		    k_fid,           // the file associated with mmap
		    (off_t) 0);    // Offset within a page frame

    if (k_mmap_ptr == MAP_FAILED){
        printf("Parent's memory map has failed, about to quit!\n");
	terminate(0);  // do cleanup and terminate
    };

    // create the shared memory pointer
    in_mem_p = (struct inbuf *) malloc(sizeof (struct inbuf));

    // if the pointer is created successfully
    if (in_mem_p){

        in_mem_p->indata = (char *) k_mmap_ptr;
          // pointer to shared memory
          // we can now use 'in_mem_p' as a standard C pointer to access the created shared memory segment

        // now start doing whatever work you are supposed to do
        // in this case, do nothing; only the keyboard handler will do work

        // link the flag to the end of the buffer and set it 
        in_mem_p->ok_flag = &in_mem_p->indata[k_bufsize + 1];
        *in_mem_p->ok_flag = 0;
     }
    
    // if the shared memory pointer is stupid
    else
        printf("KB shared memory pointer not initialized\n");
    
    // FORKING THE CRT  
    
    // Create a new mmap file for read/write access with permissions restricted to owner rwx access only 
   
    c_fid = open(c_sfilename, O_RDWR | O_CREAT | O_EXCL, (mode_t) 0755);
    if (c_fid < 0) {
        printf("Bad open of mmap file <%s>\n", c_sfilename);
        terminate(0);
    };

    // make the file the same size as the buffer 
    c_status = ftruncate(c_fid, c_bufsize);
    if (c_status) {
        printf("Failed to ftruncate the file <%s>, status = %d\n", c_sfilename, c_status);
        terminate(0);
    }
    
    // pass parent's process id and the file id to child
    char c_childarg1[20], c_childarg2[20]; // arguments to pass to child process(es)
    int c_mypid = getpid(); // get current process pid
    sprintf(c_childarg1, "%d", c_mypid); // convert to string to pass to child
    sprintf(c_childarg2, "%d", c_fid);   // convert the file identifier
    
    // create the CRT output process
    // fork() creates a second process identical to the current process,
    // except that the "parent" process has in_pid = new process's ID,
    // while the new (child) process has in_pid = 0.
    // After fork(), we do execl() to start the actual child program.
    // (see the fork and execl man pages for more info)
    out_pid = fork();
    if (out_pid == 0)	// is this the child process ?
    {
            execl("./crt_child", "crt", c_childarg1, c_childarg2, (char *)0);
            // should never reach here
            fprintf(stderr,"Can't exec crt, errno %d\n",errno);
            cleanup();
            terminate(0);
    };
    // the parent process continues executing here

    // sleep for a second to give the child process time to start
    sleep(1);
    
    // allocate a shared memory region using mmap 
	// the child process also uses this region

    c_mmap_ptr = mmap((caddr_t)0,   // Memory location, 0 lets O/S choose 
		    c_bufsize + 1,              // How many bytes to mmap 
		    PROT_READ | PROT_WRITE, // Read and write permissions
		    MAP_SHARED,    // Accessible by another process 
		    c_fid,           // the file associated with mmap 
		    (off_t) 0);    // Offset within a page frame 

    if (c_mmap_ptr == MAP_FAILED){
        printf("Parent's memory map has failed, about to quit!\n");
	terminate(0);  // do cleanup and terminate
    };
	
    // create the shared memory pointer
    out_mem_p = (struct outbuf *) malloc(sizeof (struct outbuf));

    // if the pointer rocks out
    if (out_mem_p){

        out_mem_p->outdata = (char *) c_mmap_ptr;   // pointer to shared memory
      // we can now use 'in_mem_p' as a standard C pointer to access the created shared memory segment

    // now start doing whatever work you are supposed to do
    // in this case, do nothing; only the keyboard handler will do work
    
    // link the flag to the end of the shared memory and set it
    out_mem_p->oc_flag = &out_mem_p->outdata[c_bufsize + 1];
    *out_mem_p->oc_flag = 0;
    }
    
    // if the shared pointer needs help
    else
        printf("CRT shared memory pointer not initialized\n");
}

int init_clocks(){
     //Allocate memory for the systemclock structure
        systemclock = (struct clock *) malloc (sizeof (struct clock));
        //Allocate memory for the wallclock structure
        wallclock = (struct clock *) malloc (sizeof (struct clock));
        
        //Initialize and set the wallclock to 0
        if(wallclock) {
             wallclock->ss = 0;
             wallclock->mm = 0;
             wallclock->hh = 0;
        }
        else 
            return 0;
        
        //Initialize and set the wallclock to 0
        if(systemclock) {
             systemclock->ss = 0;
             systemclock->mm = 0;
             systemclock->hh = 0;
             }
        else {
            return 0;
        }
        
        return 1;        
     }

/*******************************************************************/
// ***** THE MAIN MAIN MAIN RTOS FUNCTION *****
int main ()
{
        // if init_clocks returns 1
        if (init_clocks())
             printf("Wall clock & System clock created successfully\n");
        // if init_clocks returns 0
         else {
             printf("Error, wallclock or systemclock initialization failed!!!\n");
             terminate(0);
         }
        
        // if init_queues returned 1
        if (init_queues())
                printf("Initialized queues correctly\n");        
        // if init_queues dropped the ball
        else {
                printf("Error, queue initialization failed!!!\n");
                terminate(0);
        }
        
        // if init_env returned 1
        if (init_env())
                printf("Initialized envelopes correctly\n");        
        // if init_env is dumb
        else {
                printf("Error, envelope initialization failed!!!\n");
                terminate(0);
        }
       
        // if init_i_processes returned 1
        if (init_i_processes())
                printf("Initialized I-processes correctly\n");
        // if init_-_processes failed
        else {
                printf("Error, i-process initialization failed!!!\n");
                terminate(0);
        }
        
        // if init_processes returned 1
        if (init_processes())
                printf("Initialized processes correctly\n");
        // if init_processes failed
        else {
                printf("Error, process initialization failed!!!\n");
                terminate(0);
        }

    
        // set the current process to the NULL process
        // ***** should be the first process in the first ready queue? does that mean we can just hardcode this since it will be the same every time?
        curr_process = convert_PID(7);
        
        curr_process->state = RUNNING;
        
        // if init_msg_trace returned 1
        if (init_msg_trace())
                printf("Initialized msg_trace correctly\n");
        // if init_msg_trace failed
        else {
                printf("Error, msg_trace initialization failed!!!\n");
                terminate(0);
        }
        // *****CODE FROM HERE TO THE BOTTOM WAS TAKEN FROM DEMO.C*****

        // catch signals so we can clean up everything before exiting
        // signals defined in /usr/include/signal.h
        sigset(SIGINT,terminate);	// catch kill signals 
        sigset(SIGBUS,terminate);	// catch bus errors
        sigset(SIGHUP,terminate);		
        sigset(SIGILL,terminate);	// illegal instruction
        sigset(SIGQUIT,terminate);
        sigset(SIGABRT,terminate);
        sigset(SIGTERM,terminate);
        sigset(SIGSEGV,terminate);	// catch segmentation faults
        sigset(SIGUSR1,kbd_iproc);
        sigset(SIGUSR2,crt_iproc);
        sigset(SIGALRM,timer_iproc); //Catch clock ticks

        
        // INITIALIZE KB AND CRT
        kb_crt_start();
        
        //set a repeating alarm to send SIGALRM every 100000 usec, or 0.1sec
        int alarmstatus = ualarm(100000, 100000);
        //ualarm returns zero if functioning normally, otherwise return an error message
        if(alarmstatus != 0)
             printf("Error: Something is wrong with the system timer!\n");
        

      

        //*** BACK TO MAIN RTOS STUFF ***

        // code to say we've started!!
        //printf("Proc A Ready! Waiting to go!\n");
        longjmp(curr_process->pcb_buf,1);
        sleep(1000000);
        //ClockTest(systemclock); // remove later?
        
        // should never reach here, but in case we do, clean up after ourselves
        terminate(0);
}	
