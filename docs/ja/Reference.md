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

