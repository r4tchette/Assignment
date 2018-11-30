time xargs -P 3 -I {} sh -c 'eval "$1"' - {} <<'EOF'

./tcpcli02 203.249.75.14 < infile.txt
./tcpcli02 203.249.75.14 < infile.txt
./tcpcli02 203.249.75.14 < infile.txt
./tcpcli02 203.249.75.14 < infile.txt

EOF

cat output.txt
echo "B411037 김연서 2분반 30061"
