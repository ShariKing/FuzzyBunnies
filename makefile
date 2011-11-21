ALL: iRTX-G11 crt_child kbd_child

iRTX-G11: init.o iproc.o kernel.o ProcessP.o userprocesses.o CCI.o ProcessSwitch.o ClockTest.o
	gcc init.o iproc.o kernel.o ProcessP.o -o iRTX-G11

crt_child: crt.o
	gcc crt.o -o crt_child
	
kbd_child: keyboard.o
	gcc keyboard.o -o kbd_child
	
crt.o: crt.c kbcrt.h
	gcc -c crt.c

keyboard.o: keyboard.c kbcrt.h
	gcc -c keyboard.c

init.o: init.c rtx.h
	gcc -c init.c

iproc.o: iproc.c rtx.h
	gcc -c iproc.c

kernel.o: kernel.c rtx.h
	gcc -c kernel.c

ProcessP.o: ProcessP.c rtx.h
	gcc -c ProcessP.c

userprocesses.o: userprocesses.c rtx.h
	gcc -c userprocesses.c

CCI.o: CCI.c rtx.h
	gcc -c CCI.c

ProcessSwitch.o: ProcessSwitch.c rtx.h
	gcc -c ProcessSwitch.c

ClockTest.o: ClockTest.c rtx.h
	gcc -c ClockTest.c

clean:
	rm *.o 
	rm RTX 
	rm crt_child 
	rm kbd_child
	rm KBjunkDemo
	rm CRTjunkDemo