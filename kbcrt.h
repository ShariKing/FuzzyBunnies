#ifndef KBCRT_H
#define KBCRT_H


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


// ***CONSTANTS***
#define BUFFERSIZE 50;
#define MAXCHAR 20	


// ***STRUCTS***

// input buffer
typedef struct {
    int ok_flag;
    char indata[MAXCHAR];
    int length;
} inputbuf;

// output buffer
typedef struct {
    int oc_flag;
    char outdata[MAXCHAR];
    int length;
} outputbuf;


// ***FUNCTION DECLARATIONS***
void kbd_iproc();
void crt_iproc();
void in_die(int signal);
void exception_handler_process(int result);
void die(int signal);
void cleanup();

// ***GLOBALS***
int bufsize = BUFFERSIZE;
int buf_index;
int parent_pid, k_fid, c_fid, in_pid, out_pid, k_status, c_status;
char c;
char * k_sfilename = "KBjunkDemo";
char * c_sfilename = "CRTjunkDemo";
void * k_mmap_ptr;
void * c_mmap_ptr;
inputbuf *in_mem_p; //pointer to the shared memory
outputbuf *out_mem_p; //pointer to the shared memory


// ***FUNCTION IMPLEMENTATIONS***

// ***FUNCTION TO CLEAN UP CHILD PROCESSES***
void in_die(int signal) {
    exit(0);
}


// *** FUNCTION TO CLEAN UP PARENT PROCESSES***
void die(int signal)
{
    cleanup();
    printf("\n\nSignal Received.   Leaving RTOS ...\n");
    exit(0);
}

// ***CLEAN UP ROUTINE***
/* routine to clean up things before terminating main program
   This stuff must be cleaned up or we have child processes and shared
   memory hanging around after the main process terminates */
void cleanup() {
    // terminate child process(es)
    kill(in_pid, SIGINT);

    /******** KEYBOARD ********/
    // remove shared memory segment and do some standard error checks
    k_status = munmap(k_mmap_ptr, bufsize);
    if (k_status == -1) {
        printf("Bad KB munmap during cleanup\n");
    }
    
    // close the temporary mmap file 
    k_status = close(k_fid);
    if (k_status == -1) {
        printf("Bad close of temporary KB mmap file during cleanup\n");
    };
    
    // unlink (i.e. delete) the temporary mmap file
    k_status = unlink(k_sfilename);
    if (k_status == -1) {
        printf("Bad unlink during KB claeanup.\n");
    }
    
    /******** CRT ********/
    // remove shared memory segment and do some standard error checks
    c_status = munmap(c_mmap_ptr, bufsize);
    if (c_status == -1) {
        printf("Bad KB munmap during cleanup\n");
    }
    
    // close the temporary mmap file 
    c_status = close(c_fid);
    if (c_status == -1) {
        printf("Bad close of temporary KB mmap file during cleanup\n");
    };
    
    // unlink (i.e. delete) the temporary mmap file
    c_status = unlink(c_sfilename);
    if (c_status == -1) {
        printf("Bad unlink during KB claeanup.\n");
    }
}
#endif
