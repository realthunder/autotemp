#!/bin/sh

. tty.sh

roll=5
i=0
freq=10
date=
dir=/tmp/thlog
mkdir -p $dir
retry=2
log=
hook=$1
shift

tty_lock

# find the last log file
for f in $dir/*.*.log; do
  d=${f%.*.log}
  d=`basename $d`
  if [ "$date" \< "$d" ]; then
    date=$d
    i=${f%.log}
    i=${i##*.}
    log=$f
  fi
done
test "$date" && ln -fs $log $dir/current.log

pid=$dir/`basename $TTYDEV`-monitor.pid
test -f $pid && \
  ps|grep `cat $pid`|grep -q `basename $0` && \
  kill -SIGTERM `cat $pid`
rm -f $pid
  
[ "$hook" = "stop" ] && exit
echo $$ > $pid

tty_unlock

if test "$hook"; then
  . $hook
  trap 'exit' INT TERM
  trap 'hook stop 0 "$@"; echo "exit">>$log' EXIT
fi

while true; do
  
  tty_lock
  d=`date +%F`
  if [ "$d" != "$date" ]; then
    date="$d"
    i=$(((i+1)%roll))
    rm -f $dir/*.$i.log
    log=$dir/$date.$i.log
    touch $log
    ln -fs $log $dir/current.log
  fi
  
  while true; do
    output=`tty_run th`
    output2="${output#*H:}"
    if [ "$output" = "$output2" ]; then
    	echo retry
    	tty_unlock
    	sleep $retry
    	tty_lock
    	continue
    fi
    h=${output2%T:*}
    t=${output2#*T:}
    line="`date +%T` $h $t"
    echo $line
    echo $line >> $log
    break;
  done
  tty_unlock
  
  sleep 1
  test "$hook" && hook $h $t "$@"
  
  sleep $freq
done
    
  	
  
