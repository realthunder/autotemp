HOOK_STATE=
hook() {
  local h=$1 t=$2
  local t_max=$3 h_max=$4 t_min=$5 h_min=$6 
  test "$t_max" || t_max=275 
  test "$h_max" || h_max=680 
  test "$t_min" || t_min=265
  test "$h_min" || h_min=640
  local action_min="./tty-irsend.sh ir-codes/mhi-aircon/${t_min%?}_auto"
  local action_max="./tty-irsend.sh ir-codes/mhi-aircon/${t_max%?}_auto"
  local s_max="$t_max-$h_max" s_min="$t_min-$h_min"
  if [ "$h" = stop ]; then
    $action_max
    return
  fi
  if [ "$HOOK_STATE" = "$s_max" ]; then
    [ $t -gt $t_max ] || { [ $h -ge $h_max ] && [ $t -ge $t_max ]; } && \
      $action_min && HOOK_STATE="$s_min"
  else
    [ "$HOOK_STATE" != "$s_min" ] || [ $t -lt $t_min ] || { [ $h -le $h_min ] && [ $t -le $t_min ]; } && \
      $action_max && HOOK_STATE="$s_max"
  fi
}
