RTX: init.o iproc.o kernel.o ProcessP.o
	gcc init.o iproc.o kernel.o ProcessP.o -o RTX

crt_child: crt.o
	gcc crt.o -o crt_child
	
kbd_child: keyboard.o
	gcc kbd.o -o kbd_child

init.o: init.c rtx.h kbcrt.h
	gcc -c init.c

crt.o: crt.c kbcrt.h
	gcc -c crt.c

keyboard.o: keyboard.c kbcrt.h
	gcc -c keyboard.c

iproc.o: iproc.c kbcrt.h
	gcc -c iproc.c

kernel.o: kernel.c rtx.h
	gcc -c kernel.c

ProcessP.o: ProcessP.c rtx.h kbcrt.h
	gcc -c ProcessP.c

clean:
	rm *.o 
	rm RTX 
	rm crt_child 
	rm kbd_child