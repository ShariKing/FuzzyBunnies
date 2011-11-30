ALL: RTX-G11 crt_child kbd_child

RTX-G11: init.o iproc.o kernel.o userprocesses.o CCI.o ProcessSwitch.o clock.o
	gcc -g init.o iproc.o kernel.o userprocesses.o CCI.o ProcessSwitch.o clock.o -o RTX-G11

crt_child: crt.o
	gcc -g crt.o -o crt_child
	
kbd_child: keyboard.o
	gcc -g keyboard.o -o kbd_child
	
crt.o: crt.c kbcrt.h
	gcc -g -c crt.c

keyboard.o: keyboard.c kbcrt.h
	gcc -g -c keyboard.c
	
ProcessSwitch.o: ProcessSwitch.c rtx.h
	gcc -g -c ProcessSwitch.c
	
init.o: init.c rtx.h
	gcc -g -c init.c

iproc.o: iproc.c rtx.h
	gcc -g -c iproc.c

kernel.o: kernel.c rtx.h
	gcc -g -c kernel.c

userprocesses.o: userprocesses.c rtx.h
	gcc -g -c userprocesses.c

CCI.o: CCI.c rtx.h
	gcc -g -c CCI.c

clock.o: clock.c rtx.h
	gcc -g -c clock.c
	
clean:
	rm *.o 
	rm RTX-G11 
	rm crt_child 
	rm kbd_child
	rm KBjunkDemo
	rm CRTjunkDemo