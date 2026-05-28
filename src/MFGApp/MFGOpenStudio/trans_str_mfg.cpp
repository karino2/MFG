/* -*- coding: utf-8 -*- マルチバイト */

// Copyright 2026 PGN Inc. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#if defined(_MSC_VER)
  #pragma execution_character_set("utf-8")
#endif

#include <trans.h>
#include "trans_str_mfg.h"


///////////////////////////////////////////////////////////////////////////
// - MFGStudioで使われる文字列(dialog_filter_mfg.cppでも使われてる)
// - The string used in the MFGStudio. (Also used in dialog_filter_mfg.cpp)
///////////////////////////////////////////////////////////////////////////
QString Trans_StrMFG( int idx )
{
  CTranslationManager* man = &Trans();

  //////////////////////////////////
  //////////////////////////////////
  if (man->TranslateJapanese())
  {
    if (idx == 0) return QObject::tr("プロジェクト新規作成...");
    if (idx == 1) return QObject::tr("プロジェクトディレクトリがすでに存在しています。");
    if (idx == 2) return QObject::tr("プロジェクトの新規作成に失敗しました。");
    if (idx == 3) return QObject::tr("プロジェクト名");
    if (idx == 4) return QObject::tr("場所");
    if (idx == 5) return QObject::tr("プロジェクトの新規作成");
    if (idx == 6) return QObject::tr("スクリプトの保存");
    if (idx == 7) return QObject::tr("(新規ファイル)");
    if (idx == 8) return QObject::tr("閉じる前に保存しますか？");
    if (idx == 9) return QObject::tr("デバッグ");
    if (idx == 10) return QObject::tr("プロジェクトを開く...");
    if (idx == 11) return QObject::tr("選択されたディレクトリはmfgファイルを含んでいません。");
    if (idx == 12) return QObject::tr("最近使ったプロジェクト(&P)");
    if (idx == 13) return QObject::tr("プロジェクトを閉じる");
    if (idx == 15) return QObject::tr("MFGファイルを開く...");
    if (idx == 16) return QObject::tr("MFGファイルを保存");
    if (idx == 17) return QObject::tr("MFGファイルを名前をつけて保存...");
    if (idx == 19) return QObject::tr("実行");
    if (idx == 20) return QObject::tr("プロジェクト");
    if (idx == 21) return QObject::tr("MARにパック");
    if (idx == 22) return QObject::tr("以下の場所にMARを生成しました: ");
    if (idx == 23) return QObject::tr("プレビュー");
    if (idx == 24) return QObject::tr("コピー");
    if (idx == 25) return QObject::tr("キャンバスを初期状態に戻す");
    if (idx == 26) return QObject::tr("クリア");
    if (idx == 27) return QObject::tr("出力");
    if (idx == 28) return QObject::tr("テンプレート");
    if (idx == 29) return QObject::tr("空のフィルタ");
    if (idx == 46) return QObject::tr("この操作の前には現在のプロジェクトを閉じなくてはいけません。\n\n現在のプロジェクトを閉じますか？");
    if (idx == 47) return QObject::tr("サンプルのプロジェクトを作る事が出来ませんでした。");
    if (idx == 48) return QObject::tr("サンプル集を開く");
    if (idx == 49) return QObject::tr("MFGStudioの新しいバージョンがあります。\n\nダウンロードページを開きますか？");
    if (idx == 50) return QObject::tr("お使いのMFGStudioは最新です。");
    if (idx == 51) return QObject::tr("更新の確認...");
    if (idx == 52) return QObject::tr("以下のMARの作成に失敗しました: ");
    if (idx == 53) return QObject::tr("ここに作成");
    if (idx == 54) return QObject::tr("画像ファイルのコピーに失敗しました: ");
    if (idx == 55) return QObject::tr("strings.jsonファイルの作成二失敗しました。");
    if (idx == 56) return QObject::tr("ファイルを閉じる(&C)");
    if (idx == 57) return QObject::tr("乱数の更新");
    if (idx == 58) return QObject::tr("MFGのフィルタ一覧");
    if (idx == 59) return QObject::tr("Ver:");
    if (idx == 60) return QObject::tr("追加日:");
    if (idx == 61) return QObject::tr("現在のプレビューをサムネイルにインポート");
    if (idx == 62) return QObject::tr("現在のプレビューをthumbnail.pngとしてインポートしました");
    if (idx == 63) return QObject::tr("thumbnail.pngとしての保存に失敗しました");
  }

  //////////////////////////////////
  //////////////////////////////////
  if (man->TranslateChineseSimp())
  {
  }

  //////////////////////////////////
  //////////////////////////////////
  if (man->TranslateChineseTrad())
  {
  }

  //////////////////////////////////
  //////////////////////////////////
  if (man->TranslateKorean())
  {
  }

  //////////////////////////////////
  //////////////////////////////////
  if (man->TranslatePortugues())
  {
  }

  //////////////////////////////////
  //////////////////////////////////
  if (man->TranslateSpanish())
  {
  }

  //////////////////////////////////
  //////////////////////////////////
  if (man->TranslateGerman())
  {
  }

  //////////////////////////////////
  //////////////////////////////////
  if (man->TranslateFrench())
  {
  }

  //////////////////////////////////
  //////////////////////////////////
  if (man->TranslateRussian())
  {
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
  if (idx == 0) return QObject::tr("New Project..."); // プロジェクト新規作成...
  if (idx == 1) return QObject::tr("Project dir already exists."); // プロジェクトディレクトリがすでに存在しています。
  if (idx == 2) return QObject::tr("Fail to create project."); // プロジェクトの新規作成に失敗しました。
  if (idx == 3) return QObject::tr("Project Name"); // "プロジェクト名"
  if (idx == 4) return QObject::tr("Directory"); // "場所" プロジェクト作成時の起点となるディレクトリを表すラベル。
  if (idx == 5) return QObject::tr("New Project"); // "プロジェクトの新規作成"
  if (idx == 6) return QObject::tr("Save Script"); // "スクリプトの保存"
  if (idx == 7) return QObject::tr("(Untitled)"); // "(新規ファイル)"
  if (idx == 8) return QObject::tr("Save data before closing?"); // "閉じる前に保存しますか？"
  if (idx == 9) return QObject::tr("Debug"); // "デバッグ"
  if (idx == 10) return QObject::tr("Open Project..."); // "プロジェクトを開く..."
  if (idx == 11) return QObject::tr("Selected directory does not contain MFG file."); // "選択されたディレクトリはmfgファイルを含んでいません。"
  if (idx == 12) return QObject::tr("Open Recent Project(&P)"); // 最近使ったプロジェクト(&P)
  if (idx == 13) return QObject::tr("Close Project"); // プロジェクトを閉じる
  // not used anymore. if (idx == 14) return QObject::tr("New MFG File..."); "MFGファイルを新規作成..."
  if (idx == 15) return QObject::tr("Open MFG File..."); // MFGファイルを開く...
  if (idx == 16) return QObject::tr("Save MFG File"); // MFGファイルを保存
  if (idx == 17) return QObject::tr("Save MFG File As..."); // MFGファイルを名前をつけて保存...
  // not used anymore. if (idx == 18) return QObject::tr("Close MFG File"); // MFGファイルを閉じる
  if (idx == 19) return QObject::tr("Run"); // "実行"
  if (idx == 20) return QObject::tr("Project"); // プロジェクト
  if (idx == 21) return QObject::tr("Pack to MAR"); // MARにパック
  if (idx == 22) return QObject::tr("MAR generated: "); // MARを生成しました: (このあとにフルパスが追加される)
  if (idx == 23) return QObject::tr("Preview"); // "プレビュー"
  if (idx == 24) return QObject::tr("Copy"); // "コピー"
  if (idx == 25) return QObject::tr("Revert Canvas"); // "キャンバスを初期状態に戻す"
  if (idx == 26) return QObject::tr("Clear"); // "クリア"
  if (idx == 27) return QObject::tr("Output"); // "出力"
  if (idx == 28) return QObject::tr("Template"); // "テンプレート"
  if (idx == 29) return QObject::tr("Blank Filter"); // "空のフィルタ"
  // not used anymore. if (idx == 30) return QObject::tr("Mosaic"); // "モザイク"
  // not used anymore. if (idx == 31) return QObject::tr(""); // "クリスタライズ"
  // not used anymore. if (idx == 32) return QObject::tr(""); // "虫眼鏡"
  // not used anymore. if (idx == 33) return QObject::tr(""); // "ブルーム"
  // not used anymore. if (idx == 34) return QObject::tr(""); // "球面射影"
  // not used anymore. if (idx == 35) return QObject::tr(""); // "ジグザグ"
  // not used anymore. if (idx == 36) return QObject::tr(""); // "パーリンノイズ"
  // not used anymore. if (idx == 37) return QObject::tr(""); // "暗視スコープ"
  // not used anymore. if (idx == 38) return QObject::tr(""); // "Grainフィルタ"
  // not used anymore. if (idx == 39) return QObject::tr(""); // "RGずらし"
  // not used anymore. if (idx == 40) return QObject::tr(""); // "領域ずらし"
  // not used anymore. if (idx == 41) return QObject::tr(""); // "繰り返し"
  // not used anymore. if (idx == 42) return QObject::tr("Water Mirror"); // "水鏡"
  // not used anymore. if (idx == 43) return QObject::tr("Motion Blur"); // "移動ぼかし"
  // not used anymore.  if (idx == 44) return QObject::tr("Denoise"); // "ノイズ除去"
  // not used anymore. if (idx == 45) return QObject::tr("Antialias"); // "アンチエイリアス"
  if (idx == 46) return QObject::tr("Before doing this action, you must close current project.\n\nDo you want to close current project?"); // "この操作の前には現在のプロジェクトを閉じなくてはいけません。\n\n現在のプロジェクトを閉じますか？"
  if (idx == 47) return QObject::tr("Fail to create example project."); // "サンプルのプロジェクトを作る事が出来ませんでした。"
  if (idx == 48) return QObject::tr("Open Examples Project"); // "サンプル集を開く"
  if (idx == 49) return QObject::tr("Newer version of MFGStudio is available.\n\nDo you want to go to page?"); // "新しいバージョンがあります。\nダウンロードページを開きますか？"
  if (idx == 50) return QObject::tr("Your MFGStudio is latest."); // お使いのバージョンは最新です。
  if (idx == 51) return QObject::tr("Check Update..."); // 更新の確認...
  if (idx == 52) return QObject::tr("Fail to create MAR: ");
  if (idx == 53) return QObject::tr("Create Here");
  if (idx == 54) return QObject::tr("Fail to copy image file: ");
  if (idx == 55) return QObject::tr("Fail to create strings.json file.");
  if (idx == 56) return QObject::tr("Close File(&C)");
  if (idx == 57) return QObject::tr("Update seed"); // 乱数の更新
  if (idx == 58) return QObject::tr("MFG Filters"); // MFGのフィルタ一覧
  if (idx == 59) return QObject::tr("Ver:"); // バージョン:
  if (idx == 60) return QObject::tr("Added:"); // 追加日:
  if (idx == 61) return QObject::tr("Import current preview as thumbnail."); //現在のプレビューをサムネイルにインポート
  if (idx == 62) return QObject::tr("Imported current preview as thumbnail.png."); // 現在のプレビューをthumbnail.pngとしてインポートしました
  if (idx == 63) return QObject::tr("Fail to save as thumbnail.png."); // thumbnail.pngとしての保存に失敗しました

  return "";
}
