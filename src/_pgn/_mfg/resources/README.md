## 国際化関連情報

mfg_resource_trans_str.cppを翻訳してもらう形式になっている。

普段の作業では、mfg_resource.hppにenumを追加して使っていき、
このenumのidのうち、まだ翻訳されてないものをpythonのスクリプトでコメントとして追加するようにしている。

### ファイル概要

- tasks.py リソース関連の処理をするスクリプト
- ../mfg_resource.hpp enumのIDが追加されるファイル
- ../mfg_resource_gen.cpp enumからの逆ルックアップテーブルを生成してこのファイルに保存している。いつも自動生成されて人間はいじらない。
- ../mfg_resource_trans_str.cpp 翻訳をしてもらうファイル。mfg_resoure.hppのenumにあってここのcaseに無いものはpythonスクリプトで追記している。人間とスクリプトの両方が更新
- ../mfg_resource_ui_trans_str.cpp サンプルコードのラベルなど、アプリケーションが使うUIResId関連の翻訳を含むファイル。MFGのコアには不要なので別のファイルとしている。


### mfg_resource_gen.cppの更新

`invoke gen-emap-cpp` でmfg_resource.hppを元にmfg_resource_gen.cppが生成される。enumを追加したらこの作業を行う。

### mfg_resource_trans_str.cppに未翻訳エントリを足す

`invoke update-trans-cpp` で、mfg_resource.hppのenumにあって、mfg_resource_trans_str.cppのcaseが無いものをコメントとして追加。