#!/usr/bin/python

import sys

stack = {}

File = open(sys.argv[1],"r")
lines = File.readlines()
File.close()

for line in lines:
    p = line.split()
    if len(p) == 0:
        continue
    if p[0] == 'about':
        continue
    if p[0] == "[TRACE_ENTRY-":
        threadId = p[1].split(':')[1]

        if threadId not in stack.keys():
            stack[threadId] = [[],[]]


        stack[threadId][0].append(p[3])
    elif p[0] == "[TRACE_EXIT-":
        threadId = p[1].split(':')[1]

        if threadId not in stack.keys():
            stack[threadId] = [[],[]]

        stack[threadId][0] = stack[threadId][0][:-1]
        if len(stack[threadId][1]) == 3:
            stack[threadId][1] = stack[threadId][1][1:]
        stack[threadId][1].append(p[3])
    else:
        pass

for key,val in stack.iteritems():
    print key+ " "+str(val)

