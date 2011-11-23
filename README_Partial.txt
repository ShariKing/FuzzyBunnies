Group #11 - README
Group members:	Kal Sobel		ID# 20322299
				Shari King		ID# 20328892
				Peter Robertson	ID# 20332330
				Yasser Al-Khder	ID# 20356957

Directory Contents:
Makefile: 	This is the makefile for the RTX.
init.c:		This code covers the initialization of the RTX.
crt.c:		This code initializes and runs the crt child process.
keyboard.c:	This code initializes and runs the keyboard child process.
iproc.c:	This contains the code for both the keyboard and crt iprocesses.
kernel.c:	This contains the primitives for the RTX.
ProcessP.c:	This code is the test case for testing the partial RTX.
itable.txt:	This is a txt file containing the initialization table for the RTX. The format of this table is several columns containing two numbers. The first number is the PID of that process, and the second number is its priority. Since this is mostly unused for the Partial RTX, all priorities are set to zero and there is an arbitrary number of processes matching the arbitrary constant PROC_NUM in init.c.
rtx.h:		This header file contains all of the function and variable prototypes for the RTX kernel.
kbcrt.h:	This header file contains all of the function and variable prototypes for the keyboard and crt child processes.

RTX Error Description:
Our RTX has a segfault, which we have determined to be located in the crt_iprocess do/while loop in iproc.c. We tried to isolate it, but ran out of time to fix it. We're looking into the solution, but unfortunately we could not solve it before the deadline.