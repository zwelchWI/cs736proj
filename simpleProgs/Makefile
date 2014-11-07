CC = gcc
COPTS = -Wall -g -std=gnu99
CPPOPTS = -Wall -g
DYNINST_ROOT = /usr/local
CPP = g++
EXECNAME= scheduleInst

all: deadlock sample_multithread segFault $(EXECNAME) libcreateFunc.so fork

$(EXECNAME): $(EXECNAME).o
	$(CPP) $(EXECNAME).o -L$(DYNINST_ROOT)/lib -std=c++0x \
	    -ldyninstAPI -lsymtabAPI -ldl -ldwarf -lelf  -o $(EXECNAME)

$(EXECNAME).o: ./$(EXECNAME).cpp
	$(CPP) -g -c $(CPPOPTS) -I$(DYNINST_ROOT)/include -std=c++0x $(EXECNAME).cpp

libcreateFunc.so: createFunc.c
	gcc -o libcreateFunc.so -shared createFunc.c -lpthread -fPIC $(OPTS)


deadlock: deadlock.o
	$(CC) -o deadlock deadlock.o -pthread

segFault: segFault.o
	$(CC) -o segFault segFault.o -pthread


sample_multithread: sample_multithread.o
	$(CC) -o sample_multithread sample_multithread.o -pthread

fork: fork.o
	$(CC) -o fork fork.o 

%.o: %.c
	$(CC) $(COPTS) -c $< -o $@

clean:
	rm -f deadlock.o deadlock sample_multithread.o sample_multithread $(EXECNAME).o $(EXECNAME) libcreateFunc.so segFault.o segFault fork fork.o
