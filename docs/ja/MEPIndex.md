# MEPの目次

MEPはMFG Enhanced Proposalの略。PythonのPEPなどのMFG版。

現在は文法の開発中で、何が最新版かを正しく残すのは難しいので、
何か文法を変更したり追加したりする都度MEPを書く事にし、
それらを辿っていく事で頑張れば最新版の仕様が理解出来るようにしておく。

また、どうしてそういう文法にしたのか、という検討もなるべく残す。

現時点ではMEPといいつつenhancementという感じでは無いものが多いが、将来のためにもこの形式で統一しておく。

## MEP

### ステータス実装済み

**v1.0.00**

- [MPE 1: Selectの改善](MEP/1.md)
- [MEP 2: rsum](MEP/2.md)
- [MEP 3: パラメータ](MEP/3.md)
- [MEP 4: タプルのvectorization](MEP/4.md)
- [MEP 5: BlankIdの実装](MEP/5.md)
- [MEP 6: find_first_index](MEP/6.md)
- [MEP 7: 0次元へのreduce](MEP/7.md)
- [MEP 8: sort版メディアンフィルタのスクリプト検討](MEP/8.md)
- [MEP 9: repeat\_edgeとconstant\_exterior](MEP/9.md)
- [MEP 10: whereの廃止](MEP/10.md)
- [MEP 11: rsumの一般化](MEP/11.md)
- [MEP 12: デバッグ出力](MEP/12.md)
- [MEP 13: 0次元へのreduceのexpr化](MEP/13.md)
- [MEP 14: リテラルの型指定](MEP/14.md)
- [MEP 15: 型指定ベクトル](MEP/15.md)
- [MEP 16: swizzle演算子対応](MEP/16.md)
- [MEP 17: スカラーの繰り返しでベクトルを作る](MEP/17.md)
- [MEP 18: デコレータをsamplerと揃える](MEP/18.md)
- [MEP 19: paramの国際化](MEP/19.md)
- [MEP 20: 他のレイヤーの参照](MEP/20.md)
- [MEP 21: テンソルリテラル対応](MEP/21.md)

**v1.0.01**

- [MEP 22: パイプライン演算子](MEP/22.md)

**v1.0.02**

- [MEP 25: mar内での国際化](MEP/25.md)

**v1.0.03**

- [MEP 26: ユーザー定義関数](MEP/26.md)

**v1.0.06**

- [MEP 27: パイプライン演算子だけは次の行に書ける例外とするルール](MEP/27.md)


### ステータス、Proposed

- [MEP 23: result_u8などをresultに](MEP/23.md)
- [MEP 28: 前景色の取得](MEP/28.md)

### ステータス、Declined

- [MEP 24: mapのサポート](MEP/24.md)
