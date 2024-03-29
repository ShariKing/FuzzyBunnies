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
// #include <setjmp.h>
#include <signal.h>
// #include <string.h>
// #include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/wait.h>
// #include <sys/types.h>
// #include <sys/ipc.h>
// #include <sys/shm.h>
// #include <errno.h>
#include "kbcrt.h"

// ***FUNCTION TO CLEAN UP CHILD PROCESSES***
void c_child_die(int signal) {
    //printf("You're in c_child_die\n");
    exit(0);
}

// ***CRT UART SIMULATION***
int main(int argc, char * argv[]) {
    //printf("You're in crt main\n");
    
    // if parent tells us to terminate, then clean up first
    sigset(SIGINT, c_child_die);
    
    // delay after being forked from parent
    usleep(1000000);

    // get id of process to signal when we have output and the file id of the memory mapped file
    sscanf(argv[1], "%d", &parent_pid);
    sscanf(argv[2], "%d", &c_fid); // get the file id
    
    // attach to shared memory so we can pass output to crt interrupt handler
    c_mmap_ptr = mmap((caddr_t)0, // Memory Location, 0 lets O/S choose 
            c_bufsize + 1, // How many bytes to mmap -> +1 is the flag
            PROT_READ | PROT_WRITE, // Read and write permissions 
            MAP_SHARED, // Accessible by another process 
            c_fid, // which file is associated with mmap 
            (off_t) 0); // Offset in page frame
    
    // if the pointer to the memory sucks
    if (c_mmap_ptr == MAP_FAILED) 
    {
        printf("Child memory map has failed, CRT is aborting!\n");
        c_child_die(0);
    }
   
    // create a shared memory pointer
    out_mem_p = (struct outbuf *) malloc(sizeof (struct outbuf));
     
    // ensuring the pointer is not NULL
    if (out_mem_p)
    {
        // initialize stuff
        
        out_mem_p->outdata = c_mmap_ptr;
        buf_index = 0;
        
        // link the flag to the end of the buffer and set it 
        
        out_mem_p->oc_flag = &out_mem_p->outdata[c_bufsize];
        *out_mem_p->oc_flag = 0;
       
        //c is our temp character to read from the buffer
        char c;
        usleep(100000);
        
        // regular running, infinite loop - exit when parent signals us
        while(1) 
        {
            //printf("crt 1, flag = %d\n", *out_mem_p->oc_flag);
            while(*out_mem_p->oc_flag == 0)
            {    
                
                //wait for the iprocess to load the buffer (if applicable)
                usleep(100000);
                //printf("crt 2.5\n");
                // signal the crt i-process periodically to start doing it's thing
                kill(parent_pid,SIGUSR2);

            }
            //printf("crt 2\n");
            // reset the buffer index to read from the beginning
            buf_index = 0;
        
/*
            // while there is something in the buffer
            while(*out_mem_p->oc_flag != 0)
            {
               //printf("crt 3\n");
                if (buf_index < MAXCHAR - 1) 
                {
                    c = out_mem_p->outdata[buf_index];

                    // while we're not at the end of the output (NULL character)
                    if (c != '\0')
                    {
                        printf("%c", c);
                    }

                    // when we reach the null character
                    else {
                        // reset the flag and array start point
                        *out_mem_p->oc_flag = 0;
                        buf_index = 0;
                        //printf("crt 5\n");
                        
                        //send a signal to parent to start handler to start crt_iproc
                        // kill(parent_pid, SIGUSR2);
                    }

                    // increment the buffer
                    buf_index++;
                }
            }
*/
// while there is something in the buffer
            while(out_mem_p->outdata[buf_index] != '\0')
            {
                printf("%c", out_mem_p->outdata[buf_index]);
                buf_index++;
                fflush(stdout);
            }

            // when we reach the null character reset the flag and array start point
            *out_mem_p->oc_flag = 0;
            buf_index = 0;
 
        }
        
    }
    
    // if the shared pointer is initialized as NULL
    else{
        printf("CRT shared memory pointer initialization failed\n");
    }
}

