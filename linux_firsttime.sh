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
# wget https://protobuf.googlecode.com/files/protobuf-2.5.0.tar.bz2

if [ -d ./int/lua-protobuf]; then
    rm -rf ./int/lua-protobuf/*
    rmdir ./int/lua-protobuf
if
git clone https://github.com/JoJo2nd/lua-protobuf ./int/lua-protobuf
pushd ./int/lua-protobuf
wget https://bootstrap.pypa.io/ez_setup.py -O - | python - --user
python setup.py install
popd
#sudo apt-get -y install protobuf-compiler
#sudo apt-get -y install libprotobuf-dev
#sudo apt-get -y install libprotoc-dev
if [ ! -d ./int/cmake]; then
    mkdir ./int/cmake
if
# create cmake directory
pushd ./int/cmake