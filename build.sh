#!/bin/bash

if [[ ! -d /opt/hvs ]]; then
    echo "Generating hvs.conf in /opt/hvs/hvs.conf..."
    sudo mkdir -p /opt/hvs
    sudo chown -R `whoami`:`whoami` /opt/hvs
fi

if [[ ! -e /opt/hvs/var/log/hvs.log ]]; then
    mkdir -p /opt/hvs/var/log
    touch /opt/hvs/var/log/hvs.log
fi

if [[ ! -d /opt/hvs/var/data ]]; then
    mkdir -p /opt/hvs/var/data
fi

cmake -H. -Bbuild -DCMAKE_BUILD_TYPE=RELEASE -DBUILD_TESTING=OFF -DCMAKE_INSTALL_PREFIX=/opt/hvs
cmake --build build -j 4

