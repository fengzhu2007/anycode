# anycode
Multi protocol file transfer tool, supports FTP SFTP OSS COS  



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
#### 编译 curl
```
python build_curl.py
```


#### 编译 subversion  
```
build_subversion_x64.bat
```

使用Qt Creator 打开项目（根目录CMakeLists.txt文件）
由于项目是使用CMake管理。所以需要将Qt框架下的CMake 加入到环境变量PATH中。最好从PATH中删除其他版本的CMake(Strawberry Perl安装也有CMake)

https://www.freedesktop.org/wiki/Software/uchardet/  

https://github.com/KDE/syntax-highlighting  


https://www.riverbankcomputing.com/static/Docs/QScintilla/classQsciScintilla.html  

https://astyle.sourceforge.net/  



https://github.com/geoffstevens8/best-light-themes-pack?tab=readme-ov-file