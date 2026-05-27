/* -*- coding: utf-8 -*- マルチバイト */

// Copyright 2026 PGN Inc. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#if defined(_MSC_VER)
  #pragma execution_character_set("utf-8")
#endif

#include <trans.h>
#include "trans_str_main.h"

///////////////////////////////////////////////////////////////////////////
// - メインウィンドウで使用される文字列
// - String used in the main window
///////////////////////////////////////////////////////////////////////////
QString Trans_StrMain( int idx )
{
  CTranslationManager* man = &Trans();

  //////////////////////////////////
  //////////////////////////////////
  if (man->TranslateJapanese())
  {
    if (idx == 1) return QObject::tr("ブラシツール");
    if (idx == 2) return QObject::tr("ドットツール");
    if (idx == 3) return QObject::tr("消しゴムツール");
    if (idx == 4) return QObject::tr("移動ツール");
    if (idx == 5) return QObject::tr("バケツツール");
    if (idx == 6) return QObject::tr("グラデーションツール");
    if (idx == 7) return QObject::tr("塗りつぶしツール");
    if (idx == 8) return QObject::tr("選択ツール");
    if (idx == 9) return QObject::tr("自動選択ツール");
    if (idx == 10) return QObject::tr("選択ペンツール");
    if (idx == 11) return QObject::tr("選択消しツール");
    if (idx == 12) return QObject::tr("テキストツール");
    if (idx == 13) return QObject::tr("スポイトツール");
    if (idx == 14) return QObject::tr("手のひらツール");

    if (idx == 15) return QObject::tr("スナップ オフ");
    if (idx == 16) return QObject::tr("平行スナップ");
    if (idx == 17) return QObject::tr("十字スナップ");
    if (idx == 18) return QObject::tr("集中線スナップ");
    if (idx == 19) return QObject::tr("スナップ設定");

    if (idx == 20) return QObject::tr("アンチエイリアス");
    if (idx == 21) return QObject::tr("手ぶれ補正");
    if (idx == 22) return QObject::tr("丸める");
    if (idx == 23) return QObject::tr("不透明度");
    if (idx == 24) return QObject::tr("参照");

    if (idx == 27) return QObject::tr("拡張");
    if (idx == 28) return QObject::tr("形状");
    if (idx == 29) return QObject::tr("タイプ");
    if (idx == 30) return QObject::tr("線形");
    if (idx == 31) return QObject::tr("放射状");
    if (idx == 32) return QObject::tr("前景色～背景色");
    if (idx == 33) return QObject::tr("前景色～透明");
    if (idx == 34) return QObject::tr("矩形");
    if (idx == 35) return QObject::tr("楕円");
    if (idx == 36) return QObject::tr("多角形");
    if (idx == 37) return QObject::tr("設定なし");
    if (idx == 38) return QObject::tr("投げなわ");
    if (idx == 39) return QObject::tr("投げなわ選択ツール");

    if (idx == 40) return QObject::tr("カラー");
    if (idx == 41) return QObject::tr("ブラシコントロール");
    if (idx == 42) return QObject::tr("ブラシ");
    if (idx == 43) return QObject::tr("ナビゲーター");
    if (idx == 44) return QObject::tr("レイヤー");
    if (idx == 45) return QObject::tr("ブラシプレビュー");
    if (idx == 46) return QObject::tr("カラーパレット");
    if (idx == 47) return QObject::tr("資料");
    if (idx == 48) return QObject::tr("ブラシサイズ");
    if (idx == 49) return QObject::tr("カラーヒストリー");

    if (idx == 50) return QObject::tr("(反転)");
    if (idx == 51) return QObject::tr("ファイルが存在しません");
    if (idx == 52) return QObject::tr("既に開いていますが、続行しますか？");
    if (idx == 54) return QObject::tr("消失点スナップ");
    if (idx == 55) return QObject::tr("画像を開く");
    if (idx == 56) return QObject::tr("画像");
    if (idx == 57) return QObject::tr("同心円スナップ");
    if (idx == 58) return QObject::tr("曲線スナップ");

    if (idx == 60) return QObject::tr("拡張");
    if (idx == 61) return QObject::tr("収縮");
    if (idx == 62) return QObject::tr("ウィンドウ位置を初期化しますか？");
    if (idx == 63) return QObject::tr("取り消し");
    if (idx == 64) return QObject::tr("やり直し");
    if (idx == 65) return QObject::tr("スナップ");
    if (idx == 66) return QObject::tr("言語を切り替えるため、アプリケーションを一旦終了します。終了後、アプリケーションを立ち上げ直してください。");
    if (idx == 67) return QObject::tr("読み込みに失敗しました");
    if (idx == 68) return QObject::tr("閾値");

    if (idx == 70) return QObject::tr("プリンタの取得に失敗しました");
    if (idx == 71) return QObject::tr("ソフトエッジ");
    if (idx == 72) return QObject::tr("未設定");
    if (idx == 73) return QObject::tr("クリック位置のレイヤーを選択");

    if (idx == 75) return QObject::tr("平行");
    if (idx == 76) return QObject::tr("集中線");
    if (idx == 77) return QObject::tr("同心円");
    if (idx == 78) return QObject::tr("曲線");
    if (idx == 79) return QObject::tr("消失点");

    if (idx == 80) return QObject::tr("操作ツール");
    if (idx == 81) return QObject::tr("コマ分割ツール");
    if (idx == 82) return QObject::tr("左右の間隔");
    if (idx == 83) return QObject::tr("上下の間隔");
    if (idx == 84) return QObject::tr("コマ素材の追加");

    if (idx == 85) return QObject::tr("中央から選択");
    if (idx == 86) return QObject::tr("縦横比を固定");

    if (idx == 87) return QObject::tr("ショートカットを反映するため、アプリケーションを一旦終了します。終了後、アプリケーションを立ち上げ直してください。");

    if (idx == 88) return QObject::tr("その他");
    if (idx == 89) return QObject::tr("ブラシサイズを大きく");
    if (idx == 90) return QObject::tr("ブラシサイズを小さく");
    if (idx == 91) return QObject::tr("無効なMDPファイルです。");
    if (idx == 92) return QObject::tr("%1 は正しい形式のMDPファイルではないため開けません。");
    if (idx == 93) return QObject::tr("ファイルが既に存在します");
    if (idx == 94) return QObject::tr("アニメGIF変換サービス 「アルパカ動画」 をブラウザで開きますか？");
    if (idx == 95) return QObject::tr("背景を透過させますか？");
    if (idx == 96) return QObject::tr("フォルダを作成しますか？");
    if (idx == 97) return QObject::tr("グリッドスナップ");
    if (idx == 98) return QObject::tr("ガイドスナップ");
    if (idx == 99) return QObject::tr("コマ分割");

    if (idx == 110) return QObject::tr("フリーハンド");
    if (idx == 111) return QObject::tr("直線");
    if (idx == 112) return QObject::tr("折れ線");
    if (idx == 113) return QObject::tr("多角形");
    if (idx == 114) return QObject::tr("矩形");
    if (idx == 115) return QObject::tr("楕円");
    if (idx == 116) return QObject::tr("曲線");
    if (idx == 117) return QObject::tr("図形");
    if (idx == 118) return QObject::tr("正方形");
    if (idx == 119) return QObject::tr("正円");

    if (idx == 120) return QObject::tr("3Dパース スナップ");
    if (idx == 121) return QObject::tr("フィルター処理中に別のファイルを開くことはできません。");
    if (idx == 122) return QObject::tr("選択解除");
    if (idx == 123) return QObject::tr("グリッドにスナップ");
    if (idx == 124) return QObject::tr("レイヤーカラーを無視");
    if (idx == 125) return QObject::tr("下書きレイヤーを無視");

    if (idx == 126) return QObject::tr("平均化");
    if (idx == 127) return QObject::tr("なし");
    if (idx == 128) return QObject::tr("3 * 3 px 平均");
    if (idx == 129) return QObject::tr("5 * 5 px 平均");

    if (idx == 130) return QObject::tr("自動保存中...");
    if (idx == 131) return QObject::tr("自動保存完了");
    if (idx == 132) return QObject::tr("自動保存を無効にしました (メモリ不足)\n\nアプリケーションの安定性が著しく低下しています");
    if (idx == 133) return QObject::tr("自動保存されている画像を復旧しますか？");
    if (idx == 134) return QObject::tr("編集中のキャンバスを復元しました");

    if (idx == 135) return QObject::tr("pixiv Sketch への投稿に失敗しました");

    if (idx == 136) return QObject::tr("広がり");
    if (idx == 137) return QObject::tr("標準");
    if (idx == 138) return QObject::tr("繰り返し");
    if (idx == 139) return QObject::tr("折り返す");

    // dialog_select_expand.cpp/dialog_select_gauss.cpp
    if (idx == 140) return QObject::tr("角を丸める");
    if (idx == 141) return QObject::tr("ガウスぼかし");

    // dialog_border.cpp
    if (idx == 150) return QObject::tr("角を丸め、太さを維持する");

    if (idx == 160) return QObject::tr("隙間を塞ぐ");
    if (idx == 162) return QObject::tr("許容値");
    if (idx == 163) return QObject::tr("透明〜前景色");
    if (idx == 164) return QObject::tr("カスタム");
    if (idx == 165) return QObject::tr("バケツ消し");

    // mainwindow.cpp
    if (idx == 170) return QObject::tr("ドラッグごとに位置を確定");
    if (idx == 171) return QObject::tr("位置を確定");
    if (idx == 172) return QObject::tr("両端の筆圧をゼロにする");
    if (idx == 173) return QObject::tr("1px 移動");
    if (idx == 174) return QObject::tr("レイヤーを左に 1px 移動");
    if (idx == 175) return QObject::tr("レイヤーを上に 1px 移動");
    if (idx == 176) return QObject::tr("レイヤーを下に 1px 移動");
    if (idx == 177) return QObject::tr("レイヤーを右に 1px 移動");

    if (idx == 180) return QObject::tr("パレットファイルの読み込み");
    if (idx == 181) return QObject::tr("パレットファイルの保存");
    if (idx == 182) return QObject::tr("FireAlpacaパレット (*.fap);;FireAlpacaパレット (*.fap)");
    if (idx == 183) return QObject::tr("ACOファイルのインポート");
    if (idx == 184) return QObject::tr("ACOファイルのエクスポート");
    if (idx == 185) return QObject::tr("ACO カラーパレット (*.aco);;ACO カラーパレット (*.aco)");

    if (idx == 190) return QObject::tr("変形");
    if (idx == 191) return QObject::tr("自由変形");
    if (idx == 192) return QObject::tr("メッシュ変形");

    if (idx == 200) return QObject::tr("左右対称");
    if (idx == 201) return QObject::tr("上下対称");
    if (idx == 202) return QObject::tr("回転対称");
    if (idx == 203) return QObject::tr(" (鏡像)");
    if (idx == 204) return QObject::tr("タイル");
    if (idx == 205) return QObject::tr("対称");
    if (idx == 206) return QObject::tr(" (ハーフドロップ)");
    if (idx == 207) return QObject::tr("中心を指定");
    if (idx == 208) return QObject::tr("線対称");

    if (idx == 210) return QObject::tr("投げなわ塗りツール");
    if (idx == 211) return QObject::tr("閉領域");
    if (idx == 212) return QObject::tr("透明部と白");
    if (idx == 213) return QObject::tr("透明部");
    if (idx == 214) return QObject::tr("白");
    if (idx == 215) return QObject::tr("全て");
    if (idx == 219) return QObject::tr("狭い領域は塗らない");

    if (idx == 221) return QObject::tr("新規");
    if (idx == 222) return QObject::tr("追加");
    if (idx == 223) return QObject::tr("削除");
    if (idx == 224) return QObject::tr("共通部");

    if (idx == 230) return QObject::tr("強さ");
    if (idx == 231) return QObject::tr("ストロークごとに確定");
    if (idx == 232) return QObject::tr("確定");

    if (idx == 240) return QObject::tr("膨張");
    if (idx == 241) return QObject::tr("収縮");
    if (idx == 242) return QObject::tr("押す");
    if (idx == 243) return QObject::tr("右回転");
    if (idx == 244) return QObject::tr("左回転");
    if (idx == 249) return QObject::tr("歪みブラシツール");

    // オートセーブ関連、130がいっぱいになってしまったので…
    if (idx == 260) return QObject::tr("前回の復旧に失敗したため、復旧処理をスキップしました");

    if (idx == 270) return QObject::tr("フレーム内のレイヤーを選択 (アニメーションモード)");

    if (idx == 300) return QObject::tr("透明色をスポイトする");
    if (idx == 301) return QObject::tr("スーパー楕円");

    if (idx == 310) return QObject::tr("今後、FireAlpaca (無料版) で使っていた設定ファイルを使うようにしますか？\n\n(再起動が必要なためアプリケーションを終了します)");
    if (idx == 311) return QObject::tr("FireAlpaca SE 標準の設定ファイルを使うように戻しますか？\n\n(再起動が必要なためアプリケーションを終了します)");
    if (idx == 312) return QObject::tr("旧バージョンの設定を引き継ぎますか？");
    if (idx == 313) return QObject::tr("設定ファイルをバックアップしました。バックアップしたフォルダを開きますか？");
    if (idx == 314) return QObject::tr("ブラシ、カラー、グラデーション、ショートカット設定をバックアップしますか？");
    if (idx == 315) return QObject::tr("現在の設定は破棄されますが、宜しいですか？");

    if (idx == 320) return QObject::tr("頂点・オブジェクトを選択");
    if (idx == 321) return QObject::tr("レイヤーを選択");

    if (idx == 330) return QObject::tr("全ての形式");
    if (idx == 331) return QObject::tr("FireAlpaca SE プロジェクト");

    if (idx == 425) return QObject::tr("キャンバス");
    if (idx == 426) return QObject::tr("アクティブレイヤー");
    if (idx == 427) return QObject::tr("指定レイヤー");

    if (idx == 440) return QObject::tr("ドラッグ時にもスポイト");

    if (idx == 450) return QObject::tr("本当にレイヤーを削除しますか？");
    if (idx == 451) return QObject::tr("環境設定から確認なしに削除するよう設定できます");

    if (idx == 500) return QObject::tr("メモリ消費量が半分になりますが、色精度が低下します。宜しいですか？");
    if (idx == 501) return QObject::tr("色精度が向上し編集時の劣化が軽減しますが、メモリ使用量が倍になります。宜しいですか？");
    if (idx == 502) return QObject::tr("既存のレイヤーを 8bit/ch に変換しますか？");
    if (idx == 503) return QObject::tr("既存のレイヤーを 16bit/ch に変換しますか？");
    if (idx == 510) return QObject::tr("上級者向けの設定ですが、本当に有効にしますか？");

    if (idx == 600) return QObject::tr("MFG アーカイブ形式(*.mar)");
    if (idx == 601) return QObject::tr("MARファイルの読み込み");
    if (idx == 602) return QObject::tr("MARが壊れています: ");
    if (idx == 603) return QObject::tr("ファイルのコピーに失敗しました: ");
    if (idx == 604) return QObject::tr("%1 個のフィルタをインポートしました");
    if (idx == 605) return QObject::tr("処理が長くかかっています…");

    if (idx == 700) return QObject::tr("処理に失敗しました。メインメモリが不足している可能性が高いです。");
  }

  //////////////////////////////////
  //////////////////////////////////
  if (man->TranslateChineseSimp())
  {
    if (idx == 1) return QObject::tr("笔刷工具");
    if (idx == 2) return QObject::tr("点刷工具");
    if (idx == 3) return QObject::tr("橡皮擦工具");
    if (idx == 4) return QObject::tr("移动工具");
    if (idx == 5) return QObject::tr("油漆桶工具");
    if (idx == 6) return QObject::tr("渐层工具");
    if (idx == 7) return QObject::tr("填充工具");
    if (idx == 8) return QObject::tr("选择工具");
    if (idx == 9) return QObject::tr("自动选择工具");
    if (idx == 10) return QObject::tr("选择笔工具");
    if (idx == 11) return QObject::tr("选择消除工具");
    if (idx == 12) return QObject::tr("文本工具");
    if (idx == 13) return QObject::tr("吸管工具");
    if (idx == 14) return QObject::tr("手掌工具");

    if (idx == 15) return QObject::tr("停止辅助");
    if (idx == 16) return QObject::tr("平行辅助");
    if (idx == 17) return QObject::tr("十字辅助");
    if (idx == 18) return QObject::tr("射線辅助");
    if (idx == 19) return QObject::tr("辅助设定");

    if (idx == 20) return QObject::tr("边缘柔化抗锯齿");
    if (idx == 21) return QObject::tr("抖动修正");
    if (idx == 22) return QObject::tr("圆角");
    if (idx == 23) return QObject::tr("不透明度");
    if (idx == 24) return QObject::tr("参照");

    if (idx == 27) return QObject::tr("扩张");
    if (idx == 28) return QObject::tr("形状");
    if (idx == 29) return QObject::tr("类型");
    if (idx == 30) return QObject::tr("线形");
    if (idx == 31) return QObject::tr("圆形");
    if (idx == 32) return QObject::tr("前景～背景");
    if (idx == 33) return QObject::tr("前景");
    if (idx == 34) return QObject::tr("矩形");
    if (idx == 35) return QObject::tr("椭圆");
    if (idx == 36) return QObject::tr("多角形");
    if (idx == 37) return QObject::tr("无设定");
    if (idx == 38) return QObject::tr("套索");

    if (idx == 40) return QObject::tr("颜色");
    if (idx == 41) return QObject::tr("笔刷控制");
    if (idx == 42) return QObject::tr("笔刷");
    if (idx == 43) return QObject::tr("导览");
    if (idx == 44) return QObject::tr("图层");
    if (idx == 45) return QObject::tr("笔刷预览");
    if (idx == 46) return QObject::tr("色板");
    if (idx == 47) return QObject::tr("资料");
    if (idx == 48) return QObject::tr("笔刷大小");
    if (idx == 49) return QObject::tr("色彩历史");

    if (idx == 50) return QObject::tr("(反转)");
    if (idx == 51) return QObject::tr("档案不存在");
    if (idx == 52) return QObject::tr("已开启，是否继续执行？");
    if (idx == 54) return QObject::tr("消失点辅助");
    if (idx == 55) return QObject::tr("打开图像");
    if (idx == 56) return QObject::tr("图像");
    if (idx == 57) return QObject::tr("同心圆辅助");
    if (idx == 58) return QObject::tr("曲线辅助");

    if (idx == 60) return QObject::tr("扩张");
    if (idx == 61) return QObject::tr("收缩");
    if (idx == 62) return QObject::tr("初始化窗口位置吗?");
    if (idx == 63) return QObject::tr("取消");
    if (idx == 64) return QObject::tr("还原");
    if (idx == 65) return QObject::tr("辅助");
    if (idx == 66) return QObject::tr("为了转换语言，需先关闭应用软件。结束后，请重新启动应用软件。");
    if (idx == 67) return QObject::tr("读取失败");
    if (idx == 68) return QObject::tr("阈值");

    if (idx == 70) return QObject::tr("无法连接至打印机。");
    if (idx == 71) return QObject::tr("柔边");
    if (idx == 72) return QObject::tr("未设定");
    if (idx == 73) return QObject::tr("选择点击位置的图层");

    if (idx == 75) return QObject::tr("平行");
    if (idx == 76) return QObject::tr("集中线");
    if (idx == 77) return QObject::tr("同心圆");
    if (idx == 78) return QObject::tr("曲线");
    if (idx == 79) return QObject::tr("消失点");

    if (idx == 80) return QObject::tr("操作工具");
    if (idx == 81) return QObject::tr("分割工具");
    if (idx == 82) return QObject::tr("左右间隔");
    if (idx == 83) return QObject::tr("上下间隔");
    if (idx == 84) return QObject::tr("添加框格素材");

    if (idx == 85) return QObject::tr("从中心选择");
    if (idx == 86) return QObject::tr("固定纵横比");

    if (idx == 87) return QObject::tr("为了更新快捷键，请先关闭程序后再次重新启动程序。");

    if (idx == 88) return QObject::tr("其他");
    if (idx == 89) return QObject::tr("放大笔刷尺寸");
    if (idx == 90) return QObject::tr("缩小笔刷尺寸");
    if (idx == 91) return QObject::tr("无效的MDP文件。");
    if (idx == 92) return QObject::tr("%1 因不是正确的MDP文件形式，无法打开。");
    if (idx == 93) return QObject::tr("文件已存在");
    if (idx == 94) return QObject::tr("使用浏览器打开动画GIF转换服务「Alpaca动画」吗？");
    if (idx == 95) return QObject::tr("要透过背景吗？");
    if (idx == 96) return QObject::tr("制作资料夹吗？");
    if (idx == 97) return QObject::tr("网格辅助");
    if (idx == 98) return QObject::tr("辅助线吸附");
    if (idx == 99) return QObject::tr("框格分割");

    if (idx == 110) return QObject::tr("手绘");
    if (idx == 111) return QObject::tr("直线");
    if (idx == 112) return QObject::tr("折线");
    if (idx == 113) return QObject::tr("多边形");
    if (idx == 114) return QObject::tr("矩形");
    if (idx == 115) return QObject::tr("椭圆");
    if (idx == 116) return QObject::tr("曲线");
    if (idx == 117) return QObject::tr("形状");
    if (idx == 118) return QObject::tr("正方形");
    if (idx == 119) return QObject::tr("正圆");
    if (idx == 120) return QObject::tr("3D视角辅助");
    if (idx == 121) return QObject::tr("档案处理中，无法开骑其他的档案");
    if (idx == 122) return QObject::tr("解除选择");
    if (idx == 123) return QObject::tr("辅助网格");
    if (idx == 124) return QObject::tr("无视彩色图层");
    if (idx == 125) return QObject::tr("无视草稿图层");
    if (idx == 126) return QObject::tr("平均化");
    if (idx == 127) return QObject::tr("无");
    if (idx == 128) return QObject::tr("3 * 3 px 平均");
    if (idx == 129) return QObject::tr("5 * 5 px 平均");

    if (idx == 130) return QObject::tr("正在自动保存...");
    if (idx == 131) return QObject::tr("自动保存完毕");
    if (idx == 132) return QObject::tr("已禁用自动保存(内存不足)\n\n应用程序的稳定性会显著下降");
    if (idx == 133) return QObject::tr("要将已经自动保存的图像恢复原状吗?");
    if (idx == 135) return QObject::tr("pixiv Sketch投稿失败");
    if (idx == 136) return QObject::tr("延展");
    if (idx == 137) return QObject::tr("标准");
    if (idx == 138) return QObject::tr("反覆");
    if (idx == 139) return QObject::tr("折返");

    if (idx == 140) return QObject::tr("修至圆角");
    //if (idx == 141) return QObject::tr("Gaussian Blur"); // ガウスぼかし

    if (idx == 150) return QObject::tr("维持圆角及粗细");

    if (idx == 160) return QObject::tr("填满间隙");
    if (idx == 162) return QObject::tr("容许值");
    if (idx == 163) return QObject::tr("透明〜前景色");
    if (idx == 164) return QObject::tr("客制化");
    if (idx == 165) return QObject::tr("消除油漆桶");

    if (idx == 170) return QObject::tr("确定每个拖曳点");
    if (idx == 171) return QObject::tr("确定位置");
    if (idx == 172) return QObject::tr("两端的笔压设为0");
    if (idx == 173) return QObject::tr("移动1px");
    if (idx == 174) return QObject::tr("将图层向左移动 1px");
    if (idx == 175) return QObject::tr("将图层向上移动 1px");
    if (idx == 176) return QObject::tr("将图层向下移动 1px");
    if (idx == 177) return QObject::tr("将图层向右移动 1px");

    if (idx == 180) return QObject::tr("载入色板档案");
    if (idx == 181) return QObject::tr("储存色板档案");
    if (idx == 182) return QObject::tr("FireAlpaca色板 (*.fap);;FireAlpaca色板 (*.fap)");
    if (idx == 183) return QObject::tr("载入ACO档案");
    if (idx == 184) return QObject::tr("输出ACO档案");
    if (idx == 185) return QObject::tr("ACO色彩色板(*.aco);;ACO色板 (*.aco)");

    if (idx == 190) return QObject::tr("变形");
    if (idx == 191) return QObject::tr("自由变形");
    if (idx == 192) return QObject::tr("网格变形");

    if (idx == 200) return QObject::tr("双侧对称");
    if (idx == 201) return QObject::tr("纵向对称");
    if (idx == 202) return QObject::tr("旋转对称");
    if (idx == 203) return QObject::tr(" (镜像)");
    if (idx == 204) return QObject::tr("瓷砖");
    if (idx == 205) return QObject::tr("对称");
    if (idx == 206) return QObject::tr("(半滴)");
    if (idx == 207) return QObject::tr("指定中心"); // 中心を指定
    if (idx == 208) return QObject::tr("线对称"); // 線対称

    if (idx == 210) return QObject::tr("套索上色工具");
    if (idx == 211) return QObject::tr("封闭区");
    if (idx == 212) return QObject::tr("透明和白色");
    if (idx == 213) return QObject::tr("透明");
    if (idx == 214) return QObject::tr("白");
    if (idx == 215) return QObject::tr("全部");
    if (idx == 219) return QObject::tr("狭窄区间不上色");

    if (idx == 221) return QObject::tr("新");
    if (idx == 222) return QObject::tr("追加");
    if (idx == 223) return QObject::tr("删除");
    if (idx == 224) return QObject::tr("共通");

    if (idx == 230) return QObject::tr("强度");
    if (idx == 231) return QObject::tr("确定每个描边");
    if (idx == 232) return QObject::tr("确定");

    if (idx == 240) return QObject::tr("膨胀");
    if (idx == 241) return QObject::tr("收缩");
    if (idx == 242) return QObject::tr("押");
    if (idx == 243) return QObject::tr("向右旋转");
    if (idx == 244) return QObject::tr("向左旋转");
    if (idx == 249) return QObject::tr("变形工具");

    if (idx == 260) return QObject::tr("上次的恢复失败，已跳过恢复过程。"); // 前回の復旧に失敗したため、復旧処理をスキップしました

    //if (idx == 270) return QObject::tr("Select the layer within the frame (Animation Mode)"); // フレーム内のレイヤーを選択 (アニメーションモード)

    if (idx == 300) return QObject::tr("用于透明色的滴管");

    if (idx == 310) return QObject::tr("希望使用曾FireAlpaca (免费版) 使用的设定吗 \n\n(应用程式需重新启动先行关闭)");
    if (idx == 311) return QObject::tr("恢复FireAlpaca SE标准设定 \n\n(应用程式需重新启动先行关闭)");
    if (idx == 312) return QObject::tr("是否要继承旧版本的设置？"); // 旧バージョンの設定を引き継ぎますか？
    if (idx == 313) return QObject::tr("已备份配置文件。要打开备份文件夹吗？"); // 設定ファイルをバックアップしました。バックアップしたフォルダを開きますか？
    if (idx == 314) return QObject::tr("是否要备份画笔、颜色、渐变和快捷键设置？"); // ブラシ、カラー、グラデーション、ショートカット設定をバックアップしますか？
    if (idx == 315) return QObject::tr("当前的设置将被丢弃，是否继续？"); // 現在の設定は破棄されますが、宜しいですか？

    if (idx == 320) return QObject::tr("选择顶点/对象");
    if (idx == 321) return QObject::tr("选择图层");

    if (idx == 330) return QObject::tr("所有形式");
    if (idx == 331) return QObject::tr("FireAlpaca SE 项目");

    if (idx == 425) return QObject::tr("画布"); // キャンバス
    if (idx == 426) return QObject::tr("图层"); // アクティブレイヤー
    if (idx == 427) return QObject::tr("指定图层"); // 指定レイヤー

    if (idx == 440) return QObject::tr("拖拽时也可使用吸管工具"); // ドラッグ時にもスポイト

    if (idx == 450) return QObject::tr("确定要删除图层吗？"); // 本当にレイヤーを削除しますか？
    if (idx == 451) return QObject::tr("您可以在首选项中设置为删除时不再确认。"); // 環境設定から確認なしに削除するよう設定できます

    if (idx == 500) return QObject::tr("内存消耗减少一半，但颜色精度会降低。确定吗？"); // メモリ消費量が半分になりますが、色精度が低下します。宜しいですか？
    if (idx == 501) return QObject::tr("颜色精度提高，编辑时的损耗减少，但内存使用量会加倍。确定吗？"); // 色精度が向上し編集時の劣化が軽減しますが、メモリ使用量が倍になります。宜しいですか？
    if (idx == 502) return QObject::tr("要将现有图层转换为 8bit/ch 吗？"); // 既存のレイヤーを 8bit/ch に変換しますか？
    if (idx == 503) return QObject::tr("要将现有图层转换为 16bit/ch 吗？"); // 既存のレイヤーを 16bit/ch に変換しますか？
    if (idx == 510) return QObject::tr("这是高级设置，确定要启用吗？"); // 上級者向けの設定ですが、本当に有効にしますか？

    if (idx == 600) return QObject::tr("MFG 存档格式（*.mar）");
    if (idx == 601) return QObject::tr("读取 MAR 文件");
    if (idx == 602) return QObject::tr("MAR 文件已损坏: ");
    if (idx == 603) return QObject::tr("复制文件失败: ");
    if (idx == 604) return QObject::tr("已导入 %1 个滤镜");
    if (idx == 605) return QObject::tr("处理时间较长…");

    //if (idx == 700) return QObject::tr("The process failed. This is likely due to insufficient main memory."); // 処理に失敗しました。メインメモリが不足している可能性が高いです。
  }

  //////////////////////////////////
  //////////////////////////////////
  if (man->TranslateChineseTrad())
  {
    if (idx == 1) return QObject::tr("筆刷工具");
    if (idx == 2) return QObject::tr("點刷工具");
    if (idx == 3) return QObject::tr("橡皮擦工具");
    if (idx == 4) return QObject::tr("移動工具");
    if (idx == 5) return QObject::tr("油漆桶工具");
    if (idx == 6) return QObject::tr("漸層工具");
    if (idx == 7) return QObject::tr("填充工具");
    if (idx == 8) return QObject::tr("選擇工具");
    if (idx == 9) return QObject::tr("魔術棒工具");
    if (idx == 10) return QObject::tr("選擇筆工具");
    if (idx == 11) return QObject::tr("選擇消除工具");
    if (idx == 12) return QObject::tr("文本工具");
    if (idx == 13) return QObject::tr("滴管工具");
    if (idx == 14) return QObject::tr("手掌工具");

    if (idx == 15) return QObject::tr("停止輔助");
    if (idx == 16) return QObject::tr("平行輔助");
    if (idx == 17) return QObject::tr("十字輔助");
    if (idx == 18) return QObject::tr("集中線輔助");
    if (idx == 19) return QObject::tr("輔助設定");

    if (idx == 20) return QObject::tr("反鋸齒");
    if (idx == 21) return QObject::tr("抖動修正");
    if (idx == 22) return QObject::tr("圓角");
    if (idx == 23) return QObject::tr("不透明度");
    if (idx == 24) return QObject::tr("參照");

    if (idx == 27) return QObject::tr("擴張");
    if (idx == 28) return QObject::tr("形狀");
    if (idx == 29) return QObject::tr("類型");
    if (idx == 30) return QObject::tr("線形");
    if (idx == 31) return QObject::tr("圓形");
    if (idx == 32) return QObject::tr("前景～背景");
    if (idx == 33) return QObject::tr("前景");
    if (idx == 34) return QObject::tr("矩形");
    if (idx == 35) return QObject::tr("楕圓");
    if (idx == 36) return QObject::tr("多角形");
    if (idx == 37) return QObject::tr("無設定");
    if (idx == 38) return QObject::tr("套索");

    if (idx == 40) return QObject::tr("色彩");
    if (idx == 41) return QObject::tr("筆刷參數");
    if (idx == 42) return QObject::tr("筆刷");
    if (idx == 43) return QObject::tr("導覽");
    if (idx == 44) return QObject::tr("圖層");
    if (idx == 45) return QObject::tr("筆刷預覽");
    if (idx == 46) return QObject::tr("色板");
    if (idx == 47) return QObject::tr("資料");
    if (idx == 48) return QObject::tr("筆刷大小");
    if (idx == 49) return QObject::tr("色彩歷史");

    if (idx == 50) return QObject::tr("(反轉)");
    if (idx == 51) return QObject::tr("檔案不存在");
    if (idx == 52) return QObject::tr("已開啟，確定繼續執行嗎？");
    if (idx == 54) return QObject::tr("消失點輔助");
    if (idx == 55) return QObject::tr("開啟圖像");
    if (idx == 56) return QObject::tr("圖像");
    if (idx == 57) return QObject::tr("同心圓輔助");
    if (idx == 58) return QObject::tr("曲線輔助");

    if (idx == 60) return QObject::tr("擴張");
    if (idx == 61) return QObject::tr("收縮");
    if (idx == 62) return QObject::tr("確定要初始化視窗位置嗎？");
    if (idx == 63) return QObject::tr("取消");
    if (idx == 64) return QObject::tr("復原");
    if (idx == 65) return QObject::tr("輔助");
    if (idx == 66) return QObject::tr("為了切換語言，必須先關閉應用軟體。稍後請重新啟動。");
    if (idx == 67) return QObject::tr("讀取失敗");
    if (idx == 68) return QObject::tr("閾值");

    if (idx == 70) return QObject::tr("無法連結至印表機");
    if (idx == 71) return QObject::tr("羽化邊緣");
    if (idx == 72) return QObject::tr("未設定");
    if (idx == 73) return QObject::tr("選擇點擊處的圖層");

    if (idx == 75) return QObject::tr("平行");
    if (idx == 76) return QObject::tr("集中線");
    if (idx == 77) return QObject::tr("同心圓");
    if (idx == 78) return QObject::tr("曲線");
    if (idx == 79) return QObject::tr("消失點");

    if (idx == 80) return QObject::tr("操作工具");
    if (idx == 81) return QObject::tr("分割工具");
    if (idx == 82) return QObject::tr("左右間距");
    if (idx == 83) return QObject::tr("上下間距");
    if (idx == 84) return QObject::tr("添加框格素材");    

    if (idx == 85) return QObject::tr("從中央選擇");
    if (idx == 86) return QObject::tr("固定縱橫比例");

    if (idx == 87) return QObject::tr("為了套用快捷鍵設定，必須先關閉應用軟體。稍後請重新啟動。");

    if (idx == 88) return QObject::tr("其他");
    if (idx == 89) return QObject::tr("放大筆刷尺寸");
    if (idx == 90) return QObject::tr("縮小筆刷尺寸");
    if (idx == 91) return QObject::tr("此為無效MDP檔案。");
    if (idx == 92) return QObject::tr("%1 並非正確的MDP檔案格式，無法開啟。");
    if (idx == 93) return QObject::tr("檔案已存在");
    if (idx == 94) return QObject::tr("請問要在瀏覽器中開啟GIF動畫轉換服務「羊駝動畫」嗎？");
    if (idx == 95) return QObject::tr("要透過背景嗎？");
    if (idx == 96) return QObject::tr("製作資料夾嗎？");
    if (idx == 97) return QObject::tr("網格輔助");
    if (idx == 98) return QObject::tr("輔助線吸附");
    if (idx == 99) return QObject::tr("框格分割");

    if (idx == 110) return QObject::tr("手繪");
    if (idx == 111) return QObject::tr("直線");
    if (idx == 112) return QObject::tr("折線");
    if (idx == 113) return QObject::tr("多角形");
    if (idx == 114) return QObject::tr("矩形");
    if (idx == 115) return QObject::tr("楕圓");
    if (idx == 116) return QObject::tr("曲線");
    if (idx == 117) return QObject::tr("圖形");
    if (idx == 118) return QObject::tr("正方形");
    if (idx == 119) return QObject::tr("正圓");

    if (idx == 120) return QObject::tr("3D視角輔助");
    if (idx == 121) return QObject::tr("檔案處理中，無法開騎其他的檔案");
    if (idx == 122) return QObject::tr("解除選擇");
    if (idx == 123) return QObject::tr("輔助網格");
    if (idx == 124) return QObject::tr("無視彩色圖層");
    if (idx == 125) return QObject::tr("無視草稿圖層");

    if (idx == 126) return QObject::tr("平均化");
    if (idx == 127) return QObject::tr("無");
    if (idx == 128) return QObject::tr("3 * 3 px 平均");
    if (idx == 129) return QObject::tr("5 * 5 px 平均");

    if (idx == 130) return QObject::tr("自動儲存中...");
    if (idx == 131) return QObject::tr("完成自動儲存");
    if (idx == 132) return QObject::tr("自動儲存無效化 (内存不足)\n\n整體展現的穩定性下降");
    if (idx == 133) return QObject::tr("要恢復自動儲存的圖像嗎？");

    if (idx == 135) return QObject::tr("pixiv Sketch投稿失敗");

    if (idx == 136) return QObject::tr("延展");
    if (idx == 137) return QObject::tr("標準");
    if (idx == 138) return QObject::tr("反覆");
    if (idx == 139) return QObject::tr("折返");

    if (idx == 140) return QObject::tr("修至圓角");
    //if (idx == 141) return QObject::tr("Gaussian Blur"); // ガウスぼかし

    if (idx == 150) return QObject::tr("維持圓角及粗細");

    if (idx == 160) return QObject::tr("填滿間隙");
    if (idx == 162) return QObject::tr("容許值");
    if (idx == 163) return QObject::tr("透明〜前景色");
    if (idx == 164) return QObject::tr("客製化");
    if (idx == 165) return QObject::tr("消除油漆桶");

    if (idx == 170) return QObject::tr("確定每個拖曳點");
    if (idx == 171) return QObject::tr("確定位置");
    if (idx == 172) return QObject::tr("兩端的筆壓設為0");
    if (idx == 173) return QObject::tr("移動1px");
    if (idx == 174) return QObject::tr("將圖層向左移動 1px");
    if (idx == 175) return QObject::tr("將圖層向上移動 1px");
    if (idx == 176) return QObject::tr("將圖層向下移動 1px");
    if (idx == 177) return QObject::tr("將圖層向右移動 1px");

    if (idx == 180) return QObject::tr("載入色板檔案");
    if (idx == 181) return QObject::tr("儲存色板檔案");
    if (idx == 182) return QObject::tr("FireAlpaca色板 (*.fap);;FireAlpaca色板 (*.fap)");
    if (idx == 183) return QObject::tr("載入ACO檔案");
    if (idx == 184) return QObject::tr("輸出ACO檔案");
    if (idx == 185) return QObject::tr("ACO色彩色板(*.aco);;ACO色板 (*.aco)");

    if (idx == 190) return QObject::tr("變形");
    if (idx == 191) return QObject::tr("自由變形");
    if (idx == 192) return QObject::tr("網格變形");

    if (idx == 200) return QObject::tr("左右對稱");
    if (idx == 201) return QObject::tr("上下對稱");
    if (idx == 202) return QObject::tr("迴轉對稱");
    if (idx == 203) return QObject::tr(" (鏡子)");
    if (idx == 204) return QObject::tr("磁磚");
    if (idx == 205) return QObject::tr("對稱");
    if (idx == 206) return QObject::tr(" (半滴)");
    if (idx == 207) return QObject::tr("指定中心"); // 中心を指定
    if (idx == 208) return QObject::tr("線對稱"); // 線対称

    if (idx == 210) return QObject::tr("套索上色工具");
    if (idx == 211) return QObject::tr("封閉區");
    if (idx == 212) return QObject::tr("透明和白色");
    if (idx == 213) return QObject::tr("透明");
    if (idx == 214) return QObject::tr("白");
    if (idx == 215) return QObject::tr("全部");
    if (idx == 219) return QObject::tr("狹窄區間不上色");

    if (idx == 221) return QObject::tr("新");
    if (idx == 222) return QObject::tr("追加");
    if (idx == 223) return QObject::tr("刪除");
    if (idx == 224) return QObject::tr("共通");

    if (idx == 230) return QObject::tr("強度");
    if (idx == 231) return QObject::tr("確定每個描邊");
    if (idx == 232) return QObject::tr("確定");

    if (idx == 240) return QObject::tr("膨脹");
    if (idx == 241) return QObject::tr("收縮");
    if (idx == 242) return QObject::tr("押");
    if (idx == 243) return QObject::tr("向右旋轉");
    if (idx == 244) return QObject::tr("向左旋轉");
    if (idx == 249) return QObject::tr("變形工具");

    if (idx == 260) return QObject::tr("上次的恢復失敗，已跳過恢復過程。"); // 前回の復旧に失敗したため、復旧処理をスキップしました

    //if (idx == 270) return QObject::tr("Select the layer within the frame (Animation Mode)"); // フレーム内のレイヤーを選択 (アニメーションモード)

    if (idx == 300) return QObject::tr("用於透明色的滴管");

    if (idx == 310) return QObject::tr("希望使用曾FireAlpaca (免費版) 使用的設定嗎 \n\n(應用程式需重新啟動先行關閉)");
    if (idx == 311) return QObject::tr("恢復FireAlpaca SE標準設定 \n\n(應用程式需重新啟動先行關閉)");
    if (idx == 312) return QObject::tr("是否要繼承舊版本的設定？"); // 旧バージョンの設定を引き継ぎますか？
    if (idx == 313) return QObject::tr("已備份設定檔。要開啟備份資料夾嗎？"); // 設定ファイルをバックアップしました。バックアップしたフォルダを開きますか？
    if (idx == 314) return QObject::tr("是否要備份筆刷、顏色、漸層與快捷鍵設定？"); // ブラシ、カラー、グラデーション、ショートカット設定をバックアップしますか？
    if (idx == 315) return QObject::tr("目前的設定將被捨棄，是否繼續？"); // 現在の設定は破棄されますが、宜しいですか？

    if (idx == 320) return QObject::tr("選擇頂點/物件"); // 
    if (idx == 321) return QObject::tr("選擇圖層"); // 

    if (idx == 330) return QObject::tr("所有形式"); // 
    if (idx == 331) return QObject::tr("FireAlpaca SE 專案"); // 

    if (idx == 425) return QObject::tr("畫布"); // キャンバス
    if (idx == 426) return QObject::tr("圖層"); // アクティブレイヤー
    if (idx == 427) return QObject::tr("指定圖層"); // 指定レイヤー

    if (idx == 440) return QObject::tr("拖曳時也可使用滴管工具"); // ドラッグ時にもスポイト

    if (idx == 450) return QObject::tr("確定要刪除圖層嗎？"); // 本当にレイヤーを削除しますか？
    if (idx == 451) return QObject::tr("您可以在偏好設定中設為刪除時不需再次確認。"); // 環境設定から確認なしに削除するよう設定できます

    if (idx == 500) return QObject::tr("記憶體消耗減少一半，但顏色精度會降低。確定嗎？"); // メモリ消費量が半分になりますが、色精度が低下します。宜しいですか？
    if (idx == 501) return QObject::tr("顏色精度提高，編輯時的損耗減少，但記憶體使用量會加倍。確定嗎？"); // 色精度が向上し編集時の劣化が軽減しますが、メモリ使用量が倍になります。宜しいですか？
    if (idx == 502) return QObject::tr("要將現有圖層轉換為 8bit/ch 嗎？"); // 既存のレイヤーを 8bit/ch に変換しますか？
    if (idx == 503) return QObject::tr("要將現有圖層轉換為 16bit/ch 嗎？"); // 既存のレイヤーを 16bit/ch に変換しますか？
    if (idx == 510) return QObject::tr("這是進階設定，確定要啟用嗎？"); // 上級者向けの設定ですが、本当に有効にしますか？

    if (idx == 600) return QObject::tr("MFG 封存格式（*.mar）");
    if (idx == 601) return QObject::tr("讀取 MAR 檔案");
    if (idx == 602) return QObject::tr("MAR 檔案已損壞: ");
    if (idx == 603) return QObject::tr("複製檔案失敗: ");
    if (idx == 604) return QObject::tr("已匯入 %1 個濾鏡");
    if (idx == 605) return QObject::tr("處理需要較長時間…");

    //if (idx == 700) return QObject::tr("The process failed. This is likely due to insufficient main memory."); // 処理に失敗しました。メインメモリが不足している可能性が高いです。
  }

  //////////////////////////////////
  //////////////////////////////////
  if (man->TranslateKorean())
  {
    if (idx == 1) return QObject::tr("브러시 도구");
    if (idx == 2) return QObject::tr("도트 도구");
    if (idx == 3) return QObject::tr("지우개 도구");
    if (idx == 4) return QObject::tr("레이어 이동 도구");
    if (idx == 5) return QObject::tr("채우기 도구");
    if (idx == 6) return QObject::tr("그라데이션 도구");
    if (idx == 7) return QObject::tr("채우기 도구");
    if (idx == 8) return QObject::tr("선택 도구");
    if (idx == 9) return QObject::tr("자동 선택 도구");
    if (idx == 10) return QObject::tr("선택 펜 도구");
    if (idx == 11) return QObject::tr("선택 지우개 도구");
    if (idx == 12) return QObject::tr("텍스트 도구");
    if (idx == 13) return QObject::tr("스포이트 도구");
    if (idx == 14) return QObject::tr("이동 도구");

    if (idx == 15) return QObject::tr("스냅 Off");
    if (idx == 16) return QObject::tr("평행 스냅");
    if (idx == 17) return QObject::tr("십자 스냅");
    if (idx == 18) return QObject::tr("집중선 스냅");
    if (idx == 19) return QObject::tr("스냅 설정");

    if (idx == 20) return QObject::tr("안티앨리어싱");
    if (idx == 21) return QObject::tr("손떨림 보정");
    if (idx == 22) return QObject::tr("둥근 모서리");
    if (idx == 23) return QObject::tr("불투명도");
    if (idx == 24) return QObject::tr("참조");

    if (idx == 27) return QObject::tr("영역 확대/축소");
    if (idx == 28) return QObject::tr("모양");
    if (idx == 29) return QObject::tr("타입");
    if (idx == 30) return QObject::tr("선형");
    if (idx == 31) return QObject::tr("원형");
    if (idx == 32) return QObject::tr("전경∼배경");
    if (idx == 33) return QObject::tr("전경");
    if (idx == 34) return QObject::tr("직사각형");
    if (idx == 35) return QObject::tr("타원");
    if (idx == 36) return QObject::tr("다각형");
    if (idx == 37) return QObject::tr("설정 없음");
    if (idx == 38) return QObject::tr("올가미");
    if (idx == 39) return QObject::tr("올가미 도구");
    if (idx == 40) return QObject::tr("컬러");
    if (idx == 41) return QObject::tr("브러시 컨트롤");
    if (idx == 42) return QObject::tr("브러시");
    if (idx == 43) return QObject::tr("내비게이터");
    if (idx == 44) return QObject::tr("레이어");
    if (idx == 45) return QObject::tr("브러시 미리보기");
    if (idx == 46) return QObject::tr("팔레트");
    if (idx == 47) return QObject::tr("참고 자료");
    if (idx == 48) return QObject::tr("브러시 사이즈");
    if (idx == 49) return QObject::tr("컬러 히스토리");
    if (idx == 50) return QObject::tr("(반전)");
    if (idx == 51) return QObject::tr("파일이 존재하지 않습니다.");
    if (idx == 52) return QObject::tr("이미 열려 있는 파일입니다. 계속하시겠습니까?");
    if (idx == 54) return QObject::tr("소실점 스냅");
    if (idx == 55) return QObject::tr("이미지 열기");
    if (idx == 56) return QObject::tr("이미지");
    if (idx == 57) return QObject::tr("원형 스냅");
    if (idx == 58) return QObject::tr("곡선 스냅");
    if (idx == 60) return QObject::tr("확장");
    if (idx == 61) return QObject::tr("수축");
    if (idx == 62) return QObject::tr("창의 위치를 초기화하시겠습니까?");
    if (idx == 63) return QObject::tr("실행 취소");
    if (idx == 64) return QObject::tr("재실행");
    if (idx == 65) return QObject::tr("스냅");
    if (idx == 66) return QObject::tr("언어를 바꾸기 위하여 프로그램을 일단 종료합니다. 종료 후 다시 시작해 주십시오.");
    if (idx == 67) return QObject::tr("불러오기에 실패했습니다.");
    if (idx == 68) return QObject::tr("한계값");

    if (idx == 70) return QObject::tr("프린터를 찾을 수 없습니다.");
    if (idx == 71) return QObject::tr("부드러운 테두리");
    if (idx == 72) return QObject::tr("미설정");
    if (idx == 73) return QObject::tr("클릭 위치의 레이어를 선택");

    if (idx == 75) return QObject::tr("평행");
    if (idx == 76) return QObject::tr("집중선");
    if (idx == 77) return QObject::tr("원형");
    if (idx == 78) return QObject::tr("곡선");
    if (idx == 79) return QObject::tr("소실점");

    if (idx == 80) return QObject::tr("조작 도구");
    if (idx == 81) return QObject::tr("분할 도구");
    if (idx == 82) return QObject::tr("좌우 간격");
    if (idx == 83) return QObject::tr("상하 간격");
    if (idx == 84) return QObject::tr("컷 테두리 추가");

    if (idx == 85) return QObject::tr("중앙에서 선택");
    if (idx == 86) return QObject::tr("종횡비 고정");

    if (idx == 87) return QObject::tr("단축키를 반영하기 위하여 프로그램을 일단 종료합니다. 종료 후 다시 시작해 주십시오.");

    if (idx == 88) return QObject::tr("그 외");
    if (idx == 89) return QObject::tr("브러시 사이즈를 크게");
    if (idx == 90) return QObject::tr("브러시 사이즈를 작게");
    if (idx == 91) return QObject::tr("잘못된 MDP 파일입니다.");
    if (idx == 92) return QObject::tr("%1는 잘못된 MDP 파일이므로 열 수 없습니다.");
    if (idx == 93) return QObject::tr("이미 존재하는 파일입니다.");
    if (idx == 94) return QObject::tr("애니메이션 GIF 변환 서비스 'AlpacaDouga'를 브라우저로 여시겠습니까?");
    if (idx == 95) return QObject::tr("배경을 투명하게 만드시겠습니까?");
    if (idx == 96) return QObject::tr("폴더를 만드시겠습니까?");
    if (idx == 97) return QObject::tr("그리드 스냅");
    if (idx == 98) return QObject::tr("가이드 스냅");
    if (idx == 99) return QObject::tr("컷 테두리 분할");

    if (idx == 110) return QObject::tr("프리핸드");
    if (idx == 111) return QObject::tr("직선");
    if (idx == 112) return QObject::tr("꺾은선");
    if (idx == 113) return QObject::tr("다각형");
    if (idx == 114) return QObject::tr("직사각형");
    if (idx == 115) return QObject::tr("타원형");
    if (idx == 116) return QObject::tr("곡선");
    if (idx == 117) return QObject::tr("도형");
    if (idx == 118) return QObject::tr("정사각형");
    if (idx == 119) return QObject::tr("원");

    if (idx == 120) return QObject::tr("3D 퍼스 스냅");
    if (idx == 121) return QObject::tr("필터 처리 중에는 다른 파일을 열 수 없습니다.");
    if (idx == 122) return QObject::tr("선택 해제");
    if (idx == 123) return QObject::tr("그리드에 스냅");
    if (idx == 124) return QObject::tr("레이어 색 무시");
    if (idx == 125) return QObject::tr("드래프트 레이어 무시");

    if (idx == 126) return QObject::tr("평균화");
    if (idx == 127) return QObject::tr("없음");
    if (idx == 128) return QObject::tr("3 * 3 px 평균");
    if (idx == 129) return QObject::tr("5 * 5 px 평균");

    if (idx == 130) return QObject::tr("자동 저장 중...");
    if (idx == 131) return QObject::tr("자동 저장 완료");
    if (idx == 132) return QObject::tr("자동 저장을 비활성화하였습니다. (메모리 부족)\n\n프로그램의 안정성이 현저하게 낮아졌습니다.");
    if (idx == 133) return QObject::tr("자동 저장한 이미지를 복구하시겠습니까?");

    if (idx == 135) return QObject::tr("pixiv Sketch에 게시하지 못했습니다.");

    if (idx == 136) return QObject::tr("확산 방법");
    if (idx == 137) return QObject::tr("표준");
    if (idx == 138) return QObject::tr("반복");
    if (idx == 139) return QObject::tr("반사");

    if (idx == 140) return QObject::tr("모서리를 둥글게");
    //if (idx == 141) return QObject::tr("Gaussian Blur"); // ガウスぼかし

    if (idx == 150) return QObject::tr("모서리를 둥글게, 두께 유지");

    if (idx == 160) return QObject::tr("선 틈새 닫기");
    if (idx == 162) return QObject::tr("허용치");
    if (idx == 163) return QObject::tr("투명색 ~ 전경색");
    if (idx == 164) return QObject::tr("커스텀");
    if (idx == 165) return QObject::tr("페인트 통 지우개");

    if (idx == 170) return QObject::tr("드래그할 때마다 위치 결정");
    if (idx == 171) return QObject::tr("위치 결정");
    if (idx == 172) return QObject::tr("양끝의 필압을 0으로 만들기");
    if (idx == 173) return QObject::tr("1px 이동");
    if (idx == 174) return QObject::tr("레이어 왼쪽으로 1px 이동");
    if (idx == 175) return QObject::tr("레이어 위로 1px 이동");
    if (idx == 176) return QObject::tr("레이어 아래로 1px 이동");
    if (idx == 177) return QObject::tr("레이어 오른쪽으로 1px 이동");

    if (idx == 180) return QObject::tr("팔레트 파일 불러오기");
    if (idx == 181) return QObject::tr("팔레트 파일 내보내기");
    if (idx == 182) return QObject::tr("FireAlapca 팔레트 (*.fap);;FireAlapca 팔레트 (*.fap);;");
    if (idx == 183) return QObject::tr("ACO 파일 불러오기");
    if (idx == 184) return QObject::tr("ACO 파일 내보내기");
    if (idx == 185) return QObject::tr("ACO 컬러 팔레트 (*.aco);;ACO 컬러 팔레트 (*.aco)");

    if (idx == 190) return QObject::tr("변형");
    if (idx == 191) return QObject::tr("자유 변형");
    if (idx == 192) return QObject::tr("메쉬 변형");

    if (idx == 200) return QObject::tr("좌우 대칭");
    if (idx == 201) return QObject::tr("상하 대칭");
    if (idx == 202) return QObject::tr("회전 대칭");
    if (idx == 203) return QObject::tr("(거울)");
    if (idx == 204) return QObject::tr("타이틀");
    if (idx == 205) return QObject::tr("대칭");
    if (idx == 206) return QObject::tr("(하프 드롭)");
    if (idx == 207) return QObject::tr("중심 지정"); // 中心を指定
    if (idx == 208) return QObject::tr("선 대칭"); // 線対称

    if (idx == 210) return QObject::tr("올가미 도구");
    if (idx == 211) return QObject::tr("둘러싸인 영역");
    if (idx == 212) return QObject::tr("투명색/흰색");
    if (idx == 213) return QObject::tr("투명색");
    if (idx == 214) return QObject::tr("흰색");
    if (idx == 215) return QObject::tr("모두");
    if (idx == 219) return QObject::tr("좁은 영역에 칠하지 않기");

    if (idx == 221) return QObject::tr("신규");
    if (idx == 222) return QObject::tr("추가");
    if (idx == 223) return QObject::tr("삭제");
    if (idx == 224) return QObject::tr("공통 부분");

    if (idx == 230) return QObject::tr("세기");
    if (idx == 231) return QObject::tr("획 별로 고정");
    if (idx == 232) return QObject::tr("고정");

    if (idx == 240) return QObject::tr("팽창");
    if (idx == 241) return QObject::tr("수축");
    if (idx == 242) return QObject::tr("누르기");
    if (idx == 243) return QObject::tr("우회전");
    if (idx == 244) return QObject::tr("좌회전");
    if (idx == 249) return QObject::tr("픽셀 유동화 도구");

    if (idx == 260) return QObject::tr("이전 복구에 실패하여 복구 작업을 건너뛰었습니다."); // 前回の復旧に失敗したため、復旧処理をスキップしました

    //if (idx == 270) return QObject::tr("Select the layer within the frame (Animation Mode)"); // フレーム内のレイヤーを選択 (アニメーションモード)

    if (idx == 300) return QObject::tr("투명색 스포이트로 선택하기");

    if (idx == 310) return QObject::tr("FireAlpaca(무료판)에서 사용하던 설정 파일을 적용하시겠습니까?\n\n(재부팅이 필요하므로 프로그램을 종료합니다.)");
    if (idx == 311) return QObject::tr("FireAlpaca SE의 표준 설정 파일을 적용하시겠습니까?\n\n\n(재부팅이 필요하므로 프로그램을 종료합니다.)");
    if (idx == 312) return QObject::tr("이전 버전의 설정을 가져오시겠습니까?"); // 旧バージョンの設定を引き継ぎますか？
    if (idx == 313) return QObject::tr("설정 파일이 백업되었습니다. 백업 폴더를 열까요?"); // 設定ファイルをバックアップしました。バックアップしたフォルダを開きますか？
    if (idx == 314) return QObject::tr("브러시, 색상, 그라데이션, 단축키 설정을 백업하시겠습니까?"); // ブラシ、カラー、グラデーション、ショートカット設定をバックアップしますか？
    if (idx == 315) return QObject::tr("현재 설정은 삭제됩니다. 진행하시겠습니까?"); // 現在の設定は破棄されますが、宜しいですか？

  if (idx == 320) return QObject::tr("정점/객체 선택"); // 頂点・オブジェクトを選択
  if (idx == 321) return QObject::tr("레이어 선택"); // レイヤーを選択

  if (idx == 330) return QObject::tr("모든 형식"); // 全ての形式
  if (idx == 331) return QObject::tr("FireAlpaca SE 프로젝트"); // FireAlpaca SE プロジェクト

    if (idx == 425) return QObject::tr("캔버스"); // キャンバス
    if (idx == 426) return QObject::tr("레이어"); // アクティブレイヤー
    if (idx == 427) return QObject::tr("지정된 레이어"); // 指定レイヤー

    if (idx == 440) return QObject::tr("드래그 중에도 스포이트 도구 사용"); // ドラッグ時にもスポイト

    if (idx == 450) return QObject::tr("레이어를 정말 삭제하시겠습니까?"); // 本当にレイヤーを削除しますか？
    if (idx == 451) return QObject::tr("환경 설정에서 확인 없이 삭제하도록 설정할 수 있습니다."); // 環境設定から確認なしに削除するよう設定できます

    if (idx == 500) return QObject::tr("메모리 사용량이 절반으로 줄어들지만 색상 정확도가 낮아집니다. 괜찮습니까?"); // メモリ消費量が半分になりますが、色精度が低下します。宜しいですか？
    if (idx == 501) return QObject::tr("색상 정확도가 향상되고 편집 시 열화가 감소하지만, 메모리 사용량이 두 배로 증가합니다. 괜찮습니까?"); // 色精度が向上し編集時の劣化が軽減しますが、メモリ使用量が倍になります。宜しいですか？
    if (idx == 502) return QObject::tr("기존 레이어를 8bit/ch로 변환하시겠습니까?"); // 既存のレイヤーを 8bit/ch に変換しますか？
    if (idx == 503) return QObject::tr("기존 레이어를 16bit/ch로 변환하시겠습니까?"); // 既存のレイヤーを 16bit/ch に変換しますか？
    if (idx == 510) return QObject::tr("고급 설정입니다. 정말 활성화하시겠습니까?"); // 上級者向けの設定ですが、本当に有効にしますか？

    if (idx == 600) return QObject::tr("MFG 아카이브 형식 (*.mar)"); // MFG アーカイブ形式(*.mar)
    if (idx == 601) return QObject::tr("MAR 파일 불러오기"); // MARファイルの読み込み
    if (idx == 602) return QObject::tr("MAR 파일이 손상되었습니다: "); // MARファイルが壊れています：
    if (idx == 603) return QObject::tr("파일 복사에 실패했습니다: "); // ファイルのコピーに失敗しました:
    if (idx == 604) return QObject::tr("%1개의 필터를 가져왔습니다"); // %1 個のフィルタをインポートしました
    if (idx == 605) return QObject::tr("처리에 시간이 오래 걸리고 있습니다..."); // 処理が長くかかっています…

    //if (idx == 700) return QObject::tr("The process failed. This is likely due to insufficient main memory."); // 処理に失敗しました。メインメモリが不足している可能性が高いです。
  }

  //////////////////////////////////
  //////////////////////////////////
  if (man->TranslatePortugues())
  {
    if (idx == 1) return QObject::tr("Ferramenta Pincel");
    if (idx == 2) return QObject::tr("Ferramenta Ponto");
    if (idx == 3) return QObject::tr("Ferramenta Borracha");
    if (idx == 4) return QObject::tr("Ferramenta Mover");
    if (idx == 5) return QObject::tr("Ferramenta Balde");
    if (idx == 6) return QObject::tr("Ferramenta Gradiente");
    if (idx == 7) return QObject::tr("Ferramenta Preenchimento");
    if (idx == 8) return QObject::tr("Ferramenta Seleção");
    if (idx == 9) return QObject::tr("Ferramenta Varinha Mágica");
    if (idx == 10) return QObject::tr("Ferramenta Caneta de Seleção");
    if (idx == 11) return QObject::tr("Ferramenta Seleção de Borracha");
    if (idx == 12) return QObject::tr("Ferramenta de Texto");
    if (idx == 13) return QObject::tr("Ferramenta Conta-gotas");
    if (idx == 14) return QObject::tr("Ferramenta Mão");

    if (idx == 15) return QObject::tr("Ajuste");
    if (idx == 16) return QObject::tr("Ajuste Paralelo");
    if (idx == 17) return QObject::tr("Ajuste Entrecruzado");
    if (idx == 18) return QObject::tr("Ajuste Radial");
    if (idx == 19) return QObject::tr("Configuração de Ajuste");

    if (idx == 20) return QObject::tr("Suavização");
    if (idx == 21) return QObject::tr("Correção");
    if (idx == 22) return QObject::tr("Arredondar Canto");
    if (idx == 23) return QObject::tr("Opacidade");
    if (idx == 24) return QObject::tr("Referência");

    if (idx == 27) return QObject::tr("Expandir");
    if (idx == 28) return QObject::tr("Formato");
    if (idx == 29) return QObject::tr("Tipo");
    if (idx == 30) return QObject::tr("Linear");
    if (idx == 31) return QObject::tr("Circular");
    if (idx == 32) return QObject::tr("Primeiro plano-Plano de fundo");
    if (idx == 33) return QObject::tr("Primeiro plano");
    if (idx == 34) return QObject::tr("Retângulo");
    if (idx == 35) return QObject::tr("Elipse");
    if (idx == 36) return QObject::tr("Polígono");
    if (idx == 37) return QObject::tr("Nenhuma Opção");
    if (idx == 38) return QObject::tr("Laço");

    if (idx == 40) return QObject::tr("Cor");
    if (idx == 41) return QObject::tr("Controle de Pincel");
    if (idx == 42) return QObject::tr("Pincel");
    if (idx == 43) return QObject::tr("Navegador");
    if (idx == 44) return QObject::tr("Camada");
    if (idx == 45) return QObject::tr("Visualização do Pincel");
    if (idx == 46) return QObject::tr("Paleta");
    if (idx == 47) return QObject::tr("Referência");

    if (idx == 50) return QObject::tr("(Inverter)");
    if (idx == 51) return QObject::tr("O arquivo não existe.");
    if (idx == 52)
    {
      QString s = QObject::tr("%1 já está aberto. Continuar?");
      s.replace( "%1", man->StrParam() );
      return s;
    }
    if (idx == 54) return QObject::tr("Ajuste de Ponto de Fuga");
    if (idx == 55) return QObject::tr("Abrir uma imagem");
    if (idx == 56) return QObject::tr("Imagem");

    if (idx == 60) return QObject::tr("Expandir");
    if (idx == 61) return QObject::tr("Encolher");
    if (idx == 62) return QObject::tr("Inicializar uma posição de janela?");
    if (idx == 63) return QObject::tr("Desfazer");
    if (idx == 64) return QObject::tr("Refazer");
    if (idx == 65) return QObject::tr("Ajustar");
    if (idx == 66) return QObject::tr("O programa será fechado para atualizar a configuração de idioma.  Reinicie o programa após fechá-lo.");
    if (idx == 67) return QObject::tr("Falha ao abrir");
    if (idx == 68) return QObject::tr("Limite");

    if (idx == 70) return QObject::tr("Falha em localizar a impressora.");
    if (idx == 71) return QObject::tr("Suavização de Bordas");
    if (idx == 72) return QObject::tr("Sem nome");
    if (idx == 73) return QObject::tr("Selecione uma camada no ponto clicado");

    if (idx == 75) return QObject::tr("Paralelo");
    if (idx == 76) return QObject::tr("Radial");
    if (idx == 77) return QObject::tr("Círculo");
    if (idx == 78) return QObject::tr("Curva");
    if (idx == 79) return QObject::tr("Ponto de fuga");

    if (idx == 80) return QObject::tr("Ferramenta de operação");
    if (idx == 81) return QObject::tr("Ferramenta de divisão");
    if (idx == 82) return QObject::tr("Espaçamento lateral");
    if (idx == 83) return QObject::tr("Espaçamento vertical");

    if (idx == 85) return QObject::tr("Selecionar do centro");
    if (idx == 86) return QObject::tr("Restringir proporções");

    if (idx == 87) return QObject::tr("Para aplicar o atalho, feche o aplicativo. Após o fechamento, reinicie o aplicativo.");

    if (idx == 88) return QObject::tr("Outros");
    if (idx == 89) return QObject::tr("Aumentar Tamanho do Pincel");
    if (idx == 90) return QObject::tr("Diminuir Tamanho do Pincel");
    if (idx == 91) return QObject::tr("Arquivo MDP inválido.");
    if (idx == 92) return QObject::tr("%1 não é um arquivo MDP válido e não pode ser aberto.");
    if (idx == 93) return QObject::tr("O arquivo já existe");
    if (idx == 94) return QObject::tr("Abrir o serviço de conversão de GIF animado \"Alpaca Douga\" no navegador?");
    if (idx == 95) return QObject::tr("Tornar o fundo transparente?");
    if (idx == 96) return QObject::tr("Deseja criar uma pasta?");
    if (idx == 97) return QObject::tr("Encaixe na Grade");
    if (idx == 98) return QObject::tr("Encaixe no Guia");
    if (idx == 99) return QObject::tr("Dividir Quadros");

    if (idx == 110) return QObject::tr("Mão Livre");
    if (idx == 111) return QObject::tr("Linha Reta");
    if (idx == 112) return QObject::tr("Linha Quebrada");
    if (idx == 113) return QObject::tr("Polígono");
    if (idx == 114) return QObject::tr("Retângulo");
    if (idx == 115) return QObject::tr("Elipse");
    if (idx == 116) return QObject::tr("Curva");
    if (idx == 117) return QObject::tr("Forma");
    if (idx == 118) return QObject::tr("Quadrado");
    if (idx == 119) return QObject::tr("Círculo Perfeito");

    if (idx == 120) return QObject::tr("Encaixe de Perspectiva 3D");
    if (idx == 121) return QObject::tr("Não é possível abrir outro arquivo durante o processamento de filtro.");
    if (idx == 122) return QObject::tr("Remover Seleção");
    if (idx == 123) return QObject::tr("Encaixar na Grade");
    if (idx == 124) return QObject::tr("Ignorar Cor da Camada");
    if (idx == 125) return QObject::tr("Ignorar Camada de Rascunho");

    if (idx == 126) return QObject::tr("Média");
    if (idx == 127) return QObject::tr("Nenhum");
    if (idx == 128) return QObject::tr("Média de 3 * 3 px");
    if (idx == 129) return QObject::tr("Média de 5 * 5 px");

    if (idx == 130) return QObject::tr("Salvamento Automático...");
    if (idx == 131) return QObject::tr("Salvamento Automático Concluído");
    if (idx == 132) return QObject::tr("自Salvamento Automático Desativado (Falta de Memória)\n\nA estabilidade do aplicativo foi severamente reduzida");
    if (idx == 133) return QObject::tr("Deseja recuperar a imagem salva automaticamente?");

    if (idx == 135) return QObject::tr("Falha ao postar no pixiv Sketch");

    if (idx == 136) return QObject::tr("Expansão");
    if (idx == 137) return QObject::tr("Padrão");
    if (idx == 138) return QObject::tr("Repetir");
    if (idx == 139) return QObject::tr("Alternar");

    if (idx == 140) return QObject::tr("Arredondar Cantos");
    //if (idx == 141) return QObject::tr("Gaussian Blur"); // ガウスぼかし

    if (idx == 150) return QObject::tr("Arredondar Cantos e Manter Espessura");

    if (idx == 160) return QObject::tr("Preencher Lacunas");
    if (idx == 162) return QObject::tr("Tolerância");
    if (idx == 163) return QObject::tr("Transparente para Cor de Primeiro Plano");
    if (idx == 164) return QObject::tr("Personalizado");
    if (idx == 165) return QObject::tr("Borracha de Balde");

    if (idx == 170) return QObject::tr("Confirmar Posição por Arraste");
    if (idx == 171) return QObject::tr("Confirmar Posição");
    if (idx == 172) return QObject::tr("Definir Pressão Zero nas Extremidades");
    if (idx == 173) return QObject::tr("Mover 1px");
    if (idx == 174) return QObject::tr("Mover Camada para a Esquerda 1px");
    if (idx == 175) return QObject::tr("Mover Camada para Cima 1px");
    if (idx == 176) return QObject::tr("Mover Camada para Baixo 1px");
    if (idx == 177) return QObject::tr("Mover Camada para a Direita 1px");

    if (idx == 180) return QObject::tr("Carregar Arquivo de Paleta");
    if (idx == 181) return QObject::tr("Salvar Arquivo de Paleta");
    if (idx == 182) return QObject::tr("Paleta FireAlpaca (.fap);;Paleta FireAlpaca (.fap)");
    if (idx == 183) return QObject::tr("Importar Arquivo ACO");
    if (idx == 184) return QObject::tr("Exportar Arquivo ACO");
    if (idx == 185) return QObject::tr("Paleta de Cores ACO (.aco);;Paleta de Cores ACO (.aco)");

    if (idx == 190) return QObject::tr("Transformação");
    if (idx == 191) return QObject::tr("Transformação Livre");
    if (idx == 192) return QObject::tr("Transformação de Malha");

    if (idx == 200) return QObject::tr("Simetria Esquerda-Direita"); // 左右対称
    if (idx == 201) return QObject::tr("Simetria Cima-Baixo"); // 上下対称
    if (idx == 202) return QObject::tr("Simetria Rotacional"); // 回転対称
    if (idx == 203) return QObject::tr(" (Imagem Espelhada)"); //  (鏡像)
    if (idx == 204) return QObject::tr("Mosaico"); // タイル
    if (idx == 205) return QObject::tr("Simetria"); // 対称
    if (idx == 206) return QObject::tr("(Meio Drop)"); //  (ハーフドロップ)
    if (idx == 207) return QObject::tr("Definir o centro"); // 中心を指定
    if (idx == 208) return QObject::tr("Simetria axial"); // 線対称

    if (idx == 210) return QObject::tr("Ferramenta de Pintura com Laço");
    if (idx == 211) return QObject::tr("Área Fechada");
    if (idx == 212) return QObject::tr("Partes Transparentes e Brancas");
    if (idx == 213) return QObject::tr("Partes Transparentes");
    if (idx == 214) return QObject::tr("Branco");
    if (idx == 215) return QObject::tr("Tudo");
    if (idx == 219) return QObject::tr("Não Pintar Áreas Pequenas");

    if (idx == 221) return QObject::tr("Novo");
    if (idx == 222) return QObject::tr("Adicionar");
    if (idx == 223) return QObject::tr("Remover");
    if (idx == 224) return QObject::tr("Interseção");

    if (idx == 230) return QObject::tr("Intensidade");
    if (idx == 231) return QObject::tr("Confirmar a Cada Traço");
    if (idx == 232) return QObject::tr("Confirmar");

    if (idx == 240) return QObject::tr("Expansão");
    if (idx == 241) return QObject::tr("Contração");
    if (idx == 242) return QObject::tr("Empurrar");
    if (idx == 243) return QObject::tr("Girar para a Direita");
    if (idx == 244) return QObject::tr("Girar para a Esquerda");
    if (idx == 249) return QObject::tr("Pincel de Distorção");

    if (idx == 260) return QObject::tr("A restauração anterior falhou. O processo de recuperação foi ignorado."); // 前回の復旧に失敗したため、復旧処理をスキップしました

    //if (idx == 270) return QObject::tr("Select the layer within the frame (Animation Mode)"); // フレーム内のレイヤーを選択 (アニメーションモード)

    if (idx == 300) return QObject::tr("Conta-gotas de Cor Transparente");
    if (idx == 301) return QObject::tr("Super Ellipse");

    if (idx == 310) return QObject::tr("Deseja usar os arquivos de configuração que estavam no FireAlpaca (versão gratuita) no futuro?\n\n(O aplicativo será encerrado para reinicialização)");
    if (idx == 311) return QObject::tr("Deseja voltar aos arquivos de configuração padrão do FireAlpaca SE?\n\n(O aplicativo será encerrado para reinicialização)");
    if (idx == 312) return QObject::tr("Deseja manter as configurações da versão anterior?"); // 旧バージョンの設定を引き継ぎますか？
    if (idx == 313) return QObject::tr("Os arquivos de configuração foram salvos. Deseja abrir a pasta de backup?"); // 設定ファイルをバックアップしました。バックアップしたフォルダを開きますか？
    if (idx == 314) return QObject::tr("Deseja fazer backup dos pincéis, cores, gradientes e atalhos?"); // ブラシ、カラー、グラデーション、ショートカット設定をバックアップしますか？
    if (idx == 315) return QObject::tr("As configurações atuais serão descartadas. Deseja continuar?"); // 現在の設定は破棄されますが、宜しいですか？

    if (idx == 320) return QObject::tr("Selecionar vértices/objetos");
    if (idx == 321) return QObject::tr("Selecionar camada");

    if (idx == 330) return QObject::tr("Todos os formatos"); // 全ての形式
    if (idx == 331) return QObject::tr("Projeto FireAlpaca SE"); // FireAlpaca SE プロジェクト

    if (idx == 425) return QObject::tr("Tela"); // キャンバス
    if (idx == 426) return QObject::tr("Camada ativa"); // アクティブレイヤー
    if (idx == 427) return QObject::tr("Camada específica"); // 指定レイヤー

    if (idx == 440) return QObject::tr("Usar a Ferramenta Conta-gotas ao Arrastar"); // ドラッグ時にもスポイト

    if (idx == 450) return QObject::tr("Tem certeza de que deseja excluir a camada?"); // 本当にレイヤーを削除しますか？
    if (idx == 451) return QObject::tr("Você pode configurar nas Preferências para excluir sem confirmação."); // 環境設定から確認なしに削除するよう設定できます

    if (idx == 500) return QObject::tr("A utilização de memória será reduzida para metade, mas a precisão das cores será comprometida. Está de acordo?"); // メモリ消費量が半分になりますが、色精度が低下します。宜しいですか？
    if (idx == 501) return QObject::tr("A precisão das cores será aprimorada e a degradação durante a edição será reduzida, mas o consumo de memória será dobrado. Está de acordo?"); // 色精度が向上し編集時の劣化が軽減しますが、メモリ使用量が倍になります。宜しいですか？
    if (idx == 502) return QObject::tr("Deseja converter as camadas existentes para 8bit/ch?"); // 既存のレイヤーを 8bit/ch に変換しますか？
    if (idx == 503) return QObject::tr("Deseja converter as camadas existentes para 16bit/ch?"); // 既存のレイヤーを 16bit/ch に変換しますか？
    if (idx == 510) return QObject::tr("Esta é uma configuração avançada. Tem certeza de que deseja ativá-la?"); // 上級者向けの設定ですが、本当に有効にしますか？

    if (idx == 600) return QObject::tr("Formato de Arquivo MFG (*.mar)"); // MFG アーカイブ形式(*.mar)
    if (idx == 601) return QObject::tr("Carregar arquivo MAR"); // MARファイルの読み込み
    if (idx == 602) return QObject::tr("O arquivo MAR está corrompido: "); // MARファイルが壊れています：
    if (idx == 603) return QObject::tr("Falha ao copiar o arquivo: "); // ファイルのコピーに失敗しました:
    if (idx == 604) return QObject::tr("%1 filtros importados"); // %1 個のフィルタをインポートしました
    if (idx == 605) return QObject::tr("O processamento está demorando..."); // 処理が長くかかっています…

    //if (idx == 700) return QObject::tr("The process failed. This is likely due to insufficient main memory."); // 処理に失敗しました。メインメモリが不足している可能性が高いです。
  }

  //////////////////////////////////
  //////////////////////////////////
  if (man->TranslateSpanish())
  {
    if (idx == 1) return QObject::tr("Herramienta Pincel");
    if (idx == 2) return QObject::tr("Herramienta Puntos");
    if (idx == 3) return QObject::tr("Herramienta Borrador");
    if (idx == 4) return QObject::tr("Herramienta Mover");
    if (idx == 5) return QObject::tr("Herramienta Bote de pintura");
    if (idx == 6) return QObject::tr("Herramienta Degradado");
    if (idx == 7) return QObject::tr("Herramienta Relleno");
    if (idx == 8) return QObject::tr("Herramienta Seleccionar");
    if (idx == 9) return QObject::tr("Herramienta Varita mágica");
    if (idx == 10) return QObject::tr("Herramienta Seleccionar Pluma");
    if (idx == 11) return QObject::tr("Herramienta Seleccionar Borrador");
    if (idx == 12) return QObject::tr("Herramienta Texto");
    if (idx == 13) return QObject::tr("Herramienta Cuentagotas");
    if (idx == 14) return QObject::tr("Herramienta Mano");
    
    if (idx == 15) return QObject::tr("Desactivar Ajuste");
    if (idx == 16) return QObject::tr("Ajuste paralelo");
    if (idx == 17) return QObject::tr("Ajuste entrecruzado");
    if (idx == 18) return QObject::tr("Ajuste Radial");
    if (idx == 19) return QObject::tr("Configuración de Ajuste");
   
    if (idx == 20) return QObject::tr("Suavizado");
    if (idx == 21) return QObject::tr("Estabilizador");
    if (idx == 22) return QObject::tr("Redondear");
    if (idx == 23) return QObject::tr("Opacidad");
    if (idx == 24) return QObject::tr("Referencia");

    if (idx == 27) return QObject::tr("Expandir");
    if (idx == 28) return QObject::tr("Forma");
    if (idx == 29) return QObject::tr("Tipo");
    if (idx == 30) return QObject::tr("Lineal");
    if (idx == 31) return QObject::tr("Radial");
    if (idx == 32) return QObject::tr("Frontal - Fondo");
    if (idx == 33) return QObject::tr("Frontal");
    if (idx == 34) return QObject::tr("Rectángulo");
    if (idx == 35) return QObject::tr("Elipse");
    if (idx == 36) return QObject::tr("Polígono");
    if (idx == 37) return QObject::tr("Sin ajustes");
    if (idx == 38) return QObject::tr("Lazo");
    if (idx == 39) return QObject::tr("Herramienta de selección Lazo");
    
    if (idx == 40) return QObject::tr("Color");
    if (idx == 41) return QObject::tr("Control del pinceles");
    if (idx == 42) return QObject::tr("Pincel");
    if (idx == 43) return QObject::tr("Navegador");
    if (idx == 44) return QObject::tr("Capas");
    if (idx == 45) return QObject::tr("Vista previa del pincel");
    if (idx == 46) return QObject::tr("Paleta de colores");

    if (idx == 50) return QObject::tr("(Voltear)");
    if (idx == 51) return QObject::tr("El archivo no existe.");
    if (idx == 52)
    {
      QString s = QObject::tr("%1 Ya está abierto. ¿Desea continuar?");
      s.replace( "%1", man->StrParam() );
      return s;
    }
    if (idx == 54) return QObject::tr("Ajuste Punto de fuga");
    if (idx == 55) return QObject::tr("Abrir imagen");
    if (idx == 56) return QObject::tr("Imagen");
    if (idx == 57) return QObject::tr("Ajuste Concéntrico");
    if (idx == 58) return QObject::tr("Ajuste Curva");
    
    if (idx == 60) return QObject::tr("Expandir");
    if (idx == 61) return QObject::tr("Contraer");
    if (idx == 62) return QObject::tr("¿Desea inicializar una posición de ventana?");
    if (idx == 63) return QObject::tr("Deshacer");
    if (idx == 64) return QObject::tr("Rehacer");
    if (idx == 65) return QObject::tr("Ajuste");
    if (idx == 66) return QObject::tr("Para cambiar el idioma, la aplicación se cerrará temporalmente. Después de cerrarla, por favor, vuelva a iniciarla.");
    if (idx == 67) return QObject::tr("Fallo al cargar");
    if (idx == 68) return QObject::tr("Umbral");

    if (idx == 70) return QObject::tr("Fallo para obtener la impresora");
    if (idx == 71) return QObject::tr("Bordes Suaves");
    if (idx == 72) return QObject::tr("Sin configuración");
    if (idx == 73) return QObject::tr("Seleccionar capa en el punto clicado");
    
    if (idx == 75) return QObject::tr("Paralelo");
    if (idx == 76) return QObject::tr("Líneas de concentración");
    if (idx == 77) return QObject::tr("Círculos concéntricos");
    if (idx == 78) return QObject::tr("Curva");
    if (idx == 79) return QObject::tr("Punto de fuga");
    
    if (idx == 80) return QObject::tr("Herramienta de operación");
    if (idx == 81) return QObject::tr("Herramienta de división");
    if (idx == 82) return QObject::tr("Espaciado izquierda-derecha");
    if (idx == 83) return QObject::tr("Espaciado vertical");
    
    if (idx == 85) return QObject::tr("Seleccionar desde el centro");
    if (idx == 86) return QObject::tr("Mantener proporción de aspecto");
    
    if (idx == 87) return QObject::tr("Para implementar el acceso directo, la aplicación se cerrará temporalmente. Después de cerrarla, por favor, vuelva a iniciarla.");
    
    if (idx == 88) return QObject::tr("Otros");
    if (idx == 89) return QObject::tr("Aumentar tamaño del Pincel");
    if (idx == 90) return QObject::tr("Disminuir tamaño del Pincel");
    if (idx == 91) return QObject::tr("Archivo MDP no válido.");
    if (idx == 92) return QObject::tr("No se puede abrir %1 porque no es un archivo MDP en el formato correcto.");
    if (idx == 93) return QObject::tr("El archivo ya existe.");
    if (idx == 94) return QObject::tr("¿Desea abrir el servicio de conversión de GIF animados \"Alpaca Video\" en su navegador?");
    if (idx == 95) return QObject::tr("¿Desea que el fondo sea transparente?");
    if (idx == 96) return QObject::tr("¿Crear una carpeta?");
    if (idx == 97) return QObject::tr("Ajuste de cuadrícula");

    if (idx == 110) return QObject::tr("A mano alzada");
    if (idx == 111) return QObject::tr("Línea recta");
    if (idx == 112) return QObject::tr("Líneas conectadas");
    if (idx == 113) return QObject::tr("Polígono");
    if (idx == 114) return QObject::tr("Rectángulo");
    if (idx == 115) return QObject::tr("Óvalo");
    if (idx == 116) return QObject::tr("Curva");
    if (idx == 117) return QObject::tr("Forma");
    if (idx == 118) return QObject::tr("Cuadrado");
    if (idx == 119) return QObject::tr("Círculo");

    if (idx == 120) return QObject::tr("Ajuste de Perspectiva 3D");
    if (idx == 121) return QObject::tr("No es posible abrir otro archivo durante el proceso de filtrado.");
    if (idx == 122) return QObject::tr("Deseleccionar");
    if (idx == 123) return QObject::tr("Ajustar a la cuadrícula");
    if (idx == 124) return QObject::tr("Ignorar color de capa");
    if (idx == 125) return QObject::tr("Ignorar capa borrador");
    if (idx == 126) return QObject::tr("Promedio");
    if (idx == 127) return QObject::tr("Ninguno");
    if (idx == 128) return QObject::tr("3 * 3 px promedio");
    if (idx == 129) return QObject::tr("5 * 5 px promedio");

    if (idx == 130) return QObject::tr("Autoguardado en curso...");
    if (idx == 131) return QObject::tr("Autoguardado completado");
    if (idx == 132) return QObject::tr("Autoguardado desactivado (memoria insuficiente)\n\nEstabilidad de la aplicación muy baja");
    if (idx == 133) return QObject::tr("¿Quieres recuperar las imágenes guardadas automáticamente?");
    if (idx == 135) return QObject::tr("Fallo al publicar en pixiv Sketch");
    if (idx == 136) return QObject::tr("Difundir");
    if (idx == 137) return QObject::tr("Estándar");
    if (idx == 138) return QObject::tr("Repetir");
    if (idx == 139) return QObject::tr("Doblar");

    if (idx == 140) return QObject::tr("Esquinas redondeadas");
    //if (idx == 141) return QObject::tr("Gaussian Blur"); // ガウスぼかし

    if (idx == 150) return QObject::tr("Redondear las esquinas y mantener el grosor");

    if (idx == 160) return QObject::tr("Llenar el espacio");
    if (idx == 162) return QObject::tr("Tolerancia");
    if (idx == 163) return QObject::tr("Transparente a color de primer plano");
    if (idx == 164) return QObject::tr("Personalizar");
    if (idx == 165) return QObject::tr("Borrado cubeta");

    if (idx == 170) return QObject::tr("Determinar posición en cada arrastre");
    if (idx == 171) return QObject::tr("Determinar la posición");
    if (idx == 172) return QObject::tr("Presión cero en ambos extremos");
    if (idx == 173) return QObject::tr("Mover 1px");
    if (idx == 174) return QObject::tr("Mover capa 1px a la izquierda");
    if (idx == 175) return QObject::tr("Mover capa 1px hacia arriba");
    if (idx == 176) return QObject::tr("Mover capa 1px hacia abajo 1px");
    if (idx == 177) return QObject::tr("Mover capa 1px a la derecha");

    if (idx == 180) return QObject::tr("Importar archivo de paleta");
    if (idx == 181) return QObject::tr("Guardar archivo de paleta");
    if (idx == 182) return QObject::tr("Paleta FireAlpaca (*.fap);;Paleta FireAlpaca (*.fap)");
    if (idx == 183) return QObject::tr("Importar archivos ACO");
    if (idx == 184) return QObject::tr("Exportar archivos ACO");
    if (idx == 185) return QObject::tr("Paleta de colores ACO (*.aco);;Paleta de colores ACO (*.aco)");

    if (idx == 190) return QObject::tr("Transformar");
    if (idx == 191) return QObject::tr("Transformación libre");
    if (idx == 192) return QObject::tr("Transformación de malla");

    if (idx == 200) return QObject::tr("Simetría bilateral");
    if (idx == 201) return QObject::tr("Simetría vertical");
    if (idx == 202) return QObject::tr("Simetría rotacional");
    if (idx == 203) return QObject::tr("(imagen reflejada)");
    if (idx == 204) return QObject::tr("Mosaico");
    if (idx == 205) return QObject::tr("Simetría");
    if (idx == 206) return QObject::tr("(Media gota)");
    if (idx == 207) return QObject::tr("Especificar el centro"); // 中心を指定
    if (idx == 208) return QObject::tr("Simetría axial"); // 線対称

    if (idx == 210) return QObject::tr("Herramienta de lazo");
    if (idx == 211) return QObject::tr("Zona cerrada");
    if (idx == 212) return QObject::tr("Zona transparente/Blanco");
    if (idx == 213) return QObject::tr("Zona transparente");
    if (idx == 214) return QObject::tr("Blanco");
    if (idx == 215) return QObject::tr("Todos");
    if (idx == 219) return QObject::tr("No rellenar zonas estrechas");

    if (idx == 221) return QObject::tr("Nuevo");
    if (idx == 222) return QObject::tr("Añadir");
    if (idx == 223) return QObject::tr("Eliminar");
    if (idx == 224) return QObject::tr("Zona común");

    if (idx == 230) return QObject::tr("Fuerza");
    if (idx == 231) return QObject::tr("Fijar en cada trazo");
    if (idx == 232) return QObject::tr("Fijar");

    if (idx == 240) return QObject::tr("Expansión");
    if (idx == 241) return QObject::tr("Contracción");
    if (idx == 242) return QObject::tr("Empujar");
    if (idx == 243) return QObject::tr("Rotar hacia la derecha");
    if (idx == 244) return QObject::tr("Rotar hacia la izquierda");
    if (idx == 249) return QObject::tr("Herramienta Licuar");

    if (idx == 260) return QObject::tr("La recuperación anterior ha fallado. El proceso de restauración se ha omitido."); // 前回の復旧に失敗したため、復旧処理をスキップしました

    //if (idx == 270) return QObject::tr("Select the layer within the frame (Animation Mode)"); // フレーム内のレイヤーを選択 (アニメーションモード)

    if (idx == 300) return QObject::tr("Obtener color transparente");

    if (idx == 310) return QObject::tr("¿Desea utilizar a partir de ahora el archivo de configuración utilizado en FireAlpaca (versión gratuita)? \n\n(la aplicación se cerrará ya que es necesario reiniciarla)");   
    if (idx == 311) return QObject::tr("¿Desea volver a utilizar el archivo de configuración estándar de FireAlpaca SE? \n\n(la aplicación se cerrará y será necesario reiniciarla)");
    if (idx == 312) return QObject::tr("¿Quieres conservar la configuración de la versión anterior?"); // 旧バージョンの設定を引き継ぎますか？
    //if (idx == 313) return QObject::tr("Your configuration files have been backed up. Do you want to open the backup folder?"); // 設定ファイルをバックアップしました。バックアップしたフォルダを開きますか？
    //if (idx == 314) return QObject::tr("Do you want to back up your brushes, colors, gradients and shortcut settings?"); // ブラシ、カラー、グラデーション、ショートカット設定をバックアップしますか？
    //if (idx == 315) return QObject::tr("Your current settings will be discarded. Is this OK?"); // 現在の設定は破棄されますが、宜しいですか？

    if (idx == 425) return QObject::tr("Lienzo"); // キャンバス
    if (idx == 426) return QObject::tr("Capa activa"); // アクティブレイヤー
    if (idx == 427) return QObject::tr("Capa específica"); // 指定レイヤー

    //if (idx == 440) return QObject::tr("Eyedropper when Dragging"); // ドラッグ時にもスポイト

    //if (idx == 450) return QObject::tr("Are you sure you want to delete this layer?"); // 本当にレイヤーを削除しますか？
    //if (idx == 451) return QObject::tr("You can set it to be deleted without confirmation in the preferences."); // 環境設定から確認なしに削除するよう設定できます

    if (idx == 500) return QObject::tr("La cantidad de memoria consumida se reducirá a la mitad, pero la precisión del color disminuirá. ¿Está de acuerdo?"); // メモリ消費量が半分になりますが、色精度が低下します。宜しいですか？
    if (idx == 501) return QObject::tr("La precisión del color mejorará y la degradación durante la edición se reducirá, pero el uso de memoria se duplicará. ¿Está de acuerdo?"); // 色精度が向上し編集時の劣化が軽減しますが、メモリ使用量が倍になります。宜しいですか？
    //if (idx == 502) return QObject::tr("Do you want to convert existing layers to 8bit/ch?"); // 既存のレイヤーを 8bit/ch に変換しますか？
    //if (idx == 503) return QObject::tr("Do you want to convert existing layers to 16bit/ch?"); // 既存のレイヤーを 16bit/ch に変換しますか？
    //if (idx == 510) return QObject::tr("This is an advanced setting, do you really want to enable it?"); // 上級者向けの設定ですが、本当に有効にしますか？

    //if (idx == 600) return QObject::tr("MFG Archive Format (*.mar)"); // MFG アーカイブ形式(*.mar)
    //if (idx == 601) return QObject::tr("Import MAR Files"); // MARファイルの読み込み
    //if (idx == 602) return QObject::tr("Invalid MAR: "); // MARファイルが壊れています：
    //if (idx == 603) return QObject::tr("Failed to copy: "); // ファイルのコピーに失敗しました:
    //if (idx == 604) return QObject::tr("Import %1 filters"); // %1 個のフィルタをインポートしました
    //if (idx == 605) return QObject::tr("The process is taking longer than usual..."); // 処理が長くかかっています…

    //if (idx == 700) return QObject::tr("The process failed. This is likely due to insufficient main memory."); // 処理に失敗しました。メインメモリが不足している可能性が高いです。
  }

  //////////////////////////////////
  //////////////////////////////////
  if (man->TranslateGerman())
  {
    if (idx == 1) return QObject::tr("Pinsel-Tool");
    if (idx == 2) return QObject::tr("Pünktchen-Tool");
    if (idx == 3) return QObject::tr("Radierer-Tool");
    if (idx == 4) return QObject::tr("Verschiebe-Tool");
    if (idx == 5) return QObject::tr("Eimer-Tool");
    if (idx == 6) return QObject::tr("Verlauf-Tool");
    if (idx == 7) return QObject::tr("Füll-Tool");
    if (idx == 8) return QObject::tr("Auswahl-Tool");
    if (idx == 9) return QObject::tr("Zauberstab-Tool");
    if (idx == 10) return QObject::tr("Stiftwahl-Tool");
    if (idx == 11) return QObject::tr("Radiererwahl-Tool");
    if (idx == 12) return QObject::tr("Text-Tool");
    if (idx == 13) return QObject::tr("Pipette-Tool");
    if (idx == 14) return QObject::tr("Hand-Tool");

    if (idx == 15) return QObject::tr("Schnappfunktion abbrechen");
    if (idx == 16) return QObject::tr("Paralelle Schnappfunktion");
    if (idx == 17) return QObject::tr("Kreuz-und-Quer-Schnappfunktion");
    if (idx == 18) return QObject::tr("Radikale Schnappfunktion");
    if (idx == 19) return QObject::tr("Einstellung der Schnappfunktion");

    if (idx == 20) return QObject::tr("Anti-Aliasing");
    if (idx == 21) return QObject::tr("Korrektur");
    if (idx == 22) return QObject::tr("Runde Ecke");
    if (idx == 23) return QObject::tr("Deckkraft");
    if (idx == 24) return QObject::tr("Referenz");

    if (idx == 27) return QObject::tr("Erweitern");
    if (idx == 28) return QObject::tr("Form");
    if (idx == 29) return QObject::tr("Typ");
    if (idx == 30) return QObject::tr("Linienförmig");
    if (idx == 31) return QObject::tr("Kreisförmig");
    if (idx == 32) return QObject::tr("Vordergrund - Hintergrund");
    if (idx == 33) return QObject::tr("Vordergrund");
    if (idx == 34) return QObject::tr("Rechteck");
    if (idx == 35) return QObject::tr("Ellipse");
    if (idx == 36) return QObject::tr("Polygon");
    if (idx == 37) return QObject::tr("Keine Option");
    if (idx == 38) return QObject::tr("Lasso");

    if (idx == 40) return QObject::tr("Farbe");
    if (idx == 41) return QObject::tr("Pinselkontrolle");
    if (idx == 42) return QObject::tr("Pinsel");
    if (idx == 43) return QObject::tr("Navigator");
    if (idx == 44) return QObject::tr("Ebene");
    if (idx == 45) return QObject::tr("Pinselprüfung ");
    if (idx == 46) return QObject::tr("Palette");

    if (idx == 50) return QObject::tr("(Umdrehen)");
    if (idx == 51) return QObject::tr("Diese Datei existiert nicht.");
    if (idx == 52)
    {
      QString s = QObject::tr("%1 ist bereits geöffnet.  Weitermachen?");
      s.replace( "%1", man->StrParam() );
      return s;
    }
    if (idx == 54) return QObject::tr("Fluchtpunkt-Schnappfunktion");
    if (idx == 55) return QObject::tr("Ein Bild öffnen");
    if (idx == 56) return QObject::tr("Ein Bild");

    if (idx == 60) return QObject::tr("Erweitern");
    if (idx == 61) return QObject::tr("Kontakt");
    if (idx == 62) return QObject::tr("Eine Fensterposition initialisieren?");
    if (idx == 63) return QObject::tr("Rückgängig machen");
    if (idx == 64) return QObject::tr("Wiederholen");
    if (idx == 65) return QObject::tr("Schnappfunktion");
    if (idx == 66) return QObject::tr("Bei der Aktualisierung der Spracheinstellung wird das Programm beendet. Bitte starten Sie das Programm nach der Beendung wieder.");
    if (idx == 67) return QObject::tr("Konnte nicht geöffnet werden");
    if (idx == 68) return QObject::tr("Schwellenwert");

    if (idx == 70) return QObject::tr("Druckererfassung fehlgeschlagen");
    if (idx == 71) return QObject::tr("Abgerundete Ecken");
    if (idx == 72) return QObject::tr("Ohne Titel");
    if (idx == 73) return QObject::tr("Wählen Sie eine Ebene an dem angeklickten Punkt.");

    if (idx == 75) return QObject::tr("Parallel");
    if (idx == 76) return QObject::tr("Radial");
    if (idx == 77) return QObject::tr("Kreis");
    if (idx == 78) return QObject::tr("Kurve");
    if (idx == 79) return QObject::tr("Fluchtpunkt");

    if (idx == 80) return QObject::tr("Bearbeitungswerkzeug");
    if (idx == 81) return QObject::tr("Teilungswerkzeug");
    if (idx == 82) return QObject::tr("Unterschneiden");
    if (idx == 83) return QObject::tr("Führen");

    if (idx == 85) return QObject::tr("Aus der Mitte wählen");
    if (idx == 86) return QObject::tr("Proportionen beschränken");

    if (idx == 87) return QObject::tr("Um die Verknüpfungseinstellungen zu aktualisieren, wird das Programm geschlossen. Bitte starten Sie das Programm nach dem Schließen neu.");

    if (idx == 88) return QObject::tr("Andere");
    if (idx == 89) return QObject::tr("Pinselgröße erhöhen");
    if (idx == 90) return QObject::tr("Pinselgröße verkleinern");
    if (idx == 91) return QObject::tr("Ungültige MDP-Datei");
    if (idx == 92) return QObject::tr("%1 konnte nicht geöffnet werden, da es sich nicht um eine gültige MDP-Datei handelt.");
    if (idx == 93) return QObject::tr("Datei existiert bereits");
    if (idx == 94) return QObject::tr("Öffnen Sie „AlpacaDouga“, den Dienst zum Konvertieren von animierten GIFs, in einem Browser.");
    if (idx == 95) return QObject::tr("Den Hintergrund transparent machen?");
    if (idx == 96) return QObject::tr("Einen Ordner erstellen?");
    if (idx == 97) return QObject::tr("Raster-Fang");
    if (idx == 98) return QObject::tr("Hilfslinien-Snap");
    if (idx == 99) return QObject::tr("Panelaufteilung");

    if (idx == 110) return QObject::tr("Freihand");
    if (idx == 111) return QObject::tr("Linie");
    if (idx == 112) return QObject::tr("Polylinie");
    if (idx == 113) return QObject::tr("Vieleck");
    if (idx == 114) return QObject::tr("Rechteck");
    if (idx == 115) return QObject::tr("Ellipse");
    if (idx == 116) return QObject::tr("Kurve");
    if (idx == 117) return QObject::tr("Form");
    if (idx == 118) return QObject::tr("Quadrat");
    if (idx == 119) return QObject::tr("Kreis");

    if (idx == 120) return QObject::tr("3D-Perspektive fangen");
    if (idx == 121) return QObject::tr("Während der Filterverarbeitung kann keine andere Datei geöffnet werden.");
    if (idx == 122) return QObject::tr("Auswahl aufheben");
    if (idx == 123) return QObject::tr("Am Gitter fangen");
    if (idx == 124) return QObject::tr("Ebenenfarbe ignorieren");
    if (idx == 125) return QObject::tr("Entwurfsebene ignorieren");

    if (idx == 126) return QObject::tr("Mittelwertbildung");
    if (idx == 127) return QObject::tr("Keine");
    if (idx == 128) return QObject::tr("3 * 3 px Durchschnitt");
    if (idx == 129) return QObject::tr("5 * 5 px Durchschnitt");

    if (idx == 130) return QObject::tr("Automatisch speichern...");
    if (idx == 131) return QObject::tr("Automatisch speichern Geschehen");
    if (idx == 132) return QObject::tr("Automatisches Speichern deaktiviert (unzureichender Speicher) Die Stabilität der Anwendung wird erheblich beeinträchtigt.");
    if (idx == 133) return QObject::tr("Automatisch gespeichertes Bild wiederherstellen?");

    if (idx == 135) return QObject::tr("Veröffentlichung auf pixiv Sketch fehlgeschlagen");

    if (idx == 136) return QObject::tr("Verbreiten");
    if (idx == 137) return QObject::tr("Standard");
    if (idx == 138) return QObject::tr("Standard");
    if (idx == 139) return QObject::tr("Spiegeln");

    if (idx == 140) return QObject::tr("Eine Ecke abrunden");
    //if (idx == 141) return QObject::tr("Gaussian Blur"); // ガウスぼかし

    if (idx == 150) return QObject::tr("Eine Ecke abrunden, Linienbreite beibehalten");

    if (idx == 160) return QObject::tr("Schließen der Lücke");
    if (idx == 162) return QObject::tr("Toleranz");
    if (idx == 163) return QObject::tr("Transparent-Vordergrund");
    if (idx == 164) return QObject::tr("Benutzerdefiniert");
    if (idx == 165) return QObject::tr("Eimer löschen");

    if (idx == 170) return QObject::tr("Bestimmen bei jedem Ziehen");
    if (idx == 171) return QObject::tr("Bestimmen Sie die Position");
    if (idx == 172) return QObject::tr("Null Druck an beiden Enden");
    if (idx == 173) return QObject::tr("1px verschieben");
    if (idx == 174) return QObject::tr("Ebene um 1px nach links verschieben");
    if (idx == 175) return QObject::tr("Ebene um 1px nach oben verschieben");
    if (idx == 176) return QObject::tr("Ebene um 1px nach unten verschieben");
    if (idx == 177) return QObject::tr("Ebene um 1px nach rechts verschieben");

    if (idx == 180) return QObject::tr("Palettendatei importieren");
    if (idx == 181) return QObject::tr("Palettendatei exportieren");
    if (idx == 182) return QObject::tr("FireAlpaca Palette (*.fap);;FireAlpaca Palette (*.fap)");
    if (idx == 183) return QObject::tr("ACO-Format importieren");
    if (idx == 184) return QObject::tr("Als ACO-Format exportieren");
    if (idx == 185) return QObject::tr("ACO Farbpalette (*.aco);;ACO Farbpalette (*.aco)");

    if (idx == 190) return QObject::tr("Transformieren");
    if (idx == 191) return QObject::tr("Frei transformieren");
    if (idx == 192) return QObject::tr("Mesh-Transformation");

    if (idx == 200) return QObject::tr("Bilateral"); // 左右対称
    if (idx == 201) return QObject::tr("Vertikal"); // 上下対称
    if (idx == 202) return QObject::tr("Drehen"); // 回転対称
    if (idx == 203) return QObject::tr(" (Spiegeln)"); //  (鏡像)
    if (idx == 204) return QObject::tr("Kacheln"); // タイル
    if (idx == 205) return QObject::tr("Symmetrie"); // 対称
    if (idx == 206) return QObject::tr("(Halber Tropfen)"); //  (ハーフドロップ)
    if (idx == 207) return QObject::tr("Mittelpunkt festlegen"); // 中心を指定
    if (idx == 208) return QObject::tr("Achsen-symmetrie"); // 線対称

    if (idx == 210) return QObject::tr("Lasso-Füllung");
    if (idx == 211) return QObject::tr("Geschlossene Fläche");
    if (idx == 212) return QObject::tr("Transparent/Weiß");
    if (idx == 213) return QObject::tr("Transparent");
    if (idx == 214) return QObject::tr("Weiß");
    if (idx == 215) return QObject::tr("Alle");
    if (idx == 219) return QObject::tr("Schmale Bereiche nicht ausfüllen");

    if (idx == 221) return QObject::tr("Neu");
    if (idx == 222) return QObject::tr("hinzufügen");
    if (idx == 223) return QObject::tr("Löschen");
    if (idx == 224) return QObject::tr("Gemeinsam");

    if (idx == 230) return QObject::tr("Streng");
    if (idx == 231) return QObject::tr("Fix pro Hub");
    if (idx == 232) return QObject::tr("Fix");

    if (idx == 240) return QObject::tr("Ausdehnung");
    if (idx == 241) return QObject::tr("Kontraktion");
    if (idx == 242) return QObject::tr("Drücken");
    if (idx == 243) return QObject::tr("Drehen Rechts");
    if (idx == 244) return QObject::tr("Drehen links");
    if (idx == 249) return QObject::tr("Werkzeug verflüssigen");

    if (idx == 260) return QObject::tr("Die vorherige Wiederherstellung ist fehlgeschlagen. Der Wiederherstellungsprozess wurde übersprungen."); // 前回の復旧に失敗したため、復旧処理をスキップしました

    //if (idx == 270) return QObject::tr("Select the layer within the frame (Animation Mode)"); // フレーム内のレイヤーを選択 (アニメーションモード)

    if (idx == 300) return QObject::tr("Transparente Farbe erhalten");
    if (idx == 301) return QObject::tr("Superellipse");

    if (idx == 310) return QObject::tr("Möchten Sie die in FireAlpaca (kostenlose Version) verwendete Konfigurationsdatei von nun an verwenden? \n\n(Schließen Sie die Anwendung, da ein Neustart erforderlich ist)");
    if (idx == 311) return QObject::tr("Möchten Sie zur Verwendung der Standardkonfigurationsdatei von FireAlpaca SE zurückkehren? \n\n(Schließen Sie die Anwendung, da ein Neustart erforderlich ist");
    if (idx == 312) return QObject::tr("Möchten Sie die Einstellungen der vorherigen Version behalten?"); // 旧バージョンの設定を引き継ぎますか？
    //if (idx == 313) return QObject::tr("Your configuration files have been backed up. Do you want to open the backup folder?"); // 設定ファイルをバックアップしました。バックアップしたフォルダを開きますか？
    //if (idx == 314) return QObject::tr("Do you want to back up your brushes, colors, gradients and shortcut settings?"); // ブラシ、カラー、グラデーション、ショートカット設定をバックアップしますか？
    //if (idx == 315) return QObject::tr("Your current settings will be discarded. Is this OK?"); // 現在の設定は破棄されますが、宜しいですか？

    if (idx == 320) return QObject::tr("Ecken/Objekte auswählen");
    if (idx == 321) return QObject::tr("Ebenen auswählen");

    //if (idx == 330) return QObject::tr("All Formats"); // 全ての形式
    //if (idx == 331) return QObject::tr("FireAlpaca SE Project"); // FireAlpaca SE プロジェクト

    if (idx == 425) return QObject::tr("Leinwand");
    if (idx == 426) return QObject::tr("Aktive Ebene");
    if (idx == 427) return QObject::tr("Spezifizierte Ebene");

    //if (idx == 440) return QObject::tr("Eyedropper when Dragging"); // ドラッグ時にもスポイト

    //if (idx == 450) return QObject::tr("Are you sure you want to delete this layer?"); // 本当にレイヤーを削除しますか？
    //if (idx == 451) return QObject::tr("You can set it to be deleted without confirmation in the preferences."); // 環境設定から確認なしに削除するよう設定できます

    if (idx == 500) return QObject::tr("Der Speicherverbrauch wird halbiert, aber die Farbgenauigkeit wird verringert. Ist das in Ordnung?"); // メモリ消費量が半分になりますが、色精度が低下します。宜しいですか？
    if (idx == 501) return QObject::tr("Die Farbgenauigkeit wird verbessert und die Bearbeitungsverluste werden verringert, aber der Speicherverbrauch verdoppelt sich. Ist das in Ordnung?"); // 色精度が向上し編集時の劣化が軽減しますが、メモリ使用量が倍になります。宜しいですか？
    //if (idx == 502) return QObject::tr("Do you want to convert existing layers to 8bit/ch?"); // 既存のレイヤーを 8bit/ch に変換しますか？
    //if (idx == 503) return QObject::tr("Do you want to convert existing layers to 16bit/ch?"); // 既存のレイヤーを 16bit/ch に変換しますか？
    //if (idx == 510) return QObject::tr("This is an advanced setting, do you really want to enable it?"); // 上級者向けの設定ですが、本当に有効にしますか？

    //if (idx == 600) return QObject::tr("MFG Archive Format (*.mar)"); // MFG アーカイブ形式(*.mar)
    //if (idx == 601) return QObject::tr("Import MAR Files"); // MARファイルの読み込み
    //if (idx == 602) return QObject::tr("Invalid MAR: "); // MARファイルが壊れています：
    //if (idx == 603) return QObject::tr("Failed to copy: "); // ファイルのコピーに失敗しました:
    //if (idx == 604) return QObject::tr("Import %1 filters"); // %1 個のフィルタをインポートしました
    //if (idx == 605) return QObject::tr("The process is taking longer than usual..."); // 処理が長くかかっています…

    //if (idx == 700) return QObject::tr("The process failed. This is likely due to insufficient main memory."); // 処理に失敗しました。メインメモリが不足している可能性が高いです。
  }

  //////////////////////////////////
  //////////////////////////////////
  if (man->TranslateFrench())
  {
    if (idx == 1) return QObject::tr("Outil Pinceau");
    if (idx == 2) return QObject::tr("Outil Point");
    if (idx == 3) return QObject::tr("Outil Gomme");
    if (idx == 4) return QObject::tr("Outil Déplacer");
    if (idx == 5) return QObject::tr("Outil Pot de peinture");
    if (idx == 6) return QObject::tr("Outil Dégradé");
    if (idx == 7) return QObject::tr("Outil Remplissage");
    if (idx == 8) return QObject::tr("Outil Sélection");
    if (idx == 9) return QObject::tr("Outil Baguette magique");
    if (idx == 10) return QObject::tr("Outil Sélection du crayon");
    if (idx == 11) return QObject::tr("Outil Sélection de la gomme");
    if (idx == 12) return QObject::tr("Outil Texte");
    if (idx == 13) return QObject::tr("Outil Pipette");
    if (idx == 14) return QObject::tr("Outil Main");

    if (idx == 15) return QObject::tr("Désaligner");
    if (idx == 16) return QObject::tr("Alignement parallèle");
    if (idx == 17) return QObject::tr("Alignement axial");
    if (idx == 18) return QObject::tr("Alignement radial");
    if (idx == 19) return QObject::tr("Paramètre d'alignement");

    if (idx == 20) return QObject::tr("Anticrénelage");
    if (idx == 21) return QObject::tr("Correction");
    if (idx == 22) return QObject::tr("Angle arrondi");
    if (idx == 23) return QObject::tr("Opacité");
    if (idx == 24) return QObject::tr("Référence");

    if (idx == 27) return QObject::tr("Dilater");
    if (idx == 28) return QObject::tr("Forme");
    if (idx == 29) return QObject::tr("Type");
    if (idx == 30) return QObject::tr("Linéaire");
    if (idx == 31) return QObject::tr("Circulaire");
    if (idx == 32) return QObject::tr("Premier plan-Arrière-plan");
    if (idx == 33) return QObject::tr("Premier plan");
    if (idx == 34) return QObject::tr("Rectangle");
    if (idx == 35) return QObject::tr("Ellipse");
    if (idx == 36) return QObject::tr("Polygone");
    if (idx == 37) return QObject::tr("Aucune option");
    if (idx == 38) return QObject::tr("Lasso");

    if (idx == 40) return QObject::tr("Couleur");
    if (idx == 41) return QObject::tr("Option du pinceau");
    if (idx == 42) return QObject::tr("Pinceau");
    if (idx == 43) return QObject::tr("Navigateur");
    if (idx == 44) return QObject::tr("Masque");
    if (idx == 45) return QObject::tr("Aperçu du pinceau");
    if (idx == 46) return QObject::tr("Palette");

    if (idx == 50) return QObject::tr("(Faire basculer)");
    if (idx == 51) return QObject::tr("Le fichier n'existe pas.");
    if (idx == 52)
    {
      QString s = QObject::tr("%1 est déjà ouvert. Voulez-vous continuer ?");
      s.replace( "%1", man->StrParam() );
      return s;
    }
    if (idx == 54) return QObject::tr("Alignement du point de fuite");
    if (idx == 55) return QObject::tr("Ouvrir une image");
    if (idx == 56) return QObject::tr("Image");

    if (idx == 60) return QObject::tr("Dilater");
    if (idx == 61) return QObject::tr("Contracter");
    if (idx == 62) return QObject::tr("Voulez-vous initialiser la position d'une fenêtre ?");
    if (idx == 63) return QObject::tr("Annuler");
    if (idx == 64) return QObject::tr("Rétablir");
    if (idx == 65) return QObject::tr("Aligner");
    if (idx == 66) return QObject::tr("Le programme va se fermer afin de mettre à jour les paramètres linguistiques. Veuillez redémarrer le programme après sa fermeture.");
    if (idx == 67) return QObject::tr("Impossible d'ouvrir");
    if (idx == 68) return QObject::tr("Seuil");

    if (idx == 70) return QObject::tr("Échec de l'obtention d'une imprimante");
    if (idx == 71) return QObject::tr("Soft Edge");
    if (idx == 72) return QObject::tr("Sans-titre");
    if (idx == 73) return QObject::tr("Sélectionner un calque au point désigné");

    if (idx == 75) return QObject::tr("Parallèle");
    if (idx == 76) return QObject::tr("Radial");
    if (idx == 77) return QObject::tr("Cercle");
    if (idx == 78) return QObject::tr("Courbe");
    if (idx == 79) return QObject::tr("Point de fuite");

    if (idx == 80) return QObject::tr("Outil opération");
    if (idx == 81) return QObject::tr("Outil division");
    if (idx == 82) return QObject::tr("Crénage");
    if (idx == 83) return QObject::tr("Interlignage");
    if (idx == 84) return QObject::tr("Ajouter du matériel Koma");

    if (idx == 85) return QObject::tr("Choisir à partir du centre");
    if (idx == 86) return QObject::tr("Fixer les proportions");

    if (idx == 87) return QObject::tr("Pour mettre à jour les paramètres du raccourci, le programme sera fermé. Veuillez redémarrer le programme après la fermeture.");

    if (idx == 88) return QObject::tr("Autres");
    if (idx == 89) return QObject::tr("Augmenter la taille du pinceau");
    if (idx == 90) return QObject::tr("Réduire la taille du pinceau");
    if (idx == 91) return QObject::tr("Fichier MDP invalide");
    if (idx == 92) return QObject::tr("%1 n'a pas pu être ouvert car ce n'est pas un fichier MDP valide.");
    if (idx == 93) return QObject::tr("Le fichier existe déjà");
    if (idx == 94) return QObject::tr("Ouvrir \"AlpacaDouga\", le service de conversion GIF animé, dans un navigateur ?");
    if (idx == 95) return QObject::tr("Rendre le fond transparent ?");
    if (idx == 96) return QObject::tr("Créer un dossier ?");
    if (idx == 97) return QObject::tr("Accrochage à la grille");
    if (idx == 98) return QObject::tr("Accrochage au guide");
    if (idx == 99) return QObject::tr("Division de Koma");

    if (idx == 110) return QObject::tr("À main levée");
    if (idx == 111) return QObject::tr("Ligne");
    if (idx == 112) return QObject::tr("Polyligne");
    if (idx == 113) return QObject::tr("Polygone");
    if (idx == 114) return QObject::tr("Rectangle");
    if (idx == 115) return QObject::tr("Ellipse");
    if (idx == 116) return QObject::tr("Courbe");
    if (idx == 117) return QObject::tr("Forme");
    if (idx == 118) return QObject::tr("Carré");
    if (idx == 119) return QObject::tr("Cercle");

    if (idx == 120) return QObject::tr("Accrochage à la perspective 3D");
    if (idx == 121) return QObject::tr("Il n'est pas possible d'ouvrir un autre fichier pendant le traitement du filtre.");
    if (idx == 122) return QObject::tr("Désélectionner");
    if (idx == 123) return QObject::tr("Accrocher à la grille");
    if (idx == 124) return QObject::tr("Ignorer la couleur du calque");
    if (idx == 125) return QObject::tr("Ignorer le calque brouillon");

    if (idx == 126) return QObject::tr("Moyenne");
    if (idx == 127) return QObject::tr("Aucun");
    if (idx == 128) return QObject::tr("Moyenne 3 * 3 px");
    if (idx == 129) return QObject::tr("Moyenne 5 * 5 px");

    if (idx == 130) return QObject::tr("Sauvegarde automatique...");
    if (idx == 131) return QObject::tr("Sauvegarde automatique terminée");
    if (idx == 132) return QObject::tr("Sauvegarde automatique désactivée (mémoire insuffisante)");
    if (idx == 133) return QObject::tr("Récupérer l'image sauvegardée automatiquement ?");

    if (idx == 135) return QObject::tr("Échec de la publication sur pixiv Sketch");

    if (idx == 136) return QObject::tr("Répartition");
    if (idx == 137) return QObject::tr("Standard");
    if (idx == 138) return QObject::tr("Répéter");
    if (idx == 139) return QObject::tr("Réfléchir");

    if (idx == 140) return QObject::tr("Arrondir un coin");
    //if (idx == 141) return QObject::tr("Gaussian Blur"); // ガウスぼかし

    if (idx == 150) return QObject::tr("Arrondir un coin, garder la largeur de ligne");

    if (idx == 160) return QObject::tr("Fermer le fossé");
    if (idx == 162) return QObject::tr("Tolérance");
    if (idx == 163) return QObject::tr("Transparent-Avant-plan");
    if (idx == 164) return QObject::tr("Personnalisé");
    if (idx == 165) return QObject::tr("Effacer avec le seau");

    if (idx == 170) return QObject::tr("Déterminer par chaque glissement");
    if (idx == 171) return QObject::tr("Déterminer la position");
    if (idx == 172) return QObject::tr("Zéro pression aux deux extrémités");
    if (idx == 173) return QObject::tr("Déplacer de 1px");
    if (idx == 174) return QObject::tr("Déplacer le calque de 1px vers la gauche");
    if (idx == 175) return QObject::tr("Déplacer le calque de 1px vers le haut");
    if (idx == 176) return QObject::tr("Déplacer le calque de 1px vers le bas");
    if (idx == 177) return QObject::tr("Déplacer le calque de 1px vers la droite");

    if (idx == 180) return QObject::tr("Importer un fichier de palette");
    if (idx == 181) return QObject::tr("Exporter un fichier de palette");
    if (idx == 182) return QObject::tr("FireAlpaca Palette (*.fap);;FireAlpaca Palette (*.fap)");
    if (idx == 183) return QObject::tr("Importer le format ACO");
    if (idx == 184) return QObject::tr("Exporter au format ACO");
    if (idx == 185) return QObject::tr("Palette de couleurs ACO (*.aco);;Palette de couleurs ACO (*.aco)");

    if (idx == 190) return QObject::tr("Transformer");
    if (idx == 191) return QObject::tr("Transformation libre");
    if (idx == 192) return QObject::tr("Transformation du maillage");

    if (idx == 200) return QObject::tr("Bilatérale"); // 左右対称
    if (idx == 201) return QObject::tr("Vertical"); // 上下対称
    if (idx == 202) return QObject::tr("Rotation"); // 回転対称
    if (idx == 203) return QObject::tr(" (Miroir)"); //  (鏡像)
    if (idx == 204) return QObject::tr("Carreau"); // タイル
    if (idx == 205) return QObject::tr("Symétrie"); // 対称
    if (idx == 206) return QObject::tr(" (Demi-décalage)"); //  (ハーフドロップ)
    if (idx == 207) return QObject::tr("Spécifier le centre"); // 中心を指定
    if (idx == 208) return QObject::tr("Ligne"); // 線対称

    if (idx == 210) return QObject::tr("Remplissage au lasso");
    if (idx == 211) return QObject::tr("Zone fermée");
    if (idx == 212) return QObject::tr("Transparent/Blanc");
    if (idx == 213) return QObject::tr("Transparent");
    if (idx == 214) return QObject::tr("Blanc");
    if (idx == 215) return QObject::tr("Tout");
    if (idx == 219) return QObject::tr("Ne pas remplir les zones étroites");

    if (idx == 221) return QObject::tr("Nouveau");
    if (idx == 222) return QObject::tr("Ajouter");
    if (idx == 223) return QObject::tr("Supprimer");
    if (idx == 224) return QObject::tr("Commun");

    if (idx == 230) return QObject::tr("Force");
    if (idx == 231) return QObject::tr("Fixé par trait");
    if (idx == 232) return QObject::tr("Fixer");

    if (idx == 240) return QObject::tr("Expansion");
    if (idx == 241) return QObject::tr("Contraction");
    if (idx == 242) return QObject::tr("Pousser");
    if (idx == 243) return QObject::tr("Rotation à droite");
    if (idx == 244) return QObject::tr("Rotation à gauche");
    if (idx == 249) return QObject::tr("Outil liquéfier");

    if (idx == 260) return QObject::tr("La restauration précédente a échoué. Le processus de récupération a été ignoré."); // 前回の復旧に失敗したため、復旧処理をスキップしました

    //if (idx == 270) return QObject::tr("Select the layer within the frame (Animation Mode)"); // フレーム内のレイヤーを選択 (アニメーションモード)

    if (idx == 300) return QObject::tr("Obtenir la couleur transparente");
    if (idx == 301) return QObject::tr("Super Ellipse");

    if (idx == 310) return QObject::tr("Souhaitez-vous utiliser le fichier de configuration de FireAlpaca (version gratuite) à partir de maintenant ?\n\n (Fermez l'application car un redémarrage est nécessaire)");
    if (idx == 311) return QObject::tr("Souhaitez-vous revenir au fichier de configuration standard de FireAlpaca SE ?\n\n (Fermez l'application car un redémarrage est nécessaire)");
    if (idx == 312) return QObject::tr("Voulez-vous conserver les paramètres de la version précédente ?"); // 旧バージョンの設定を引き継ぎますか？
    //if (idx == 313) return QObject::tr("Your configuration files have been backed up. Do you want to open the backup folder?"); // 設定ファイルをバックアップしました。バックアップしたフォルダを開きますか？
    //if (idx == 314) return QObject::tr("Do you want to back up your brushes, colors, gradients and shortcut settings?"); // ブラシ、カラー、グラデーション、ショートカット設定をバックアップしますか？
    //if (idx == 315) return QObject::tr("Your current settings will be discarded. Is this OK?"); // 現在の設定は破棄されますが、宜しいですか？

    if (idx == 320) return QObject::tr("Sélectionner un sommet ou un objet");
    if (idx == 321) return QObject::tr("Sélectionner un calque");

    //if (idx == 330) return QObject::tr("All Formats"); // 全ての形式
    //if (idx == 331) return QObject::tr("FireAlpaca SE Project"); // FireAlpaca SE プロジェクト

    if (idx == 425) return QObject::tr("Zone de travail"); // キャンバス
    if (idx == 426) return QObject::tr("Masque"); // アクティブレイヤー
    if (idx == 427) return QObject::tr("Calque spécifié"); // 指定レイヤー

    //if (idx == 440) return QObject::tr("Eyedropper when Dragging"); // ドラッグ時にもスポイト

    //if (idx == 450) return QObject::tr("Are you sure you want to delete this layer?"); // 本当にレイヤーを削除しますか？
    //if (idx == 451) return QObject::tr("You can set it to be deleted without confirmation in the preferences."); // 環境設定から確認なしに削除するよう設定できます

    if (idx == 500) return QObject::tr("La consommation de mémoire est réduite de moitié, mais la précision des couleurs diminue. Cela vous convient-il ?"); // メモリ消費量が半分になりますが、色精度が低下します。宜しいですか？
    if (idx == 501) return QObject::tr("Cela améliore la précision des couleurs et réduit la détérioration lors de l'édition, mais double la consommation de mémoire. Cela vous convient-il ?"); // 色精度が向上し編集時の劣化が軽減しますが、メモリ使用量が倍になります。宜しいですか？
    //if (idx == 502) return QObject::tr("Do you want to convert existing layers to 8bit/ch?"); // 既存のレイヤーを 8bit/ch に変換しますか？
    //if (idx == 503) return QObject::tr("Do you want to convert existing layers to 16bit/ch?"); // 既存のレイヤーを 16bit/ch に変換しますか？
    //if (idx == 510) return QObject::tr("This is an advanced setting, do you really want to enable it?"); // 上級者向けの設定ですが、本当に有効にしますか？

    //if (idx == 600) return QObject::tr("MFG Archive Format (*.mar)"); // MFG アーカイブ形式(*.mar)
    //if (idx == 601) return QObject::tr("Import MAR Files"); // MARファイルの読み込み
    //if (idx == 602) return QObject::tr("Invalid MAR: "); // MARファイルが壊れています：
    //if (idx == 603) return QObject::tr("Failed to copy: "); // ファイルのコピーに失敗しました:
    //if (idx == 604) return QObject::tr("Import %1 filters"); // %1 個のフィルタをインポートしました
    //if (idx == 605) return QObject::tr("The process is taking longer than usual..."); // 処理が長くかかっています…

    //if (idx == 700) return QObject::tr("The process failed. This is likely due to insufficient main memory."); // 処理に失敗しました。メインメモリが不足している可能性が高いです。
  }

  //////////////////////////////////
  //////////////////////////////////
  if (man->TranslateRussian())
  {
    if (idx == 1) return QObject::tr("Инструмент КИСТЬ");
    if (idx == 2) return QObject::tr("Инструмент ТОЧЕЧНАЯ КИСТЬ");
    if (idx == 3) return QObject::tr("Инструмент ЛАСТИК");
    if (idx == 4) return QObject::tr("Инструмент ПЕРЕМЕЩЕНИЕ");
    if (idx == 5) return QObject::tr("Инструмент ВЕДРО");
    if (idx == 6) return QObject::tr("Инструмент ГРАДИЕНТ ");
    if (idx == 7) return QObject::tr("Инструмент ЗАЛИВКА");
    if (idx == 8) return QObject::tr("Инструмент ВЫДЕЛЕНИЕ");
    if (idx == 9) return QObject::tr("Инструмент ВОЛШЕБНАЯ ПАЛОЧКА");
    if (idx == 10) return QObject::tr("Инструмент ВЫБОР ПЕРА");
    if (idx == 11) return QObject::tr("Инструмент ВЫБОР ЛАСТИКА");
    if (idx == 12) return QObject::tr("Инструмент ТЕКСТ");
    if (idx == 13) return QObject::tr("Инструмент ПИПЕТКА");
    if (idx == 14) return QObject::tr("Инструмент РУКА");

    if (idx == 15) return QObject::tr("Привязка отключена");
    if (idx == 16) return QObject::tr("Параллельная привязка");
    if (idx == 17) return QObject::tr("Перекрестная привязка");
    if (idx == 18) return QObject::tr("Радиальная привязка");
    if (idx == 19) return QObject::tr("Настройки привязки");

    if (idx == 20) return QObject::tr("Сглаживание");
    if (idx == 21) return QObject::tr("Коррекция");
    if (idx == 22) return QObject::tr("Скругление углов");
    if (idx == 23) return QObject::tr("Непрозрачность");
    if (idx == 24) return QObject::tr("Ссылка");

    if (idx == 27) return QObject::tr("Расширить");
    if (idx == 28) return QObject::tr("Форма");
    if (idx == 29) return QObject::tr("Тип");
    if (idx == 30) return QObject::tr("Линейный");
    if (idx == 31) return QObject::tr("Круговой");
    if (idx == 32) return QObject::tr("Цвет рисунка - фона");
    if (idx == 33) return QObject::tr("Цвет рисунка - Прозрачный");
    if (idx == 34) return QObject::tr("Прямоугольник");
    if (idx == 35) return QObject::tr("Эллипс");
    if (idx == 36) return QObject::tr("Многоугольник");
    if (idx == 37) return QObject::tr("Нет варианта");
    if (idx == 38) return QObject::tr("Лассо");
    if (idx == 39) return QObject::tr("Инструмент ЛАССО"); // 投げ縄ツール

    if (idx == 40) return QObject::tr("Цвет");
    if (idx == 41) return QObject::tr("Параметры кисти");
    if (idx == 42) return QObject::tr("Кисть");
    if (idx == 43) return QObject::tr("Навигатор");
    if (idx == 44) return QObject::tr("Слой");
    if (idx == 45) return QObject::tr("Просмотр кисти");
    if (idx == 46) return QObject::tr("Палитра");
    if (idx == 47) return QObject::tr("Модель");
    if (idx == 48) return QObject::tr("Размер кисти"); // ブラシサイズ [ウィンドウのキャプション]
    if (idx == 49) return QObject::tr("История цвета"); // カラーヒストリー

    if (idx == 50) return QObject::tr("(Отразить)");
    if (idx == 51) return QObject::tr("Файл не существует.");
    if (idx == 52)
    {
      QString s = QObject::tr("%1 уже открыт. Продолжить?");
      s.replace( "%1", man->StrParam() );
      return s;
    }
    if (idx == 54) return QObject::tr("Исправление перспективы");
    if (idx == 55) return QObject::tr("Открыть изображение");
    if (idx == 56) return QObject::tr("Изображения");
    if (idx == 57) return QObject::tr("Круговая привязка");
    if (idx == 58) return QObject::tr("Привязка к кривой");

    if (idx == 60) return QObject::tr("Расширить");
    if (idx == 61) return QObject::tr("Сжать");
    if (idx == 62) return QObject::tr("Инициализировать положение окна?");
    if (idx == 63) return QObject::tr("Отменить");
    if (idx == 64) return QObject::tr("Восстановить");
    if (idx == 65) return QObject::tr("Привязка");
    if (idx == 66) return QObject::tr("Для обновления настроек языка программа будет закрыта. Перезапустите программу после закрытия.");
    if (idx == 67) return QObject::tr("Не удалось открыть");
    if (idx == 68) return QObject::tr("Изогелия");

    if (idx == 70) return QObject::tr("Не удалось определить принтер");
    if (idx == 71) return QObject::tr("Размытый край"); // ソフトエッジ
    if (idx == 72) return QObject::tr("Без названия");
    if (idx == 73) return QObject::tr("Выберите слой в точке клика");

    if (idx == 75) return QObject::tr("Параллельный");
    if (idx == 76) return QObject::tr("Радиальный");
    if (idx == 77) return QObject::tr("Круговой");
    if (idx == 78) return QObject::tr("Кривая");
    if (idx == 79) return QObject::tr("Исправление перспективы");

    if (idx == 80) return QObject::tr("Инструмент РАБОТА ОБЪЕКТА");
    if (idx == 81) return QObject::tr("Инструмент РАЗДЕЛЕНИЕ КАДРЫ");
    if (idx == 82) return QObject::tr("Кернинг");
    if (idx == 83) return QObject::tr("Интерлиньяж");

    if (idx == 85) return QObject::tr("Выбрать из центра");
    if (idx == 86) return QObject::tr("Сохранять пропорции");

    if (idx == 87) return QObject::tr("Для обновления настроек программа будет закрыта. Перезапустите программу после закрытия");

    if (idx == 88) return QObject::tr("Прочее");
    if (idx == 89) return QObject::tr("Увеличить размер кисти");
    if (idx == 90) return QObject::tr("Уменьшить размер кисти");
    if (idx == 91) return QObject::tr("Недопустимый MDP файл");
    if (idx == 92) return QObject::tr("%1 не может быть открыт так как не является MDP файлом");
    if (idx == 93) return QObject::tr("Файл уже существует");
    if (idx == 94) return QObject::tr("Открыть \"AlpacaDouga\", сервис для создания анимации GIF, в браузере?");
    if (idx == 95) return QObject::tr("Прозрачный фон??");
    if (idx == 96) return QObject::tr("Создать папку?");
    if (idx == 97) return QObject::tr("Привязка к сетке");
    if (idx == 98) return QObject::tr("Привязка к гиду");
    if (idx == 99) return QObject::tr("Разделить кадры"); // コマ分割

    if (idx == 110) return QObject::tr("Свободно");
    if (idx == 111) return QObject::tr("Линия");
    if (idx == 112) return QObject::tr("Полилиния");
    if (idx == 113) return QObject::tr("Многоугольник");
    if (idx == 114) return QObject::tr("Прямоугольник");
    if (idx == 115) return QObject::tr("Эллипс");
    if (idx == 116) return QObject::tr("Кривая");
    if (idx == 117) return QObject::tr("Фигура");
    if (idx == 118) return QObject::tr("Квадрат");
    if (idx == 119) return QObject::tr("Круг");
  
    if (idx == 120) return QObject::tr("Привязка 3D-перспективы");
    if (idx == 122) return QObject::tr("Отменить");
    if (idx == 123) return QObject::tr("Привязка к сетке");
    if (idx == 124) return QObject::tr("Игнорировать цвет слоя");
    if (idx == 125) return QObject::tr("Игнорировать черновой слой");
  
    if (idx == 126) return QObject::tr("Усреднение");
    if (idx == 127) return QObject::tr("Нет");
    if (idx == 128) return QObject::tr("3 * 3 px среднее");
    if (idx == 129) return QObject::tr("5 * 5 px среднее");
  
    if (idx == 130) return QObject::tr("Автосохранение...");
    if (idx == 131) return QObject::tr("Автосохранение выполнено.");
    if (idx == 132) return QObject::tr("Автосохранение отключено (нехватка памяти)\n\nСтабильность приложения значительно снижается");
    if (idx == 133) return QObject::tr("Восстановить автосохраненное изображение?");
  
    if (idx == 135) return QObject::tr("Не удалось пост в pixiv Sketch.");
  
    if (idx == 136) return QObject::tr("Развертывание");
    if (idx == 137) return QObject::tr("Нормальный");
    if (idx == 138) return QObject::tr("Повтор");
    if (idx == 139) return QObject::tr("Отражение");
  
    if (idx == 140) return QObject::tr("Скругление углов");
    //if (idx == 141) return QObject::tr("Gaussian Blur"); // ガウスぼかし

    if (idx == 150) return QObject::tr("Скругление углов, сохранение ширины линии");
  
    if (idx == 160) return QObject::tr("Заполнение пробелы");
    if (idx == 162) return QObject::tr("Толерантность");
    if (idx == 163) return QObject::tr("Прозрачный - Цвет рисунка");
    if (idx == 164) return QObject::tr("Настройка");
    if (idx == 165) return QObject::tr("Ведро ластик");
  
    if (idx == 170) return QObject::tr("Установить как перетащить");
    if (idx == 171) return QObject::tr("Установка");
    if (idx == 172) return QObject::tr("Нет давление на обоих концах");
    if (idx == 173) return QObject::tr("Переместить на 1px");
    if (idx == 174) return QObject::tr("Переместить слой на 1px влево");
    if (idx == 175) return QObject::tr("Переместить слой на 1px вверху");
    if (idx == 176) return QObject::tr("Переместить слой на 1px внизу");
    if (idx == 177) return QObject::tr("Переместить слой на 1px вправо");
  
    if (idx == 180) return QObject::tr("Импорт палитры");
    if (idx == 181) return QObject::tr("Экспорт палитры");
    if (idx == 182) return QObject::tr("FireAlpaca Palette (*.fap);;FireAlpaca Palette (*.fap)");
    if (idx == 183) return QObject::tr("Импорт ACO");
    if (idx == 184) return QObject::tr("Экспорт как ACO");
    if (idx == 185) return QObject::tr("ACO Color Palette (*.aco);;ACO Color Palette (*.aco)");
  
    if (idx == 190) return QObject::tr("Преобразовать");
    if (idx == 191) return QObject::tr("Свободно");
    if (idx == 192) return QObject::tr("Сетка");

    if (idx == 200) return QObject::tr("Двусторонний"); // 左右対称
    if (idx == 201) return QObject::tr("Вертикальный"); // 上下対称
    if (idx == 202) return QObject::tr("Поворот"); // 回転対称
    if (idx == 203) return QObject::tr(" (Зеркало)"); //  (鏡像)
    if (idx == 204) return QObject::tr("Плитка"); // タイル
    if (idx == 205) return QObject::tr("Симметрия"); // 対称
    if (idx == 206) return QObject::tr(" (Половина)"); //  (ハーフドロップ)
    if (idx == 207) return QObject::tr("Указать центр"); // 中心を指定
    if (idx == 208) return QObject::tr("Линейная симметрия"); // 線対称

    if (idx == 210) return QObject::tr("Инструмент ЗАЛИВКА (ЛАССО)");
    if (idx == 211) return QObject::tr("Закрытая");
    if (idx == 212) return QObject::tr("Прозрачный/Белый");
    if (idx == 213) return QObject::tr("Прозрачный");
    if (idx == 214) return QObject::tr("Белый");
    if (idx == 215) return QObject::tr("Все");
    if (idx == 219) return QObject::tr("Кроме узких участков");
  
    if (idx == 221) return QObject::tr("Новый");
    if (idx == 222) return QObject::tr("Добавить");
    if (idx == 223) return QObject::tr("Удалить");
    if (idx == 224) return QObject::tr("Общий");
  
    if (idx == 230) return QObject::tr("Усиление");
    if (idx == 231) return QObject::tr("Установить как перетащить");
    if (idx == 232) return QObject::tr("Установка");
  
    if (idx == 240) return QObject::tr("Развертывать");
    if (idx == 241) return QObject::tr("Сокращать");
    if (idx == 242) return QObject::tr("Нажимать");
    if (idx == 243) return QObject::tr("Повертывать направо");
    if (idx == 244) return QObject::tr("Повертывать налево");
    if (idx == 249) return QObject::tr("Инструмент ПЛАСТИКА"); // 歪みツール

    //if (idx == 260) return QObject::tr("Previous resume seems fail, skip and launch normaly."); // 前回の復旧に失敗したため、復旧処理をスキップしました

    //if (idx == 270) return QObject::tr("Select the layer within the frame (Animation Mode)"); // フレーム内のレイヤーを選択 (アニメーションモード)

    if (idx == 300) return QObject::tr("Получить прозрачный цвет");
    if (idx == 301) return QObject::tr("Суперэллипс"); // スーパー楕円
  
    if (idx == 310) return QObject::tr("Хотите ли вы впредь использовать файл конфигурации, используемый в FireAlpaca (бесплатная версия)? \n\n(Закройте приложение, так как требуется перезапуск)"); // 今後、FireAlpaca (無料版) で使っていた設定ファイルを使うようにしますか？\n\n(再起動が必要なためアプリケーションを終了します)
    if (idx == 311) return QObject::tr("Хотите ли вы впредь использовать файл конфигурации, используемый в FireAlpaca (бесплатная версия)? \n\n(Закройте приложение, так как требуется перезапуск)"); // FireAlpaca SE 標準の設定ファイルを使うように戻しますか？\n\n(再起動が必要なためアプリケーションを終了します)
    if (idx == 312) return QObject::tr("Хотите сохранить настройки предыдущей версии?"); // 旧バージョンの設定を引き継ぎますか？
    //if (idx == 313) return QObject::tr("Your configuration files have been backed up. Do you want to open the backup folder?"); // 設定ファイルをバックアップしました。バックアップしたフォルダを開きますか？
    //if (idx == 314) return QObject::tr("Do you want to back up your brushes, colors, gradients and shortcut settings?"); // ブラシ、カラー、グラデーション、ショートカット設定をバックアップしますか？
    //if (idx == 315) return QObject::tr("Your current settings will be discarded. Is this OK?"); // 現在の設定は破棄されますが、宜しいですか？

    if (idx == 320) return QObject::tr("Выбрать вершины/объекты"); // 頂点・オブジェクトを選択
    if (idx == 321) return QObject::tr("Выбрать слой"); // レイヤーを選択

    //if (idx == 330) return QObject::tr("All Formats"); // 全ての形式
    //if (idx == 331) return QObject::tr("FireAlpaca SE Project"); // FireAlpaca SE プロジェクト

    if (idx == 425) return QObject::tr("Холст"); // キャンバス
    if (idx == 426) return QObject::tr("Текущий слой"); // アクティブレイヤー
    if (idx == 427) return QObject::tr("Указанный слой"); // 指定レイヤー

    //if (idx == 440) return QObject::tr("Eyedropper when Dragging"); // ドラッグ時にもスポイト

    //if (idx == 450) return QObject::tr("Are you sure you want to delete this layer?"); // 本当にレイヤーを削除しますか？
    //if (idx == 451) return QObject::tr("You can set it to be deleted without confirmation in the preferences."); // 環境設定から確認なしに削除するよう設定できます

    if (idx == 500) return QObject::tr("Потребление памяти уменьшилось вдвое, но точность цветопередачи снизилась. Хотите продолжить?"); // メモリ消費量が半分になりますが、色精度が低下します。宜しいですか？
    if (idx == 501) return QObject::tr("Это повышает точность цветопередачи и уменьшает ухудшение качества при редактировании, но удваивает объем используемой памяти. Хотите продолжить??"); // 色精度が向上し編集時の劣化が軽減しますが、メモリ使用量が倍になります。宜しいですか？
    //if (idx == 502) return QObject::tr("Do you want to convert existing layers to 8bit/ch?"); // 既存のレイヤーを 8bit/ch に変換しますか？
    //if (idx == 503) return QObject::tr("Do you want to convert existing layers to 16bit/ch?"); // 既存のレイヤーを 16bit/ch に変換しますか？
    //if (idx == 510) return QObject::tr("This is an advanced setting, do you really want to enable it?"); // 上級者向けの設定ですが、本当に有効にしますか？

    //if (idx == 600) return QObject::tr("MFG Archive Format (*.mar)"); // MFG アーカイブ形式(*.mar)
    //if (idx == 601) return QObject::tr("Import MAR Files"); // MARファイルの読み込み
    //if (idx == 602) return QObject::tr("Invalid MAR: "); // MARファイルが壊れています：
    //if (idx == 603) return QObject::tr("Failed to copy: "); // ファイルのコピーに失敗しました:
    //if (idx == 604) return QObject::tr("Import %1 filters"); // %1 個のフィルタをインポートしました
    //if (idx == 605) return QObject::tr("The process is taking longer than usual..."); // 処理が長くかかっています…

    //if (idx == 700) return QObject::tr("The process failed. This is likely due to insufficient main memory."); // 処理に失敗しました。メインメモリが不足している可能性が高いです。
  }

  //////////////////////////////////
  //////////////////////////////////
  if (man->TranslateHindi())
  {
  }

  if (man->TranslateBengali())
  {
  }

  if (man->TranslatePolish())
  {
  }

  ///////////////////
  // 翻訳がないので英語で
  ///////////////////
  if (idx == 1) return QObject::tr("Brush Tool"); // ブラシツール
  if (idx == 2) return QObject::tr("Dot Brush Tool"); // ドットツール
  if (idx == 3) return QObject::tr("Eraser Tool"); // 消しゴムツール
  if (idx == 4) return QObject::tr("Move Tool"); // 移動ツール
  if (idx == 5) return QObject::tr("Bucket Tool"); // バケツツール
  if (idx == 6) return QObject::tr("Gradient Tool"); // グラデーションツール
  if (idx == 7) return QObject::tr("Fill Tool"); // 塗りつぶしツール
  if (idx == 8) return QObject::tr("Select Tool"); // 選択ツール
  if (idx == 9) return QObject::tr("MagicWand Tool"); // 自動選択ツール
  if (idx == 10) return QObject::tr("SelectPen Tool"); // 選択ペンツール
  if (idx == 11) return QObject::tr("SelectEraser Tool"); // 選択消しツール
  if (idx == 12) return QObject::tr("Text Tool"); // テキストツール
  if (idx == 13) return QObject::tr("Eyedropper Tool"); // スポイトツール
  if (idx == 14) return QObject::tr("Hand Tool"); // 手のひらツール

  if (idx == 15) return QObject::tr("Snap Off"); // スナップ・オフ [スナップしない]
  if (idx == 16) return QObject::tr("Parallel Snap"); // 並行スナップ
  if (idx == 17) return QObject::tr("Crisscross Snap"); // 十字スナップ
  if (idx == 18) return QObject::tr("Radial Snap"); // 集中線スナップ
  if (idx == 19) return QObject::tr("Snap Setting"); // スナップ設定 [スナップ設定を開始する]

  if (idx == 20) return QObject::tr("Anti-aliasing"); // アンチエイリアス
  if (idx == 21) return QObject::tr("Stabilizer"); // 手ぶれ補正 [ブラシの補正]
  if (idx == 22) return QObject::tr("RoundCorner"); // 角丸 [矩形の角を丸める処理]
  if (idx == 23) return QObject::tr("Opacity"); // 不透明度
  if (idx == 24) return QObject::tr("Reference"); // 参照 [レイヤーの参照]

  if (idx == 27) return QObject::tr("Expand"); // 拡張 [自動選択やバケツ塗りつぶしの拡張処理]
  if (idx == 28) return QObject::tr("Shape"); // 形状 [グラデーションの形状]
  if (idx == 29) return QObject::tr("Type");  // タイプ [グラデーションの型]
  if (idx == 30) return QObject::tr("Linear"); // 線形 [グラデーションの形状]
  if (idx == 31) return QObject::tr("Radial"); // 放射状 [グラデーションの形状]
  if (idx == 32) return QObject::tr("Foreground-Background"); // 前景～背景 [グラデーションの型]
  if (idx == 33) return QObject::tr("Foreground"); // 前景 [グラデーションの型]
  if (idx == 34) return QObject::tr("Rectangle"); // 矩形 [塗りつぶしの形状]
  if (idx == 35) return QObject::tr("Ellipse"); // 楕円 [塗りつぶしの形状]
  if (idx == 36) return QObject::tr("Polygon"); // 多角形 [塗りつぶしの形状]
  if (idx == 37) return QObject::tr("No Option"); // 設定なし [設定がないツール用のパネルで表示する文字列]
  if (idx == 38) return QObject::tr("Lasso"); // 投げ縄 [選択範囲の取り方]
  if (idx == 39) return QObject::tr("Lasso Tool"); // 投げ縄ツール

  if (idx == 40) return QObject::tr("Color"); // 色 [ウィンドウのキャプション]
  if (idx == 41) return QObject::tr("Brush Control"); // ブラシコントロール [ウィンドウのキャプション]
  if (idx == 42) return QObject::tr("Brush"); // ブラシ [ウィンドウのキャプション]
  if (idx == 43) return QObject::tr("Navigator"); // ナビゲータ [ウィンドウのキャプション]
  if (idx == 44) return QObject::tr("Layer"); // レイヤー [ウィンドウのキャプション]
  if (idx == 45) return QObject::tr("Brush Preview"); // ブラシプレビュー [ウィンドウのキャプション]
  if (idx == 46) return QObject::tr("Palette"); // パレット [ウィンドウのキャプション]
  if (idx == 47) return QObject::tr("Reference"); // 資料 [ウィンドウのキャプション]
  if (idx == 48) return QObject::tr("Brush Size"); // ブラシサイズ [ウィンドウのキャプション]
  if (idx == 49) return QObject::tr("Color History"); // カラーヒストリー

  if (idx == 50) return QObject::tr("(Flip)"); // 反転 [表示の左右反転]
  if (idx == 51) return QObject::tr("File does not exist"); // ファイルは存在しません
  if (idx == 52)
  {
    // xx は既に開いていますが、続行しますか？
    QString s = QObject::tr("%1 is already opened, proceed?");
    s.replace( "%1", man->StrParam() );
    return s;
  }
  if (idx == 54) return QObject::tr("Vanishing Point Snap");
  if (idx == 55) return QObject::tr("Open Image");
  if (idx == 56) return QObject::tr("Images");
  if (idx == 57) return QObject::tr("Circle Snap");
  if (idx == 58) return QObject::tr("Curve Snap");

  if (idx == 60) return QObject::tr("Expand");
  if (idx == 61) return QObject::tr("Contract");
  if (idx == 62) return QObject::tr("Initialize a window position?");
  if (idx == 63) return QObject::tr("Undo");
  if (idx == 64) return QObject::tr("Redo");
  if (idx == 65) return QObject::tr("Snap");
  if (idx == 66) return QObject::tr("In order to update the language setting, the program will be closed.  Please restart the program after closing.");
  if (idx == 67) return QObject::tr("Failed to open");
  if (idx == 68) return QObject::tr("Threshold");

  if (idx == 70) return QObject::tr("Failed to obtain the printer.");
  if (idx == 71) return QObject::tr("Soft Edge");
  if (idx == 72) return QObject::tr("Untitled");
  if (idx == 73) return QObject::tr("Select a layer at the clicked point");

  if (idx == 75) return QObject::tr("Parallel");
  if (idx == 76) return QObject::tr("Radial");
  if (idx == 77) return QObject::tr("Circle");
  if (idx == 78) return QObject::tr("Curve");
  if (idx == 79) return QObject::tr("Vanishing Point");

  if (idx == 80) return QObject::tr("Operation Tool");
  if (idx == 81) return QObject::tr("Divide Tool");
  if (idx == 82) return QObject::tr("Kerning");
  if (idx == 83) return QObject::tr("Leading");
  if (idx == 84) return QObject::tr("Add Koma Material");

  if (idx == 85) return QObject::tr("Select From Center");
  if (idx == 86) return QObject::tr("Constrain Proportions");

  if (idx == 87) return QObject::tr("In order to update the shortcut setting, the program will be closed.  Please restart the program after closing.");

  if (idx == 88) return QObject::tr("Others");
  if (idx == 89) return QObject::tr("Increase Brush Size");
  if (idx == 90) return QObject::tr("Decrease Brush Size");
  if (idx == 91) return QObject::tr("Invalid MDP file");
  if (idx == 92) return QObject::tr("%1 could not be opened because it isn’t a valid MDP file.");
  if (idx == 93) return QObject::tr("File already exists");
  if (idx == 94) return QObject::tr("Open \"AlpacaDouga\", the animated GIF convert service, in a browser?");
  if (idx == 95) return QObject::tr("Transparent the background?");
  if (idx == 96) return QObject::tr("Create a folder?");
  if (idx == 97) return QObject::tr("Grid Snap");
  if (idx == 98) return QObject::tr("Guide Snap");
  if (idx == 99) return QObject::tr("Koma Division"); // コマ分割

  if (idx == 110) return QObject::tr("Freehand");
  if (idx == 111) return QObject::tr("Line");
  if (idx == 112) return QObject::tr("Polyline");
  if (idx == 113) return QObject::tr("Polygon");
  if (idx == 114) return QObject::tr("Rectangle");
  if (idx == 115) return QObject::tr("Ellipse");
  if (idx == 116) return QObject::tr("Curve");
  if (idx == 117) return QObject::tr("Shape");
  if (idx == 118) return QObject::tr("Square");
  if (idx == 119) return QObject::tr("Circle");

  if (idx == 120) return QObject::tr("3D Perspective Snap");
  if (idx == 122) return QObject::tr("Deselect");
  if (idx == 123) return QObject::tr("Snap to grid");
  if (idx == 124) return QObject::tr("Ignore Layer Color");
  if (idx == 125) return QObject::tr("Ignore Draft Layer");

  if (idx == 126) return QObject::tr("Averaging");
  if (idx == 127) return QObject::tr("None");
  if (idx == 128) return QObject::tr("3 * 3 px Average");
  if (idx == 129) return QObject::tr("5 * 5 px Average");

  if (idx == 130) return QObject::tr("Auto Saving...");
  if (idx == 131) return QObject::tr("Auto Save Done");
  if (idx == 132) return QObject::tr("Auto Save Disabled (Insufficient Memory)\n\nThe stability of the application is significantly reduced");
  if (idx == 133) return QObject::tr("Recover Auto Saved Image?");
  if (idx == 134) return QObject::tr("Recover Editing Canvas");

  if (idx == 135) return QObject::tr("Failed to post to pixiv Sketch");

  if (idx == 136) return QObject::tr("Spread");
  if (idx == 137) return QObject::tr("Standard");
  if (idx == 138) return QObject::tr("Repeat");
  if (idx == 139) return QObject::tr("Reflect");

  if (idx == 140) return QObject::tr("Round a corner"); // 角を丸める
  if (idx == 141) return QObject::tr("Gaussian Blur"); // ガウスぼかし

  if (idx == 150) return QObject::tr("Round a corner, Keep line width");

  if (idx == 160) return QObject::tr("Close the gap");
  if (idx == 162) return QObject::tr("Tolerance");
  if (idx == 163) return QObject::tr("Transparent-Foreground");
  if (idx == 164) return QObject::tr("Custom");
  if (idx == 165) return QObject::tr("Bucket Erase");

  if (idx == 170) return QObject::tr("Determine by Each Drag");
  if (idx == 171) return QObject::tr("Determine Position");
  if (idx == 172) return QObject::tr("Zero Pressure on Both Ends");
  if (idx == 173) return QObject::tr("Move 1px");
  if (idx == 174) return QObject::tr("Move layer 1px to the left");
  if (idx == 175) return QObject::tr("Move layer up 1px");
  if (idx == 176) return QObject::tr("Move layer down 1px");
  if (idx == 177) return QObject::tr("Move layer 1px to the right");

  if (idx == 180) return QObject::tr("Import Palette File");
  if (idx == 181) return QObject::tr("Export Palette File");
  if (idx == 182) return QObject::tr("FireAlpaca Palette (*.fap);;FireAlpaca Palette (*.fap)");
  if (idx == 183) return QObject::tr("Import ACO　Format");
  if (idx == 184) return QObject::tr("Export as ACO Format");
  if (idx == 185) return QObject::tr("ACO Color Palette (*.aco);;ACO Color Palette (*.aco)");

  if (idx == 190) return QObject::tr("Transform");
  if (idx == 191) return QObject::tr("Free Transform");
  if (idx == 192) return QObject::tr("Mesh Transform");

  if (idx == 200) return QObject::tr("Bilateral"); // 左右対称
  if (idx == 201) return QObject::tr("Vertical"); // 上下対称
  if (idx == 202) return QObject::tr("Rotate"); // 回転対称
  if (idx == 203) return QObject::tr(" (Mirror)"); //  (鏡像)
  if (idx == 204) return QObject::tr("Tile"); // タイル
  if (idx == 205) return QObject::tr("Symmetry"); // 対称
  if (idx == 206) return QObject::tr(" (Half Drop)"); //  (ハーフドロップ)
  if (idx == 207) return QObject::tr("Specify Center"); // 中心を指定
  if (idx == 208) return QObject::tr("Line"); // 線対称

  if (idx == 210) return QObject::tr("Lasso Fill");
  if (idx == 211) return QObject::tr("Closed Area");
  if (idx == 212) return QObject::tr("Transparent/White");
  if (idx == 213) return QObject::tr("Transparent");
  if (idx == 214) return QObject::tr("White");
  if (idx == 215) return QObject::tr("All");
  if (idx == 219) return QObject::tr("Do not fill narrow areas");

  if (idx == 221) return QObject::tr("New");
  if (idx == 222) return QObject::tr("Add");
  if (idx == 223) return QObject::tr("Sub");
  if (idx == 224) return QObject::tr("Common");

  if (idx == 230) return QObject::tr("Strength");
  if (idx == 231) return QObject::tr("Fixed per Stroke");
  if (idx == 232) return QObject::tr("Fix");

  if (idx == 240) return QObject::tr("Expansion");
  if (idx == 241) return QObject::tr("Contraction");
  if (idx == 242) return QObject::tr("Push");
  if (idx == 243) return QObject::tr("Rotate Right");
  if (idx == 244) return QObject::tr("Rotate Left");
  if (idx == 249) return QObject::tr("Liquify Tool"); // 歪みツール

  if (idx == 260) return QObject::tr("Previous resume seems fail, skip and launch normaly."); // 前回の復旧に失敗したため、復旧処理をスキップしました

  if (idx == 270) return QObject::tr("Select the layer within the frame (Animation Mode)"); // フレーム内のレイヤーを選択 (アニメーションモード)

  if (idx == 300) return QObject::tr("Get Transparent Color");
  if (idx == 301) return QObject::tr("Super Ellipse"); // スーパー楕円

  if (idx == 310) return QObject::tr("Do you want to use the configuration file used in FireAlpaca (free version) from now on? \n\n(Close the application as a restart is required)"); // 今後、FireAlpaca (無料版) で使っていた設定ファイルを使うようにしますか？\n\n(再起動が必要なためアプリケーションを終了します)
  if (idx == 311) return QObject::tr("Do you want to revert to using the FireAlpaca SE standard configuration file? \n\n(Close the application as a restart is required)"); // FireAlpaca SE 標準の設定ファイルを使うように戻しますか？\n\n(再起動が必要なためアプリケーションを終了します)
  if (idx == 312) return QObject::tr("Do you want to keep the settings from the previous version?"); // 旧バージョンの設定を引き継ぎますか？
  if (idx == 313) return QObject::tr("Your configuration files have been backed up. Do you want to open the backup folder?"); // 設定ファイルをバックアップしました。バックアップしたフォルダを開きますか？
  if (idx == 314) return QObject::tr("Do you want to back up your brushes, colors, gradients and shortcut settings?"); // ブラシ、カラー、グラデーション、ショートカット設定をバックアップしますか？
  if (idx == 315) return QObject::tr("Your current settings will be discarded. Is this OK?"); // 現在の設定は破棄されますが、宜しいですか？

  if (idx == 320) return QObject::tr("Select Vertices/Objects"); // 頂点・オブジェクトを選択
  if (idx == 321) return QObject::tr("Select Layers"); // レイヤーを選択

  if (idx == 330) return QObject::tr("All Formats"); // 全ての形式
  if (idx == 331) return QObject::tr("FireAlpaca SE Project"); // FireAlpaca SE プロジェクト

  if (idx == 425) return QObject::tr("Canvas"); // キャンバス
  if (idx == 426) return QObject::tr("Active Layer"); // アクティブレイヤー
  if (idx == 427) return QObject::tr("Specified Layer"); // 指定レイヤー

  if (idx == 440) return QObject::tr("Eyedropper when Dragging"); // ドラッグ時にもスポイト

  if (idx == 450) return QObject::tr("Are you sure you want to delete this layer?"); // 本当にレイヤーを削除しますか？
  if (idx == 451) return QObject::tr("You can set it to be deleted without confirmation in the preferences."); // 環境設定から確認なしに削除するよう設定できます

  if (idx == 500) return QObject::tr("Memory consumption is halved, but color accuracy is reduced. Is that okay?"); // メモリ消費量が半分になりますが、色精度が低下します。宜しいですか？
  if (idx == 501) return QObject::tr("This improves color accuracy and reduces deterioration during editing, but doubles the amount of memory used. Is that okay?"); // 色精度が向上し編集時の劣化が軽減しますが、メモリ使用量が倍になります。宜しいですか？
  if (idx == 502) return QObject::tr("Do you want to convert existing layers to 8bit/ch?"); // 既存のレイヤーを 8bit/ch に変換しますか？
  if (idx == 503) return QObject::tr("Do you want to convert existing layers to 16bit/ch?"); // 既存のレイヤーを 16bit/ch に変換しますか？
  if (idx == 510) return QObject::tr("This is an advanced setting, do you really want to enable it?"); // 上級者向けの設定ですが、本当に有効にしますか？

  if (idx == 600) return QObject::tr("MFG Archive Format (*.mar)"); // MFG アーカイブ形式(*.mar)
  if (idx == 601) return QObject::tr("Import MAR Files"); // MARファイルの読み込み
  if (idx == 602) return QObject::tr("Invalid MAR: "); // MARファイルが壊れています：
  if (idx == 603) return QObject::tr("Failed to copy: "); // ファイルのコピーに失敗しました:
  if (idx == 604) return QObject::tr("Import %1 filters"); // %1 個のフィルタをインポートしました
  if (idx == 605) return QObject::tr("The process is taking longer than usual..."); // 処理が長くかかっています…

  if (idx == 700) return QObject::tr("The process failed. This is likely due to insufficient main memory."); // 処理に失敗しました。メインメモリが不足している可能性が高いです。

  return "";
}
