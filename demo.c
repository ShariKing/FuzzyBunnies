// demo.c
//
// demo program for MTE 241
// demonstrates	use of signals and setting up shared memory
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/*  modified June 5, 2007 by P. Dasiewicz
    modified May 6, 2009 by P. Dasiewicz

	The example was modified to remove the traditional UNIX approach of using
	shared memory get/attach since this was suddenly causing problems on eceunix.
	
	It has been modified to use the POSIX-style using functions mmap etc. to
	defined a memory mapped file which can then be used as a shared memory
	segment (between multiple processes) and accessed by standard C-type pointers.
	Basically, behaves exactly the same as the traditional UNIX shared memory.
	
	
*/
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#include <stdlib.h>
#include <signal.h>
#include <unistd.h>			// getpid() definition
#include <kbcrt.h>
#include <rtx.h>

#include <fcntl.h>

#include <sys/mman.h>
#include <sys/wait.h>
#include <stdio.h>



// globals
inputbuf  * in_mem_p;		// pointer to structure that is the shared memory
int 	in_pid;				// pid of keyboard child process
caddr_t mmap_ptr;
int bufsize = BUFFERSIZE;
int fid, status;		//used to create the shared memory

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
char * sfilename = "junkDemo";  //the name of the shared_memory file
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//**************************************************************************
// routine to clean up things before terminating main program
// This stuff must be cleaned up or we have child processes and shared
//	memory hanging around after the main process terminates

void cleanup()
{

	

	// terminate child process(es)
	kill(in_pid,SIGINT);

	// remove shared memory segment and do some standard error checks
	status = munmap(mmap_ptr, bufsize);
    if (status == -1){
      printf("Bad munmap during cleanup\n");
    }
	// close the temporary mmap file 
    status = close(fid); 
    if (status == -1){
      printf("Bad close of temporary mmap file during cleanup\n");
    };
	// unlink (i.e. delete) the temporary mmap file
    status = unlink(sfilename); 
    if (status == -1){
      printf("Bad unlink during claeanup.\n");
    }
}


//**************************************************************************
// routine to call before exitting
// This routine gets called when certain signals occur

void die(int signal)
{
	cleanup();
	printf( "\n\nSignal Received.   Leaving demo ...\n" );
	exit(0);
}

// kbd_handler
// Called by signal SIGUSR1 from keyboard reader process


void kbd_handler(int signum)
{
	inputbuf command;	
	
	// copy input buffer
	if (in_mem_p->indata[0] != '\0')
	{
	    strcpy(command.indata,in_mem_p->indata); 

	    // we should parse the input string and execute the command given,
	    //  but for now we just echo the input
	    // 
	    printf("Keyboard input was: %s\n",command.indata);
	    in_mem_p->ok_flag = 0;  // tell child that the buffer has been emptied
	    
	}

}

//**************************************************************************
int main()
{

	
	// catch signals so we can clean up everything before exitting
	// signals defined in /usr/include/signal.h
	// e.g. when we recieved an interrupt signal SIGINT, call die()
	sigset(SIGINT,die);		// catch kill signals 
	sigset(SIGBUS,die);		// catch bus errors
	sigset(SIGHUP,die);		
	sigset(SIGILL,die);		// illegal instruction
	sigset(SIGQUIT,die);
	sigset(SIGABRT,die);
	sigset(SIGTERM,die);
	sigset(SIGSEGV,die);	// catch segmentation faults

        

	// signal from keyboard reader is SIGUSR1 (user-defined signal)
	// When there is input from the keyboard, call the kbd_handler() routine
	sigset(SIGUSR1,keyboard);	
	
	
  /* Create a new mmap file for read/write access with permissions restricted
     to owner rwx access only */
  fid = open(sfilename, O_RDWR | O_CREAT | O_EXCL, (mode_t) 0755 );
  if (fid < 0){
    printf("Bad Open of mmap file <%s>\n", sfilename);
	exit(0);
  };

  // make the file the same size as the buffer 
  status = ftruncate(fid, bufsize ); 
  if (status){
      printf("Failed to ftruncate the file <%s>, status = %d\n", sfilename, status );
      exit(0);
  }


	// pass parent's process id and the file id to child
	char childarg1[20], childarg2[20]; // arguments to pass to child process(es)
	int mypid = getpid();			// get current process pid
  
	sprintf(childarg1, "%d", mypid); // convert to string to pass to child
    sprintf(childarg2, "%d", fid);   // convert the file identifier
  

	// create the keyboard reader process
	// fork() creates a second process identical to the current process,
	// except that the "parent" process has in_pid = new process's ID,
	// while the new (child) process has in_pid = 0.
	// After fork(), we do execl() to start the actual child program.
	// (see the fork and execl man pages for more info)

	in_pid = fork();
	if (in_pid == 0)	// is this the child process ?
	{
		execl("./keyboard", "keyboard", childarg1, childarg2, (char *)0);
		// should never reach here
		fprintf(stderr,"demo: can't exec keyboard, errno %d\n",errno);
		cleanup();
		exit(1);
	};
	// the parent process continues executing here

	// sleep for a second to give the child process time to start
	sleep(1);

	// allocate a shared memory region using mmap 
	// the child process also uses this region
	
    mmap_ptr = mmap((caddr_t) 0,   /* Memory location, 0 lets O/S choose */
		    bufsize,              /* How many bytes to mmap */
		    PROT_READ | PROT_WRITE, /* Read and write permissions */
		    MAP_SHARED,    /* Accessible by another process */
		    fid,           /* the file associated with mmap */
		    (off_t) 0);    /* Offset within a page frame */
    if (mmap_ptr == MAP_FAILED){
      printf("Parent's memory map has failed, about to quit!\n");
	  die(0);  // do cleanup and terminate
    };
	
	in_mem_p = (inputbuf *) mmap_ptr;   // pointer to shared memory
	  // we can now use 'in_mem_p' as a standard C pointer to access 
	  // the created shared memory segment 
	
	
	
	

	// now start doing whatever work you are supposed to do
	// in this case, do nothing; only the keyboard handler will do work
	in_mem_p->ok_flag = 0;
	printf("\nType something folowed by end-of-line and it will be echoed\n\n");
	while (1);

	// should never reach here, but in case we do, clean up after ourselves
	cleanup();
	exit(1);
} // main
