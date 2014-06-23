TTYDEV=/dev/ttyACM0
TTYLOCK=/tmp/`basename $TTYDEV`.lock
TTYLOCK2=/tmp/`basename $TTYDEV`.lock2

if [ -z "$TTYINCLUDED" ]; then
  exec 100> $TTYLOCK
  exec 101> $TTYLOCK2
  TTYINCLUDED=1
fi

tty_lock() {
  flock 100
}

tty_lock2() {
  flock 101
}

tty_unlock() {
  flock -u 100
}

tty_unlock2() {
  flock -u 101
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
