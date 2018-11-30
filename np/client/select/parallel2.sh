max_children=4

function parallel {
  local time1=$(date +"%H:%M:%S")
  local time2=""

  # for the sake of the example, I'm using $2 as a description, you may be interested in other description
  echo "starting $2 ($time1)..."
  "$@" && time2=$(date +"%H:%M:%S") && echo "finishing $2 ($time1 -- $time2)..." &

  local my_pid=$$
  local children=$(ps -eo ppid | grep -w $my_pid | wc -w)
  children=$((children-1))
  if [[ $children -ge $max_children ]]; then
    wait -n
  fi
}

parallel sleep 5
parallel sleep 6
parallel sleep 7
parallel sleep 8
parallel sleep 9
wait
