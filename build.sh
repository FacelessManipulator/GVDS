#!/bin/bash
cmake -H. -Bbuild -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON -DCMAKE_INSTALL_PREFIX=/tmp/hvs
cmake --build build -j