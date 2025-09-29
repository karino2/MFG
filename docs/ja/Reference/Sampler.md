# サンプラー

- 前: [ifelとループ](IfelLoop.md)
- [トップへ](README.md)


テンソルのロード方法をカスタマイズする機能として、サンプラーというものがあります。

```mfg
let finput = sampler<input_u8>(address=.ClampToEdge, coord=.NormalizedLinear)
```

以後、このfinputは通常のテンソルのように使えます。

samplerには以下の３つを基本的には指定します。

- テンソル
- アドレスモード(address引数)
- 座標指定(coord引数)

この３つを順番に見ていきましょう。

## テンソルの指定

samplerは対象とするテンソルから値を取得する時に、その取得の仕方をカスタマイズするものです。
ですから必ず、対象とするテンソルを指定します。

MFGではテンソルは変数とは違う特別なものになるため、シンタックス上も他の引数とは区別しています。
テンソルの指定はどこでも角括弧、つまり`<>`で囲んで指定します。

以下は`input_u8`を指定する例です。

```
sampler<input_u8>(...)
```

## アドレスモード（address引数）

境界より外の座標を指定した時の振る舞いを指定するものを、アドレスモードと呼んでいます。

アドレスモードはEnum型で以下の３つのうちのどれかを指定します。

| Enumのシンボル | 効果 |
| ---- | ---- |
| None | デフォルト。はみ出し処理を何もしない、動作は未定義（プログラム的にはみ出さない事が分かっている時の処理） |
| ClampToEdge | 端の値を繰り返す |
| ClampToBorderValue | border_valueで指定した値 |

Enumを引数で使う時は`.`で始める事に注意。

ClampToBorderValueの時だけborder_valueという追加の引数が必要になります。

```mfg
let extend = sampler<input_u8>(address=.ClampToBorderValue, border_value=u8[0, 0, 0, 0])
```

border_valueの値の型はテンソルの要素の型と揃っている必要があります。

## 座標指定(coord引数)

座標指定には、通常のピクセル単位のi32と、0.0〜1.0にnormalizeされたf32の座標指定が選べます。
f32にnormalizeされた座標では、間を指定した時の挙動により二種類のEnumがあります。

| Enumのシンボル | 解説 |
| ---- | ---- |
| Pixel |  デフォルト。i32で指定 |
| NormalizedNearest | 0.0〜1.0で、間の値は一番近いピクセルが使われる |
| NormalizedLinear | 0.0〜1.0で間の値はbinlinearで補間される |

NormalizedXXX系を指定すると、以下のように引数をf32で指定するようになります。

```mfg
let finput = sampler<input_u8>(coord=.NormalizedLinear)
finput(0.3, 0.7)
```

テンソル定義で現在の座標をこのf32にnormalizeされた座標に変換するためには、
テンソルのto_ncoordが使えます。

```mfg
let finput = sampler<input_u8>(coord=.NormalizedLinear)

def result_u8 |x, y| {
   let fxy = to_ncoord([x, y])
   let col = finput(*fxy)
   ...
}
```

なお、このアスタリスクはsplat演算子です。
詳細は[テンソル](Tensor.md)を参照ください。

## サンプラーの引数は名前つき引数として使えます

coordやaddress、border_valueは名前つき引数として使えます。

## 次

[transとreduce](TransReduce.md)