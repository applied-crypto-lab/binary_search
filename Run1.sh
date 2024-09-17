COMMAND="./bs 1 runtime-config private-1.pem"
SLEEPTIME=(5)
for i in {4..7}
do
    for k in {1..3}
    do
        echo "COMMAND: $COMMAND 20 32 1 $i 8"
        eval "$COMMAND 20 32 1 $i 8"
        sleep $SLEEPTIME
    done
done

for i in {5..7}
do
    for k in {1..3}
    do
        echo "COMMAND: $COMMAND 21 32 1 $i 8"
        eval "$COMMAND 21 32 1 $i 8"
        sleep $SLEEPTIME
    done
done

for i in {6..9}
do
    for k in {1..3}
    do
        echo "COMMAND: $COMMAND 22 32 1 $i 8"
        eval "$COMMAND 22 32 1 $i 8"
        sleep $SLEEPTIME
    done
done

for i in {7..11}
do
    for k in {1..3}
    do
        echo "COMMAND: $COMMAND 23 32 1 $i 8"
        eval "$COMMAND 23 32 1 $i 8"
        sleep $SLEEPTIME
    done
done

for i in {8..11}
do
    for k in {1..3}
    do
        echo "COMMAND: $COMMAND 24 32 1 $i 8"
        eval "$COMMAND 24 32 1 $i 8"
        sleep $SLEEPTIME
    done
done

for i in {8..11}
do
    for k in {1..3}
    do
        echo "COMMAND: $COMMAND 25 32 1 $i 8"
        eval "$COMMAND 25 32 1 $i 8"
        sleep $SLEEPTIME
    done
done
