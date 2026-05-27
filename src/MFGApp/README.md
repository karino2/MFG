# MFGApp

ここは各プロジェクトのCMakeLists.txtやその関連ファイルを含むディレクトリです。

## プロジェクト

大きく二つのプロジェクトがあります

- mfg_cli コマンドラインから実行するUnitTestやIRやコンパイル結果のダンプ、pngファイルに適用してみる機能などがあります。
- MFGOpenStudio Qt製のIDEです。MFGStudioから社内ライブラリがないと実現が難しい機能を削ってオープンソース化したものです。

## その他のディレクトリ

- OpenCommon MFGOpenStudioとmfg_cliで共通のファイルが置かれています
- StudioLib MFGStudioとMFGOpenStudioで共通のファイルが置かれています