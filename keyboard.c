// kbd_child.c
//
// Keyboard Reader - emulate a hardware interrupt
// read the keyboard and signal the parent process when a key is received
//
//+++++++++++++++++++++++
// modifed to use the POSIX-style of obtaining shared memory
// by P. Dasiewicz, June 5, 2007
//+++++++++++++++++++++++++

#include <stdio.h>
#include <signal.h>
#include "kbcrt.h"
#include "rtx.h"
#include <fcntl.h>

#include <sys/mman.h>
#include <sys/wait.h>



// do any necessary cleanup before exitting
// ( in this case, there is no cleanup to do)
// Basically, we rely on the parent process to cleanup shared memory
void in_die(int signal)
{
	exit(0);
}


int main (int argc, char * argv[])
{



	// if parent tells us to terminate, then clean up first
	sigset(SIGINT,in_die);

	// get id of process to signal when we have input
	// and the file id of the memory mapped file
	// i.e. process input arguments 
	sscanf(argv[1], "%d", &parent_pid );
	sscanf(argv[2], "%d", &fid );  // get the file id

	// attach to shared memory so we can pass input to 
	// keyboard interrupt handler
	
	
	/*
	mmap_ptr = mmap(0,   // Memory Location, 0 lets O/S choose 
		    bufsize,// How many bytes to mmap 
		    PROT_READ | PROT_WRITE, // Read and write permissions 
		    MAP_SHARED,    // Accessible by another process 
		    fid,           // which file is associated with mmap 
		    (off_t) 0);    // Offset in page frame 
    if (mmap_ptr == MAP_FAILED){
      printf("Child memory map has failed, KB is aborting!\n");
	  in_die(0);
    }
    */
	
	in_mem_p = (inputbuf *) mmap_ptr; // now we have a shared memory pointer

	// read keyboard
	buf_index = 0;
	in_mem_p->ok_flag = 0; //0 is empty
	char c[40];
	int n=0;
	do
	{
		c[n] = getchar();
		if ( c != '\0' ) {
					if( buf_index < MAXCHAR-1 ) {
						in_mem_p->indata[buf_index++] = c;
					} 
				} else {
					in_mem_p->indata[buf_index] = '\0';
					in_mem_p->ok_flag = 1;  //set ready status bit
					kill(parent_pid,SIGUSR1); //send a signal to parent	to start handler to start kbd_iproc
					buf_index = 0;  // for now, just restart
					while( in_mem_p->ok_flag == 1)
						usleep(100000);
				}
				n++;
	}
	while(1);  //an infinite loop - exit when parent signals us

} // keyboard
