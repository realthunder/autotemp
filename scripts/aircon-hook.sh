HOOK_STATE=
HOOK_TICK=10
hook() {
  local h=$1 t=$2
  local arg t_max t_min h_max h_min a_min a_max
  local profile=$3
  test "$profile" || profile=default.conf
  test -f "$profile" && . $profile
  test "$t_max" || t_max=276
  test "$h_max" || h_max=660
  test "$t_min" || t_min=267 
  test "$h_min" || h_min=550
  test "$a_min" || a_min=${t_min%?}_auto
  test "$a_max" || a_max=${t_max%?}_auto
  local action_min="./tty-irsend.sh ircode/aircon/${a_min}"
  local action_max="./tty-irsend.sh ircode/aircon/${a_max}"
  local s_max="$t_max-$h_max" s_min="$t_min-$h_min"
  if [ "$h" = stop ]; then
    $action_max
    return
  fi
  
  HOOK_TICK=$((HOOK_TICK+1))
  # make sure we don't change state too often
  [ $HOOK_TICK -lt 10 ] && return
  
  if [ "$HOOK_STATE" = "$s_max" ]; then
    [ $t -gt $t_max ] || { [ $t -gt $t_min ] && [ $h -gt $h_max ]; } && \
      $action_min && HOOK_STATE="$s_min" && HOOK_TICK=0
  elif [ "$HOOK_STATE" != "$s_min" ]; then
      $action_max && HOOK_STATE="$s_max"
  else
     [ $t -lt $t_min ] || { [ $t -lt $t_max ] && [ $h -lt $h_min ]; } && \
      $action_max && HOOK_STATE="$s_max" && HOOK_TICK=0
  fi
}
