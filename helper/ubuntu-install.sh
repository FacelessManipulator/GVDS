#!/bin/bash

function LibraryExists () {
  if [ -z $2 ];then
      SEARCH_PATH="/usr/lib"
    else
      SEARCH_PATH=$2
  fi
  result=`find ${SEARCH_PATH} -name ${1} | wc -l`
  if [[ $result -ge 1 ]] ;
  then return 1
  else
       return 0
  fi
}

function InstallGNU7 () {
  echo "Installing GNU 7"
  sudo apt-get install -y software-properties-common
  sudo add-apt-repository ppa:ubuntu-toolchain-r/test
  sudo apt update
  sudo apt install g++-7 -y
  sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-7 60 \
                           --slave /usr/bin/g++ g++ /usr/bin/g++-7
  sudo update-alternatives --config gcc
  echo "GNU 7 installed"
  gcc --version
}

sudo apt-get install -y doxygen libev-dev libevent-dev build-essential

GCC_VERSION=`gcc --version | head -n 1 | awk '{print $4}'`
if [[ $GCC_VERSION > 7.0 ]]; then
	echo "You gcc version is greater than 7.0, passed."
else
	InstallGNU7
fi
