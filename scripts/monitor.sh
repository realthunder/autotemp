#!/bin/sh

roll=5
i=0
freq=10
date=
dir=/tmp/thlog
mkdir -p $dir
retry=2
log=

while true; do
  sleep $freq
  d=`date +%F`
  if [ "$d" != "$date" ]; then
    date="$d"
    i=$(((i+1)%roll))
    rm -f $dir/*.$i.log
    log=$dir/$date.$i.log
  fi
  
  while true; do
    output=`./tty.sh th`
    output2="${output#*H:}"
    if [ "$output" = "$output2" ]; then
    	echo retry
    	sleep $retry
    	continue
    fi
    h=${output2%T:*}
    t=${output2#*T:}
    line="`date +%T` $h $t"
    echo $line
    echo $line >> $log
    break;
  done
done
    
  	
  
