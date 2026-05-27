# MFGApp

ここは各プロジェクトのCMakeLists.txtやその関連ファイルを含むディレクトリです。

## プロジェクト

大きく二つのプロジェクトがあります

- mfg_cli コマンドラインから実行するUnitTestやIRやコンパイル結果のダンプ、pngファイルに適用してみる機能などがあります。
- MFGOpenStudio Qt製のIDEです。MFGStudioから社内ライブラリがないと実現が難しい機能を削ってオープンソース化したものです。

## その他のディレクトリ

- OpenCommon MFGOpenStudioとmfg_cliで共通のファイルが置かれています
- StudioLib MFGStudioとMFGOpenStudioで共通のファイルが置かれています

## ビルドのための共通の設定

mfg_cliとMFGOpenStudioをビルドするのに必要な、共通な設定をここに記述します。
どちらもvcpkgやcmakeなどの基本的なものは共通になります。

### Windows

- Visual Studio Community 2022
- cmake

**vcpkgの設定は不要**

VisualStudioをインストールする時にインストールされる、「Developer PowerShell for VS2022」から作業すればすべて設定されている状態になるので、
等に他の設定は不要です。

### Mac

Macでは以下を必要としています。

- XCode
- ninja
- cmake
- vcpkg

ninjaとcmakeはhomebrewなどでインストールしてください。

**vcpkgの設定**

minizipのライブラリを使うためにvcpkgを使っています。
vcpkgはこのプロジェクトローカルでは無くマシンで一回設定すれば良いものです。

Mac版では手動でセットアップする必要があります。

vcpkgのフォルダはどこでもOKです。以下を実行してください。

```
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
./bootstrap-vcpkg.sh
./vcpkg integrate install
```

さらにCMakeから使えるように以下を~/.zshrcに設定。

```
export VCPKG_ROOT=/path/to/vcpkg
```

これは、以下があるように設定。

```
ls ${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake
```

