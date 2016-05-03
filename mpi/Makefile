SHELL=/bin/bash

CC=mpicc
CCFLAGS= -D ENABLE_DEBUG -g 


all: main.exe

main.exe: *.c
	$(CC) $(CCFLAGS) *.c -o main.exe

clean:
	rm -f *.exe $(TARGETS)
