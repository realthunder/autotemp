TTYDEV=/dev/ttyACM0
TTYLOCK=/tmp/`basename $TTYDEV`.lock

if [ -z "$TTYINCLUDED" ]; then
  exec 100> $TTYLOCK
  TTYINCLUDED=1
fi

tty_lock() {
  flock 100
}

tty_unlock() {
  flock -u 100
}

tty_read()  {
  (
    exec < $TTYDEV
    stty -echo igncr
    
    read line
    while read line; do
      [ "$line" = ">" ] && break
      echo $line
    done
  )
}

tty_send() {
  echo "$@" > $TTYDEV
}

tty_run() {
  tty_read &
  sleep 0.05 &> /dev/null || sleep 1
  tty_send "$@"
  wait
}
