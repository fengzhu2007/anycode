## Windows 构建  
开发环境 Qt5.15.2  
Visual Studio 2019以上 C++17  

以管理员身份打开 VS Command Tool(x64)  

####  编译 "zlib","apr","libexpat/expat","apr-util","libssh2","libgit2"  
需要安装cmake,python3.10 以上版本  
```
cd 3party
python build_all.py
```

#### 编译 openssl  
需要安装perl (ActiveStatePerl或者Strawberry Perl https://strawberryperl.com/)   
```
build_openssl_x64.bat
```
#### 编译 curl 选择7.88; 8.x以上依赖库（zlib,openssl,libssh）需要更新更高版本
```
python build_curl.py
```


#### 编译 subversion  
```
build_subversion_x64.bat
```

使用Qt Creator 打开项目（根目录CMakeLists.txt文件）
由于项目是使用CMake管理。所以需要将Qt框架下的CMake和Ninja 加入到环境变量PATH中。最好从PATH中删除其他版本的CMake(Strawberry Perl安装也有CMake)
Ninja必须使用Qt自带的，否则会有问题；并且如果不加入环境变量CMake可能会生成VS项目文件进行编译，会有问题。  
Qt Creator  编译生成之后将Qt 依赖库和插件库（iconengines，imageformats，platforms，sqldrivers）复制到 anycode.exe目录下；并且将第三方编译生成的动态链接库（3party\bin\x64\bin）也复制到相同目录中。



## Mac OS 构建  
开发环境 Qt5.15.2  
XCode 13.4 以上

将Qt Cmake 和 Ninja 加入环境变量



####  编译依赖 "zlib","apr","openssl","apr-util"
```
chmod -R 777 build.sh
./build.sh
```

#### 编译依赖 "libexpat/expat","libssh2","libgit2"
需要安装cmake,python3.8 以上版本  

```
python3 build_mac.py
```


#### 编译subversion
```
./autogen.sh
./configure --prefix=$PWD/../bin/x64   --enable-static --with-utf8proc=internal --with-ssl --with-openssl=$PWD/../bin/x64 --with-zlib=$PWD/../bin/x64 --with-apr=$PWD/../bin/x64 --with-apr-util=$PWD/../bin/x64  --with-sqlite=$PWD/../sqlite/sqlite3.c
make
make install
```

## linux (Ubuntu 22) 构建  

开发环境 Qt5.15.2  

```
sudo apt install libxcb-cursor0
sudo apt-get install libxcb-xinerama0
```

gcc `apt install gcc`  
g++  `sudo apt install g++`  
make `apt install make`  
autoconf  `apt install autoconf`  
libtool   `apt install libtool-bin`  



将Qt Cmake 和 Ninja 加入环境变量  

####  编译依赖 "zlib","apr","openssl" ,"libexpat/expat","libssh2",
```
chmod -R 777 build_linux.sh
./build_linux.sh
```

#### 编译依赖 "libgit2"  

需要安装cmake,python3.8 以上版本  

```
python3 build_linux.py
```


####  编译依赖 "apr-util"

```
cd apr-util
#make clean
./buildconf
./configure --prefix=$PWD/../bin/x64 --with-apr=$PWD/../bin/x64  --with-expat=$PWD/../bin/x64
make
make install
```

##### 重要！！修改文件apu-1-config，在show_usage之前（大约54行）添加 location=installed；主要修正编译subversion时包含的目录的问题


#### 编译subversion



```
cd subversion
./autogen.sh
./configure --prefix=$PWD/../bin/x64  --disable-shared  --enable-static --with-utf8proc=internal --with-lz4=internal  --with-zlib=$PWD/../bin/x64 --with-apr=$PWD/../bin/x64 --with-apr-util=$PWD/../bin/x64 --with-expat=$PWD/../bin/x64/include:$PWD/../bin/x64/lib:expat  --with-sqlite=$PWD/../sqlite/sqlite3.c  CFLAGS="-fPIC" CXXFLAGS="-fPIC"

make
make install
```


