// crt_child.c
//
// Send characters to be displayed on screen
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

	// get id of process to signal when we have output
	// and the file id of the memory mapped file
	// i.e. process input arguments 
	sscanf(argv[1], "%d", &parent_pid );
	sscanf(argv[2], "%d", &fid );  // get the file id

	// attach to shared memory so we can pass output to 
	// crt interrupt handler
	
	
	
	
	/*
	c_mmap_ptr = mmap(0,   // Memory Location, 0 lets O/S choose 
		    bufsize,// How many bytes to mmap 
		    PROT_READ | PROT_WRITE, // Read and write permissions 
		    MAP_SHARED,    // Accessible by another process 
		    fid,           // which file is associated with mmap 
		    (off_t) 0);    // Offset in page frame 
    if (c_mmap_ptr == MAP_FAILED){
      printf("Child memory map has failed, CRT is aborting!\n");
	  in_die(0);
    }
    */
	
	out_mem_p = (outputbuf *) c_mmap_ptr; // now we have a shared memory pointer

	// print to CRT
	buf_index = 0;
	out_mem_p->ok_flag = 0; //0 is empty
	char c[40];
    int n = 0;
	do
	{
		if( buf_index < MAXCHAR-1 ) {
			
            c[n]= out_mem_p->outdata[buf_index];
            
			if ( c[n] != '\0' )
					printf("%c", c[n]); 
							 
			else {
					out_mem_p->ok_flag = 1;  //set ready status bit
					kill(parent_pid,SIGUSR2); //send a signal to parent	to start handler to start crt_iproc
					buf_index = 0;  // for now, just restart
					while( out_mem_p->ok_flag == 1)
						usleep(100000);
				}
			buf_index++;
			n++;
    	}
		
	while(1);  //an infinite loop - exit when parent signals us

    } // crt
}
