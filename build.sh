#!/bin/bash
git submodule init
git submodule update
cmake -H. -Bbuild -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON
cd build/ && make

