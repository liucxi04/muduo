#!/bin/bash

set -e

# 如果没有 build 目录，创建该目录
if [ ! -d `pwd`/build ]; then
  mkdir `pwd`/"build"
fi

rm -rf `pwd`/build/*

cd build &&
   cmake .. &&
   make

cd ..

# 把头文件拷贝到 /usr/include/muduo_ 下， so 库拷贝到 /usr/lib/muduo_ 下
if [ ! -d /usr/include/muduo_ ]; then
  mkdir /usr/include/muduo_
fi

if [ ! -d /usr/lib/muduo_ ]; then
  mkdir /usr/lib/muduo_
fi

for header in `ls ./muduo/base/*.h`
do
  cp $header /usr/include/muduo_
done

for header in `ls ./muduo/net/*.h`
do
  cp $header /usr/include/muduo_
done

for header in `ls ./muduo/net/poller/*.h`
do
  cp $header /usr/include/muduo_
done

cp `pwd`/lib/libmuduo.so /usr/lib/muduo_

ldconfig