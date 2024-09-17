COMMAND="./BS_ALL_trans 1 runtime-config private-1.pem 1 1 ./Thread/Dotinput1 out"
SLEEPTIME=(6)
for i in {1..3}
do
    echo "COMMAND: $COMMAND 1024 4"
    eval "$COMMAND 1024 4"
    sleep $SLEEPTIME    
    touch flag.txt
done

for i in {1..3}
do
    echo "COMMAND: $COMMAND"
    eval "$COMMAND 2048 8"
    sleep $SLEEPTIME    
    touch flag.txt
done

for i in {1..3}
do
    echo "COMMAND: $COMMAND"
    eval "$COMMAND 4096 16"
    sleep $SLEEPTIME    
    touch flag.txt
done

for i in {1..3}
do
    echo "COMMAND: $COMMAND"
    eval "$COMMAND 8192 16"
    sleep $SLEEPTIME    
    touch flag.txt
done

for i in {1..3}
do
    echo "COMMAND: $COMMAND"
    eval "$COMMAND 16384 16"
    sleep $SLEEPTIME    
    touch flag.txt
done

for i in {1..3}
do
    echo "COMMAND: $COMMAND"
    eval "$COMMAND 32768 32"
    sleep $SLEEPTIME    
    touch flag.txt
done

for i in {1..3}
do
    echo "COMMAND: $COMMAND"
    eval "$COMMAND 65536 32"
    sleep $SLEEPTIME    
    touch flag.txt
done

for i in {1..3}
do
    echo "COMMAND: $COMMAND"
    eval "$COMMAND 262144 64"
    sleep $SLEEPTIME    
    touch flag.txt
done

for i in {1..3}
do
    echo "COMMAND: $COMMAND"
    eval "$COMMAND 524288 128"
    sleep $SLEEPTIME    
    touch flag.txt
done

for i in {1..3}
do
    echo "COMMAND: $COMMAND"
    eval "$COMMAND 1048576 128"
    sleep $SLEEPTIME    
    touch flag.txt
done

for i in {1..3}
do
    echo "COMMAND: $COMMAND"
    eval "$COMMAND 2097152 256"
    sleep $SLEEPTIME    
    touch flag.txt
done

for i in {1..3}
do
    echo "COMMAND: $COMMAND"
    eval "$COMMAND 4192304 512"
    sleep $SLEEPTIME    
    touch flag.txt
done
