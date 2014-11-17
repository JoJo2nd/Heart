#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

if [ -d ./int/ ]; then
  rm -rf ./int/*
else
  mkdir ./int
fi

sudo apt-get -y install git
sudo apt-get -y install python
sudo apt-get -y install cmake
sudo apt-get -y install build-essential
sudo apt-get -y install clang llvm libc++-dev libc++abi-dev
sudo update-alternatives --set cc /usr/bin/clang
sudo update-alternatives --set c++ /usr/bin/clang++-libc++
sudo apt-get -y install libsdl2-dev
sudo apt-get -y install libfreetype6-dev
sudo apt-get -y install libglew-dev
sudo apt-get -y install libfreeimage-dev

# get and build protobuf
sudo apt-get -y install dh-autoreconf

if [ ! -d ./int/usr ]; then
    mkdir ./int/usr
fi

pushd ./int
wget https://protobuf.googlecode.com/files/protobuf-2.5.0.tar.bz2
tar -xvjpf protobuf-2.5.0.tar.bz2
pushd ./protobuf-2.5.0
./autogen.sh
./configure --prefix="$DIR/int/usr" CC=clang CXX="clang++ -std=c++11 -stdlib=libc++" CXXFLAGS="-O3"
make -j8
make install
popd
rm protobuf-2.5.0.tar.bz2
popd

git clone https://github.com/JoJo2nd/lua-protobuf ./int/lua-protobuf
pushd ./int/lua-protobuf
wget https://bootstrap.pypa.io/ez_setup.py -O - | python - --user
python setup.py install
popd

if [ ! -d ./int/cmake_debug ]; then
    mkdir ./int/cmake_debug
fi
# create cmake directory
pushd ./int/cmake_debug
cmake -G "Unix Makefiles" ../../ -DCMAKE_INCLUDE_PATH="$DIR/int/usr/include" -DCMAKE_LIBRARY_PATH="$DIR/int/usr/lib" -DCMAKE_PROGRAM_PATH="$DIR/int/usr/bin" -DCMAKE_BUILD_TYPE=Debug
make -j8
popd

if [ ! -d ./int/cmake_release ]; then
    mkdir ./int/cmake_release
fi
pushd ./int/cmake_release
cmake -G "Unix Makefiles" ../../ -DCMAKE_INCLUDE_PATH="$DIR/int/usr/include" -DCMAKE_LIBRARY_PATH="$DIR/int/usr/lib" -DCMAKE_PROGRAM_PATH="$DIR/int/usr/bin" -DCMAKE_BUILD_TYPE=Release
popd
