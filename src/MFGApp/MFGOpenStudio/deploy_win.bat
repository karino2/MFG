 mkdir .\build\MFGOpenStudio
 cp .\build\Release\MFGOpenStudio.exe .\build\MFGOpenStudio\
 @rem なぜかワイルドカードの展開をするとバックスラッシュがエスケープになる…
 cp ./build/Release/*.dll .\build\MFGOpenStudio\
 windeployqt .\build\MFGOpenStudio\MFGOpenStudio.exe
