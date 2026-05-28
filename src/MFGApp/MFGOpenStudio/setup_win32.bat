@echo off
echo このbatとそのあとのcmakeコマンドのためには、Qtのmsvc2019_64/binにパスを通してください。
echo 以下を適宜バージョン変えて実行してから一連の作業をやってね。
echo:
echo set PATH=%%PATH%%;C:\Qt\6.8.1\msvc2019_64\bin
echo $Env:Path += ';C:\Qt\6.8.1\msvc2022_64\bin\'
echo:
@echo on
mkdir build
qt-cmake . -B build -G "Visual Studio 17 2022" -A x64