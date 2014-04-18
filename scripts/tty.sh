TTYDEV=/dev/ttyACM0
TTYLOCK=/tmp/`basename $TTYDEV`.lock
TTYREADLOCK=/tmp/`basename $TTYDEV`.rlock

if [ -z "$TTYINCLUDED" ]; then
  exec 100> $TTYLOCK
  exec 101> $TTYREADLOCK
  TTYINCLUDED=1
fi

tty_lock() {
  flock 100
}

tty_read()  {

  # If we need to read the tty output after sending
  # a command, we use this read lock to ensure the command
  # sending starts after we've ready to receive the data.
  flock 101
  (
    exec < $TTYDEV
    stty igncr
    
    # unlock once we are ready
    flock -u 101
    
    read line
    while read line; do
      [ "$line" = ">" ] && break
      echo $line
    done
  ) &
}

tty_send() {
  # The lock is harmless, if we don't need to read any feedback
  flock 101
  echo "$@" > $TTYDEV
}

tty_run() {
  tty_read
  tty_send "$@"
  wait
}
