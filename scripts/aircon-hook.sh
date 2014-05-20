HOOK_STATE=
HOOK_TICK=10
hook() {
  local h=$1 t=$2
  local t_max=$3 t_min=$4 h_max=$5 h_min=$6 
  case "$t_max" in
  cool)
    t_max=274
    h_max=650
    t_min=265
    h_min=550
    ;;
  *)
    test "$t_max" || t_max=276
    test "$h_max" || h_max=680
    test "$t_min" || t_min=267 
    test "$h_min" || h_min=550
    ;;
  esac
  local action_min="./tty-irsend.sh ircode/aircon/${t_min%?}_auto"
  local action_max="./tty-irsend.sh ircode/aircon/${t_max%?}_auto"
  local s_max="$t_max-$h_max" s_min="$t_min-$h_min"
  if [ "$h" = stop ]; then
    $action_max
    return
  fi
  
  HOOK_TICK=$((HOOK_TICK+1))
  # make sure we don't change state too often
  [ $HOOK_TICK -lt 10 ] && return
  
  if [ "$HOOK_STATE" = "$s_max" ]; then
    [ $t -gt $t_max ] || [ $h -gt $h_max ] && \
      $action_min && HOOK_STATE="$s_min" && HOOK_TICK=0
  elif [ "$HOOK_STATE" != "$s_min" ]; then
      $action_max && HOOK_STATE="$s_max"
  else
    [ $t -lt $t_min ] || [ $h -lt $h_min ]  && \
      $action_max && HOOK_STATE="$s_max" && HOOK_TICK=0
  fi
}
