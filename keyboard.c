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
#include "kbcrt.h"

// ***KEYBOARD UART SIMULATION***
int main(int argc, char * argv[]) {
    // if parent tells us to terminate, then clean up first
    sigset(SIGINT, in_die);

    // get id of process to signal when we have input and the file id of the memory mapped file
    sscanf(argv[1], "%d", &parent_pid);
    sscanf(argv[2], "%d", &k_fid); // get the file id

    // attach to shared memory so we can pass input to keyboard interrupt handler
    k_mmap_ptr = mmap(0, // Memory Location, 0 lets O/S choose 
            bufsize, // How many bytes to mmap 
            PROT_READ | PROT_WRITE, // Read and write permissions 
            MAP_SHARED, // Accessible by another process 
            k_fid, // which file is associated with mmap 
            (off_t) 0); // Offset in page frame 

    // if the pointer to the memory sucks
    if (k_mmap_ptr == MAP_FAILED) {
        printf("Child memory map has failed, KB is aborting!\n");
        in_die(0);
    }

    // create a shared memory pointer
    in_mem_p = (inputbuf *) k_mmap_ptr;

    // initialize stuff
    buf_index = 0;
    in_mem_p->ok_flag = 0;
    char c[MAXCHAR];
    int n = 0;

    // read from keyboard, infinite loop - exit when parent signals us
    do {
        c[n] = getchar();
        if (c != '\0') {

            if (buf_index < MAXCHAR - 1) {
                in_mem_p->indata[ buf_index++ ] = c[n];
            }

                // when we reach the null character
            else {
                in_mem_p->indata[buf_index] = '\0';

                // reset flag and array start point
                in_mem_p->ok_flag = 1;
                buf_index = 0;

                //send a signal to parent to start handler to start kbd_iproc
                kill(parent_pid, SIGUSR1);

                // infinite cycle until the buffer has been emptied
                while (in_mem_p->ok_flag == 1)
                    usleep(100000);
            }
        }

        n++;
    } while (1);

}
