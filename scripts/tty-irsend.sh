#!/bin/sh
[ ! -f $1 ] && echo invalid command file && exit 1
. tty.sh
tty_lock
while read line; do
  [ -z "$line" ] && break;
  ans=`tty_run "$line"`
  [ -z "$ans" ] && continue;
  [ "$ans" != "${ans#Sent RAW}" ] && exit 0
  echo $ans
  exit 1
done < $1
echo unexpected ending
exit 1
