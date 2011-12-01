#ifndef KBCRT_H
#define KBCRT_H

// ***CONSTANTS***
#define BUFFERSIZE 1028
#define MAXCHAR 128

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

// ***STRUCTS***

// input buffer from the perspective of kb-iproc
struct inbuf{
    char* ok_flag;
    char* indata;
}; typedef struct inbuf inputbuf;

// output buffer from the perspective of crt-iproc
struct outbuf{
    char* oc_flag;
    char* outdata;
}; typedef struct outbuf outputbuf;

// ***GLOBALS***
static int k_bufsize = BUFFERSIZE;
static int c_bufsize = BUFFERSIZE;
static int k_buf_index;
static int buf_index;

static int parent_pid, k_fid, c_fid, in_pid, out_pid, k_status, c_status;
static char * k_sfilename = "KBjunkDemo";
static char * c_sfilename = "CRTjunkDemo";

static char * k_mmap_ptr;
static char * c_mmap_ptr;
inputbuf *in_mem_p; //pointer to the shared memory
outputbuf *out_mem_p; //pointer to the shared memory

// ***FUNCTION DECLARATIONS***

void k_child_die(int signal);
void c_child_die(int signal);

#endif
