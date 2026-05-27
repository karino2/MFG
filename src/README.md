# MFG関連ソースコードのディレクトリ

このディレクトリの下にはMFG関連のソースコードが置かれています。

## ディレクトリ構成

- _pgn MFGのコアのコードとそれが依存するライブラリが置かれています。これはポータブルな部分となります。
- MFGApp MFGを使用したアプリケーションのCMakeLists.txtと関連ファイルが置かれています

実行バイナリやビルド方法は[MFGApp/README.md](MFGApp/README.md)を参照ください。

## ライセンス

以下で特別に言及するもの以外はApache 2.0ライセンスです。
詳細は[LICENSE](LICENSE)を参照してください。

picojsonは [2-clause BSD license](https://opensource.org/license/BSD-2-Clause) です。

- [_pgn/_picojson/picojson.h](_pgn/_picojson/picojson.h) [kazuho/picojson: a header-file-only, JSON parser serializer in C++](https://github.com/kazuho/picojson) 

以下の二つはMITライセンスです。

- [_pgn/tools/cppunzip.hpp](_pgn/tools/cppunzip.hpp) [karino2/cppunzip: Header only unzip library only depend on zlib and STL.](https://github.com/karino2/cppunzip)
- [MFGApp/mfg_cli/nfiftest.hpp](MFGApp/mfg_cli/nfiftest.hpp) [karino2/nfiftest: Not fancy, intellisence friendly, header only, C++11 supported test library](https://github.com/karino2/nfiftest)