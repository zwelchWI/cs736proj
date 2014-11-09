#!/bin/bash
#first argument is num procs, second argument is flags to pass to pbzip2, third argument is file

#writes out each command executed
set -x

pbzip2 -p$1 $2 $3 || exit $?

pbzip2 -tv $3.bz2 || exit $? 
