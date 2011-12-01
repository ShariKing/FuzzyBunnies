// kbd_child
//
// Keyboard Reader - emulate a hardware interrupt
// read the keyboard and signal the parent process when a key is received
//
//+++++++++++++++++++++++
// modifed to use the POSIX-style of obtaining shared memory
// by P. Dasiewicz, June 5, 2007
//+++++++++++++++++++++++++


#include <stdio.h>
#include <stdlib.h>
//#include <setjmp.h>
#include <signal.h>
//#include <string.h>
//#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/wait.h>
//#include <sys/types.h>
//include <sys/ipc.h>
//#include <sys/shm.h>
//#include <errno.h>
#include "kbcrt.h"

// ***FUNCTION TO CLEAN UP CHILD PROCESSES***
void k_child_die(int signal) {
    //printf("You're in k_child_die\n");
    exit(0);
}

// ***KEYBOARD UART SIMULATION***
int main(int argc, char * argv[]) {
     //printf("You're in keyboard main\n");
    // delay after being forked from parent
    usleep(1000000);
     
    // if parent tells us to terminate, then clean up first
    sigset(SIGINT, k_child_die);
     
    // get id of process to signal when we have input and the file id of the memory mapped file
    sscanf(argv[1], "%d", &parent_pid);
    sscanf(argv[2], "%d", &k_fid); // get the file id
     
    // attach to shared memory so we can pass input to keyboard interrupt handler
    k_mmap_ptr = mmap((caddr_t)0, // Memory Location, 0 lets O/S choose 
            k_bufsize + 1, // How many bytes to mmap 
            PROT_READ | PROT_WRITE, // Read and write permissions 
            MAP_SHARED, // Accessible by another process 
            k_fid, // which file is associated with mmap 
            (off_t) 0); // Offset in page frame 
    
    // if the pointer to the memory sucks
    if (k_mmap_ptr == MAP_FAILED) {
        printf("Child memory map has failed, KB is aborting!\n");
        k_child_die(0);
    }
     
    // create a shared memory pointer
    in_mem_p = (struct inbuf *) malloc(sizeof (struct inbuf));
    
    // the pointer is initialized correctly (ie not NULL)
    if (in_mem_p){
        
        // initialize stuff
        //in_mem_p->indata = (char *) malloc(sizeof (k_bufsize+1));
        in_mem_p->indata = k_mmap_ptr;
        buf_index = 0;
        
        // link the flag to the end of the buffer and set it 
        //in_mem_p->ok_flag = (char *) malloc(sizeof (char));
        in_mem_p->ok_flag = &in_mem_p->indata[k_bufsize];
        *in_mem_p->ok_flag = 0;
        
        // variable to copy input to buffer
        int c;

        // regular running stuff, infinite loop - exit when parent signals us
        do {    
                printf("kb");        
            c = getchar(); //*** SITS HERE AND WAITS FOR INPUT, OKAY

            // if there's still input (ie not NULL)
            if (c != '\n') {

                if (buf_index < k_bufsize + 1) {
                    in_mem_p->indata[ buf_index++ ] = c;
                }
            }
            
            // when we reach the null character
            else {
                in_mem_p->indata[buf_index++] = '\0';

                // reset flag and array start point
                *in_mem_p->ok_flag = 1;
                buf_index = 0;
printf ("flag:%i\n", *in_mem_p->ok_flag);

                //send a signal to parent to start handler to start kbd_iproc
                kill(parent_pid, SIGUSR1);

                // infinite cycle until the buffer has been emptied
                while (*in_mem_p->ok_flag == 1)
                    usleep(100000);
             }

        } while (1);
    }
    
    // if the shared memory pointer is stupid and is NULL
    else {
        printf("KB shared memory pointer not initialized\n");
        exit(0);
    }   
}
