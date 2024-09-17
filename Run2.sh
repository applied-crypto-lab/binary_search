COMMAND="./bs $1 runtime-config private-$1.pem"
SLEEPTIME=(5 5 3)

for i in {26..30}
do
    for j in {8..12}
    do
	for k in {1..3}
	do
            echo "COMMAND: $COMMAND $i 32 1 $j 8"
            eval "$COMMAND $i 32 1 $j 8"
            sleep ${SLEEPTIME[$1]}
	done
    done
done
