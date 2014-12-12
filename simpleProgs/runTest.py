#!/usr/bin/python3.4
import time
import subprocess
import datetime
import os
import sys
import math

executable = "/home/$USER/cs736proj/simpleProgs"
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

def CmdDefined ( tag ):
	i = CmdLineFindIndex(tag)
	if i > 0: 
		return True
	return False

scheds     = CmdLineFind("-t","")
numThreads = int(CmdLineFind("-s",1000))
insts      = CmdLineFind("-i","")
execName   = CmdLineFind("-e","scheduleInst")
joblabel     = CmdLineFind("-l","run")
num        = int(CmdLineFind("-n",1000))
timeOut    = int(CmdLineFind("-timeout",120))
inclusive  = CmdLineFind("--inclusive","")
frame = 1

resDir = "/tmp/"  + joblabel
ts = time.time()
st = datetime.datetime.fromtimestamp(ts).strftime('%Y-%m-%d.%H:%M:%S')
os.mkdir(resDir+st)

sumFile = open(resDir+st+"/summary.txt","w")

print(resDir+st)
sumFile.write(resDir+st+"\n")

extraLine = ""
extras = int( CmdLineFindIndex("-d") )
if extras > 0:
	extraLine = extraLine + " -d "
	for task in sys.argv[extras+1:]:
		extraLine = extraLine + str(task) + " "

#probably doesn't hurt to put this in your .bashrc either
os.system("ulimit -c unlimited") 

if(os.path.isfile("/tmp/threadLog.txt")):
	os.system("rm -rf /tmp/threadLog.txt")

if(os.path.isfile("rm /tmp/core")):
	os.system("rm /tmp/core")

while frame <= num:
	padframe = str(frame)
	if frame < 1000:
		padframe = "0" + padframe
	if frame < 100:
		padframe = "0" + padframe
	if frame < 10:
		padframe = "0" + padframe
	if inclusive: 
		command =executable +"/"+execName +" -r "+str(frame) +  " -t "+scheds+ " -s "+str(numThreads)+ " -i "+insts+" --inclusive " + inclusive + " " + extraLine
	else:	
		command =executable +"/"+execName +" -r "+str(frame) +  " -t "+scheds+ " -s "+str(numThreads)+ " -i "+insts+" " + extraLine
	if frame == 1:
            sumFile.write("Execution results for "+command+"\n")
	print(command)
	sumFile.write(command + "\n")
	stdouterr = open("stderrout.txt","w")

	try:
		subprocess.check_call(command,shell=True,timeout=timeOut,stdout=stdouterr,stderr=stdouterr)
		#always move the stdout to check later
		os.system("mv stderrout.txt "+resDir+st+"/out."+str(frame)+".txt")
	except subprocess.CalledProcessError:
		stdouterr.close()
		print("rand seed "+str(frame)+" caused an error \n")
		sumFile.write("rand seed "+str(frame)+" caused an error \n")
		os.system("killall -9 scheduleInst "+sys.argv[extras+1].split("/")[-1])
		os.system("mv /tmp/threadLog.txt "+resDir+st+"/threadLog"+str(frame)+".txt")
		if(os.path.isfile("stderrout.txt")):
			os.system("mv stderrout.txt "+resDir+st+"/stderrout."+str(frame)+".txt")
		else:
			sumFile.write("rand seed " + str(frame) + " has no stderrout.txt\n")
	except subprocess.TimeoutExpired:
		stdouterr.close()
		os.system("killall -9 scheduleInst "+sys.argv[extras+1].split("/")[-1])
		print("rand seed "+str(frame)+" timed out \n")
		sumFile.write("rand seed "+str(frame)+"  timed out \n")
		os.system("mv /tmp/threadLog.txt "+resDir+st+"/threadLog"+str(frame)+".txt")
		if(os.path.isfile("stderrout.txt")):
			os.system("mv stderrout.txt "+resDir+st+"/stderrout."+str(frame)+".txt")
		else:
			sumFile.write("rand seed " + str(frame) + " has no stderrout.txt\n")
	
	#move the threadlog regardless
	#os.system("mv /tmp/threadLog.txt "+resDir+st+"/threadLog"+str(frame)+".txt")
	#changed this due to ffmpeg, only move if the test failed and remove. 
	os.system("rm /tmp/threadLog.txt"); 

	#look for a core file and move
	if(os.path.isfile("core")):
		os.system("mv /tmp/core " + resDir + st + "/core." + str(frame))

	print("DONE WITH " +str(frame) + "\n")
	sumFile.write("DONE WITH " +str(frame) + "\n")
	sumFile.flush()
	frame += 1

sumFile.close()


