# mfg_cli

コマンドライン関連のプロジェクトです。

主に以下の二つのtargetがあります。

- mfg_cli
- unit_test

## ビルド方法

以下を前提としてます。

- cmake
- vcpkg

また、Mac版ではninja、Windows版ではmsbuildを前提としています。
詳細は[一つ上のディレクトリのREADME.md](../README.md)の「ビルドのための共通の設定」を参照ください。

### Macでのビルド

UnitTestは以下の通りです。

1. setup.shを実行(buildディレクトリが出来る)
2. cmake --build build --config Debug --target unit_test を実行
3. build/Debug/unit_test を実行

なお、2と3のunit_testをmfg_cliに変えるとmfg_cliがビルド出来ます。

### Windowsでのビルド

1. 「Developer PowerShell for VS2022」で作業する
2. setup_win.batを実行（buildディレクトリが出来る）
3. cmake --build build --config Debug --target unit_test を実行
4. build/Debug/unit_test.exeを実行

こちらも3と4のunit_testをmfg_cliに変えるとmfg_cliがビルド出来ます。

## mfg_cliの使い方

ビルド結果はbuild/Release/mfg_cliに生成されます。

`mfg_cli --help`で使い方が表示されます。

### 概要

mfg_cliは３つの機能があります。

1. IRのダンプ
2. shaderコードのダンプ
3. pngファイルにフィルタを適用して結果を保存

すべて最後の引数はmfgファイルとなっています。

主な使い方は以下となります。

```sh
$ mfg_cli --help
# ヘルプの表示

$ mfg_cli -ir example.mfg
# example.mfgをパースして最終irを出力

$ mfg_cli -shader example.mfg
# example.mfgをコンパイルして最終シェーダーを出力

$ mfg_cli -target input.png -result result.png example.mfg
# input.pngに対してexample.mfgを実行して結果をresult.pngに保存
```

## unit_testの使い方

unit_testはbuild/Debug/unit_testに生成されます。

実行時のCWDは現在のディレクトリである事を想定して書かれています（test_data下のファイルを相対パスで読みます）。

このREADME.mdのあるディレクトリから

```
$ build/Debug/unit_test
```

などと実行します（Windowsなら最後はunit_test.exe）。

### UnitTestのライブラリ

UnitTestはnfiftestを用いて書かれています。

[karino2/nfiftest: Not fancy, intellisence friendly, header only, C++11 supported test library](https://github.com/karino2/nfiftest)

## 外部ライブラリ

ソースコードに含まれているライブラリについては、[../../README.md](../../README.md)を参照ください。

mfg_cli及びunit_testは、上記以外に、vcpkgの以下のライブラリを利用しています。

- libpng [vcpkg package - libpng](https://vcpkg.io/en/package/libpng.html)
- zlib [vcpkg package - zlib](https://vcpkg.io/en/package/zlib.html)