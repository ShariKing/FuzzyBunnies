Group #11 - Final Implementation README
Group members:	Kal Sobel		ID# 20322299
				Shari King		ID# 20328892
				Peter Robertson	ID# 20332330
				Yasser Al-Khder	ID# 20356957

Directory Contents:
makefile: 		This is the makefile for the RTX.
init.c:			This code covers the initialization of the RTX.
crt.c:			This code initializes and runs the crt child process.
keyboard.c:		This code initializes and runs the keyboard child process.
iproc.c:		This contains the code for the keyboard, crt, and timer iprocesses.
kernel.c:		This contains the user and kernel primitives for the RTX.
clock.c:		This contains the clock primitives.
CCI.c:			This contains the code for handling the console command interpreter.
rtx.h:			This header file contains all of the function and variable prototypes for the RTX kernel.
kbcrt.h:		This header file contains all of the function and variable prototypes for the keyboard and crt child processes.
ProcessSwitch.c:This contains the code for Process switching, context switching, and the null process.
userprocesses.c:This contains user processes A, B, and C used for the demonstration.

RTX Error Description:
Currently, the RTX compiles, makes, and executes. The user processes are disabled because Process A is not sending to Process B and sitting in a loop, which block the rest of the system from functioning. The send and receive trace don't keep track of all the messages but are functional other than that. The clock displays, increments, and sets, but the wall clock doesn't stop at 24 hours for some reason. Another CCI functionality was added to echo back input to the screen.