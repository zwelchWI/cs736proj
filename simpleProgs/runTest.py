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
filename   = CmdLineFind("-f", "") 
noInstrument = CmdDefined("--noinstrument")
frame = 1





resDir = "/tmp/"  + joblabel
ts = time.time()
st = datetime.datetime.fromtimestamp(ts).strftime('%Y-%m-%d.%H:%M:%S')
os.mkdir(resDir+st)

sumFile = open(resDir+st+"/summary.txt","w")


extraLine = ""
extras = int( CmdLineFindIndex("-d") )
if extras > 0:
	if(noInstrument == False):
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
	if(noInstrument): 
		command = extraLine
		print("Not instrumenting\n")
	else:
		command =executable +"/"+execName +" -r "+str(frame) +  " -t "+scheds+ " -s "+str(numThreads)+ " -i "+insts+" " + extraLine#+ " &> stderrout.txt"
	if frame == 1:
            sumFile.write("Execution results for "+command+"\n")
	print(command)
	stdouterr = open("stderrout.txt","w")
	check_archive = False
	try:
		command ="rm -f /tmp/threadLog.txt;"+command
		subprocess.check_call(command,shell=True,timeout=timeOut,stdout=stdouterr,stderr=stdouterr)
		check_archive = True
	except subprocess.CalledProcessError:
		stdouterr.close()
		print("rand seed "+str(frame)+" caused an error \n")
		sumFile.write("rand seed "+str(frame)+" caused an error \n")
		if(noInstrument == False):
			os.system("mv /tmp/threadLog.txt "+resDir+st+"/threadLog."+str(frame)+".txt")
	except subprocess.TimeoutExpired:
		stdouterr.close()
		os.system("killall -9 scheduleInst "+sys.argv[extras+1].split("/")[-1])
		print("rand seed "+str(frame)+" timed out \n")
		sumFile.write("rand seed "+str(frame)+"  timed out \n")
		if(noInstrument == False):
			os.system("mv /tmp/threadLog.txt "+resDir+st+"/threadLog."+str(frame)+".txt")
		os.system("mv stderrout.txt "+resDir+st+"/stderrout."+str(frame)+".txt")
	if check_archive:
		try: 
			check_command = "pbzip2 -tv " + filename
			sumFile.write("Running check with command " + check_command + "\n")
			subprocess.check_call(check_command,shell=True,timeout=timeOut,stdout=stdouterr,stderr=stdouterr)
			os.system("mv stderrout.txt "+resDir+st+"/out."+str(frame)+".txt")
			stdouterr.close()
		except subprocess.CalledProcessError:
			stdouterr.close()
			print("archive test failed \n")
			sumFile.write("rand seed "+str(frame)+" compression failure \n")
			if(noInstrument == False):
				os.system("mv /tmp/threadLog.txt "+resDir+st+"/threadLog"+str(frame)+".txt")
			os.system("mv stderrout.txt "+resDir+st+"/stderrout.check."+str(frame)+".txt")
		except subprocess.TimeoutExpired:
			stdouterr.close()
			os.system("killall -9 scheduleInst "+sys.argv[extras+1].split("/")[-1])
			print("rand seed "+str(frame)+" archive test timed out \n")
			sumFile.write("rand seed "+str(frame)+" archive test timed out \n")
			if(noInstrument == False):
				os.system("mv /tmp/threadLog.txt "+resDir+st+"/threadLog."+str(frame)+".txt")
			os.system("mv stderrout.txt "+resDir+st+"/stderrout.check."+str(frame)+".txt")
	print("DONE WITH " +str(frame))
	sumFile.flush()
	frame += 1

sumFile.close()


