#!/bin/bash

i=0
for line in `cat book.txt | sed 's/ /,/g'`; do
	(echo "xboard"
	echo "protover 2"
	echo "new"
	echo "force"
	for move in `echo $line | sed 's/,/ /g'`; do echo $move; done
	echo "quit") | ./bistromath | grep Illegal >/dev/null
	if [ "$?" = "0" ]; then
		ILLEGALS="$ILLEGALS\n[$i] $line"
		echo -e "\033[01;31m$ILLEGALS\033[00m"
	fi
	i=$(($i+1))
done
echo -e "\033[01;31m$ILLEGALS\033[00m"
