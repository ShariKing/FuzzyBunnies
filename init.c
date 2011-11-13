// *** CONSTANTS ***
#define TOTAL_NUM_PROC 12 //total number of processes, will change
#define TOTAL_NUM_IPROC 3 //total number of i-processes, will change

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


// *** CREATE PCB QUEUE FUNCTION ***
PCB_Q* create_Q( )
{
      PCB_Q *p;
      p->head = NULL;
      p->tail = NULL;
      return p;
}


// *** CREATE ENV QUEUE FUNCTION ***
env_Q* create_env_Q( )
{
        env_Q *bob;
        bob->head = NULL;
        bob->tail = NULL;
        return bob;
}        


// *** INITIALIZE QUEUES ***
int init_queues( )
{
    // ready queue: Prio = 0
     if(ready_q_priority0 = create_Q())
          printf("Creating Priority Queue 0...");
     
     else {
          printf("Error Creating Priority Queue 0.");
          return 0;
     }
          
     // ready queue: Prio = 1
     if(ready_q_priority1 = create_Q())
          printf("\nCreating Priority Queue 1...");
     
     else {
          printf("Error Creating Priority Queue 1.");
          return 0;
     }
          
     // ready queue: Prio = 2
     if(ready_q_priority2 = create_Q())
          printf("\nCreating Priority Queue 2...");
     
     else {
          printf("Error Creating Priority Queue 2.");
          return 0;
     }
          
     // ready queue: Prio = 3
     if(ready_q_priority3 = create_Q())
          printf("\nCreating Priority Queue 3...");
     
     else {
          printf("Error Creating Priority Queue 3.");
          return 0;
     }
	 
     // Blocked On Receive queue
     //blocked_on_receive = create_Q(); This is not necessary
          
     // Blocked On Envelope queue
     if(blocked_on_envelope = create_env_Q())
          printf("\nCreating Blocked on Envelope Queue...");
     
     else {
          printf("Error Creating Blocked on Envelope Queue.");
          return 0;
     }
          
     // Queue of Free Envelopes
     if(envelope_q = create_env_Q())
          printf("\nCreating Envelope Queue...");
     
     else {
          printf("Error Creating Envelope Queue.");
          return 0;
     }
          
     return 1;
}


// *** INITIALIZE PROCESSES **** (Which ones?)
int init_processes ( )
{
    //read in file for itable
    FILE* itablefile;
    itablefile = fopen("itable.txt", "r");

    int itable[TOTAL_NUM_PROC][2], pid, priority, i;
    for (i = 0; i < TOTAL_NUM_PROC; i++) {
        
        // reading PID
        if (fscanf(itablefile, "%d", &pid))
            itable[i][0] = pid;
        
        else {
            printf("Error, initialization table missing pid for process ", i, ".\n");
            return 0;
        }
        
        // reading Priority
        if (fscanf(itablefile, "%d", &priority))
            itable[i][1] = priority; //ALL PRIORITIES ARE SET TO ZERO FOR NOW. FIX THIS.
        
        else {
            printf("Error, initialization table missing priority for process ", i, ".\n");
            return 0;
        }
    }
    
    //finish reading in file for itable
    fclose(itablefile);
    
    int j;
    for (j = 0; j < TOTAL_NUM_PROC; j++) {
        struct pcb* new_pcb = (struct pcb *) malloc(sizeof (struct pcb));
        new_pcb->pid = itable[i][0];
        new_pcb->priority = itable[i][1];
        new_pcb->SP = NULL; //FOR CONTEXT SWITCHING. TO BE CHANGED LATER.
        //new_pcb->PC = itable[i][2]; // WHAT IS THIS?!
        strcpy(new_pcb->state, "READY"); //This is how you set the state
        new_pcb->p = NULL;
        new_pcb->receive_msg_Q = create_env_Q();
        pointer_2_PCB[TOTAL_NUM_IPROC + j] = new_pcb; //This code creates a pointer to the pcb, based on its pid
        if (new_pcb->priority == 0)
            PCB_ENQ(new_pcb, ready_q_priority0);
        else if (new_pcb->priority == 1)
            PCB_ENQ(new_pcb, ready_q_priority1);
        else if (new_pcb->priority == 2)
            PCB_ENQ(new_pcb, ready_q_priority2);
        else if (new_pcb->priority == 3)
            PCB_ENQ(new_pcb, ready_q_priority3);
        else {
            printf("Error, invalid priority for process ", j, ".\n");
            break;
        }
    }

              /*
// now set up the process context and stack
if (setjmp (kernel_buf) == 0) {  
jmpsp = apcb->proc_stack;    
#ifdef i386
__asm__ ("movl %0,%%esp" :"=m" (jmpsp)); // if Linux i386 target
#endif     //  line 2
#ifdef __sparc
_set_sp( jmpsp ); // if Sparc target (eceunix)
#endif
if (setjmp (apcb->context) == 0)
{
longjmp (kernel_buf, 1);           
}
else
{
void (*tmp_fn) ();                 
tmp_fn = (void *) current_process->start_PC;
tmp_fn (); // process starts for the first time here
*/ 
//WHAT?!
 //   return 0;
   return 1;
}

int init_env()
{
     int i;
     for(i = 0; i < 128; i++)
     {
        struct msgenv* new_env = (struct msgenv *) malloc (sizeof (struct msgenv));
        new_env->p = NULL;
        new_env->sender_id = -1; //setting the id to an int of -1 just for initialize
        new_env->target_id = -1;
        new_env->msg_type ; //initialize it to character array
        new_env->msg_text ;
		env_ENQ(new_env, envelope_q);
      }
      return 1;
}

int init_i_processes()
{
      //finish reading in file for iproctable
     int k; 
     for (k = 0; k < TOTAL_NUM_IPROC; k++)
     {
         struct pcb* new_pcb = (struct pcb *) malloc (sizeof (struct pcb));
         new_pcb->priority = -1;
         new_pcb->SP = NULL; //FOR CONTEXT SWITCHING. TO BE CHANGED LATER.
         new_pcb->pid = k;
         //new_pcb->PC = itable[k][1];
         strcpy(new_pcb->state,"READY"); //This is how you set the state
         new_pcb->p = NULL;
         new_pcb->receive_msg_Q = create_env_Q();
         pointer_2_PCB[k] = new_pcb;             //This code creates a pointer to the pcb, based on its pid
     }   
     return 1;
}


// ***** THE MAIN MAIN MAIN RTOS FUNCTION *****
int main ()
{
	if(init_queues()) 
	     printf("Initialized correctly");
	else
    {
         printf("Error, queue initialization failed!!!\n");
         exit;
    }
    if(init_env())
         printf("Initialized correctly");
	else
    {
         printf("Error, envelope initialization failed!!!\n");
         exit;
    }
    if(init_processes())
	     printf("Initialized correctly");
	else 
	{
         printf("Error, process initialization failed!!!\n");
         exit;
    }
	if (init_i_processes())
	     printf("Initialized correctly");
    else
    {
         printf("Error, i-process initialization failed!!!\n");
         exit;
    }
    curr_process = convert_PID(0);
    strcpy(curr_process->state,"NEVER_BLK_PROC");

    
    
    /************************************************************/
    /*****CODE FROM HERE TO THE BOTTOM WAS TAKEN FROM DEMO.C*****/
   
    // catch signals so we can clean up everything before exiting
    // signals defined in /usr/include/signal.h
    sigset(SIGINT,die);		// catch kill signals 
    sigset(SIGBUS,die);		// catch bus errors
    sigset(SIGHUP,die);		
    sigset(SIGILL,die);		// illegal instruction
    sigset(SIGQUIT,die);
    sigset(SIGABRT,die);
    sigset(SIGTERM,die);
    sigset(SIGSEGV,die);	// catch segmentation faults
    
    sigset(SIGUSR1,kbd_iproc);
    sigset(SIGUSR2,crt_iproc);

    
    /* FORKING THE KEYBOARD  */
    
    /* Create a new mmap file for read/write access with permissions restricted
   to owner rwx access only */
    k_fid = open(k_sfilename, O_RDWR | O_CREAT | O_EXCL, (mode_t) 0755);
    if (k_fid < 0) {
        printf("Bad open of mmap file <%s>\n", k_sfilename);
        exit(0);
    };

    // make the file the same size as the buffer 
    k_status = ftruncate(k_fid, bufsize);
    if (k_status) {
        printf("Failed to ftruncate the file <%s>, status = %d\n", k_sfilename, k_status);
        exit(0);
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
            execl("./keyboard", "keyboard", k_childarg1, k_childarg2, (char *)0);
            // should never reach here
            fprintf(stderr,"Can't exec keyboard, errno %d\n",errno);
            cleanup();
            exit(1);
    };
    // the parent process continues executing here

    // sleep for a second to give the child process time to start
    sleep(1);
    
    // allocate a shared memory region using mmap 
	// the child process also uses this region
	
    k_mmap_ptr = mmap((caddr_t) 0,   /* Memory location, 0 lets O/S choose */
		    bufsize,              /* How many bytes to mmap */
		    PROT_READ | PROT_WRITE, /* Read and write permissions */
		    MAP_SHARED,    /* Accessible by another process */
		    k_fid,           /* the file associated with mmap */
		    (off_t) 0);    /* Offset within a page frame */
    
    if (k_mmap_ptr == MAP_FAILED){
        printf("Parent's memory map has failed, about to quit!\n");
	die(0);  // do cleanup and terminate
    };
	
    in_mem_p = (inputbuf *) k_mmap_ptr;   // pointer to shared memory
      // we can now use 'in_mem_p' as a standard C pointer to access the created shared memory segment

    // now start doing whatever work you are supposed to do
    // in this case, do nothing; only the keyboard handler will do work
    
    in_mem_p->ok_flag = 0;
    
    
    /* FORKING THE CRT  */
    
    /* Create a new mmap file for read/write access with permissions restricted
   to owner rwx access only */
    c_fid = open(c_sfilename, O_RDWR | O_CREAT | O_EXCL, (mode_t) 0755);
    if (c_fid < 0) {
        printf("Bad open of mmap file <%s>\n", c_sfilename);
        exit(0);
    };

    // make the file the same size as the buffer 
    c_status = ftruncate(c_fid, bufsize);
    if (c_status) {
        printf("Failed to ftruncate the file <%s>, status = %d\n", c_sfilename, c_status);
        exit(0);
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
            execl("./crt", "crt", c_childarg1, c_childarg2, (char *)0);
            // should never reach here
            fprintf(stderr,"Can't exec crt, errno %d\n",errno);
            cleanup();
            exit(1);
    };
    // the parent process continues executing here

    // sleep for a second to give the child process time to start
    sleep(1);
    
    // allocate a shared memory region using mmap 
	// the child process also uses this region
	
    c_mmap_ptr = mmap((caddr_t) 0,   /* Memory location, 0 lets O/S choose */
		    bufsize,              /* How many bytes to mmap */
		    PROT_READ | PROT_WRITE, /* Read and write permissions */
		    MAP_SHARED,    /* Accessible by another process */
		    c_fid,           /* the file associated with mmap */
		    (off_t) 0);    /* Offset within a page frame */
    
    if (c_mmap_ptr == MAP_FAILED){
        printf("Parent's memory map has failed, about to quit!\n");
	die(0);  // do cleanup and terminate
    };
	
    out_mem_p = (outputbuf *) c_mmap_ptr;   // pointer to shared memory
      // we can now use 'in_mem_p' as a standard C pointer to access the created shared memory segment

    // now start doing whatever work you are supposed to do
    // in this case, do nothing; only the keyboard handler will do work
    
    out_mem_p->oc_flag = 0;
    
    
    /*** BACK TO MAIN RTOS STUFF ***/
    
    printf("\nType something folowed by end-of-line and it will be echoed by the superbly awesome Process P\n\n");
    while (1);

    // should never reach here, but in case we do, clean up after ourselves
    cleanup();
    exit(1);
    
    //The following functions are not necessary for the partial implementation, but will be for the full.
    //allocate system_clock = (struct clock *) malloc (sizeof (struct clock))
    //system_clock.ss = 0
    //system_clock.mm = 0
    //system_clock.hh = 0
	//release_processor( )
	//process_switch( )
}	
