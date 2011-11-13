// crt_child
//
// Send characters to be displayed on screen
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

// ***CRT UART SIMULATION***
int main(int argc, char * argv[]) {
    // if parent tells us to terminate, then clean up first
    sigset(SIGINT, in_die);

    // get id of process to signal when we have output and the file id of the memory mapped file
    sscanf(argv[1], "%d", &parent_pid);
    sscanf(argv[2], "%d", &c_fid); // get the file id

    // attach to shared memory so we can pass output to crt interrupt handler
    c_mmap_ptr = mmap(0, // Memory Location, 0 lets O/S choose 
            bufsize, // How many bytes to mmap 
            PROT_READ | PROT_WRITE, // Read and write permissions 
            MAP_SHARED, // Accessible by another process 
            c_fid, // which file is associated with mmap 
            (off_t) 0); // Offset in page frame

    // if the pointer to the memory sucks
    if (c_mmap_ptr == MAP_FAILED) {
        printf("Child memory map has failed, CRT is aborting!\n");
        in_die(0);
    }

    // create a shared memory pointer
    out_mem_p = (outputbuf *) c_mmap_ptr;

    // initialize stuff
    buf_index = 0;
    out_mem_p->oc_flag = 0; //0 is empty
    char c[MAXCHAR];
    int n = 0;

    // send to crt, infinite loop - exit when parent signals us
    do {
        if (buf_index < MAXCHAR - 1) {
            c[n] = out_mem_p->outdata[buf_index];

            if (c[n] != '\0')
                printf("%c", c[n]);

                // when we reach the null character
            else {
                // reset the flag and array start point
                out_mem_p->oc_flag = 1;
                buf_index = 0;

                //send a signal to parent to start handler to start crt_iproc
                kill(parent_pid, SIGUSR2);

                // infinite cycle until the buffer has been filled
                while (out_mem_p->oc_flag == 1)
                    usleep(100000);
            }

            buf_index++;
            n++;

        }

    } while (1);
}
