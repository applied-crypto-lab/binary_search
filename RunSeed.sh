COMMAND="../compiler/bin/picco-seed runtime-config u_bsall"
echo "COMMAND: $COMMAND"
eval "$COMMAND"
while true
do
    if [ -a "flag.txt" ]; then
	eval "rm flag.txt"
	eval "$COMMAND"
    else
	echo "sleep"
	sleep 10
    fi
done


