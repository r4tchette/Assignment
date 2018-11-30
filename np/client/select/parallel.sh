# Usage bash run_many_clients.sh <SERVER_IP> <# of clients> <input textfile>
# EX) bash run_many_clients.sh   192.168.0.2      16         fakelog.txt

echo "" > output.txt

for (( i=1; i<$2; i++ ))
do
    echo "[$(($i/10))$(($i%10))]"  $(/usr/bin/time ./tcpcli01 $1 < $3 2>&1 >/dev/null) >> output.txt &
done
echo "[$(($2/10))$(($2%10))]" $(/usr/bin/time ./tcpcli01 $1 < $3 2>&1 >/dev/null) >> output.txt

for job in `jobs -p`
do
    wait $job
done

cat output.txt | sort
