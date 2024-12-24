# メディアンフィルタのスクリプト案を検討

- Created: 2022-08-19 11:33:59

メディアンフィルタの対応に必要な機能と、そのシンタックスを考える。

シンタックスに関しては、検討した事を残しておきたいのでここに置いておく。

----

Updateが必要な例が無くなってしまったので、weighted median filterを考えてみたい。

```
def weight |x, y| { ... }

def median |x, y| {
  @bounds(256, 3)
  def hist |i, rgb| { 0 }

  weight.for_each |ix, iy, wval| {
     let [a, r, g, b] = input(ix+x, iy+y)
     hist(r, 0) += wval
     hist(g, 1) += wval
     hist(b, 2) += wval
  }

  @bounds(256, 3)
  def hist_cumsum |i, rgb| { 0 }

  # TODO: これでいいかはもう少し考えたい
  hist_cumsum.update |i, rgb, _| {
    select( i == 0, hist(i, rgb), hist_cumsum(i-1, rgb)+hist(i, rgb) )
  }

  let medianR = hist_cumsum(255, 0)/2
  let medianG = hist_cumsum(255, 1)/2
  let medianB = hist_cumsum(255, 2)/2

  # TODO: ここでhist_cumsumが最初にmedianXより大きくなる値を探してそれらのインデックスを返す
}
```

histはローカルのバッファなので+=のアップデートでもatomicである必要は無い。このシンタックスで良いかは考える必要あり。パーサーには優しくないが。

cumsumをどう扱うか、というのはもうちょっと考える必要がありそう。accumulateとかfold的なものでちょうど良いのを考えたい気もする。

この例ではhistとhist\_cumsumにupdateを使っている。hist, hist\_cumsumはローカルバッファ。

medianXより最初に大きくなる値の探し方は考えたい。

----

cumsum、reduce的なものに出来ないだろうか？左辺の定義の仕方は分からないのでとりあえずdefにしてみるか？

```
def hist_cumsum = hist.reduce | i, rgb, val, accm | {
  accm+val
}
```

これではrgb側にうまくaccumlate出来ていないな。初期値も指定出来ていないし。iについてだけaccmしてく必要がある

```
def hist_cumsum = hist.reduce(1, [0, 0, 0]) | i, col, accm | {
   [accm[0]+col[0], accm[1]+col[1], accm[2]+col[2]]
}
```

次元1つめだけにreduceし、colには各行のデータが来る。これはテンソル的では無いのでだめだよなぁ。これはもうちょっと考えてみる必要がありそう。

----

PyTorchのcumsumを参考にしてみる。 [torch.cumsum — PyTorch 1.13 documentation](https://pytorch.org/docs/stable/generated/torch.cumsum.html#torch.cumsum)

```
def hist_cumsum = hist.cumsum(0)
```

うーむ、これはシンプルでいいが、もうちょっと一般化した概念が欲しいな。 tensorをコンバートしてtensorを作る演算を一般化出来るといいよなぁ。

```
def hist_cumsum = conv_cumsum(hist, 0)
```

うーむ、converter的なものを一般化したい。もう少し考えるか。

```
def hist_cumsum = conv_cum hist 0 |accm, cur| { accm+cur }
```

これは惜しい所まで来ている気がするな。次元が同じで特定の軸に対してaccumulateしていく。

----

次元が同じで特定の軸に関してのtransform、というのがやりたい事だよな。軸を潰すreduceと、軸を潰さないtransformがあるのか。

１つ目をdim, ２つ目を初期値とする。戻りは１つ目が値、２つ目がaccmとする。

```
def hist_cumsum = hist.transform(0, 0) |i, rgb, val, accm | {
  val+accm, val+accm
}
```

0, 0は意味が分からないな。やはり名前付き引数が欲しいか。

```
def hist_cumsum = transform hist of dim=0, init=0 by |i, rgb,  val, accm | {
   val+accm, val+accm
}
```

これができていればcumはこの機能制限版で作れそうか。

```
def hist_cumsum = cum hist of dim=0 by |i, rgb, val, accm | {
  val+accm
}
```

うーむ、あんまり読みやすくないな。もっと普通の関数呼び出しの方がマシか？

```
def hist_cumsum = hist.accumulate(dim=0) |i, rgb, val, accm| { val + accm }
```

transformも同じ感じだとどうだろう？

```
def hist_cumsum = hist.transform(dim=0, init=0) | i, rgb, val, accm | {
  val+accm, val+accm
}
```

あと一歩な感じがするな。パーサー的にはtransformが先の方がありがたいんだが。

```
def hist_cumsum = transform(hist, dim=0, init=0) |i, rgb, val, accm| {...}
```

----

以前作ったtensor sumのシンタックスは以下みたいになってたが、これも合わせたいな。

```
let area = edge.sum |i, val| { 2*val+1 }
```

同じ感じにすると以下か？

```
let area = reduce_sum(edge) |i, val| { 2*val+1 }
```

tensorのtransformなどは新しいテンソルを作るのでカーネルになるからexpressionとは違うんだよなぁ。その辺がシンタックスで書いている人に見るからに分かる方が嬉しいんだが、このシンタックスでは同じに見えるなぁ。letとdefで違うんだけど、イコールっていまいちなんだよなぁ。

イコールじゃなくてbyとかにしてみようか？

```
def hist_cumsum by transform(hist, dim=0, init=0) |i, rgb, val, accm| {...}
```

こっちの方がいいな。accumulateも書いてみるか。

```
def hist_cumsum by accumulate(hist, dim=0) |i, rgb, val, accm| { val+accm }
```

reduce\_sumも全部の次元が潰れるからexpressionなだけで、次元が残ればカーネルなんだよなぁ。この違いはもうちょっと文法でわかりやすくしたい所だが。

----

良く考えるとaccmはトップレベルでカーネルにする事は出来ないんだった。むしろletの方がそれがあからさまで良いか？

```
let hist_cumsum = transform(hist, dim=0, init=0) |i, rgb, val, accm| {...}
```

でもhistとかの定義はトップレベルテンソルと同じシンタックスになるよなぁ、と思うと、こっちだけexpressionっぽいのもおかしいか。defとbyの方がいい気がするな。

----

以上をまとめてみよう。

```
def weight |x, y| { ... }

def median |x, y| {
  @bounds(256, 3)
  def hist |i, rgb| { 0 }

  weight.for_each |ix, iy, wval| {
     let [a, r, g, b] = input(ix+x, iy+y)
     hist(r, 0) += wval
     hist(g, 1) += wval
     hist(b, 2) += wval
  }

  def hist_cumsum by accumulate(hist, dim=0) |i, rgb, val, accm | { val+accm }

  def med by reduce(hist_cumsum, dim=0, init=-1) |i, rgb, val, accm| {
    select(accm != -1, accm,
      select(val < hist_cumsum(255, rgb)/2, -1, i)
    )
  }
  [med(0), med(1), med(2)]
}
```

medのreduceは値じゃないのでdefとbyになる。これはややこしいなぁ。

むしろトップレベルのreduce\_sumもdefで統一する方がいいかもしれない。0次元だと値として内部では処理する。

あと一歩という感じはするな。rsumだけめっちゃ文法が違うのもなんとかしたい気はする。

----

-   transform: 特定の軸について順番に変換を実行、値とコンテキストを返すブロックを引数にとる。最初の初期値も必要。
-   reduce: 特定の軸について、二項演算を繰り返して軸を潰す。値とコンテキストを返すブロックを引数にとる。最初の初期値も必要。

この２つで全ては表現出来る。だがいちいち２つ返す必要が無い時に毎回変えしたり初期値を指定するのも面倒なので、それらを簡略化したaccumulateみたいなものを定義したい。

----

こうしたものをアドホックに全部足すんじゃなくて、いい感じに拡張可能に作れないものか？

```
def hist_cumsum by transform(hist, dim=0, init=0) |i, rgb, val, accm| {
  val+accm, val+accm
}
def hist_cumsum by accumulate(hist, dim=0) |i, rgb, val, accm| {
  val+accm
}
```

ようするに、これが関数みたいにユーザー定義出来そうななにかならいいんだよな（定義出来なくても）。

```
deftrans accumulate(ts, dim=d, block) {
  transform(ts, dim=d, init=0) |i, rgb, val, accm |{
    let res = block(i, rgb, val, accm)
    res, res
  }
}
```

なんかこういう感じで定義出来るようなスッキリした概念があればいいんだよな。

accumulateはtransformの一種だと分からないのもいまいちだよな。

trans\_accmとかにするか？trans\_gen, trans\_accmとか。reduceもreduce\_genとreduce\_sumとか。でもreduceはtensorを対象にする場合とrangeを対象にする場合があるんだよな。

tensorを変形してtensorを作る一般的な概念が欲しい気がする。

----

byとかはローマ字が並びすぎて読みづらいな。やはり記号で区切りたい気がする。パーセントとかどうだろう？

```
def hist_cumsum %trans_accm(hist, dist=0) |i, rgb, val, accm | {
   val+accm
}
def hist_cumsum %trans(hist, dist=0, init=0) |i, rgb, val, accm| {
  val+accm, val+accm
}
def med %reduce(hist_cumsum, dim=0, init=-1) |i, rgb, val, accm| {
  select(accm != -1, accm,
    select(val < hist_cumsum(255, rgb)/2, -1, i)
  )
}
```

悪くないな。概念の並列性が分かりやすい気がする。パースの都合も良いし。

テンソルのメソッドにするとどうだろう？

```
def hist_cumsum %hist.trans_accm(dist=0) |i, rgb, val, accm | {
   val+accm
}
def med %hist_cumsum.reduce(dim=0, init=-1) |i, rgb, val, accm| {
  select(accm != -1, accm,
    select(val < hist_cumsum(255, rgb)/2, -1, i)
  )
}
```

これでもいい気がするな。for\_eachとかとの連続性を思えばこっちの方がいいかもしれない。

----

ローカルのテンソルはレースコンディションを考えなくていいので、シンタックス上も別物として扱いたい気はするんだよな。ローカルテンソルはアンダースコア必須にしてみようか？

```
def weight |x, y| { ... }

def median |x, y| {
  @bounds(256, 3)
  def _hist |i, rgb| { 0 }

  weight.for_each |ix, iy, wval| {
     let [a, r, g, b] = input(ix+x, iy+y)
     _hist(r, 0) += wval
     _hist(g, 1) += wval
     _hist(b, 2) += wval
  }

  def _hist_cumsum %_hist.trans_accm(dim=0) |i, rgb, val, accm | { val+accm }

  def _med %_hist_cumsum.reduce(dim=0, init=-1) |i, rgb, val, accm| {
    select(accm != -1, accm,
      select(val < _hist_cumsum(255, rgb)/2, -1, i)
    )
  }
  [_med(0), _med(1), _med(2)]
}
```

アンダースコアで良いかは別として、区別はした方がいい気がするな。

----

やはりパーセントは読みにくいな。イコールにするか？

```
def _hist_cumsum = _hist.trans_accm(dim=0) |i, rgb, val, accm | { val+accm }
```

右辺はexpressionでは無いし、ストレージの宣言も入るのでletにはしたくないんだが、使う側はそういう区別がいるのか？という気もするな。元のbyと比べてどっちがいいかなぁ。

```
def _hist_cumsum by _hist.trans_accm(dim=0) |i, rgb, val, accm | { val+accm }
```

やはり通常のメソッド呼び出しのように見えるのはいまいちだよな。パーセントつけた関数っぽいのに戻すか？

```
def _hist_cumsum by %trans_accm(_hist, dim=0) |i, rgb, val, accm | { val+accm }
```

パーセント無し。

```
def _hist_cumsum by trans_accm(_hist, dim=0) |i, rgb, val, accm| { val+accm }
```

やはりメソッド呼び出しよりもこっちの方がコンバートという特別な事をやっているというのは分かりやすい気はするな。コンバート関連は特別な印を導入したいが、どうもいいものを思いつかない。

----

角括弧でくくってみるか？

```
def _hist_cumsum by <trans_accm>(_hist, dim=0) |i, rgb, val, accm| { val+accm }
```

悪くは無いかもしれない。スカラーになるreduce\_sumも書いてみよう。

```
let area = <reduce_sum>(edge) |i, val| { 2*val+1 }
```

このreduce\_sumはスカラーの時だけexpressionになるのは違和感が強いよなぁ。このケースだけメソッドにするか？

```
let area = edge.reduce_sum |i, val | { 2*val+1 }
```

expressionになるものはfor\_eachと同じ表記、コンバートして新たなテンソルを生み出すものはコンバート形式。うーん、それならreduceと言わずにsumにしてしまっても良いかもしれない。

```
let area = edge.sum |i, val| { 2*val+1 }
```

振り出しに戻るが、これならテンソルの次元がいくつでもexpressionとして使えるというのが分かりやすいのでこっちの方がいい気もするな。

----

ここまでの考えを元にメディアンフィルタを書いてみる。

```
def median |x, y| {
  @bounds(256, 3)
  def _hist |i, rgb| { 0 }

  weight.for_each |ix, iy, wval| {
     let [a, r, g, b] = input(ix+x, iy+y)
     _hist(r, 0) += wval
     _hist(g, 1) += wval
     _hist(b, 2) += wval
  }

  def _hist_cumsum by <trans_accm>(_hist, dim=0) |i, rgb, val, accm | { val+accm }

  def _med by <reduce>(_hist_cumsum, dim=0, init=-1) |i, rgb, val, accm| {
    select(accm != -1, accm,
      select(val < _hist_cumsum(255, rgb)/2, -1, i)
    )
  }
  [_med(0), _med(1), _med(2)]
}
```

悪くない気もする。

----

```
_hist(r, 0) += wval
```

をパースしていた所、Exprと区別がつかずにバックトラックが必要になる事が判明した。 バックトラックしても良いのだが、シンタックスハイライトとかを考えると文法的に曖昧じゃない方が良いと思い、mutというキーワードを入れる事にした。

```
mut _hist(r, 0) += wval
```

mut!の方がいいかもしれないが、とりあえずmutで実装してみる。

----

やはりmutだとキーワードが埋もれてわかりにくいので、mut!に変更。

```
mut! _hist(r, 0) += wval
```

----

実機でローカルバッファをどこまで取れるか試したら、 `float buf[3][512];` は取れて、 `float buf[3][1024];` は取れなかった。 transformで新しく`3x256`を取り直すのは不安が残る。 transformは型が変わらなければ破壊的に行える訳で。 破壊的な文法を考えるか？

```
  mut! trans_accm(_hist, dim=0) |i, rgb, val, accm | { val+accm }
```

ローカルバッファはGPUにおいては貴重な資源なので、MFGにおいても貴重な資源という前提でコードを書くべきだよなぁ。 この文法にしよう。

----

trans\_accmがどうもシンタックスが気に食わないので少し検討。 まず単なる関数呼び出しだとどうも破壊的に見えない。 そしてFuncObjの型が第一引数に依存するので、解決にgenerics的な解決が必要になってしまっている。 それならメソッドにして、bangをつけるとどうだろう？

```
mut! _hist.trans_accm!(dim=0) |i, rgb, val, accm | { val+accm }
```

これは悪くない気がするな。 でもこれならreduceのコンバーターもメソッド形式にしたいよなぁ。

reduceとの一貫性を求めるなら、generics的な解決は頑張ってするとして、bangだけつけて関数だとどうだろう？

```
mut! trans_accm!(_hist, dim=0) |i, rgb, val, accm | { val+accm }
```

やっぱりメソッドの方がストレートにかんじるな。 reduceもメソッドっぽくしてみるとどうかな？

```
def _med by _hist.<reduce>(dim=0, init=-1) |i, rgb, val, accm| {
  ifel(accm != -1, accm, ...)
  elif(val < _hist_cumsum(255, rgb)/2, -1, i)
}
```

各カッコが意味が分からない。selfもくくるとどうだろう？

```
def _med by <_hist.reduce>(dim=0, init=-1) |i, rgb, val, accm| {
  ifel(accm != -1, accm, ...)
  elif(val < _hist_cumsum(255, rgb)/2, -1, i)
}
```

いまいちだなぁ。ただこれは最初の妥協点のような気もする。とりあえずこれで実装してみるかなぁ。

----

accmはPythonのaccumulateと似ているか。 [itertools — Functions creating iterators for efficient looping — Python 3.11.0 documentation](https://docs.python.org/3/library/itertools.html#itertools.accumulate)

----

trans\_accmのシンタックスを見直す。

```
mut! _hist.trans_accm!(dim=0) |i, col, val, accm | { val+accm }
mut! _hist.cumsum!(dim=0)
mut! wmat.sort!(dim=0)
```

これらは全てtransformという概念である事をもっとわかりやすくしたい。つまり\_histとかwmatというテンソルを特定の軸にそって値を変更するだけで、shapeは変わらない何か、という事。

transと角括弧でくくるとどうだろう？

```
mut! trans<_hist>.accumulate!(dim=0) |i, col, val, accm | { val+accm }
mut! trans<_hist>.cumsum!(dim=0)
mut! trans<wmat>.sort!(dim=0)
```

するとdef byの方も合わせた方がいいよな。

```
def _med by reduce<_hist>(dim=0, init=-1) |i, col, val, accm| {
    ifel(accm != -1, accm, ...)
    elif(val < _hist(255, col)/2, -1, i)
}
```

うーん、これは意味が違う気がするな。テンソルのdefを行う特殊な何か、という事を表したかったので、元の方が良い気がする。

```
def _med by <_hist.reduce>(dim=0, init=-1) |i, col, val, accm| {
    ifel(accm != -1, accm, ...)
    elif(val < _hist(255, col)/2, -1, i)
}
```

transとの角括弧の使い方の一貫性のなさが気になるな。

----

transの方をかっこじゃなくしてみるとか？trans!な関数呼び出しっぽいとどうだろう？

```
mut! trans!(_hist).accumulate(dim=0) |i, col, val, accm | { val+accm }
mut! trans!(_hist).cumsum(dim=0)
mut! trans!(wmat).sort(dim=0)
```

うーむ、分かりにくいな。

\_histをターゲットにしたaccumulateによるtransformなんだよな。

def byとの一貫性のなさは気になるが、とりあえず角括弧にしておくか。

```
mut! trans<_hist>.accumulate!(dim=0) |i, col, val, accm | { val+accm }
mut! trans<_hist>.cumsum!(dim=0)
mut! trans<wmat>.sort!(dim=0)
```

----

reduceは次元が減る縮約を表すのであって、accumulate的にreduceするのはtransと同じルールとするなら、

```
reduce<ts>.accumulate
```

と書くべきな気がしてきた。 これならtransとの並列構造も分かりやすいのでこう変更する。

-   旧： `def ts2 by <ts1.reduce> (...) |...| {...}`
-   新： `def ts2 by reduce<ts1>.accumulate (...) |...| {...}`