# なぜto_ncoordの引数はタプルなのか？

to_ncoordは以下のようにタプルを引数に取る。

```
  to_ncoord([x, y])
```

一方で、is_insideなどは普通に引数としてx, yを取る。

```
  input_u8.is_inside(x, y)
```

なぜ同じ座標を渡すのに、片方はタプルで、もう片方は通常の渡し方になっているのか？という理由を書いておく。
ただしこの一貫性のなさはいまいちとは思っているので将来変えるかもしれない。

to_ncoordの引数がタプルなのは、to_ncoordの戻りの型が理由となっている。
to_ncoordはテンソルの次元と同じ次元のタプルを返す。
けれど、関数の戻りの型が関数の引数では無く暗黙のテンソルによって変わるのは、
型の解決的にもトリッキーで専用の処理が必要になって良くない、と思った。

そこで次元が変わる戻りは、引き数の次元に対応したもの、という通常のgenType的な関数で統一した方がいいだろう、
という事でto_ncoordの引き数はタプルになっている。

一方is_insideは戻りが0かノンゼロかのi32に過ぎないので、こうした問題は無い。

全部タプルで統一した方がいいかもしれないが、少なくともis_insideとしては不要なので通常の渡し方にしてある。