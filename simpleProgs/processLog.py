

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
 
    threadId = p[1].split(':')[1]


    if threadId not in stack.keys():
        stack[threadId] = []

    if p[0] == "[TRACE_ENTRY-":
        stack[threadId].append(p[3])
    else:
        stack[threadId] = stack[threadId][:-1]

print stack




