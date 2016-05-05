SHELL=/bin/bash

CC=mpicxx
CCFLAGS= -D ENABLE_DEBUG -std=c++11 -g 


all: sequentialSort

sequentialSort: *.cpp
	$(CC) $(CCFLAGS) *.cpp -o sequentialSort

clean:
	rm -f *.exe $(TARGETS)

