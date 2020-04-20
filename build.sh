#!/bin/bash

if [[ ! -d /opt/gvds ]]; then
    echo "Generating gvds.conf in /opt/gvds/gvds.conf..."
    sudo mkdir -p /opt/gvds
    sudo chown -R `whoami`:`whoami` /opt/gvds
fi

if [[ ! -e /opt/gvds/var/log/gvds.log ]]; then
    mkdir -p /opt/gvds/var/log
    touch /opt/gvds/var/log/gvds.log
fi

if [[ ! -d /opt/gvds/var/data ]]; then
    mkdir -p /opt/gvds/var/data
fi

cmake -H. -Bbuild -DCMAKE_BUILD_TYPE=DEBUG -DBUILD_TESTING=OFF -DCMAKE_INSTALL_PREFIX=/opt/gvds
cmake --build build -j 3
