# MFGStudio Release Notes

更新履歴です。

## v1.0.01 2025-06-24

- vec2を実装
- 5次元以上のベクトライズが動いていなかったのを修正
- marの作成場所を選べるように
- フィルタ実行時の時間を表示
- f32のモジュロが動いていなかったのを修正
- CRTフィルタを追加
- 銅版画トーンフィルタとそれを試すハードミックスのレイヤーモードのプレビュー画像を追加
- CIE XYZカラー関連の変換関数を追加（後述）
- パイプ演算子を実装（後述）

カラー関連変換関数は以下を実装した

- lbgr_to_xyz, xyz_to_u8color, to_xyza
- lbgra_to_u8color, to_lbgra

xyzはCIE XYZカラー。lbgrはガンマ補正でリニアライズしたBGR（0.0〜1.0）。
詳細は[リファレンス: 組み込み関数](Reference/BuiltinFunctions.md)の「色変換とサポートしている色」を参照ください。

パイプ演算子は[リファレンス: 式とベクトライズ演算](Reference/Expression.md)の「パイプ演算子と`...`によるパイプライニング」を参照ください。

## v1.0.00 2025-06-03

最初のリリースです。