#!/bin/sh

echo "Qtのbinにパスを通してから実行してね。"
echo "例:"
echo 'export PATH=$PATH:/usr/local/Cellar/qt/6.10.3/bin'
echo 'export PATH=${HOME}/Qt/6.10.3/macos/bin:$PATH'
echo "などなど。homebrewとQtCreatorが併存している時は目的のパスを先に置く事。"
echo ""
echo ""

mkdir build;
qt-cmake . -B build -G "Ninja Multi-Config"; 
