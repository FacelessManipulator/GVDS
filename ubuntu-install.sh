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

function InstallGTest () {
  echo "Installing Gtest"
  cd /tmp
  curl -L https://github.com/google/googletest/archive/release-1.8.1.tar.gz -o gtest.tar.gz
  tar -xzf gtest.tar.gz
  cd /tmp/googletest-release-1.8.1 && cmake . -DBUILD_SHARED_LIBS=ON -DCMAKE_INSTALL_PREFIX=/usr &&  sudo cmake --build . --target install -j 6
  echo "Gtest installed."
}

function InstallRapidjson () {
  echo "Installing rapidjson"
  cd /tmp
  curl -L https://github.com/Tencent/rapidjson/archive/v1.1.0.tar.gz -o rapidjson.tar.gz
  tar -xzf rapidjson.tar.gz
  cd /tmp/rapidjson-1.1.0 && cmake . -DRAPIDJSON_BUILD_DOC=OFF -DRAPIDJSON_BUILD_EXAMPLES=OFF -DRAPIDJSON_BUILD_TESTS=OFF -DRAPIDJSON_HAS_STDSTRING=ON -DCMAKE_INSTALL_PREFIX=/usr &&  sudo cmake --build . --target install -j 6
  echo "Rapidjson installed."
}

function InstallCouchbaseClient () {
  echo "Installing couchbase client"
  cd /tmp
  curl -L https://github.com/couchbase/libcouchbase/archive/2.10.3.tar.gz -o couchbase-client.tar.gz
  tar -xzf couchbase-client.tar.gz
  cd /tmp/libcouchbase-2.10.3/ && cmake . -DCMAKE_INSTALL_PREFIX=/usr -DLCB_NO_TESTS=ON -DLCB_SKIP_GIT_VERSION=ON && sudo cmake --build . --target install -j 6
  echo "Couchbase client installed."
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

sudo apt-get install -y doxygen rapidjson-dev build-essential

LibraryExists libgtest.so
if [[ $? -eq 1 ]]; then 
	echo "Found gtest installed in /usr/lib, passed."
else
	InstallGTest
fi

LibraryExists rapidjson.h /usr/include
if [[ $? -eq 1 ]]; then 
	echo "Found rapidjson installed in /usr/include, passed."
else
	InstallRapidjson
fi

LibraryExists libcouchbase.so /usr/lib
if [[ $? -eq 1 ]]; then 
	echo "Found libcouchbase installed in /usr/lib, passed."
else
	InstallCouchbaseClient
fi

GCC_VERSION=`gcc --version | head -n 1 | awk '{print $4}'`
if [[ $GCC_VERSION > 7.0 ]]; then
	echo "You gcc version is greater than 7.0, passed."
else
	InstallGNU7
fi

