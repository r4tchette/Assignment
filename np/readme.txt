클라이언트 : 집에 있는 PC 에서 linux virtual machine을 만들어서 실행 
서버 : 학과 linux2 사용, 각자에게 배정된 TCP 포트 번호 사용 
제출하는 화면에는 학번, 이름, 분반, 자기에게 배정된 포트번호 기재할 것 (console 화면에 출력하거나 손으로 기재) 

(1) 클라이언트 : 그림 6.9 서버 : 5장의 concurrent server 
한 줄이 대략 50 바이트 정도로 된 수 천 줄 짜리 파일을 만들어서 hw 하여 실험 
입력 파일의 각 줄의 앞에는 [줄 번호] 즉 [1], [2], 식으로 줄 번호를 집어 넣을 것 
클라이언트가 입력 파일의 몇 번 째 줄부터 stdout 에 출력하지 못하는지를 shell 화면 capture 하여 제출 

strcliselect01.c / tcpsrv04.c
100000줄 중 99416번째 까지 출력

(2) 클라이언트 ; 그림 6.13 서버 : 5장의 concurrent server 입력 파일 : 앞의 실험에서 사용한 파일 

strcliselect02.c / tcpsrv04.c
SIGPIPE error를 실험할 수 있도록 그림 6.13 을 수정하여 실행한 후 SIGPIPE 발생 화면을 capture 

(3) 클라이언트 : 그림 6.13 서버 : 5장 concurrent server 입력 파일 : 위와 동일 server child를 kill 하여 클라이언트에 나타느는 화면 capture 

strcliselect02.c / tcpsrv04.c

(4) 클라이언트 그림 6.13 서버 그림 6.21, 22 입력 파일 10,000 줄 짜리 파일 
클라이언트에 데이터를 전송하는 시점의 시각을 기록 (time(0 system call 사용)하고 서버로부터 데이터를 모두 전송 받기를 완료한 ㅅ점에 시각을 기록하여 전체 소요 시간을 출력하도록 수정하여 실행한 결과를 capture 

strcliselect02.c / tcpservselect01.c
전송하는 데 24ms 소요

(5) 그림 6.13의 line 30을 제거한 후 실험 결과 화면 capture 

(6) 실험 (4)를 실행하되 클라이언트의 수를 1, 2, 4, 8, 16, 32로 늘려가며 각 클라이언트의 완료 시간을 측정, 화면 capture

앞의 실험 과제의 연속으로, 
6.13 클라리언트 프로그램을 1~2 줄 수정하고, 6.22 서버 프로그래을 1줄 수정하여 DoS 공격을 하는 클라이언트와 DoS 고역을 당하는 서버를 만들어 보고 DoS 공격이 가능하니 실험으로 확인해 봅니다. 
서버가 블록되는 것을 확인하는 화면을 capture 하여 제출.

strcliselect02.c / tcpservselect01.c