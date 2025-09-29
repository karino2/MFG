# MFGのドキュメントのページ

[English Doc](../en/README.md)

モダンなプログラム言語でGPU上のフィルタ開発を！

MFGはGPU上で動作する画像処理フィルタを開発するために、1から設計されたプログラム言語です。
現代的なプログラム言語の多くの特徴を取り入れつつ、GPUプログラムという特殊性を最初から言語のコアコンセプトの段階で考慮に入れた設計となっています。

複数のカーネルと中間バッファを使用したモザイクフィルタも、以下のように簡単に書けます。

```mfg
@title "モザイクフィルター"
@param_i32 MOSAIC_WIDTH(SLIDER, label="サイズ", min=2, max=256, init=16)

let inputEx = sampler<input_u8>(address=.ClampToEdge)

@bounds( (input_u8.extent(0)-1)/MOSAIC_WIDTH+1, (input_u8.extent(1)-1)/MOSAIC_WIDTH+1)
def avg |x, y|{
  rsum(0..<MOSAIC_WIDTH, 0..<MOSAIC_WIDTH) |rx, ry|{
    let [b, g, r, a] = i32(inputEx( MOSAIC_WIDTH*x+rx, MOSAIC_WIDTH*y+ry ))
    [*[b, g, r]*a, a]
  }
}

def result_u8 |x2, y2| {
  let [b2, g2, r2, a2] = avg( x2/MOSAIC_WIDTH, y2/MOSAIC_WIDTH )

  ifel(a2==0,
      u8[0, 0, 0, 0],
      u8[*[b2, g2, r2]/a2, a2/(MOSAIC_WIDTH*MOSAIC_WIDTH)] )
}
```

![モザイクフィルタのデモgif](imgs/mosaic_demo.gif)

- 公式ページ [MFG - Modern Filter-language for GPU / MFG Studio](https://modernfilterlanguageforgpu.org/)
- [ReleaseNotes.md](ReleaseNotes.md)

## MFGの特徴

- GPU上での画像処理専用に作られた言語
- タプルやそのdestructuring、ベクトライズ、swizzle演算子、限定された副作用、ブロックを用いた高機能なループ処理といった現代的なプログラム機能
- カーネルやグローバルメモリといったGPUプログラムの特徴を言語の中心に据えた言語設計
- ペイントソフトとのデータのやり取りやユーザーからの入力の受取りなどが最初から組み込まれている
- MFGのフィルタはFireAlpaca SE 3.0上で動作
- オープンソースとして近日公開予定

## MFGStudioのInstall

MFGの開発はMFGStudioで行います。

- [Installaion](Installation.md)

## Getting Started

- [Getting Started](GettingStarted/README.md)

## リファレンス

- [リファレンス](Reference/README.md)

## 発展的な内容

以下はコア開発者向けの、より発展的な内容となります。

### ケーススタディ

具体的なフィルタの開発を解説付きで行ったもの。

- [ガウスぼかし](study/GaussBlur.md)
- [クリスタライズ](study/Crystallize.md)
- [アンチエイリアス（MLAA）](study/MLAA.md)

### ノート

調査した時のメモ、思想など。

- [MFGとは？](notes/WhatIsMFG.md)
- [GPGPU上の言語という性質から来る特徴](notes/LangForGPGPU.md)
- [シークエンスの第二引数の意味](notes/SeqSecondArg.md)
- [なぜto_ncoordの引数はタプルなのか？](notes/WhyToNCoordArgIsTuple.md)
- [改行の扱い](notes/EOLHandling.md)

### 仕様更新の議論

- [MEPのインデックス](MEPIndex.md) 仕様

