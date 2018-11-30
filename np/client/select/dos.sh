for (( i=1; i<$1; i++ ))
do
    ./tcpcli02 203.249.75.14 < ./dos.txt &
done
./tcpcli02 203.249.75.14 < ./dos.txt
 for job in `jobs -p`
do
    wait $job
done 

#cat ./result.txt
#rm ./result.txt

echo ""
echo "네트워크 프로그래밍 프로그램 실험 과제(2)"
echo "B411037 김연서 2분반 30061"
