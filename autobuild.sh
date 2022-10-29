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

# 把头文件拷贝到 /usr/include/muduo_ 下， so 库拷贝到 /usr/lib 下
# 如何将 so 库拷贝到 /usr/lib/muduo_ 下？
if [ ! -d /usr/include/muduo_ ]; then
  mkdir /usr/include/muduo_
fi
if [ ! -d /usr/include/muduo_/base ]; then
  mkdir /usr/include/muduo_/base
fi
if [ ! -d /usr/include/muduo_/net ]; then
  mkdir /usr/include/muduo_/net
fi
if [ ! -d /usr/include/muduo_/net/poller ]; then
  mkdir /usr/include/muduo_/net/poller
fi

for header in `ls ./muduo/base/*.h`
do
  cp $header /usr/include/muduo_/base
done

for header in `ls ./muduo/net/*.h`
do
  cp $header /usr/include/muduo_/net
done

for header in `ls ./muduo/net/poller/*.h`
do
  cp $header /usr/include/muduo_/net/poller
done

cp `pwd`/lib/libmuduo.so /usr/lib

ldconfig