#!/bin/sh

oldpwd=`pwd`
cd /tmp
#wget ftp://ftp.club.cc.cmu.edu/gnu/xboard/xboard-4.4.3.tar.gz
#wget http://mirror.anl.gov/pub/gnu/xboard/xboard-4.4.3.tar.gz
#wget http://mirror.anl.gov/pub/gnu/xboard/xboard-4.5.2.tar.gz
wget http://ftp.gnu.org/gnu/xboard/xboard-4.5.2.tar.gz
tar xvzf xboard-4.5.2.tar.gz
cd xboard-4.5.2
patch < $oldpwd/xboard-4.4.2-zippypassword2.patch
./configure
# fuck ccache
export PATH="/usr/local/bin/:$PATH"
make
