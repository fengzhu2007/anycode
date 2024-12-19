#!/bin/sh
cd zlib
#make clean
./configure --prefix="$PWD/../bin/x64"
make
make install
cd ../



cd openssl
#make clean
./Configure linux-x86_64 no-asm --prefix="$PWD/../bin/x64"
make
make install
cd ../


cd apr
#make clean
./buildconf
./configure --prefix=$PWD/../bin/x64
make
#make test
make install
cd ../


#"libexpat/expat","libssh2","libgit2"]
cd libexpat/expat
./buildconf.sh
./configure --prefix=$PWD/../../bin/x64
make
make install
cd ../../

cd libssh2
autoreconf -fi
./configure --prefix=$PWD/../bin/x64 --disable-debug   --enable-static=no
make
make install
cd ../


cd curl
#切换到 7.8
git checkout 046209e561b7e9b5aab1aef7daebf29ee6e6e8c7
autoreconf -fi
./configure --prefix=$PWD/../bin/x64 --disable-debug --enable-optimize --enable-static=no --with-openssl=$PWD/../bin/x64 --with-zlib=$PWD/../bin/x64 --with-libssh2 --with-libssh2=$PWD/../bin/x64 --enable-websockets
make
make install
cd ../



