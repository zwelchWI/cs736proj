CC = gcc
COPTS = -Wall -g -std=gnu99
CPPOPTS = -Wall -g
DYNINST_ROOT = /usr/local
CPP = g++
EXECNAME= createInst

$(EXECNAME): $(EXECNAME).o
	$(CPP) $(EXECNAME).o -L$(DYNINST_ROOT)/lib -std=c++0x \
	    -ldyninstAPI -lsymtabAPI -ldl -ldwarf -lelf  -o $(EXECNAME)

$(EXECNAME).o: ./$(EXECNAME).C
	$(CPP) -g -c $(CPPOPTS) -I$(DYNINST_ROOT)/include -std=c++0x $(EXECNAME).C

libcreateFunc.so: createFunc.c
	gcc -o libcreateFunc.so -shared createFunc.c -lpthread -fPIC $(OPTS)

all: deadlock sample_multithread $(EXECNAME)

deadlock: deadlock.o
	$(CC) -o deadlock deadlock.o -pthread

segFault: segFault.o
	$(CC) -o segFault segFault.o -pthread


sample_multithread: sample_multithread.o
	$(CC) -o sample_multithread sample_multithread.o -pthread

%.o: %.c
	$(CC) $(COPTS) -c $< -o $@

clean:
	rm -f deadlock.o deadlock sample_multithread.o sample_multithread $(EXECNAME).o $(EXECNAME)
