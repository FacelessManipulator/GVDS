# GVDS

## How to pack a binary package?
1. Prepare build env or download the lattest gvds distribution
    a. use helper/ubuntu-install.sh to prepare build env in ubuntu
    b. install cmake>=3.14 by yourself, cmake install script could be download at [CMake install shell](http://10.134.150.55/CMAKE/cmake-3.15.3-Linux-x86_64.sh)
        wget http://10.134.150.55/CMAKE/cmake-3.15.3-Linux-x86_64.sh
    c. build and install libfuse, following the readme at [Libfuse github](https://github.com/libfuse/libfuse)
2. Build it with `./build.sh`, you may wait a long time for fetching third-party repo from remote gitlab server
3. make sure you have already successfully built the gvds with `build.sh`
4. change directory into `build/` and `cp ../helper/postinstall-pak ./`
5. pack it ! `../helper/pack_deb.sh`
6. you would see an deb package called `gvds_[VERSION]_[ARCH].deb` in your build directory. it could be installed on other ubuntu systems with similar version. The Only Depandencies Missed Is Libfuse. [Someone would fix it Sometime.]

## How to build an GVDS cluster with empty linux env?
1. Download couchbase and gvds pkg.
2. install couchbase-server.
[Couchbase Deb in lab's server](http://10.134.150.155/owncloud/index.php/s/HavNXn93aRfQDML)  
    a. use `sudo dpkg -i couchbase***.deb` to install pkg
    b. after install it, use `sudo apt install --fix-broken` to install its dependencies
2. use `sudo dpkg -i gvds_[VERSION]_[ARCH].deb` to install gvds
3. build and install libfuse.
    a. following the readme at [Libfuse github](https://github.com/libfuse/libfuse)
    b. please append `user_allow_other` in `/etc/fuse.conf`
4. initilize standalone gvds cluster. use the python script with `sudo python /opt/hvs/bin/init-cluster.py`, follow the instructions given by this script.
5. try with pre-built-in test account, `cd /mnt/hvs` and do something. 
    eg: `/opt/hvs/bin/userlogin -u test -p password`

## forgive my plastic English. my sogoupinyin has broken so I can't type Chinese on Ubuntu