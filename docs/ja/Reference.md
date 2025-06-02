# リファレンス

実装する都度メモを残しておいて、どこかでちゃんとしたリファレンスに整理したいと思い、最初からリファレンスという名前のページにしてく。

- [MEP Index](MEPIndex.md)
- [ウォークスルー](Walkthrough.md)

## ループ系の機能

ループは以下の種類がある。

- rangeのループ
  - rsum
  - reduce
- テンソルの要素を巡回するもの
  - for_each
  - sum
  - trans
     - trans.cumsum
     - trans.sort
     - trans.accumulate
  - reduce
     - reduce.accumulate
     - reduce.find_first_index
     - def by reduceのreduce

MFGのループは、それも実際のGPUのプロセッサの動きを自然に表したものになっていて、通常の言語のようなジャンプをベースとした無制限のループは提供していない。

### rangeのループ

rangeのループは通常の言語のループに近い。
引数にrangeとブロックを取り、rangeのそれぞれに対してblockを実行していく。

以下の２つがある。

- rsum
- reduce

rsumは結果を足したものが結果になる。

```swift
def result_u8 |x, y| {
    rsum(0..<3, 0..<5) |rx, ry| { 
        u8[rx, ry, 3, 4]
    }
} 
```

reduceは前のブロックの実行結果をaccmとして次に渡していき、最後のブロック実行の結果の値を返す。

```swift
let r1 = reduce(init=1, 2..<4) |r, accm| { r*accm }
```




## 名前付き引数

[notes: メディアンフィルタのスクリプト案を検討](notes/MedianFilter.md)

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


## テンソルのメソッド

現状ではextentとis_insideとto_ncoordがテンソルのメソッドと解釈される（ただしto_ncoordはselfが省略されて現在定義中のテンソルとなる）。

### ts.extent(dim)

以下のようにdimを指定してそのテンソルの幅や高さが取れる。

```cpp
let w = input_u8.extent(0)
let h = input_u8.extent(1)
```

wやhはinputの「最大のインデックス+1」となる（0オリジンなので）。

### ts.is_inside(x, y)

x, yがtsの範囲内ならノンゼロを、範囲内なら0を返す。

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
- [notes: シーケンスの第２引数の調査検討](notes/SeqSecondArg.md)

## 組み込み関数

### シェーダーに良くあるAPIで対応済みのもの

特に説明が必要なさそうなものはここに列挙しておく。

以下のチケットも参照のこと。 [notes: OpenGLなどのプリミティブの採用検討メモ](notes/GLLikePrimitives.md)


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
