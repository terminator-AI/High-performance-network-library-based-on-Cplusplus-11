#!/bin/bash

set -e

if [ ! -d `pwd`/build ]; then
    mkdir `pwd`/build
fi

rm -rf `pwd`/build/*

cd `pwd`/build &&
    cmake .. &&
    make

cd ..


#头文件---> /usr/include/netlib    so库---> /usr/lib

if [ ! -d /usr/include/netlib ]; then
    mkdir /usr/include/netlib
fi

for header in `ls *.hpp`
do
    cp $header /usr/include/netlib
done

cp `pwd`/bin/libmymuduo.so /usr/lib


ldconfig