# 重み付きメディアンフィルター(Weighted Median Filter)

ノイズ除去のフィルタとして、メディアンフィルターに重みをつけたものが、かなり有効である事が知られている。

ここでは一番単純な、色コンポーネントごとに別々にメディアンを求めて色とするフィルタを開発してみる。

## 参考文献

[Principles of Digital Image Processing: Advanced Methods - SpringerLink](https://link.springer.com/book/10.1007/978-1-84882-919-0) の、3.2.1のScalar median filterを実装してみる。

## 実装するアルゴリズム

## 結果のコード

```mfg
def weight by [[1, 2, 1],
               [2, 3, 2],
               [1, 2, 1]]

# xとyがmedianを求められるように、
# x: 1〜w-1
# y: 1〜h-1
# の範囲で計算する。
@bounds(input_u8.extent(0)-2, input_u8.extent(1)-2)
def median |x, y| {
  @bounds(256, 4)
  def _hist |i, col| { 0 }

  weight.for_each |ix, iy, wval| {
     let [b, g, r, a] = input_u8(ix+x, iy+y)
     mut! _hist(b, 0) += wval
     mut! _hist(g, 1) += wval
     mut! _hist(r, 2) += wval
     mut! _hist(a, 3) += wval
  }

  # cumsumを求めるが、配列としては_histを上書きしていく（サイズ制限がきついので）
  mut! trans<_hist>.accumulate!(dim=0) |i, col, val, accm | { val+accm }

  # hist(255, rgb)は総登場回数（weightはその分重複して登場したと解釈）。その半分で
  def _med by reduce<_hist>.find_first_index(dim=0) |i, col, val| {
     val >= _hist(255,col)/2
  }
  [_med(0), _med(1), _med(2), _med(3)]
}

def result_u8 |x, y| {
  ifel( x == 0 || y == 0 || x == input_u8.extent(0)-1 || y == input_u8.extent(1) -1,
      input_u8(x, y),
      median(x-1, y-1))
}
```

ヒストグラムはレジスタを大量に使うので、
重みを色の繰り返しとみなしてsortを使ってメディアンを求める版は以下。

```mfg
# xとyがmedianを求められるように、
# x: 1からw-1
# y: 1からh-1
# の範囲で計算する。
@bounds(input_u8.extent(0)-2, input_u8.extent(1)-2)
def median |x, y| {

	@bounds(9)
  def wcumsum |i| {
     let [ix, iy] = [i%3, i/3]
     weight(ix, iy)
  }

  mut! trans<wcumsum>.cumsum!(dim=0)

  # 15はweightの合計。ローカルテンソルなのでIMMでないといけない。
  @bounds(15)
  def wmat |i| {
     let i3 = reduce<wcumsum>.find_first_index(dim=0) |_, val| { i < val }

     let [ix, iy] = [i3%weight.extent(0), i3/weight.extent(0)]
     i32(input_u8(ix+x, iy+y))
  }
  
  mut! trans<wmat>.sort!(dim=0)

  u8(wmat(wmat.extent(0)/2))
}
```
