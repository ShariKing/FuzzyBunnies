#ifndef KBCRT_H
#define KBCRT_H

//#include <sys/types.h>
//#include <sys/ipc.h>
//#include <sys/shm.h>
//#include <errno.h>
#include "rtx.h"

#include <string.h>
#include <stdlib.h>

#define BUFFERSIZE 100;

// structures for shared memory
#define MAXCHAR 80	

int bufsize = BUFFERSIZE;
int buf_index;

typedef struct {
	int ok_flag;
	char indata[MAXCHAR];
	int length;
} inputbuf;

typedef struct {
	int ok_flag;
	char outdata[MAXCHAR];
	int length;
} outputbuf;

void kbd_iproc();
void crt_iproc();
void in_die(int signal);
void exception_handler_process(int result);

	int parent_pid, fid;
    void * c_mmap_ptr;
	void * mmap_ptr;
	inputbuf *in_mem_p; //pointer to the shared memory
	outputbuf *out_mem_p;
	char c;

#endif
