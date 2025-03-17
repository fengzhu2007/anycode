# anycode文本编辑器 - 多功能项目管理工具

## 项目概述

本项目是一个基于 Qt 开发的文本编辑器，集成了常用的文本编辑功能，并提供了对多种对象存储服务（如 FTP、SFTP、阿里云 OSS、腾讯云 COS、亚马逊 S3 等）的管理功能。用户可以通过该软件创建和管理项目，并将本地项目快速上传到指定的服务器。此外，软件还支持通过 Git 和 SVN 检查文件差异，确保每次只传输有变化的文件，从而提高工作效率。

## 主要功能

### 文本编辑功能
- **基本文本编辑**：支持常见的文本编辑操作，如复制、粘贴、撤销、重做等。
- **语法高亮**：支持多种编程语言的语法高亮显示。
- **查找与替换**：支持文本的查找与替换功能。
- **多标签页**：支持多标签页编辑，方便同时处理多个文件。

### 对象存储管理
- **FTP/SFTP**：支持通过 FTP 和 SFTP 协议上传、下载和管理文件。
- **阿里云 OSS**：集成阿里云对象存储服务，支持文件的上传、下载和管理。
- **腾讯云 COS**：集成腾讯云对象存储服务，支持文件的上传、下载和管理。
- **亚马逊 S3**：集成亚马逊 S3 对象存储服务，支持文件的上传、下载和管理。

### 项目管理
- **项目创建与管理**：用户可以创建和管理项目，方便组织本地文件和远程文件。
- **快速上传**：支持将本地项目快速上传到指定的对象存储服务。
- **差异检查**：通过 Git 或 SVN 检查文件的差异，确保只传输有变化的文件。



### 编译与运行
1. 克隆本项目到本地：
   ```bash
   git clone https://github.com/yourusername/yourproject.git

## 截屏

![](https://github.com/fengzhu2007/anycode/blob/dev/screen/anycode.png)  


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

https://github.com/tree-sitter/tree-sitter

https://www.freedesktop.org/wiki/Software/uchardet/  

https://github.com/KDE/syntax-highlighting  


https://www.riverbankcomputing.com/static/Docs/QScintilla/classQsciScintilla.html  

https://astyle.sourceforge.net/  


https://github.com/geoffstevens8/best-light-themes-pack?tab=readme-ov-file