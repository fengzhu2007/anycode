# anycode

文本编辑器，支持连接FTP,SFTP,阿里云OSS,腾讯云COS

Text editor, supporting connection to FTP, SFTP, AliCloud OSS, Tencent Cloud COS

## 截屏

![](./screens/anycode.png)  


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




## 参考网站

https://www.freedesktop.org/wiki/Software/uchardet/  

https://github.com/KDE/syntax-highlighting  


https://www.riverbankcomputing.com/static/Docs/QScintilla/classQsciScintilla.html  

https://astyle.sourceforge.net/  



https://github.com/geoffstevens8/best-light-themes-pack?tab=readme-ov-file