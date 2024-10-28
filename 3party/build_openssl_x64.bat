cd ./openssl
perl Configure VC-WIN64A  no-asm --prefix=%cd%/../bin/x64
nmake
nmake install
pause;