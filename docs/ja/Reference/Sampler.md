# サンプラー

テンソルのロードをカスタマイズする、サンプラーというものがある。基本的には座標指定を0.0〜1.0にノーマライズして間をどう補間するか（nearestかlinear）と、境界より外側をどう扱うか（端の値を繰り返す、指定したボーダーの値を繰り返す）の組み合わせとなる。

```cpp
let finput = sampler<input_u8>(address=.ClampToEdge, coord=.NormalizedLinear)
```

以後、このfinputは通常のテンソルのように使える。

引数はaddress, coord, border_valueの３つで、最後のborder_valueは.ClampToBorderValueの時だけ使われる。種類はenumで指定し、enumはドット始まりのPascalCaseがコンベンション。.ClampToEdgeとドットで始まる事に注意。

デフォルトの時は指定しなくても良い。

## アドレスモード（address）

境界より外の座標を指定した時の振る舞いを指定。address引数で指定。以下がある。（コード上はドットで始める必要がある事に注意、以下同じ）

- None … デフォルト。はみ出し処理を何もしない、動作は未定義（プログラム的にはみ出さない事が分かっている時の処理）
- ClampToEdge … 端の値を繰り返す
- ClampToBorderValue … border_valueで指定した値

Noneは安全のために将来はWrapにするかも。スクリプトとしてはデフォルトなので指定しないでおく方が将来の変更に対応出来て良さそう。

border_valueの例を示しておく。

```cpp
let extend = sampler<input_u8>(address=.ClampToBorderValue, border_value=u8[0, 0, 0, 0])
```

border_valueの値の型はテンソルの要素の型と揃っている必要がある事に注意。

## 座標指定(coord)

通常のピクセル単位のi32と、0.0〜1.0にnormalizeされたf32の座標指定が選べる

- Pixel …  デフォルト。i32で指定
- NormalizedNearest … 0.0〜1.0で、間の値は一番近いピクセルが使われる
- NormalizedLinear … 0.0〜1.0で間の値はbinlinearで補間される

NormalizedXXX系を指定すると、以下のように引数をf32で指定するようになる。

```cpp
let finput = sampler<input_u8>(coord=.NormalizedLinear)
finput(0.3, 0.7)
```

[MEP 18: デコレータをsamplerと揃える](../MEP/18.md)
