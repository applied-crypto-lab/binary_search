COMMAND="./bs1write $1 runtime-config private-$1.pem"
COMMAND1="./bs5write $1 runtime-config private-$1.pem"
COMMAND2="./subhbs11write $1 runtime-config private-$1.pem"
SLEEPTIME=(0 8 4 0)

for j in {10..10}
do
    for i in {5..5}
    do
	for k in {1..2}
        do
            echo "COMMAND2: $COMMAND2 $j 32 1 $i 1"
            eval "$COMMAND2 $j 32 1 $i 1"
            sleep ${SLEEPTIME[$1]}
        done
    done
done


for j in {11..13}
do
    for i in {6..6}
    do
	for k in {1..2}
        do
            echo "COMMAND2: $COMMAND2 $j 32 1 $i 1"
            eval "$COMMAND2 $j 32 1 $i 1"
            sleep ${SLEEPTIME[$1]}
        done
    done
done


for j in {14..15}
do
    for i in {7..7}
    do
	for k in {1..2}
        do
            echo "COMMAND2: $COMMAND2 $j 32 1 $i 1"
            eval "$COMMAND2 $j 32 1 $i 1"
            sleep ${SLEEPTIME[$1]}
        done
    done
done

for j in {16..17}
do
    for i in {8..8}
    do
	for k in {1..2}
        do
            echo "COMMAND2: $COMMAND2 $j 32 1 $i 1"
            eval "$COMMAND2 $j 32 1 $i 1"
            sleep ${SLEEPTIME[$1]}
        done
    done
done

for j in {18..19}
do
    for i in {9..9}
    do
	for k in {1..2}
        do
            echo "COMMAND2: $COMMAND2 $j 32 1 $i 1"
            eval "$COMMAND2 $j 32 1 $i 1"
            sleep ${SLEEPTIME[$1]}
        done
    done
done

for j in {20..21}
do
    for i in {10..10}
    do
	for k in {1..2}
        do
            echo "COMMAND2: $COMMAND2 $j 32 1 $i 1"
            eval "$COMMAND2 $j 32 1 $i 1"
            sleep ${SLEEPTIME[$1]}
        done
    done
done


