cd subversion
python gen-make.py --vsnet-version=2019  --with-openssl=..\openssl --with-zlib=..\bin\win32 --with-apr=..\bin\win32 --with-apr-util=..\bin\win32  --with-sqlite=..\sqlite
msbuild subversion_vcnet.sln /t:__MORE__ /p:Configuration=Release
pause;