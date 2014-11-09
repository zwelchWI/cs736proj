#!/usr/bin/python
import time
import subprocess
import datetime
import os
import sys
import math

executable = "/home/$USER/cs736proj/simpleProgs/"
execName = "scheduleInst"
resDir = "/tmp"

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
execName   = CmdLineFind("-e","scheduleInst")
joblabel     = CmdLineFind("-l","run")
num        = int(CmdLineFind("-n",1000))
timeOut    = int(CmdLineFind("-timeout",120))
frame = 1





resDir = "/tmp/"  + joblabel
ts = time.time()
st = datetime.datetime.fromtimestamp(ts).strftime('%Y-%m-%d.%H:%M:%S')
os.mkdir(resDir+st)

sumFile = open(resDir+st+"/summary.txt","w")


extraLine = ""
extras = int( CmdLineFindIndex("-d") )
if extras > 0:
	extraLine = extraLine + " -d "
	for task in sys.argv[extras+1:]:
		extraLine = extraLine + str(task) + " "



while frame <= num:
	padframe = str(frame)
	if frame < 1000:
		padframe = "0" + padframe
	if frame < 100:
		padframe = "0" + padframe
	if frame < 10:
		padframe = "0" + padframe
	command =executable +"/"+execName +" -r "+str(frame) +  " -t "+scheds+ " -s "+str(numThreads)+ " -i "+insts+" " + extraLine#+ " &> stderrout.txt"
	if frame == 1:
            sumFile.write("Execution results for "+command+"\n")
	print(command)
	stdouterr = open("stderrout.txt","w")
	try:
		subprocess.check_call(command,shell=True,timeout=timeOut,stdout=stdouterr,stderr=stdouterr)
		stdouterr.close()
	except subprocess.CalledProcessError:
		stdouterr.close()
		print("rand seed "+str(frame)+" caused an error \n")
		sumFile.write("rand seed "+str(frame)+" caused an error \n")
		os.system("mv /tmp/threadLog.txt "+resDir+st+"/threadLog."+str(frame)+".txt")
		os.system("mv stderrout.txt "+resDir+st+"/stderrout."+str(frame)+".txt")
	except subprocess.TimeoutExpired:
		stdouterr.close()
		os.system("killall -9 scheduleInst "+sys.argv[extras+1].split("/")[-1])
		print("rand seed "+str(frame)+" caused an error \n")
		sumFile.write("rand seed "+str(frame)+" caused an error \n")
		os.system("mv /tmp/threadLog.txt "+resDir+st+"/threadLog."+str(frame)+".txt")
		os.system("mv stderrout.txt "+resDir+st+"/stderrout."+str(frame)+".txt")
	print("DONE WITH " +str(frame))
	frame += 1

sumFile.close()

