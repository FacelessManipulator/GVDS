# GVDS

## How to build an GVDS cluster with empty linux env?
1. Download and install couchbase-server.
[Couchbase Deb in lab's server](http://10.134.150.155/owncloud/index.php/s/HavNXn93aRfQDML)  
    a. use `sudo dpkg -i couchbase***.deb` to install pkg
    b. after install it, use `sudo apt install --fix-broken` to install its dependencies
2. Prepare build env or download the lattest gvds distribution
    a. use helper/ubuntu-install.sh to prepare build env in ubuntu
    b. install cmake>=3.14 by yourself, cmake install script could be download at [CMake install shell](http://10.134.150.55/CMAKE/cmake-3.15.3-Linux-x86_64.sh)
    c. build and install libfuse, following the readme at [Libfuse github](https://github.com/libfuse/libfuse)
3. Build it with `./build.sh`, you may wait a long time for fetching third-party repo from remote gitlab server
4. install files: `cd build; sudo make install`
5. initilize standalone gvds cluster. use the python script with `sudo python /opt/hvs/bin/init-cluster.py`, follow the instructions given by this script.
6. try with pre-built-in test acconut, `cd /mnt/hvs` and do something.

## How to pack a binary package?
1. make sure you have already successfully built the gvds with `build.sh`
2. change directory into `build/` and `cp ../help/postinstall-pak ./`
3. pack it ! `../help/pack_deb.sh`
4. you would see an deb package called `gvds_[VERSION]_[ARCH].deb` in your build directory. it could be installed on other ubuntu systems with similar version. The Only Depandencies Missed Is Libfuse. [Someone would fix it Sometime.]

## forgive my plastic english. my sogoupinyin has broken so I can't type chinese on Ubuntu