cd subversion
python gen-make.py --vsnet-version=2019  --with-openssl=..\bin\x64 --with-zlib=..\bin\x64 --with-apr=..\bin\x64 --with-apr-util=..\bin\x64  --with-sqlite=..\sqlite
msbuild subversion_vcnet.sln /t:__MORE__ /p:Configuration=Release /p:Platform=x64
pause;