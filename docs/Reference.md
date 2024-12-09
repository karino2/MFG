# リファレンス

実装する都度メモを残しておいて、どこかでちゃんとしたリファレンスに整理したいと思い、最初からリファレンスという名前のページにしてく。

- [MEP Index](MEPIndex.md)
- [ウォークスルー](Walkthrough.md)

## 名前付き引数

[notes: メディアンフィルタのスクリプト案を検討](notes/median_filter.md)

いくつかの関数呼び出しは名前付き引数をサポートしている。名前付き引数は基本的には名前無しのpositional argsのシンタックスシュガーに過ぎず、名前をつけないと指定出来ない引数は存在しない。全ての引数には順番があって、名前無しで並べる事で指定できる。

名前付きで指定すると順番に関係なく指定できる（パース時に順番が解決されて通常のpositonal argumentsで呼び出されたように処理する）。

名前付き引数が無い関数もあるし、名前が無い引数もある。以下では名前がある関数の一覧を示す。

### trans<TS>.accumulate!(dim)

accumulateにはイテレートする次元を指定するdimと実行するブロックのFuncObjの２つの引数がある。そのうちdimが名前付き引数で指定できる。

```jsx
mut! trans<hist>.accumulate!(dim=1) |i, col, h, accm| { h+accm }
```

### reduce<TS>.accumulate(dim, init)

reduceのaccumulateにはイテレートする次元dim, 初期値のinit、ブロックのFuncObjの３つの引数があるが、このうちdimとinitは名前付き引数で指定できる。

```jsx
def ts2 by reduce<hist>.accumulate(init=1, dim=0) |i, col, val, accm | { accm + val }
```

### trans<TS>.cumsum!(dim)

cumsumもaccumulateと同様にdimが名前付き引数で指定できる。cumsumにはFuncObjの引数は無い。

### reduce<TS>.find_first_index(dim)

これもdimが名前付き引数で指定出来る。

## アトリビュート

アットマークで始まるアトリビュート。

- `@title`
- `@param_i32`, `@param_f32`
- `@bounds`
- `@print_expr`

などがある。

### `@print_expr`

トップレベルのブロックでのみ使える。引数のexprをログへ出力する。引数はnumericのみ。

例:

```cpp
@print_expr( sin(3.14/4) )
```

## タプル

タプルをサポートしている。大かっこ。

```cpp
let a = [1, 2, 3]
```

タプルは異なる型でも良い。

テンソルとしてタプルを返すと、カーネル的には同じループの底でセットされるようになり、タプルの次元数分掛けた回数では無いので、パフォーマンスを考えて使うのに良い。

タプルの値はSwiftと同様にドットと数字で取り出せる。

```cpp
let b = a.1
```

また、後述するdestructuringとswizzleでも取り出せる。

### タプルのdestructuring

letでタプルを展開して受ける事ができる。

```jsx
let [a, b, c] = [1, 2, 3]
```

destructuring letではアンダースコアを使って無視する事もできる。

```jsx
let [a, _, g, b] = value
```

この場合、a, g, bのletしか生成されない。

### Swizzle演算子

タプルはSwizzle演算子をサポートしている。

- xyzwの組み合わせのみサポート
- 右辺値のみ（そもそもタプルのmutationは対応してない）
- ベクトルである必要は無い

例:

```cpp
let a = [1, 2, 3].xxyy
```

aは`[1, 1, 2, 2]`のタプルとなる。

## サンプラー

テンソルのロードをカスタマイズする、サンプラーというものがある。基本的には座標指定を0.0〜1.0にノーマライズして間をどう補間するか（nearestかlinear）と、境界より外側をどう扱うか（端の値を繰り返す、指定したボーダーの値を繰り返す）の組み合わせとなる。

```cpp
let finput = sampler<input_u8>(address=.ClampToEdge, coord=.NormalizedLinear)
```

以後、このfinputは通常のテンソルのように使える。

引数はaddress, coord, border_valueの３つで、最後のborder_valueは.ClampToBorderValueの時だけ使われる。種類はenumで指定し、enumはドット始まりのPascalCaseがコンベンション。.ClampToEdgeとドットで始まる事に注意。

デフォルトの時は指定しなくても良い。

### アドレスモード（address）

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

### 座標指定(coord）

通常のピクセル単位のi32と、0.0〜1.0にnormalizeされたf32の座標指定が選べる

- Pixel …  デフォルト。i32で指定
- NormalizedNearest … 0.0〜1.0で、間の値は一番近いピクセルが使われる
- NormalizedLinear … 0.0〜1.0で間の値はbinlinearで補間される

NormalizedXXX系を指定すると、以下のように引数をf32で指定するようになる。

```cpp
let finput = sampler<input_u8>(coord=.NormalizedLinear)
finput(0.3, 0.7)
```

[MEP 18: デコレータをsamplerと揃える](MEP/18.md)

## テンソルのメソッド

現状ではextentとto_ncoordがテンソルのメソッドと解釈される（ただしto_ncoordはselfが省略されて現在定義中のテンソルとなる）。

### ts.extent(dim)

以下のようにdimを指定してそのテンソルの幅や高さが取れる。

```cpp
let w = input_u8.extent(0)
let h = input_u8.extent(1)
```

wやhはinputの「最大のインデックス-1」となる（0オリジンなので）。

### to_ncoord([x, y])

テンソルの定義内で使われて、0.0〜1.0にノーマライズされた座標を返す。引数は2次元のタプル。

```cpp
def result_u8 |x, y| {
   let [fx, fy] = to_ncoord([x, y])
   ...
}
```

なお、引数の整数座標がextent以上の値の場合は1.0以上を返すことになる。

## 入出力のテンソル

フィルタの入力はinput_uXXと、結果の出力はresult_uXXという名前で指定する事になっている。

uXXはu8かu16のどちらか。

### input_u8とinput_u16

入力のレイヤーは、u8のBGRAと見るかu16のBGRAと見るかが選べる。

input_u8とするとu8、input_u16とするとu16として見る。result_u8とresult_u16も同様。

### 他のレイヤーの参照

現在アクティブなレイヤーの一つ下のレイヤーは、`input_u8[-1]` で参照出来る。上は `input_u8[1]` 。２つ上なども参照出来る。
対象のレイヤーが無ければ0が埋められたテンソルがわたってくる。

なお1とか-1は今の所即値のみで変数とかは使えない。

## u8カラーとノーマライズドカラー

input_u8はu8のBGRAのベクトルとなっている。

また、floatの0.0〜1.0のBGRAのベクトルとして扱うノーマライズドカラー、またの名をncolorへの変換をサポートしている。

u8のBGRAからncolorへの変換はto_ncolorを使う

```jsx
let ncolor = to_ncolor(input_u8(x, y))
```

逆にncolorからu8のBGRAへは、to_u8colorを使う。

```jsx
let u8_bgra = to_u8color(ncolor)
```

to_u8colorは0.0〜1.0にclampしてu8にするので、1.0より大きな値は1.0に、マイナスの値は0.0として扱われる。

## シンタックスシュガーのメモ

ちゃんとしたリファレンスとして整備する前にとりあえず各実装のメモ程度のものをここに溜めておく。

### cumsum

transのaccumulateはだいたいsumを求めたいだけで良くあるので、cumsumを作る。cumsumは、以下の文と、

```jsx
mut! trans<hist>.accumulate!(dim=0) |i, h, accm| { h+accm }
```

以下の文が同じ意味になるようなもの。

```jsx
mut! trans<hist>.cumsum!(dim=0)
```

ts.sumもこういう感じにしたいな。

### find_first_index

reduceで条件を満たす最初のindexを求める事は良くあるので、find_first_indexというシンタックスシュガーを提供する。

[MEP 6: find_first_index](MEP/6.md)

以下のコードは、

```cpp
def _med by reduce<_hist>.find_first_index(dim=0) |i, col, val | {
    val >= _hist(255, col)/2
}
```

以下と同じ意味となる。

```cpp
def _med by reduce<_hist>.accumulate(dim=0, init=-1) |i, col, val, accm| {
  ifel(accm != -1, accm, ...)
  elif(val >= _hist(255, col)/2, i, -1)
}
```

### reduceの結果がスカラーになる時はexprとして使える

reduceの結果が0次元になる時は単なる変数に展開される。この場合は、これをexprとしても使う事も出来る。

つまり以下は、

```cpp
def _med by reduce<_hist>.find_first_index(dim=0) |i, col, val| {
    val >= _hist(255, col)/2
}
```

以下のように単なるletとしても書ける。

```cpp
let _med = reduce<_hist>.find_first_index(dim=0) |i, col, val| {
    val >= _hist(255, col)/2
}
```

## 範囲指定

rsumなどは0..<2などの範囲指定がある。Swift同様終わりは含まない、end exclusive。

-2..<3で、-2, -1, 0, 1, 2となる。

かつてはコロンだったが、RとPythonで挙動が違って最後を含むか含まないかややこしかったのでSwiftに揃える。

- [Built-in Types — Python 3.11.1 documentation](https://docs.python.org/3/library/stdtypes.html#common-sequence-operations)
- [R: Colon Operator](https://stat.ethz.ch/R-manual/R-devel/library/base/html/Colon.html)
- [notes: シーケンスの第２引数の調査検討](notes/seq_second_arg.md)

## 組み込み関数

### rand()

0.0〜1.0の乱数を返す。

### シェーダーに良くあるAPIで対応済みのもの

特に説明が必要なさそうなものはここに列挙しておく。

以下のチケットも参照のこと。 [notes: OpenGLなどのプリミティブの採用検討メモ](notes/gl_like_primitives.md)

だいたい対応するOpenGLのAPIの説明で十分と思うのでそちらを参照のこと。[OpenGL 4.x Reference Pages](https://registry.khronos.org/OpenGL-Refpages/gl4/html/indexflat.php)

- abs
- all
- any
- atan2
- ceil
- cos
- exp
- exp2 (2の累乗)
- floor
- fract
- log
- log2
- pow
- round
- sin
- sqrt
- tan
- trunc

また、ベクトル系の以下もサポートしている（ベクトルに関しては後述）

- distance
- dot
- cross（これは3次元ベクトルのみ）
- length
- normalize
- mix

vec3、vec4はスカラーだけサポートしている

- vec3
- vec4

### minとmaxは可変長引数

minとmaxは可変長引数で個々の要素が全て同じ型のケースを対応している。

つまり、以下のようなのが対応されている。

```cpp
min(1.5, 3.2, 2.0)
max(3, 2, 5, 7)
```

また、ベクトルも引数として使える。だから、以下のような事が出来る

```jsx
# 結果は[3, 3, 3]
max([1, 2, 3], [3, 2, 1], [1, 3, 2])
```

### vec3とvec4はスカラーだけサポート

vec3とvec4はスカラーだけサポートしている。

```cpp
let fvec = vec3(3.0)
let ivec = vec4(1)
```

複数要素の指定はサポートしていない。ただのタプルが同じ意味なのでそちらを使う事。

```cpp
# let fvec = vec3(1.0, 2.0, 3.0) はサポートしてない。以下で同じ意味なので
let fvec = [1.0, 2.0, 3.0]
```

## ベクトル系関数について

MFGではf32の2次元〜4次元のタプルをベクトルとして扱う場合がある。以下その背景などを簡単に説明しておく。ちなみにi32のベクトルは無い。また、用語としてややこしいがsinなどのベクトライズとは別の概念（複数の言語から概念を借用しているのでこの辺ややこしい…）

例えば以下のように、f32の二次元のタプルをベクトルとして扱えて、ベクトルを引数にとる関数を使う事が出来る。

```cpp
normalize([1.2, 3.2])
```

結果はタプルとして扱える。

```cpp
let [x0, y0] = normalize([1.2, 3.2])
```

以下のように他の関数の引数にも使える。

```cpp
let len = length(normalize([1.2, 3.2]))
```

### ベクトル系関数の一覧

以下に一覧を上げておく。詳細はOpenGLのドキュメントが参考になる。[OpenGL 4.x Reference Pages](https://registry.khronos.org/OpenGL-Refpages/gl4/html/indexflat.php)

以下では、floatの2〜4次元のベクトルをfvecと書く。

**distance**

```cpp
f32 distance(v1:fvec, v2:fvec)
```

２つのベクトル間の距離を返す

**dot**

```cpp
f32 dot(v1:fvec, v2:fvec)
```

２つのベクトルの内積（dot積）を返す

**length**

```cpp
f32 length(v1:fvec)
```

ベクトルの長さを返す

**normalize**

```cpp
fvec normalize(v1:fvec)
```

長さを1に規格化したv1と同じ向きのベクトルを返す

### ベクトルとスカラーの両方をサポートする関数

いくつかの関数はベクトルとスカラーの両方をサポートする(OpenGLで言う所のgenType相当）。

OpenGLではほとんどの関数がスカラーでも使えるが、MFGではMetalやHLSLでも容易にサポート出来るものだけをサポートしている。

floatのベクトルとスカラーのどちらかをfsvecと書く事にする。

**clamp**

```cpp
fsvec clamp(x:fsvec, minVal:fsvec, maxVal:fsvec)
fsvec clamp(x:fsvec, minVal:f32, maxVal:f32)
```

スカラーでも使える。またminValとmaxValはベクトルでもスカラーでも使える。

**mix**

```cpp
fvec mix(v1:fsvec, v2:fsvec, ratio:f32)
fvec mix(v1:fvec, v2:fvec, rvec:fvec)
```

v1とv2を線形補間したベクトルを返す。第三引数がスカラーの場合は$v1*(1-ratio)+v2*ratio$ を、第三引数がベクトルの場合はそれぞれの要素に対して同様の計算をした結果を返す。

以下もだいたい上記２つと同じでOpenGLを見れば分かると思うので詳細は省略するがサポートされている。

- sign
- smoothstep
- step
- isinf
- isnan

### ベクトルの中途半端さの背景

MFGではループの処理を一つにまとめるべく、タプルがサポートされている。これにベクトル型を追加するのは不要に複雑に思い、ベクトル型というものは存在しない。

だが、下のシェーダーでほぼどの言語でも共通にあるようなベクトル関数を使いたい事は良くあり、その実装をいちいち再実装したくは無かったため、2次元から4次元までのfloatのタプルをコード生成時にベクトルに変換してシェーダーの関数を呼ぶ、という事をしているものがある。（厳密にはさらにそれをタプルに戻すというコードを生成するためにツリーの変形をしている）

例えばlengthなどは5次元以上のタプルでもあって良さそうなものだが、現状はそういうサポートはしていない。

### 使わないと思ってサポートしてないもの

使わないかな、と思ってサポートしなかったが、必要ならサポートは難しくないものが結構ある。それをここに列挙しておく。要望があればサポート出来る。

- faceforward
- fma
- noise
- reflect
- refract

三次元を前提としたものは使うかどうか自信が持てなかったのでとりあえず入れてない。reflectなどは使いそうな気もして悩んだが、疑わしきは入れないという方針でとりあえず入れなかった。

### 実装の手間を考えて後回しにしているもの

そのうちサポートする気だけれど、他の評価を優先して後回しにしているもの。

- ldexp
- frexp
- modf

## テンソルリテラル

定数のテンソルを簡単に定義する記法。

```cpp
def ctensor by [1, 2, 3]
```

byのあとにタプルっぽい記法で定義する。但し全部同じ型である必要がある。

1次元と二次元をサポート。二次元は以下。

```cpp
def lts by [[1, 2, 3], [4, 5, 6]]
```

[MEP 21: テンソルリテラル対応](MEP/21.md)