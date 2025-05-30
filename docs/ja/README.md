# MFGのドキュメントのページ

モダンなプログラム言語でGPU上のフィルタ開発を！

MFGはGPU上で動作する画像処理フィルタを開発するために、1から設計されたプログラム言語です。
現代的なプログラム言語の多くの特徴を取り入れつつ、GPUプログラムという特殊性を最初から言語のコアコンセプトの段階で考慮に入れた設計となっています。

```swift
@title "モザイクフィルター"
@param_i32 MOSAIC_WIDTH(SLIDER, label="サイズ", min=2, max=256, init=16)

let inputEx = sampler<input_u8>(address=.ClampToEdge)

def result_u8 |x, y| {
  let [mx, my] = [x/MOSAIC_WIDTH, y/MOSAIC_WIDTH]

  let [b2, g2, r2, a2] = rsum(0..<MOSAIC_WIDTH, 0..<MOSAIC_WIDTH) |rx, ry|{
    let [b, g, r, a] = i32(inputEx( MOSAIC_WIDTH*mx+rx, MOSAIC_WIDTH*my+ry ))
    [*[b, g, r]*a, a]
  }

  ifel(a2==0,
      u8[0, 0, 0, 0],
      u8[*[b2, g2, r2]/a2, a2/(MOSAIC_WIDTH*MOSAIC_WIDTH)] )
}
```




## Getting Started

- [Getting Started](GettingStarted/) まずはここから。

## リファレンス

- [リファレンス](Reference/)

## スクリプト例

具体的なフィルタの開発を解説付きで行ったもの。

- [ガウスぼかし](examples/GaussBlur.md)
- [クリスタライズ](examples/Crystallize.md)
- [アンチエイリアス（MLAA）](examples/MLAA.md)

## ノート

調査した時のメモ、思想など。

- [MFGとは？](notes/WhatIsMFG.md)
- [GPGPU上の言語という性質から来る特徴](notes/LangForGPGPU.md)
- [シークエンスの第二引数の意味](notes/SeqSecondArg.md)
- [なぜto_ncoordの引数はタプルなのか？](notes/WhyToNCoordArgIsTuple.md)
- [改行の扱い](notes/EOLHandling.md)

## 仕様更新の議論

- [MEPのインデックス](MEPIndex.md) 仕様

