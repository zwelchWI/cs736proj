DYNINST_ROOT = /usr/local
CC = g++
CFLAGS	= -Wall
EXECNAME= coverage

$(EXECNAME): $(EXECNAME).o
	$(CC) $(EXECNAME).o -L$(DYNINST_ROOT)/lib -std=c++0x \
	    -ldyninstAPI -ldl -ldwarf -lelf  -o $(EXECNAME)

$(EXECNAME).o: ./$(EXECNAME).C
	$(CC) -g -c $(CFLAGS) -I$(DYNINST_ROOT)/include -std=c++0x $(EXECNAME).C

libincrement.so: increment.c
	gcc -o libincrement.so -shared increment.c

clean:
	@rm -f $(EXECNAME).o $(EXECNAME) core
