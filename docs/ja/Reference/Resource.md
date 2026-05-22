# 国際化とリソース

- 前: [ユーザー定義関数と組み込み関数](BuiltinFunctions.md)
- [トップへ](README.md)

ここでは国際化のための文字列リソースとサムネイルやテクスチャをプロジェクトに含める方法を解説します。

## 文字列リソースと国際化

プロジェクト（またはmar）にstrings.jsonというファイルがあったら、それを文字列リソースとして扱います。

MFGStudioのファイルエクスプローラーのTのボタンでデフォルトのリソースを作る事が出来、
これをウイジェットのラベルなどの国際化に使用する事ができます。

### おおまかな例

まず全体像を簡単に説明します。

以下のようなstrings.jsonがプロジェクト内にあったとします。

```json
{
  "ja": {
    "LABEL_TITLE": "ハーフトーンディザ",
    "LABEL_THRESHOLD": "閾値"
  },
  "en": {
    "LABEL_TITLE": "Halftone Dither", "LABEL_THRESHOLD": "Threshold"
  }
}
```

mfgファイルで、以下のようになっている時に、

```mfg
@title "ハーフトーンディザ"

@param_f32 threshold(SLIDER, label="閾値", init=1.0, min=0.0, max=1.0)
```

この`"ハーフトーンディア"`や`"閾値"`を、リソースにするには以下のようにします。

```mfg
@title $LABEL_TITLE

@param_f32 threshold(SLIDER, label=$LABEL_THRESHOLD, init=1.0, min=0.0, max=1.0)
```

`$`で始めて、ダブルクオーテーションは無しです。

こうしておくと、FireAlpaca3やMFGStudioで、言語設定を日本語から英語に変えたりすると、それぞれ適切な文字列が表示されるようになります。

なお、MFGで文字列を表示するのはアトリビュートと入力ウィジェットのみなので、国際化の対象もここだけとなります。

アトリビュートと入力ウィジェットに関しては[アトリビュートと入力ウィジェット](AttrWidget.md)を参照してください。

### strings.jsonの書式

strings.jsonファイルは以下のようになっています。

```json
{
  "ja": {
    "LABEL_TITLE": "ハーフトーンディザ",
    "LABEL_THRESHOLD": "閾値"
  },
  "en": {
    "LABEL_TITLE": "Halftone Dither", "LABEL_THRESHOLD": "Threshold"
  }
}
```

言語ごとにidとそのidに対応した文字列を指定します。

### 対応している言語

対応している言語は以下の通りです。

| キー | 説明 |
| ---- | ---- |
| zh_Hans | Chinese simplified |
| zh_Hant | Chinese traditional |
| en | 英語 |
| fr | フランス語 |
| de | ドイツ語 |
| ja | 日本語 |
| ko | 韓国語 |
| pt | ポルトガル語 |
| ru | ロシア語 |
| es | スペイン語 |
| hi | ヒンズー語 |
| bn | ベンガル語 |
| pl | ポーランド語 |

指定されている言語が無い場合はenが使われます。

### 文字列リソースの使い方

文字列リソースは、通常のダブルクオートで囲んである文字列全体を、`$`と文字列IDで置き換える事で使用出来ます。

タイトルの場合、以下のようになっていたら、

```mfg
@title "ハーフトーンディザ"
```

これを、以下のように置き換える事で使用出来ます。

```mfg
@title $LABEL_TITLE
```

ドロップダウンなどのウィジェットの場合は以下のようになります。

```mfg
@param_i32 pattern(DROPDOWN, label=$LABEL_SHAPE, items=[$LABEL_SHAPE_SQUARE, $LABEL_SHAPE_SIN, $LABEL_NEWSPAPER, $LABEL_INTERLEAVE])
```

### 実際の例

GitHubのMFGのprojectsには、国際化した多くのフィルタの例が置いてあるので実際にどう使うのかの参考にしてみてください。

[MFG/projects](https://github.com/karino2/MFG/tree/main/projects)

## 画像テクスチャ

プロジェクトの中にpngファイルを含めると、それをテクスチャとしてmfgから使用する事が出来ます。

たとえばプロジェクト内に「4rect_texture16.png」という画像ファイルを置き、以下のように定義すると、ltsというテンソルとして使う事が出来ます。


```mfg
  def lts by load("4rect_texture16.png")
```

loadにはファイル名を渡します。

pngは現時点ではいつもu8v4のBGRAを要素として持つテンソルとしてロードされます。

## サムネイル

thumbnail.pngというファイルがmar内にあると、FireALpaca SE 3.0などはそれをフィルタのサムネイルとして使用します。

thumbnail.pngは512x512のサイズのpng画像を置きます。

MFGStudioのファイルエクスプローラーの上のボタンには、
現在のプレビューを512x512にリサイズしてthumbnail.pngとしてプロジェクトに含める機能があるので、
これを用いてフィルタの特徴が一目でわかるようなサムネイルを含めてください。