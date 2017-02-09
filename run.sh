#1/bin/sh

DISPLAY=$1 /tmp/xboard-4.5.2/xboard -ics -icshost freechess.org -zp -fcp ./bistromath -zippyMaxGames 5 -zippyPassword2 '!' -zippyGameEnd 'seek 1 1 m
seek 3 0 m
seek 15 15 m' | ./ficscolors.pl
