import os
import sys
import math


executable = "/home/$USER/cs736proj/simpleProgs/"




def CmdLineFindIndex( tag ):
	for i in range(len(sys.argv)):
		if sys.argv[i] == tag:
			return i
	return -1

def CmdLineFind( tag, defaultvalue ):
	i = CmdLineFindIndex(tag)
	if i > 0:
		if i < len(sys.argv)-1:
			return sys.argv[i+1]
	return defaultvalue


scheds     = CmdLineFind("-t","RAND")
numThreads = int(CmdLineFind("-s",1000))
insts      = CmdLineFind("-i","create")
randNum    = int(CmdLineFind("-r",None))
execName   = CmdLineFind("-e","FAILZZZ")
outFile    = CmdLineFind("-outFile","/dev/null")

executable = executable + execName +" -r "+ str(randNum)+ " -t "+scheds+" -s "+str(numThreads)+" -i "+insts


job = executable
extras = int( CmdLineFindIndex("-d") )
if extras > 0:
	job = job + " -d "
	for task in sys.argv[extras+1:]:
		job = job + str(task) + " "

job = job + " &> "+outFile
print job

#os.system(job)

