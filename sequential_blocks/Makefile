SHELL=/bin/bash

CC=mpicxx
CCFLAGS= -D ENABLE_DEBUG -std=c++11 -g 


all: externalMergeSort

externalMergeSort: *.cpp
	$(CC) $(CCFLAGS) *.cpp -o externalMergeSort

clean:
	rm -f *.exe $(TARGETS)

