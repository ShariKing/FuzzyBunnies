#define TOTAL_NUM_PROC 12 //total number of processes, will change
#define TOTAL_NUM_IPROC 3 //total number of i-processes, will change

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include "rtx.h"
#include <signal.h>

PCB_Q* create_Q( )
{
      PCB_Q *p;
      p->head = NULL;
      p->tail = NULL;
      return p;
}

env_Q* create_env_Q( )
{
        env_Q *bob;
        bob->head = NULL;
        bob->tail = NULL;
        return bob;
}        

int init_queues( )
{
     if(ready_q_priority0 = create_Q())
          printf("Creating Priority Queue 0...");
     else
          printf("Error Creating Priority Queue 0.");
          return 0;
     if(ready_q_priority1 = create_Q())
          printf("\nCreating Priority Queue 1...");
     else
          printf("Error Creating Priority Queue 1.");
          return 0;
     if(ready_q_priority2 = create_Q())
          printf("\nCreating Priority Queue 2...");
     else
          printf("Error Creating Priority Queue 2.");
          return 0;
     if(ready_q_priority3 = create_Q())
          printf("\nCreating Priority Queue 3...");
     else
          printf("Error Creating Priority Queue 3.");
          return 0;
	 //blocked_on_receive = create_Q(); This is not necessary

     if(envelope_q = create_env_Q())
          printf("\nCreating Envelope Queue...");
     else
          printf("Error Creating Envelope Queue.");
          return 0;
     if(blocked_on_envelope = create_env_Q())
          printf("\nCreating Blocked on Envelope Queue...");
     else
          printf("Error Creating Blocked on Envelope Queue.");
          return 0;
          
     return 1;
}

/*int[][] init_table[TOTAL_NUM_PROC][3]( )
{
      ifstream fin("itable");
	  if(!fin)
	  {
	  	printf << "Error, failed to open initialization table.\n";
		return -1;
	  }    
	  
	  int itable[TOTAL_NUM_PROC][3], pid, priority, PC;
      for(int i = 0; i < TOTAL_NUM_PROC; i++) 
      {
      	  if(fin >> pid)
          	  itable[i][0] = pid;
      	  else
      	  {
          	  printf << "Error, initialization table missing pid for process " << i << ".\n";
          	  return -1;
          }
      	  if(fin >> priority)
          	  itable[i][1] = priority;
      	  else
      	  {
          	  printf << "Error, initialization table missing priority for process " << i << ".\n";
          	  return -1;
          }
      	  if(fin >> PC)
          	  itable[i][2] = PC;
      	  else
      	  {
          	  printf << "Error, initialization table missing Program Counter for process " << i << ".\n";
          	  return -1;
         }
      }
      return itable;
}*/
//NOT NECESSARY ANYMORE


int init_processes ( )
{
    //read in file for itable
    FILE* itablefile;
    itablefile = fopen ("itable.txt","r");
    //ifstream fin("itable");
	  /*if(!fin)
	  {
	  	printf << "Error, failed to open initialization table.\n";
		return -1;
	  } */   
	  
	  int itable[TOTAL_NUM_PROC][2], pid, priority, i;
      for(i = 0; i < TOTAL_NUM_PROC; i++) 
      {
      	  if(fscanf(itablefile, "%d", &pid))
          	  itable[i][0] = pid;
      	  else
      	  {
          	  printf("Error, initialization table missing pid for process ", i, ".\n");
          	  return 0;
          }
      	  if(fscanf(itablefile, "%d", &priority))
          	  itable[i][1] = priority; //ALL PRIORITIES ARE SET TO ZERO FOR NOW. FIX THIS.
      	  else
      	  {
          	  printf("Error, initialization table missing priority for process ", i, ".\n");
          	  return 0;
          }
      	  /*if(fin >> PC)
          	  itable[i][2] = PC;
      	  else
      	  {
          	  printf << "Error, initialization table missing Program Counter for process " << i << ".\n";
          	  return -1;
         }*/
      }
      fclose (itablefile);
      //finish reading in file for itable

    int j;
    for (j = 0; j < TOTAL_NUM_PROC; j++)
    {
          struct pcb* new_pcb = (struct pcb *) malloc (sizeof (struct pcb));
          new_pcb->pid = itable[i][0];
          new_pcb->priority = itable[i][1];
          new_pcb->SP = NULL; //FOR CONTEXT SWITCHING. TO BE CHANGED LATER.
          //new_pcb->PC = itable[i][2]; // WHAT IS THIS?!
          strcopy(new_pcb->state,"READY"); //This is how you set the state
          new_pcb->p = NULL;
          new_pcb->receive_msg_Q = create_env_Q();
          pointer_2_PCB[TOTAL_NUM_IPROC + j] = new_pcb;             //This code creates a pointer to the pcb, based on its pid
          if(new_pcb->priority == 0)
              PCB_ENQ(new_pcb,ready_q_priority0);
          else if(new_pcb->priority == 1)
              PCB_ENQ(new_pcb,ready_q_priority1);
          else if(new_pcb->priority == 2)
              PCB_ENQ(new_pcb,ready_q_priority2);
          else if(new_pcb->priority == 3)
              PCB_ENQ(new_pcb,ready_q_priority3);
          else
          {
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
     /*ifstream fin("iproctable");
	  if(!fin)
	  {
	  	printf << "Error, failed to open iprocess initialization table.\n";
		return -1;
	  }    
	  
	  
	  int iproctable[TOTAL_NUM_IPROC][2], pid, PC;
      for(int i = 0; i < TOTAL_NUM_IPROC; i++) 
      {
      	  //if(fin >> pid)
      	  iproctable[i][0] = pid;
      	  /*else
      	  {
          	  printf << "Error, initialization table missing pid for iprocess " << i << ".\n";
          	  return -1;
          }
      	  if(fin >> PC)
          	  iproctable[i][1] = PC;
      	  else
      	  {
          	  printf << "Error, initialization table missing Program Counter for iprocess " << i << ".\n";
          	  return -1;
          }
      }*/
      //Not needed, Iprocess only counts up for pids.
      //finish reading in file for iproctable
     int k; 
     for (k = 0; k < TOTAL_NUM_IPROC; k++)
     {
         struct pcb* new_pcb = (struct pcb *) malloc (sizeof (struct pcb));
         new_pcb->priority = -1;
         new_pcb->SP = NULL; //FOR CONTEXT SWITCHING. TO BE CHANGED LATER.
         new_pcb->pid = k;
         //new_pcb->PC = itable[k][1];
         strcopy(new_pcb->state,"READY"); //This is how you set the state
         new_pcb->p = NULL;
         new_pcb->receive_msg_Q = create_env_Q();
         pointer_2_PCB[k] = new_pcb;             //This code creates a pointer to the pcb, based on its pid
     }
/*     
	 PCB procnum = fork();
	 if(procnum == 0)
         execve("crt_iproc");
     else
     procnum = fork();
     if(procnum == 0)
         execve("kbd_iproc");
     else
     //DO I FORK THESE?!
     return(0);   */      
     return 1;
}


void begin_RTX( )
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
    strcopy(curr_process->state,"NEVER_BLK_PROC");
    //INITIALIZE SIGNAL SYSTEM HERE
/*    
    PCB procnum = fork();
	if(procnum == 0)
        execve("crt_child");
    else
    procnum = fork();
    if(procnum == 0)
        execve("kbd_child");
    else
    ProcessP();
*/
    //The following functions are not necessary for the partial implementation, but will be for the full.
    //allocate system_clock = (struct clock *) malloc (sizeof (struct clock))
    //system_clock.ss = 0
    //system_clock.mm = 0
    //system_clock.hh = 0
	//release_processor( )
	//process_switch( )
}


