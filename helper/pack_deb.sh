#!/bin/bash
echo "You should RUN build.sh before this script!"

sudo checkinstall --pkgname=gvds --pkgversion=0.1 --pkgrelease=SNAPSHOT --pkgsource=https://gitlab.com/buaaica/hvs-one --maintainer=miller_maxwell@buaa.edu.cn --requires="libc6 \(\>= 2.27\),libevent-core-2.1-6 \(\>= 2.1.8\),libgcc1 \(\>= 1:8.3.0\),libstdc++6 \(\>= 8.3.0\)" --backup=no --stripso=yes


