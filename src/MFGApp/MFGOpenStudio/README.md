# MFGOpenStudio

このフォルダはQt製のMFGのIDEであるMFGOpenStudioのフォルダです。

ビルドには、vcpkgやcmakeといった共通の設定（[一つ上のディレクトリのREADME.md](../README.md)の「ビルドのための共通の設定」を参照）が終わっている必要があります。

さらにそれに加えて、MFGOpenStudioはQtを使用しているため、Qtのセットアップが必要です。
qt-cmakeにパスが通っている必要があります。

## Mac版

ninja, cmake, vcpkgなどは[一つ上のディレクトリのREADME.md](../README.md)の「ビルドのための共通の設定」を参照してください。

それに加えてQtの設定が必要です。

### Qtの設定

QtはQtCreatorのMaintenanceToolで入れています（~/Qt/6.10.3/などに入る）
homebrewのQtではmacdeployが上手く動かなかったためです。


### ビルド手順

1. qtのbinにパスを通す（`export PATH=$PATH:${HOME}/Qt/6.10.3/macos/bin`など）
2. setup.shを実行（buildディレクトリが出来る）
3. build_mac.shを実行
4. open build/Release/MFGOpenStudio.app で実行


1は`qmake -query`が成功すればパスは通っています。

### デプロイに関して

Qtのアプリはリリース時にはmacdeployqtを実行して関連ライブラリをバンドルに含める必要があります。
ビルドをしたあとに、

- deploy_mac.shを実行

とすればバンドルに必要なQtのライブラリ等がコピーされて、Qtの入っていないマシンでも実行出来るようになります。

## Windows版

Windows版は今の所Visual Studio Community 2022で開発しています。

### Qtの設定

QtCreatorから、以下をインストールしました。

- Qt 6.10.3


### ビルド手順

自分はVSをインストールすると一緒にインストールされる「Developer PowerShell for VS2022」から作業しています。
（普段からこれを使っている）。
これだとVSやSDK関連のコマンドのパスが通っているので、それ系の設定をしなくて済みます。

1. 「Developer PowerShell for VS2022」で作業する
2. Qtのbinにパスを通す（$Env:Path += ';C:\Qt\6.10.3\msvc2022_64\bin\' などを実行している）
3. setup_win32.batを実行（buildディレクトリが出来る）
4. build_win.batを実行
5. build/Release/MFGOpenStudio.exe を実行

2は`qmake -query`が成功すればパスは通っています。

### windows版のdeployに関して

こちらもdeploy_win.batを実行する必要があります。

## 外部ライブラリ

ソースコードに含まれているライブラリについては、[../../README.md](../../README.md)を参照ください。

MFGOpenStudioは、上記以外に、vcpkgの以下のライブラリを利用しています。

- [vcpkg package - minizip-ng](https://vcpkg.io/en/package/minizip-ng.html)
