CC = gcc
OPTS = -Wall -g -std=gnu99

all: deadlock sample_multithread

deadlock: deadlock.o
	$(CC) -o deadlock deadlock.o -pthread

sample_multithread: sample_multithread.o
	$(CC) -o sample_multithread sample_multithread.o -pthread

%.o: %.c
	$(CC) $(OPTS) -c $< -o $@

clean:
	rm -f deadlock.o deadlock sample_multithread.o sample_multithread
