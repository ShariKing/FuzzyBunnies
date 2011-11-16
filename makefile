all : keyboard.out crt.out exceptionHandler.out

fuzzyBunnies.out : init.c kbcrt.h rtx.h 
	gcc init.c -o fuzzyBunnies.out
	
keyboard.out : keyboard.c kbcrt.h rtx.h
	gcc keyboard.c -o keyboard.out

crt.out : crt.c kbcrt.h rtx.h
	gcc crt.c -o crt.out

exceptionHandler.out : exceptionHandler.o kernel.o
	gcc $^ -o exceptionHandler.out

kernel.o : kernel.c rtx.h
	gcc kernel.c -c

exceptionHandler.o : exceptionHandler.c
	gcc exceptionHandler.c -c


