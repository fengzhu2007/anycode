#!/bin/sh
cd zlib
make clean
./configure --prefix="$PWD/../bin/x64"
make
make install
cd ../



cd openssl
make clean
./Configure darwin64-x86_64-cc no-asm --prefix="$PWD/../bin/x64"
make
make install
cd ../


cd apr
make clean
./buildconf
./configure --prefix=$PWD/../bin/x64
make
#make test
make install


cd ../
cd apr-util
make clean
./buildconf
./configure --prefix=$PWD/../bin/x64 --with-apr=$PWD/../bin/x64
make
make install
