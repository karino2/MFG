/* -*- coding: utf-8 -*- マルチバイト */

// Copyright 2026 PGN Inc. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#if defined(_MSC_VER)
  #pragma execution_character_set("utf-8")
#endif

#include <trans.h>
#include "trans_str_filter.h"

///////////////////////////////////////////////////////////////////////////
// - 各種フィルタダイアログで使用される文字列
// - Strings used in various filter dialogs.
///////////////////////////////////////////////////////////////////////////
QString Trans_StrFilter( int idx )
{
  CTranslationManager* man = &Trans();

  // カテゴリー
  const int cat = idx / 100;

  /////////////////////////////
  /////////////////////////////
  // 日本語
  /////////////////////////////
  /////////////////////////////
  if (man->TranslateJapanese())
  {
    // レベル補正・線画抽出
    if (cat == 0)
    {
      if (idx == 0) return QObject::tr("入力");
      if (idx == 1) return QObject::tr("出力");
      if (idx == 2) return QObject::tr("レベル補正");
      if (idx == 3) return QObject::tr("線画抽出");
    }

    // 色相
    if (cat == 1)
    {
      if (idx == 100) return QObject::tr("色相");
      if (idx == 101) return QObject::tr("彩度");
      if (idx == 102) return QObject::tr("明度");
      if (idx == 103) return QObject::tr("色相・彩度・明度");
      if (idx == 104) return QObject::tr("色空間");
    }

    // トーンカーブ
    if (cat == 2)
    {
      if (idx == 200) return QObject::tr("トーンカーブ");
      if (idx == 201) return QObject::tr("リセット");
    }

    // チャンネル操作
    if (cat == 3)
    {
      if (idx == 300) return QObject::tr("チャンネル操作");
      if (idx == 301) return QObject::tr("RGB平均");
      if (idx == 302) return QObject::tr("255 - RGB平均");
      if (idx == 303) return QObject::tr("リセット");
      if (idx == 304) return QObject::tr("プリセット");
      if (idx == 310) return QObject::tr("R成分");
      if (idx == 311) return QObject::tr("G成分");
      if (idx == 312) return QObject::tr("B成分");
      if (idx == 313) return QObject::tr("線画抽出");
      if (idx == 314) return QObject::tr("モノクロ化");
    }

    // 色収差
    if (cat == 4)
    {
      if (idx == 400) return QObject::tr("色収差/RGBずらし");
    }

    // アンシャープマスク
    if (cat == 5)
    {
      if (idx == 500) return QObject::tr("アンシャープマスク");
      if (idx == 501) return QObject::tr("半径");
      if (idx == 502) return QObject::tr("適用量");
      if (idx == 503) return QObject::tr("プレビュー");
    }

    // ガウスぼかし
    if (cat == 6)
    {
      if (idx == 600) return QObject::tr("ガウスぼかし");
      if (idx == 601) return QObject::tr("値");
      if (idx == 602) return QObject::tr("プレビュー");
    }

    // 移動ぼかし
    if (cat == 7)
    {
      if (idx == 700) return QObject::tr("移動ぼかし");
      if (idx == 701) return QObject::tr("強度");
      if (idx == 702) return QObject::tr("プレビュー");
    }

    // レンズぼかし
    if (cat == 8)
    {
      if (idx == 800) return QObject::tr("レンズぼかし");
      if (idx == 801) return QObject::tr("半径");
      if (idx == 802) return QObject::tr("プレビュー");
      if (idx == 803) return QObject::tr("明るさ");
    }

    // モザイク
    if (cat == 9)
    {
      if (idx == 900) return QObject::tr("モザイク");
      if (idx == 901) return QObject::tr("サイズ");
    }

    // 和柄
    if (cat == 10)
    {
      if (idx == 1000) return QObject::tr("青海波");
      if (idx == 1001) return QObject::tr("菱青海波");
      if (idx == 1002) return QObject::tr("七宝");
      if (idx == 1003) return QObject::tr("七宝 2");
      if (idx == 1004) return QObject::tr("算木崩し");
      if (idx == 1005) return QObject::tr("矢絣");
      if (idx == 1006) return QObject::tr("矢絣 2");
      if (idx == 1007) return QObject::tr("鱗");
      if (idx == 1008) return QObject::tr("千鳥格子");
      if (idx == 1009) return QObject::tr("水玉");
      if (idx == 1010) return QObject::tr("摺疋田");
      if (idx == 1011) return QObject::tr("亀甲");
      if (idx == 1012) return QObject::tr("市松");
      if (idx == 1013) return QObject::tr("麻の葉");
      if (idx == 1014) return QObject::tr("立涌");
      if (idx == 1015) return QObject::tr("分銅繋ぎ");
      if (idx == 1016) return QObject::tr("紗綾形");
      if (idx == 1017) return QObject::tr("縦縞");
      if (idx == 1018) return QObject::tr("横縞");

      if (idx == 1050) return QObject::tr("和柄");
      if (idx == 1051) return QObject::tr("名称");
      if (idx == 1052) return QObject::tr("サイズ");
      if (idx == 1053) return QObject::tr("透明背景");
      if (idx == 1054) return QObject::tr("色の入れ替え");
      if (idx == 1055) return QObject::tr("ランダム色指定");
      if (idx == 1056) return QObject::tr("密度");
      if (idx == 1057) return QObject::tr("角度");
      if (idx == 1058) return QObject::tr("度");
    }

    // 集中線
    if (cat == 11)
    {
      if (idx == 1100) return QObject::tr("集中線");
      if (idx == 1101) return QObject::tr("長さ");
      if (idx == 1102) return QObject::tr("長さ (ランダム)");
      if (idx == 1103) return QObject::tr("線の太さ");
      if (idx == 1104) return QObject::tr("密度");
      if (idx == 1105) return QObject::tr("密度 (ランダム)");
      if (idx == 1106) return QObject::tr("縦横の比率");
      if (idx == 1107) return QObject::tr("正方形");
      if (idx == 1108) return QObject::tr("横長");
      if (idx == 1109) return QObject::tr("縦長");
      if (idx == 1110) return QObject::tr("リセット");
      if (idx == 1111) return QObject::tr("ランダム値の更新");
      if (idx == 1112) return QObject::tr("中心の指定");
    }

    // 流線
    if (cat == 12)
    {
      if (idx == 1200) return QObject::tr("流線");
      if (idx == 1201) return QObject::tr("長さ");
      if (idx == 1202) return QObject::tr("長さ (ランダム)");
      if (idx == 1203) return QObject::tr("線の太さ");
      if (idx == 1204) return QObject::tr("密度");
      if (idx == 1205) return QObject::tr("密度 (ランダム)");
      if (idx == 1206) return QObject::tr("リセット");
      if (idx == 1207) return QObject::tr("ランダム値の更新");
      if (idx == 1208) return QObject::tr("方向の指定");
    }

    // ウニフラッシュ
    if (cat == 13)
    {
      if (idx == 1300) return QObject::tr("ウニフラッシュ");
      if (idx == 1301) return QObject::tr("長さ");
      if (idx == 1302) return QObject::tr("長さ (ランダム)");
      if (idx == 1303) return QObject::tr("線の太さ");
      if (idx == 1304) return QObject::tr("密度");
      if (idx == 1305) return QObject::tr("密度 (ランダム)");
      if (idx == 1306) return QObject::tr("オフセット (ランダム)");
      if (idx == 1307) return QObject::tr("入り抜き位置");
      if (idx == 1308) return QObject::tr("リセット");
      if (idx == 1309) return QObject::tr("ランダム値の更新");
    }

    // 渦巻き
    if (cat == 14)
    {
      if (idx == 1400) return QObject::tr("渦巻き");
    }

    // 渦巻き
    if (cat == 15)
    {
      if (idx == 1500) return QObject::tr("波");
    }

    // ノイズ
    if (cat == 16)
    {
      if (idx == 1600) return QObject::tr("ノイズ");
      if (idx == 1601) return QObject::tr("強さ");
      if (idx == 1602) return QObject::tr("タイプ");
      if (idx == 1603) return QObject::tr("カラー");
      if (idx == 1604) return QObject::tr("モノクロ");
    }

    // 明るさ・コントラスト
    if (cat == 17)
    {
      if (idx == 1700) return QObject::tr("明るさ・コントラスト");
      if (idx == 1701) return QObject::tr("明るさ");
      if (idx == 1702) return QObject::tr("コントラスト");
      if (idx == 1703) return QObject::tr("プレビュー");
    }

    // ポスタリゼーション
    if (cat == 18)
    {
      if (idx == 1800) return QObject::tr("ポスタリゼーション");
      if (idx == 1801) return QObject::tr("段数");
    }

    // 整列
    if (cat == 19)
    {
      if (idx == 1900) return QObject::tr("選択レイヤーの整列");

      if (idx == 1910) return QObject::tr("縦に整列 (左端)");
      if (idx == 1911) return QObject::tr("縦に整列 (中央)");
      if (idx == 1912) return QObject::tr("縦に整列 (右端)");
      if (idx == 1913) return QObject::tr("横に整列 (上端)");
      if (idx == 1914) return QObject::tr("横に整列 (中央)");
      if (idx == 1915) return QObject::tr("横に整列 (下端)");

      if (idx == 1920) return QObject::tr("画像の端で揃える");
      if (idx == 1921) return QObject::tr("画像の中心で揃える");

      if (idx == 1930) return QObject::tr("選択レイヤーの範囲を基準にする");
      if (idx == 1931) return QObject::tr("選択範囲を基準にする");
    }
  }

  /////////////////////////////
  /////////////////////////////
  // 中国語 (簡体字)
  /////////////////////////////
  /////////////////////////////
  if (man->TranslateChineseSimp())
  {
    // レベル補正・線画抽出
    if (cat == 0)
    {
      if (idx == 0) return QObject::tr("输入");
      if (idx == 1) return QObject::tr("输出");
      if (idx == 2) return QObject::tr("色阶");
      if (idx == 3) return QObject::tr("提取线稿");
    }

    // 色相
    if (cat == 1)
    {
      if (idx == 100) return QObject::tr("色相");
      if (idx == 101) return QObject::tr("彩度");
      if (idx == 102) return QObject::tr("明度");
      if (idx == 103) return QObject::tr("色相 / 彩度 / 明度");
      if (idx == 104) return QObject::tr("色彩空间"); // 色空間
    }
        // トーンカーブ
    if (cat == 2)
    {
      if (idx == 200) return QObject::tr("色调曲线");
      if (idx == 201) return QObject::tr("重置");
    }

    // チャンネル操作
    if (cat == 3)
    {
      if (idx == 300) return QObject::tr("操作频道");
      if (idx == 301) return QObject::tr("平均RGB");
      if (idx == 302) return QObject::tr("255 - 平均RGB");
      if (idx == 303) return QObject::tr("重置");
      if (idx == 304) return QObject::tr("默认");
      if (idx == 310) return QObject::tr("R成分");
      if (idx == 311) return QObject::tr("G成分");
      if (idx == 312) return QObject::tr("B成分");
      if (idx == 313) return QObject::tr("抽出线稿");
      if (idx == 314) return QObject::tr("黑白化");
    }

    // 色収差
    if (cat == 4)
    {
      if (idx == 400) return QObject::tr("色差");
    }

    // アンシャープマスク
    if (cat == 5)
    {
      if (idx == 500) return QObject::tr("非鋭化滤镜");
      if (idx == 501) return QObject::tr("半径");
      if (idx == 502) return QObject::tr("量");
      if (idx == 503) return QObject::tr("预览");
    }

    if (cat == 6)
    {
      if (idx == 600) return QObject::tr("高斯模糊");
      if (idx == 601) return QObject::tr("值");
      if (idx == 602) return QObject::tr("预览"); // プレビュー
    }

    // 移動ぼかし
    if (cat == 7)
    {
      if (idx == 700) return QObject::tr("移动模糊");
      if (idx == 701) return QObject::tr("强度");
      if (idx == 702) return QObject::tr("预览");
    }

    // レンズぼかし
    if (cat == 8)
    {
      if (idx == 800) return QObject::tr("镜头模糊处理");
      if (idx == 801) return QObject::tr("半径");
      if (idx == 802) return QObject::tr("预览");
      if (idx == 803) return QObject::tr("明亮度");
    }

    if (cat == 9)
    {
      if (idx == 900) return QObject::tr("马赛克");
      if (idx == 901) return QObject::tr("尺寸");
    }

    if (cat == 10)
    {
      if (idx == 1000) return QObject::tr("松青海波");
      if (idx == 1001) return QObject::tr("菱松青海波");
      if (idx == 1002) return QObject::tr("景泰蓝");
      if (idx == 1003) return QObject::tr("景泰蓝2");
      if (idx == 1004) return QObject::tr("算木崩");
      if (idx == 1005) return QObject::tr("矢羽根");
      if (idx == 1006) return QObject::tr("矢羽根2");
      if (idx == 1007) return QObject::tr("鳞");
      if (idx == 1008) return QObject::tr("千鸟格");
      if (idx == 1009) return QObject::tr("圆点");
      if (idx == 1010) return QObject::tr("折疋田");
      if (idx == 1011) return QObject::tr("龟甲");
      if (idx == 1012) return QObject::tr("市松");
      if (idx == 1013) return QObject::tr("麻叶");
      if (idx == 1014) return QObject::tr("云立涌");
      if (idx == 1015) return QObject::tr("砝码");
      if (idx == 1016) return QObject::tr("纱绫形");
      if (idx == 1017) return QObject::tr("固定纵横比例");
      if (idx == 1018) return QObject::tr("横线");

      if (idx == 1050) return QObject::tr("日式花纹");
      if (idx == 1051) return QObject::tr("名称");
      if (idx == 1052) return QObject::tr("尺寸");
      if (idx == 1053) return QObject::tr("透明背景");
      if (idx == 1054) return QObject::tr("切换颜色");
      if (idx == 1055) return QObject::tr("随机颜色");
      if (idx == 1056) return QObject::tr("密度");
      if (idx == 1057) return QObject::tr("角度");
      if (idx == 1058) return QObject::tr("度");
    }

    // 集中線
    if (cat == 11)
    {
      if (idx == 1100) return QObject::tr("集中线");
      if (idx == 1101) return QObject::tr("长度");
      if (idx == 1102) return QObject::tr("长度 (随机)");
      if (idx == 1103) return QObject::tr("线的宽度");
      if (idx == 1104) return QObject::tr("密度");
      if (idx == 1105) return QObject::tr("密度 (随机)");
      if (idx == 1106) return QObject::tr("长宽比例");
      if (idx == 1107) return QObject::tr("正方形");
      if (idx == 1108) return QObject::tr("宽长");
      if (idx == 1109) return QObject::tr("纵长");
      if (idx == 1110) return QObject::tr("重置");
      if (idx == 1111) return QObject::tr("更新随机值");
      if (idx == 1112) return QObject::tr("指定中心");
    }

    // 流線
    if (cat == 12)
    {
      if (idx == 1200) return QObject::tr("流线");
      if (idx == 1201) return QObject::tr("长度");
      if (idx == 1202) return QObject::tr("长度 (随机)");
      if (idx == 1203) return QObject::tr("线的宽度");
      if (idx == 1204) return QObject::tr("密度");
      if (idx == 1205) return QObject::tr("密度 (随机)");
      if (idx == 1206) return QObject::tr("重置");
      if (idx == 1207) return QObject::tr("更新随机值");
      if (idx == 1208) return QObject::tr("指定方向");
    }

    // ウニフラッシュ
    if (cat == 13)
    {
      if (idx == 1300) return QObject::tr("集中线");
      if (idx == 1301) return QObject::tr("长度");
      if (idx == 1302) return QObject::tr("长度 (随机)");
      if (idx == 1303) return QObject::tr("线的宽度");
      if (idx == 1304) return QObject::tr("密度");
      if (idx == 1305) return QObject::tr("密度 (随机)");
      if (idx == 1306) return QObject::tr("抵销 (随机)");
      if (idx == 1307) return QObject::tr("淡入淡出的位置");
      if (idx == 1308) return QObject::tr("重置");
      if (idx == 1309) return QObject::tr("更新随机值");
    }

    // 渦巻き
    if (cat == 14)
    {
      if (idx == 1400) return QObject::tr("漩涡");
    }

    // 渦巻き
    if (cat == 15)
    {
      if (idx == 1500) return QObject::tr("波浪");
    }

    // ノイズ
    if (cat == 16)
    {
      if (idx == 1600) return QObject::tr("噪音");
      if (idx == 1601) return QObject::tr("强度");
      if (idx == 1602) return QObject::tr("类型");
      if (idx == 1603) return QObject::tr("色彩");
      if (idx == 1604) return QObject::tr("黑白");
    }

    // 明るさ・コントラスト
    if (cat == 17)
    {
      if (idx == 1700) return QObject::tr("明亮度 / 对比");
      if (idx == 1701) return QObject::tr("明亮度");
      if (idx == 1702) return QObject::tr("对比");
      if (idx == 1703) return QObject::tr("预览");
    }

    // ポスタリゼーション
    if (cat == 18)
    {
      if (idx == 1800) return QObject::tr("色调分离");
      if (idx == 1801) return QObject::tr("级数");
    }

    // 整列
    if (cat == 19)
    {
      if (idx == 1900) return QObject::tr("对齐选定图层"); // 選択レイヤーの整列

      if (idx == 1910) return QObject::tr("垂直对齐 (左端)"); // 縦に整列 (左端)
      if (idx == 1911) return QObject::tr("垂直对齐 (居中)"); // 縦に整列 (中央)
      if (idx == 1912) return QObject::tr("垂直对齐 (右端)"); // 縦に整列 (右端)
      if (idx == 1913) return QObject::tr("水平对齐 (上端)"); // 横に整列 (上端)
      if (idx == 1914) return QObject::tr("水平对齐 (居中)"); // 横に整列 (中央)
      if (idx == 1915) return QObject::tr("水平对齐 (下端)"); // 横に整列 (下端)

      if (idx == 1920) return QObject::tr("与图像边缘对齐"); // 画像の端で揃える
      if (idx == 1921) return QObject::tr("与图像中心对齐"); // 画像の中心で揃える

      if (idx == 1930) return QObject::tr("以选定图层的范围为基准"); // 選択レイヤーの範囲を基準にする
      if (idx == 1931) return QObject::tr("以选区为基准"); // 選択範囲を基準にする
    }
  }


  /////////////////////////////
  /////////////////////////////
  // 中国語 (繁体字)
  /////////////////////////////
  /////////////////////////////
  if (man->TranslateChineseTrad())
  {
    // レベル補正・線画抽出
    if (cat == 0)
    {
      if (idx == 0) return QObject::tr("輸入");
      if (idx == 1) return QObject::tr("輸出");
      if (idx == 2) return QObject::tr("色階");
      if (idx == 3) return QObject::tr("提取線稿");
    }

    // 色相
    if (cat == 1)
    {
      if (idx == 100) return QObject::tr("色相");
      if (idx == 101) return QObject::tr("彩度");
      if (idx == 102) return QObject::tr("明度");
      if (idx == 103) return QObject::tr("色相 / 彩度 / 明度");
      if (idx == 104) return QObject::tr("色彩空間"); // 色空間
    }

    // トーンカーブ
    if (cat == 2)
    {
      if (idx == 200) return QObject::tr("色調曲線");
      if (idx == 201) return QObject::tr("重置");
    }

    // チャンネル操作
    if (cat == 3)
    {
      if (idx == 300) return QObject::tr("操作頻道");
      if (idx == 301) return QObject::tr("平均RGB");
      if (idx == 302) return QObject::tr("255 - 平均RGB");
      if (idx == 303) return QObject::tr("重置");
      if (idx == 304) return QObject::tr("預設");
      if (idx == 310) return QObject::tr("R成分");
      if (idx == 311) return QObject::tr("G成分");
      if (idx == 312) return QObject::tr("B成分");
      if (idx == 313) return QObject::tr("抽出線稿");
      if (idx == 314) return QObject::tr("黑白化");
    }

    // 色収差
    if (cat == 4)
    {
      if (idx == 400) return QObject::tr("色差");
    }

    // アンシャープマスク
    if (cat == 5)
    {
      if (idx == 500) return QObject::tr("非鋭化濾鏡");
      if (idx == 501) return QObject::tr("半徑");
      if (idx == 502) return QObject::tr("量");
      if (idx == 503) return QObject::tr("預覽");
    }

    if (cat == 6)
    {
      if (idx == 600) return QObject::tr("高斯模糊");
      if (idx == 601) return QObject::tr("值");
      if (idx == 602) return QObject::tr("預覽"); // プレビュー
    }
    // 移動ぼかし
    if (cat == 7)
    {
      if (idx == 700) return QObject::tr("移動模糊");
      if (idx == 701) return QObject::tr("強度");
      if (idx == 702) return QObject::tr("預覽");
    }

    // レンズぼかし
    if (cat == 8)
    {
      if (idx == 800) return QObject::tr("鏡頭模糊處理");
      if (idx == 801) return QObject::tr("半徑");
      if (idx == 802) return QObject::tr("預覽");
      if (idx == 803) return QObject::tr("明亮度");
    }

    if (cat == 9)
    {
      if (idx == 900) return QObject::tr("馬賽克");
      if (idx == 901) return QObject::tr("尺寸");
    }

    // 和柄
    if (cat == 10)
    {
      if (idx == 1000) return QObject::tr("松青海波");
      if (idx == 1001) return QObject::tr("菱松青海波");
      if (idx == 1002) return QObject::tr("景泰藍");
      if (idx == 1003) return QObject::tr("景泰藍2");
      if (idx == 1004) return QObject::tr("算木崩");
      if (idx == 1005) return QObject::tr("矢羽根");
      if (idx == 1006) return QObject::tr("矢羽根2");
      if (idx == 1007) return QObject::tr("鱗");
      if (idx == 1008) return QObject::tr("千鳥格");
      if (idx == 1009) return QObject::tr("圓點");
      if (idx == 1010) return QObject::tr("摺疋田");
      if (idx == 1011) return QObject::tr("龜甲");
      if (idx == 1012) return QObject::tr("市松");
      if (idx == 1013) return QObject::tr("麻葉");
      if (idx == 1014) return QObject::tr("雲立涌");
      if (idx == 1015) return QObject::tr("砝碼");
      if (idx == 1016) return QObject::tr("紗綾形");
      if (idx == 1017) return QObject::tr("固定縱橫比例");
      if (idx == 1018) return QObject::tr("橫線");

      if (idx == 1050) return QObject::tr("日式花紋");
      if (idx == 1051) return QObject::tr("名稱");
      if (idx == 1052) return QObject::tr("尺寸");
      if (idx == 1053) return QObject::tr("透明背景");
      if (idx == 1054) return QObject::tr("切換顏色");
      if (idx == 1055) return QObject::tr("隨機顏色");
      if (idx == 1056) return QObject::tr("密度");
      if (idx == 1057) return QObject::tr("角度");
      if (idx == 1058) return QObject::tr("度");
    }

    // 集中線
    if (cat == 11)
    {
      if (idx == 1100) return QObject::tr("集中線");
      if (idx == 1101) return QObject::tr("長度");
      if (idx == 1102) return QObject::tr("長度 (隨機)");
      if (idx == 1103) return QObject::tr("線的寬度");
      if (idx == 1104) return QObject::tr("密度");
      if (idx == 1105) return QObject::tr("密度 (隨機)");
      if (idx == 1106) return QObject::tr("長寬比例");
      if (idx == 1107) return QObject::tr("正方形");
      if (idx == 1108) return QObject::tr("寬長");
      if (idx == 1109) return QObject::tr("縱長");
      if (idx == 1110) return QObject::tr("重置");
      if (idx == 1111) return QObject::tr("更新隨機值");
      if (idx == 1112) return QObject::tr("指定中心");
    }

    // 流線
    if (cat == 12)
    {
      if (idx == 1200) return QObject::tr("流線");
      if (idx == 1201) return QObject::tr("長度");
      if (idx == 1202) return QObject::tr("長度 (隨機)");
      if (idx == 1203) return QObject::tr("線的寬度");
      if (idx == 1204) return QObject::tr("密度");
      if (idx == 1205) return QObject::tr("密度 (隨機)");
      if (idx == 1206) return QObject::tr("重置");
      if (idx == 1207) return QObject::tr("更新隨機值");
      if (idx == 1208) return QObject::tr("指定方向");
    }

    // ウニフラッシュ
    if (cat == 13)
    {
      if (idx == 1300) return QObject::tr("集中線");
      if (idx == 1301) return QObject::tr("長度");
      if (idx == 1302) return QObject::tr("長度 (隨機)");
      if (idx == 1303) return QObject::tr("線的寬度");
      if (idx == 1304) return QObject::tr("密度");
      if (idx == 1305) return QObject::tr("密度 (隨機)");
      if (idx == 1306) return QObject::tr("抵銷 (隨機)");
      if (idx == 1307) return QObject::tr("淡入淡出的位置");
      if (idx == 1308) return QObject::tr("重置");
      if (idx == 1309) return QObject::tr("更新隨機值");
    }

    // 渦巻き
    if (cat == 14)
    {
      if (idx == 1400) return QObject::tr("漩渦");
    }

    // 渦巻き
    if (cat == 15)
    {
      if (idx == 1500) return QObject::tr("波浪");
    }

    // ノイズ
    if (cat == 16)
    {
      if (idx == 1600) return QObject::tr("噪音");
      if (idx == 1601) return QObject::tr("強度");
      if (idx == 1602) return QObject::tr("類型");
      if (idx == 1603) return QObject::tr("色彩");
      if (idx == 1604) return QObject::tr("黑白");
    }

    // 明るさ・コントラスト
    if (cat == 17)
    {
      if (idx == 1700) return QObject::tr("明亮度 / 對比");
      if (idx == 1701) return QObject::tr("明亮度");
      if (idx == 1702) return QObject::tr("對比");
      if (idx == 1703) return QObject::tr("預覽");
    }

    // ポスタリゼーション
    if (cat == 18)
    {
      if (idx == 1800) return QObject::tr("色調分離");
      if (idx == 1801) return QObject::tr("級數");
    }

    // 整列
    if (cat == 19)
    {
      if (idx == 1900) return QObject::tr("對齊選定圖層"); // 選択レイヤーの整列

      if (idx == 1910) return QObject::tr("垂直對齊 (左端)"); // 縦に整列 (左端)
      if (idx == 1911) return QObject::tr("垂直對齊 (居中)"); // 縦に整列 (中央)
      if (idx == 1912) return QObject::tr("垂直對齊 (右端)"); // 縦に整列 (右端)
      if (idx == 1913) return QObject::tr("水平對齊 (上端)"); // 横に整列 (上端)
      if (idx == 1914) return QObject::tr("水平對齊 (居中)"); // 横に整列 (中央)
      if (idx == 1915) return QObject::tr("水平對齊 (下端)"); // 横に整列 (下端)

      if (idx == 1920) return QObject::tr("與圖像邊緣對齊"); // 画像の端で揃える
      if (idx == 1921) return QObject::tr("與圖像中心對齊"); // 画像の中心で揃える

      if (idx == 1930) return QObject::tr("以選定圖層的範圍為基準"); // 選択レイヤーの範囲を基準にする
      if (idx == 1931) return QObject::tr("以選區為基準"); // 選択範囲を基準にする
    }
  }

  /////////////////////////////
  /////////////////////////////
  // 韓国語
  /////////////////////////////
  /////////////////////////////
  if (man->TranslateKorean())
  {
    // レベル補正・線画抽出
    if (cat == 0)
    {
      if (idx == 0) return QObject::tr("입력");
      if (idx == 1) return QObject::tr("출력");
      if (idx == 2) return QObject::tr("레벨 보정");
      if (idx == 3) return QObject::tr("선화 추출");
    }

    // 色相
    if (cat == 1)
    {
      if (idx == 100) return QObject::tr("색조");
      if (idx == 101) return QObject::tr("채도");
      if (idx == 102) return QObject::tr("명도");
      if (idx == 103) return QObject::tr("색조/채도/명도");
      if (idx == 104) return QObject::tr("색 공간"); // 色空間
    }

     // トーンカーブ
    if (cat == 2)
    {
      if (idx == 200) return QObject::tr("톤 커브");
      if (idx == 201) return QObject::tr("리셋");
    }

    // チャンネル操作
    if (cat == 3)
    {
      if (idx == 300) return QObject::tr("채널 조정");
      if (idx == 301) return QObject::tr("RGB 평균");
      if (idx == 302) return QObject::tr("255 - RGB 평균");
      if (idx == 303) return QObject::tr("리셋");
      if (idx == 304) return QObject::tr("프리셋");
      if (idx == 310) return QObject::tr("R 성분");
      if (idx == 311) return QObject::tr("G 성분");
      if (idx == 312) return QObject::tr("B 성분");
      if (idx == 313) return QObject::tr("선화 추출");
      if (idx == 314) return QObject::tr("흑백화");
    }

    // 色収差
    if (cat == 4)
    {
      if (idx == 400) return QObject::tr("색수차");
    }

    // アンシャープマスク
    if (cat == 5)
    {
      if (idx == 500) return QObject::tr("언샤프 마스크");
      if (idx == 501) return QObject::tr("반경");
      if (idx == 502) return QObject::tr("적용량");
      if (idx == 503) return QObject::tr("미리보기");
    }

    if (cat == 6)
    {
      if (idx == 600) return QObject::tr("가우시안 블러");
      if (idx == 601) return QObject::tr("값");
      if (idx == 602) return QObject::tr("미리보기"); // プレビュー
    }

    // 移動ぼかし
    if (cat == 7)
    {
      if (idx == 700) return QObject::tr("모션 블러");
      if (idx == 701) return QObject::tr("강도");
      if (idx == 702) return QObject::tr("미리보기");
    }

    // レンズぼかし
    if (cat == 8)
    {
      if (idx == 800) return QObject::tr("렌즈 블러");
      if (idx == 801) return QObject::tr("반경");
      if (idx == 802) return QObject::tr("미리보기");
      if (idx == 803) return QObject::tr("밝기");
    }

    if (cat == 9)
    {
      if (idx == 900) return QObject::tr("모자이크");
      if (idx == 901) return QObject::tr("사이즈");
    }

    // 和柄
    if (cat == 10)
    {
      if (idx == 1000) return QObject::tr("바다 (세이가이하)");
      if (idx == 1001) return QObject::tr("바다 (히시세이가이하)");
      if (idx == 1002) return QObject::tr("싯포");
      if (idx == 1003) return QObject::tr("싯포 2");
      if (idx == 1004) return QObject::tr("산키쿠즈시");
      if (idx == 1005) return QObject::tr("화살 (야가스리)");
      if (idx == 1006) return QObject::tr("화살 2 (야가스리)");
      if (idx == 1007) return QObject::tr("비늘 (우로코)");
      if (idx == 1008) return QObject::tr("하운드 투스");
      if (idx == 1009) return QObject::tr("물방울 무늬");
      if (idx == 1010) return QObject::tr("스리비타");
      if (idx == 1011) return QObject::tr("거북 등껍질 (킷코)");
      if (idx == 1012) return QObject::tr("체크 무늬");
      if (idx == 1013) return QObject::tr("아사노하");
      if (idx == 1014) return QObject::tr("타테와쿠");
      if (idx == 1015) return QObject::tr("훈도츠나기");
      if (idx == 1016) return QObject::tr("사야가타");
      if (idx == 1017) return QObject::tr("세로줄");
      if (idx == 1018) return QObject::tr("가로줄");

      if (idx == 1050) return QObject::tr("일본풍 패턴");
      if (idx == 1051) return QObject::tr("명칭");
      if (idx == 1052) return QObject::tr("사이즈");
      if (idx == 1053) return QObject::tr("투명색 배경");
      if (idx == 1054) return QObject::tr("색 변경");
      if (idx == 1055) return QObject::tr("랜덤 색 지정");
      if (idx == 1056) return QObject::tr("밀도");
      if (idx == 1057) return QObject::tr("각도");
      if (idx == 1058) return QObject::tr("도");
    }

    // 集中線
    if (cat == 11)
    {
      if (idx == 1100) return QObject::tr("집중선");
      if (idx == 1101) return QObject::tr("길이");
      if (idx == 1102) return QObject::tr("길이 (랜덤)");
      if (idx == 1103) return QObject::tr("두께");
      if (idx == 1104) return QObject::tr("밀도");
      if (idx == 1105) return QObject::tr("밀도 (랜덤)");
      if (idx == 1106) return QObject::tr("종횡비");
      if (idx == 1107) return QObject::tr("정사각형");
      if (idx == 1108) return QObject::tr("가로");
      if (idx == 1109) return QObject::tr("세로");
      if (idx == 1110) return QObject::tr("리셋");
      if (idx == 1111) return QObject::tr("랜덤 값으로 업데이트");
      if (idx == 1112) return QObject::tr("중심부 지정");
    }

    // 流線
    if (cat == 12)
    {
      if (idx == 1200) return QObject::tr("유선");
      if (idx == 1201) return QObject::tr("길이");
      if (idx == 1202) return QObject::tr("길이 (랜덤)");
      if (idx == 1203) return QObject::tr("두께");
      if (idx == 1204) return QObject::tr("밀도");
      if (idx == 1205) return QObject::tr("밀도 (랜덤)");
      if (idx == 1206) return QObject::tr("리셋");
      if (idx == 1207) return QObject::tr("랜덤 값으로 업데이트");
      if (idx == 1208) return QObject::tr("방향 지정");
    }

    // ウニフラッシュ
    if (cat == 13)
    {
      if (idx == 1300) return QObject::tr("성게 말풍선");
      if (idx == 1301) return QObject::tr("길이");
      if (idx == 1302) return QObject::tr("길이 (랜덤)");
      if (idx == 1303) return QObject::tr("두께");
      if (idx == 1304) return QObject::tr("밀도");
      if (idx == 1305) return QObject::tr("밀도 (랜덤)");
      if (idx == 1306) return QObject::tr("오프셋 (랜덤)");
      if (idx == 1307) return QObject::tr("페이드 인/아웃 위치");
      if (idx == 1308) return QObject::tr("리셋");
      if (idx == 1309) return QObject::tr("랜덤 값으로 업데이트");
    }

    // 渦巻き
    if (cat == 14)
    {
      if (idx == 1400) return QObject::tr("소용돌이");
    }

    // 渦巻き
    if (cat == 15)
    {
      if (idx == 1500) return QObject::tr("파도");
    }

    // ノイズ
    if (cat == 16)
    {
      if (idx == 1600) return QObject::tr("노이즈");
      if (idx == 1601) return QObject::tr("강도");
      if (idx == 1602) return QObject::tr("타입");
      if (idx == 1603) return QObject::tr("컬러");
      if (idx == 1604) return QObject::tr("흑백");
    }

    // 明るさ・コントラスト
    if (cat == 17)
    {
      if (idx == 1700) return QObject::tr("밝기/대비");
      if (idx == 1701) return QObject::tr("밝기");
      if (idx == 1702) return QObject::tr("대비");
      if (idx == 1703) return QObject::tr("미리보기");
    }

    // ポスタリゼーション
    if (cat == 18)
    {
      if (idx == 1800) return QObject::tr("포스터화");
      if (idx == 1801) return QObject::tr("단계");
    }

    // 整列
    if (cat == 19)
    {
      if (idx == 1900) return QObject::tr("선택된 레이어 정렬"); // 選択レイヤーの整列

      if (idx == 1910) return QObject::tr("세로로 정렬 (왼쪽 끝)"); // 縦に整列 (左端)
      if (idx == 1911) return QObject::tr("세로로 정렬 (중앙)"); // 縦に整列 (中央)
      if (idx == 1912) return QObject::tr("세로로 정렬 (오른쪽 끝)"); // 縦に整列 (右端)
      if (idx == 1913) return QObject::tr("가로로 정렬 (위쪽 끝)"); // 横に整列 (上端)
      if (idx == 1914) return QObject::tr("가로로 정렬 (중앙)"); // 横に整列 (中央)
      if (idx == 1915) return QObject::tr("가로로 정렬 (아래쪽 끝)"); // 横に整列 (下端)

      if (idx == 1920) return QObject::tr("이미지의 가장자리에 맞추기"); // 画像の端で揃える
      if (idx == 1921) return QObject::tr("이미지의 중앙에 맞추기"); // 画像の中心で揃える

      if (idx == 1930) return QObject::tr("선택된 레이어의 범위를 기준으로 하기"); // 選択レイヤーの範囲を基準にする
      if (idx == 1931) return QObject::tr("선택 영역을 기준으로 하기"); // 選択範囲を基準にする
    }
  }

  /////////////////////////////
  /////////////////////////////
  // ポルトガル語
  /////////////////////////////
  /////////////////////////////
  if (man->TranslatePortugues())
  {
    // レベル補正・線画抽出
    if (cat == 0)
    {
      if (idx == 0) return QObject::tr("Entrada");
      if (idx == 1) return QObject::tr("Saída");
      if (idx == 2) return QObject::tr("Níveis");
      if (idx == 3) return QObject::tr("Extraindo linhas");
    }

    if (cat == 1)
    {
      if (idx == 100) return QObject::tr("Matiz");
      if (idx == 101) return QObject::tr("Saturação");
      if (idx == 102) return QObject::tr("Brilho");
      if (idx == 103) return QObject::tr("Matiz/Saturação/Brilho");
      if (idx == 104) return QObject::tr("Espaço de cor"); // 色空間
    }

      if (cat == 2)
    {
      if (idx == 200) return QObject::tr("Curva de Tons");
      if (idx == 201) return QObject::tr("Redefinir");
    }

     if (cat == 3)
    {
      if (idx == 300) return QObject::tr("Operação de Canal");
      if (idx == 301) return QObject::tr("Média RGB");
      if (idx == 302) return QObject::tr("255 - Média RGB");
      if (idx == 303) return QObject::tr("Redefinir");
      if (idx == 304) return QObject::tr("Predefinição");
      if (idx == 310) return QObject::tr("Componente R");
      if (idx == 311) return QObject::tr("Componente G");
      if (idx == 312) return QObject::tr("Componente B");
      if (idx == 313) return QObject::tr("Extração de Linha");
      if (idx == 314) return QObject::tr("Monocromático");
    }

     if (cat == 4)
    {
      if (idx == 400) return QObject::tr("Aberração de Cor/Deslocamento RGB");
    }

     if (cat == 5)
    {
     if (idx == 500) return QObject::tr("Máscara de Nitidez");
     if (idx == 501) return QObject::tr("Raio");
     if (idx == 502) return QObject::tr("Intensidade");
     if (idx == 503) return QObject::tr("Pré-visualização");
    }

    if (cat == 6)
    {
      if (idx == 600) return QObject::tr("Desfoque Gaussiano");
      if (idx == 601) return QObject::tr("Valor");
      if (idx == 602) return QObject::tr("Pré-visualização"); // プレビュー
    }

     if (cat == 7)
    {
     if (idx == 700) return QObject::tr("Desfoque de Movimento");
     if (idx == 701) return QObject::tr("Intensidade");
     if (idx == 702) return QObject::tr("Pré-visualização");
    }

     if (cat == 8)
    {
     if (idx == 800) return QObject::tr("Desfoque de Lente");
     if (idx == 801) return QObject::tr("Raio");
     if (idx == 802) return QObject::tr("Pré-visualização");
     if (idx == 803) return QObject::tr("Brilho");
    }

    if (cat == 9)
    {
      if (idx == 900) return QObject::tr("Mosaico");
      if (idx == 901) return QObject::tr("Tamanho");
    }

     if (cat == 10)
    {
     if (idx == 1000) return QObject::tr("Ondas (Seigaiha)");
     if (idx == 1001) return QObject::tr("Ondas Diamante (Hishi Seigaiha)");
     if (idx == 1002) return QObject::tr("Cloisonné");
     if (idx == 1003) return QObject::tr("Cloisonné 2");
     if (idx == 1004) return QObject::tr("Sangi");
     if (idx == 1005) return QObject::tr("Flecha (Yagasuri)");
     if (idx == 1006) return QObject::tr("Flecha 2 (Yagasuri)");
     if (idx == 1007) return QObject::tr("Escamas (Uroko)");
     if (idx == 1008) return QObject::tr("Pata de Galo");
     if (idx == 1009) return QObject::tr("Bolinhas");

     if (idx == 1010) return QObject::tr("Suribitta");
     if (idx == 1011) return QObject::tr("Casco de Tartaruga (Kikko)");
     if (idx == 1012) return QObject::tr("Xadrez");
     if (idx == 1013) return QObject::tr("Asanoha");
     if (idx == 1014) return QObject::tr("Ogee");
     if (idx == 1015) return QObject::tr("Fundo Tsunagi");
     if (idx == 1016) return QObject::tr("Sayagata");
     if (idx == 1017) return QObject::tr("Listras Verticais");
     if (idx == 1018) return QObject::tr("Listras Horizontais");

     if (idx == 1050) return QObject::tr("Padrão Japonês");
     if (idx == 1051) return QObject::tr("Nome");
     if (idx == 1052) return QObject::tr("Tamanho");
     if (idx == 1053) return QObject::tr("Fundo Transparente");
     if (idx == 1054) return QObject::tr("Trocar Cor");
     if (idx == 1055) return QObject::tr("Cor Aleatória");
     if (idx == 1056) return QObject::tr("Densidade");
     if (idx == 1057) return QObject::tr("Ângulo");
     if (idx == 1058) return QObject::tr("Graus");
    }

     if (cat == 11)
    {
     if (idx == 1100) return QObject::tr("Linhas de Concentração");
     if (idx == 1101) return QObject::tr("Comprimento");
     if (idx == 1102) return QObject::tr("Comprimento (Aleatório)");
     if (idx == 1103) return QObject::tr("Espessura da Linha");
     if (idx == 1104) return QObject::tr("Densidade");
     if (idx == 1105) return QObject::tr("Densidade (Aleatório)");
     if (idx == 1106) return QObject::tr("Proporção");
     if (idx == 1107) return QObject::tr("Quadrado");
     if (idx == 1108) return QObject::tr("Retangular Horizontal");
     if (idx == 1109) return QObject::tr("Retangular Vertical");
     if (idx == 1110) return QObject::tr("Redefinir");
     if (idx == 1111) return QObject::tr("Atualizar Valor Aleatório");
     if (idx == 1112) return QObject::tr("Definir Centro");
    }

     if (cat == 12)
    {
     if (idx == 1200) return QObject::tr("Linhas Fluídas");
     if (idx == 1201) return QObject::tr("Comprimento");
     if (idx == 1202) return QObject::tr("Comprimento (Aleatório)");
     if (idx == 1203) return QObject::tr("Espessura da Linha");
     if (idx == 1204) return QObject::tr("Densidade");
     if (idx == 1205) return QObject::tr("Densidade (Aleatório)");
     if (idx == 1206) return QObject::tr("Redefinir");
     if (idx == 1207) return QObject::tr("Atualizar Valor Aleatório");
     if (idx == 1208) return QObject::tr("Definir Direção");
    }

     if (cat == 13)
    {
     if (idx == 1300) return QObject::tr("Flash de Ouriço");
     if (idx == 1301) return QObject::tr("Comprimento");
     if (idx == 1302) return QObject::tr("Comprimento (Aleatório)");
     if (idx == 1303) return QObject::tr("Espessura da Linha");
     if (idx == 1304) return QObject::tr("Densidade");
     if (idx == 1305) return QObject::tr("Densidade (Aleatório)");
     if (idx == 1306) return QObject::tr("Deslocamento (Aleatório)");
     if (idx == 1307) return QObject::tr("Posição de Entrada/Saída");
     if (idx == 1308) return QObject::tr("Redefinir");
     if (idx == 1309) return QObject::tr("Atualizar Valor Aleatório");
    }

     if (cat == 14)
    {
     if (idx == 1400) return QObject::tr("Espiral");
    }

     if (cat == 15)
    {
     if (idx == 1500) return QObject::tr("Onda");
    }

     if (cat == 16)
    {
     if (idx == 1600) return QObject::tr("Ruído");
     if (idx == 1601) return QObject::tr("Intensidade");
     if (idx == 1602) return QObject::tr("Tipo");
     if (idx == 1603) return QObject::tr("Cor");
     if (idx == 1604) return QObject::tr("Monocromático");
    }

     if (cat == 17)
    {
     if (idx == 1700) return QObject::tr("Brilho e Contraste");
     if (idx == 1701) return QObject::tr("Brilho");
     if (idx == 1702) return QObject::tr("Contraste");
     if (idx == 1703) return QObject::tr("Pré-visualização");
    }

     if (cat == 18)
    {
     if (idx == 1800) return QObject::tr("Posterização");
     if (idx == 1801) return QObject::tr("Etapas");
    }

    if (cat == 19)
    {
      if (idx == 1900) return QObject::tr("Alinhar Camadas Selecionadas");

      if (idx == 1910) return QObject::tr("Alinhar Verticalmente (À Esquerda)");
      if (idx == 1911) return QObject::tr("Alinhar Verticalmente (Centro)");
      if (idx == 1912) return QObject::tr("Alinhar Verticalmente (À Direita)");
      if (idx == 1913) return QObject::tr("Alinhar Horizontalmente (Acima)");
      if (idx == 1914) return QObject::tr("Alinhar Horizontalmente (Centro)");
      if (idx == 1915) return QObject::tr("Alinhar Horizontalmente (Abaixo)");

      if (idx == 1920) return QObject::tr("Alinhar nas Bordas da Imagem");
      if (idx == 1921) return QObject::tr("Alinhar ao Centro da Imagem");

      if (idx == 1930) return QObject::tr("Basear-se na Faixa da Camada Selecionada");
      if (idx == 1931) return QObject::tr("Basear-se na Faixa Selecionada");
    }

  }


  /////////////////////////////
  /////////////////////////////
  // スペイン語
  /////////////////////////////
  /////////////////////////////
  if (man->TranslateSpanish())
  {
    // レベル補正・線画抽出
    if (cat == 0)
    {
      if (idx == 0) return QObject::tr("Entrada");
      if (idx == 1) return QObject::tr("Salida");
      if (idx == 2) return QObject::tr("Niveles");
      if (idx == 3) return QObject::tr("Extraer líneas");
    }

    if (cat == 1)
    {
      if (idx == 100) return QObject::tr("Tono");
      if (idx == 101) return QObject::tr("Saturación");
      if (idx == 102) return QObject::tr("Luminosidad");
      if (idx == 103) return QObject::tr("Matiz/Saturación/Brillo");
      if (idx == 104) return QObject::tr("Espacio de color"); // 色空間
    }

      if (cat == 2)
    {
      if (idx == 200) return QObject::tr("Curva de tonos");
      if (idx == 201) return QObject::tr("Restablecer");
    }

     if (cat == 3)
    {
      if (idx == 300) return QObject::tr("Operaciones de los canales");
      if (idx == 301) return QObject::tr("RGB promedio");
      if (idx == 302) return QObject::tr("255 - RGB promedio");
      if (idx == 303) return QObject::tr("Restablecer");
      if (idx == 304) return QObject::tr("Ajustes Preestablecidos");
      if (idx == 310) return QObject::tr("Componente R");
      if (idx == 311) return QObject::tr("Componente G");
      if (idx == 312) return QObject::tr("B Componente");
      if (idx == 313) return QObject::tr("Extracción de líneas");
      if (idx == 314) return QObject::tr("Monocromatización");
    }

     if (cat == 4)
    {
      if (idx == 400) return QObject::tr("Aberración cromática/Desplazamiento RGB");
    }

     if (cat == 5)
    {
     if (idx == 500) return QObject::tr("Máscara de Enfoque");
     if (idx == 501) return QObject::tr("Radio");
     if (idx == 502) return QObject::tr("Cantidad");
     if (idx == 503) return QObject::tr("Vista previa");
    }

    if (cat == 6)
    {
      if (idx == 600) return QObject::tr("Desenfoque gaussiano");
      if (idx == 601) return QObject::tr("Valor");
      if (idx == 602) return QObject::tr("Vista previa"); // プレビュー
    }

     if (cat == 7)
    {
     if (idx == 700) return QObject::tr("Desenfoque de movimiento");
     if (idx == 701) return QObject::tr("Intensidad");
     if (idx == 702) return QObject::tr("Vista previa");
    }

     if (cat == 8)
    {
     if (idx == 800) return QObject::tr("Desenfoque del objetivo");
     if (idx == 801) return QObject::tr("Radio");
     if (idx == 802) return QObject::tr("Vista previa");
     if (idx == 803) return QObject::tr("Luminosidad");
    }

    if (cat == 9)
    {
      if (idx == 900) return QObject::tr("Mosaico");
      if (idx == 901) return QObject::tr("Tamaño");
    }

     if (cat == 10)
    {
     if (idx == 1000) return QObject::tr("Olas (Seigaiha)");
     if (idx == 1001) return QObject::tr("Olas Diamante (Hishi Seigaiha)");
     if (idx == 1002) return QObject::tr("Cloisonné");
     if (idx == 1003) return QObject::tr("Cloisonné 2");
     if (idx == 1004) return QObject::tr("Sangi");
     if (idx == 1005) return QObject::tr("Flecha (Yagasuri)");
     if (idx == 1006) return QObject::tr("Flecha 2 (Yagasuri)");
     if (idx == 1007) return QObject::tr("Escamas (Uroko)");
     if (idx == 1008) return QObject::tr("Pata de Gallo");
     if (idx == 1009) return QObject::tr("Lunares");

     if (idx == 1010) return QObject::tr("Suribitta");
     if (idx == 1011) return QObject::tr("Caparazón de tortuga (Kikko)");
     if (idx == 1012) return QObject::tr("Ajedrez");
     if (idx == 1013) return QObject::tr("Asanoha");
     if (idx == 1014) return QObject::tr("Ogee");
     if (idx == 1015) return QObject::tr("Fundo Tsunagi");
     if (idx == 1016) return QObject::tr("Sayagata");
     if (idx == 1017) return QObject::tr("Rayas verticales");
     if (idx == 1018) return QObject::tr("Rayas horizontales");

     if (idx == 1050) return QObject::tr("Patrón japonés");
     if (idx == 1051) return QObject::tr("Nombre");
     if (idx == 1052) return QObject::tr("Talla");
     if (idx == 1053) return QObject::tr("Fondo transparente");
     if (idx == 1054) return QObject::tr("Intercambio de colores");
     if (idx == 1055) return QObject::tr("Especificación de color aleatoria");
     if (idx == 1056) return QObject::tr("Densidad");
     if (idx == 1057) return QObject::tr("Ángulo");
     if (idx == 1058) return QObject::tr("Grado");
    }

     if (cat == 11)
    {
     if (idx == 1100) return QObject::tr("Líneas de concentración");
     if (idx == 1101) return QObject::tr("Longitud");
     if (idx == 1102) return QObject::tr("Longitud (aleatoria)");
     if (idx == 1103) return QObject::tr("Anchura de línea");
     if (idx == 1104) return QObject::tr("Densidad");
     if (idx == 1105) return QObject::tr("Densidad (aleatoria)");
     if (idx == 1106) return QObject::tr("Relación de aspecto");
     if (idx == 1107) return QObject::tr("Cuadrado");
     if (idx == 1108) return QObject::tr("Paisaje");
     if (idx == 1109) return QObject::tr("Retrato");
     if (idx == 1110) return QObject::tr("Restablecer");
     if (idx == 1111) return QObject::tr("Actualización de valor aleatorio");
     if (idx == 1112) return QObject::tr("Especificar centro");
    }

     if (cat == 12)
    {
     if (idx == 1200) return QObject::tr("Líneas paralelas");
     if (idx == 1201) return QObject::tr("Longitud");
     if (idx == 1202) return QObject::tr("Longitud (aleatoria)");
     if (idx == 1203) return QObject::tr("Grosor de la línea");
     if (idx == 1204) return QObject::tr("Densidad");
     if (idx == 1205) return QObject::tr("Densidad (aleatoria)");
     if (idx == 1206) return QObject::tr("Restablecer");
     if (idx == 1207) return QObject::tr("Actualizar valor aleatorio");
     if (idx == 1208) return QObject::tr("Especificar dirección");
    }

     if (cat == 13)
    {
     if (idx == 1300) return QObject::tr("Flash de Erizo de Mar");
     if (idx == 1301) return QObject::tr("Longitud");
     if (idx == 1302) return QObject::tr("Longitud (aleatoria)");
     if (idx == 1303) return QObject::tr("Grosor de la línea");
     if (idx == 1304) return QObject::tr("Densidad");
     if (idx == 1305) return QObject::tr("Densidad (aleatoria)");
     if (idx == 1306) return QObject::tr("Offset (aleatorio)");
     if (idx == 1307) return QObject::tr("Posición de la Aparición/Desaparición gradual");
     if (idx == 1308) return QObject::tr("Restablecer");
     if (idx == 1309) return QObject::tr("Actualizar valor aleatorio");
    }

     if (cat == 14)
    {
     if (idx == 1400) return QObject::tr("Remolino");
    }

     if (cat == 15)
    {
     if (idx == 1500) return QObject::tr("Onda");
    }

     if (cat == 16)
    {
     if (idx == 1600) return QObject::tr("Ruido");
     if (idx == 1601) return QObject::tr("Intensidad");
     if (idx == 1602) return QObject::tr("Tipo");
     if (idx == 1603) return QObject::tr("Color");
     if (idx == 1604) return QObject::tr("Monocromo");
    }

     if (cat == 17)
    {
     if (idx == 1700) return QObject::tr("Luminosidad/Contraste");
     if (idx == 1701) return QObject::tr("Luminosidad");
     if (idx == 1702) return QObject::tr("Contraste");
     if (idx == 1703) return QObject::tr("Vista previa");
    }

     if (cat == 18)
    {
     if (idx == 1800) return QObject::tr("Posterización");
     if (idx == 1801) return QObject::tr("Número de columnas");
    }

    if (cat == 19)
    {
      if (idx == 1900) return QObject::tr("Alinear capas seleccionadas");

      if (idx == 1910) return QObject::tr("Alinear verticalmente (izquierda)");
      if (idx == 1911) return QObject::tr("Alinear verticalmente (centro)");
      if (idx == 1912) return QObject::tr("Alinear verticalmente (derecha)");
      if (idx == 1913) return QObject::tr("Alinear horizontalmente (arriba)");
      if (idx == 1914) return QObject::tr("Alinear horizontalmente (centro)");
      if (idx == 1915) return QObject::tr("Alinear horizontalmente (abajo)");

      if (idx == 1920) return QObject::tr("Alinear con el borde de la imagen");
      if (idx == 1921) return QObject::tr("Alinear con el centro de la imagen");

      if (idx == 1930) return QObject::tr("Tomar el rango de las capas seleccionadas como referencia");
      if (idx == 1931) return QObject::tr("Tomar la selección como referencia");
    }

  }


  /////////////////////////////
  /////////////////////////////
  // ドイツ語
  /////////////////////////////
  /////////////////////////////
  if (man->TranslateGerman())
  {
    // レベル補正・線画抽出
    if (cat == 0)
    {
      if (idx == 0) return QObject::tr("Eingabe");
      if (idx == 1) return QObject::tr("Ausgabe");
      if (idx == 2) return QObject::tr("Ebenen");
      if (idx == 3) return QObject::tr("Umrisse extrahieren");
    }

    if (cat == 1)
    {
      if (idx == 100) return QObject::tr("Farbton");
      if (idx == 101) return QObject::tr("Sättigung");
      if (idx == 102) return QObject::tr("Helligkeit");
      if (idx == 103) return QObject::tr("Farbton/Sättigung/Helligkeit");
      if (idx == 104) return QObject::tr("Farbraum"); // 色空間
    }

    // トーンカーブ
    if (cat == 2)
    {
      if (idx == 200) return QObject::tr("Ton-Kurve");
      if (idx == 201) return QObject::tr("Zurücksetzen");
    }

     // チャンネル操作
    if (cat == 3)
    {
      if (idx == 300) return QObject::tr("Kanalbetrieb");
      if (idx == 301) return QObject::tr("RGB-Mittelwert");
      if (idx == 302) return QObject::tr("255 - (RGB-Mittelwert)");
      if (idx == 303) return QObject::tr("Zurücksetzen");
      if (idx == 304) return QObject::tr("Voreinstellung");
      if (idx == 310) return QObject::tr("R-Komponente");
      if (idx == 311) return QObject::tr("G-Komponente");
      if (idx == 312) return QObject::tr("B-Komponente");
      if (idx == 313) return QObject::tr("Linienextraktion");
      if (idx == 314) return QObject::tr("Monochromatisierung");
    }

    // 色収差
    if (cat == 4)
    {
      if (idx == 400) return QObject::tr("Chromatische Aberration");
    }

    // アンシャープマスク
    if (cat == 5)
    {
      if (idx == 500) return QObject::tr("Unscharfe Maske");
      if (idx == 501) return QObject::tr("Radius");
      if (idx == 502) return QObject::tr("Betrag");
      if (idx == 503) return QObject::tr("Vorschau");
    }

    if (cat == 6)
    {
      if (idx == 600) return QObject::tr("Gaußscher Weichzeichner");
      if (idx == 601) return QObject::tr("Wert");
      if (idx == 602) return QObject::tr("Vorschau"); // プレビュー
    }

 // 移動ぼかし
    if (cat == 7)
    {
      if (idx == 700) return QObject::tr("Bewegungsunschärfe");
      if (idx == 701) return QObject::tr("Intensität");
      if (idx == 702) return QObject::tr("Vorschau");
    }

     // レンズぼかし
    if (cat == 8)
    {
      if (idx == 800) return QObject::tr("Objektiv-Weichzeichner");
      if (idx == 801) return QObject::tr("Radius");
      if (idx == 802) return QObject::tr("Vorschau");
      if (idx == 803) return QObject::tr("Helligkeit");
    }
   
    if (cat == 9)
    {
      if (idx == 900) return QObject::tr("Mosaik");
      if (idx == 901) return QObject::tr("Größe");
    }

     if (cat == 10)
    {
     if (idx == 1000) return QObject::tr("Meer (Seigaiha)");
     if (idx == 1001) return QObject::tr("Meer (Hishi Seigaiha)");
     if (idx == 1002) return QObject::tr("Sippou");
     if (idx == 1003) return QObject::tr("Sippou 2");
     if (idx == 1004) return QObject::tr("Sanki Kuzushi");
     if (idx == 1005) return QObject::tr("Pfeil (Yagasuri)");
     if (idx == 1006) return QObject::tr("Pfeil 2 (Yagasuri)");
     if (idx == 1007) return QObject::tr("Schuppe (Uroko)");
     if (idx == 1008) return QObject::tr("Hound's Tooth");
     if (idx == 1009) return QObject::tr("Polka-Punkt");

     if (idx == 1010) return QObject::tr("Suri Bitta");
     if (idx == 1011) return QObject::tr("Schildkröte (Kikkou)");
     if (idx == 1012) return QObject::tr("Schachbrett");
     if (idx == 1013) return QObject::tr("Asanoha");
     if (idx == 1014) return QObject::tr("Tatewaku");
     if (idx == 1015) return QObject::tr("Hundou Tsunagi");
     if (idx == 1016) return QObject::tr("Sayagata");
     if (idx == 1017) return QObject::tr("Vertikale Streifen");
     if (idx == 1018) return QObject::tr("Horizontale Streifen");

     if (idx == 1050) return QObject::tr("Japanisches Muster");
     if (idx == 1051) return QObject::tr("Name");
     if (idx == 1052) return QObject::tr("Größe");
     if (idx == 1053) return QObject::tr("Transparenter Hintergrund");
     if (idx == 1054) return QObject::tr("Farbwechsel");
     if (idx == 1055) return QObject::tr("Zufällige Farbe");
     if (idx == 1056) return QObject::tr("Dichte");
     if (idx == 1057) return QObject::tr("Winkel");
     if (idx == 1058) return QObject::tr("deg");
    }

     if (cat == 11)
    {
     if (idx == 1100) return QObject::tr("Konzentrierte Linie");
     if (idx == 1101) return QObject::tr("Länge");
     if (idx == 1102) return QObject::tr("Länge (zufällig)");
     if (idx == 1103) return QObject::tr("Breite der Linie");
     if (idx == 1104) return QObject::tr("Dichte");
     if (idx == 1105) return QObject::tr("Dichte (zufällig)");
     if (idx == 1106) return QObject::tr("Streckungsverhältnis");
     if (idx == 1107) return QObject::tr("Quadratisch");
     if (idx == 1108) return QObject::tr("Querformat");
     if (idx == 1109) return QObject::tr("Hochformat");
     if (idx == 1110) return QObject::tr("Zurücksetzen");
     if (idx == 1111) return QObject::tr("Zufälligkeit aktualisieren");
     if (idx == 1112) return QObject::tr("Zentrum festlegen");
    }

     if (cat == 12)
    {
     if (idx == 1200) return QObject::tr("Parallele Linien");
     if (idx == 1201) return QObject::tr("Länge");
     if (idx == 1202) return QObject::tr("Länge (zufällig)");
     if (idx == 1203) return QObject::tr("Linienbreite");
     if (idx == 1204) return QObject::tr("Dichte");
     if (idx == 1205) return QObject::tr("Dichte (zufällig)");
     if (idx == 1206) return QObject::tr("Zurücksetzen");
     if (idx == 1207) return QObject::tr("Zufälligkeit aktualisieren");
     if (idx == 1208) return QObject::tr("Richtung spezifizieren");
    }

     if (cat == 13)
    {
     if (idx == 1300) return QObject::tr("Seeigel-Blitz");
     if (idx == 1301) return QObject::tr("Länge");
     if (idx == 1302) return QObject::tr("Länge (zufällig)");
     if (idx == 1303) return QObject::tr("Breite der Linie");
     if (idx == 1304) return QObject::tr("Dichte");
     if (idx == 1305) return QObject::tr("Dichte (zufällig)");
     if (idx == 1306) return QObject::tr("Offset (zufällig)");
     if (idx == 1307) return QObject::tr("Einblenden/Ausblenden Pos");
     if (idx == 1308) return QObject::tr("Zurücksetzen");
     if (idx == 1309) return QObject::tr("Update Zufälligkeit");
    }

     if (cat == 14)
    {
     if (idx == 1400) return QObject::tr("Wirbel");
    }

     if (cat == 15)
    {
     if (idx == 1500) return QObject::tr("Welle");
    }

     if (cat == 16)
    {
     if (idx == 1600) return QObject::tr("Rauschen");
     if (idx == 1601) return QObject::tr("Stärke");
     if (idx == 1602) return QObject::tr("Typ");
     if (idx == 1603) return QObject::tr("Farbe");
     if (idx == 1604) return QObject::tr("Monochrom");
    }

     if (cat == 17)
    {
     if (idx == 1700) return QObject::tr("Helligkeit/Kontrast");
     if (idx == 1701) return QObject::tr("Helligkeit");
     if (idx == 1702) return QObject::tr("Kontrast");
     if (idx == 1703) return QObject::tr("Vorschau");
    }

     if (cat == 18)
    {
     if (idx == 1800) return QObject::tr("Posterisierung");
     if (idx == 1801) return QObject::tr("Schritte");
    }

    if (cat == 19)
    {
      if (idx == 1900) return QObject::tr("Ausgewählte Ebenen ausrichten");

      if (idx == 1910) return QObject::tr("Vertikal ausrichten (linkes Ende)");
      if (idx == 1911) return QObject::tr("Vertikal ausrichten (Mitte)");
      if (idx == 1912) return QObject::tr("Vertikal ausrichten (rechtes Ende)");
      if (idx == 1913) return QObject::tr("Horizontal ausrichten (oberes Ende)");
      if (idx == 1914) return QObject::tr("Horizontal ausrichten (Mitte)");
      if (idx == 1915) return QObject::tr("Horizontal ausrichten (unteres Ende)");

      if (idx == 1920) return QObject::tr("An den Rändern eines Bildes ausrichten");
      if (idx == 1921) return QObject::tr("An der Mitte des Bildes ausrichten");

      if (idx == 1930) return QObject::tr("Basierend auf einem Bereich der ausgewählten Ebene");
      if (idx == 1931) return QObject::tr("Basierend auf einer Auswahl");
    }

  }


  /////////////////////////////
  /////////////////////////////
  // フランス語
  /////////////////////////////
  /////////////////////////////
  if (man->TranslateFrench())
  {
    // レベル補正・線画抽出
    if (cat == 0)
    {
      if (idx == 0) return QObject::tr("Entrée");
      if (idx == 1) return QObject::tr("Sortie");
      if (idx == 2) return QObject::tr("Niveaux");
      if (idx == 3) return QObject::tr("Extraction de lignes");
    }

    if (cat == 1)
    {
      if (idx == 100) return QObject::tr("Teinte");
      if (idx == 101) return QObject::tr("Saturation");
      if (idx == 102) return QObject::tr("Luminosité");
      if (idx == 103) return QObject::tr("Teinte/Saturation/Luminosité");
      if (idx == 104) return QObject::tr("Espace colorimétrique"); // 色空間
    }

    // トーンカーブ
    if (cat == 2)
    {
      if (idx == 200) return QObject::tr("Courbe de tonalité");
      if (idx == 201) return QObject::tr("Réinitialiser");
    }

     // チャンネル操作
    if (cat == 3)
    {
      if (idx == 300) return QObject::tr("Opération sur le canal");
      if (idx == 301) return QObject::tr("Moyenne RGB");
      if (idx == 302) return QObject::tr("255 - (Moyenne RGB)");
      if (idx == 303) return QObject::tr("Réinitialiser");
      if (idx == 304) return QObject::tr("Préréglage");
      if (idx == 310) return QObject::tr("Composant R");
      if (idx == 311) return QObject::tr("Composant G");
      if (idx == 312) return QObject::tr("Composant B");
      if (idx == 313) return QObject::tr("Extraction de lignes");
      if (idx == 314) return QObject::tr("Monochromatisation");
    }

    // 色収差
    if (cat == 4)
    {
      if (idx == 400) return QObject::tr("Aberration chromatique");
    }

    // アンシャープマスク
    if (cat == 5)
    {
      if (idx == 500) return QObject::tr("Masque flou");
      if (idx == 501) return QObject::tr("Rayon");
      if (idx == 502) return QObject::tr("Quantité");
      if (idx == 503) return QObject::tr("Aperçu");
    }


    if (cat == 6)
    {
      if (idx == 600) return QObject::tr("Flou gaussien");
      if (idx == 601) return QObject::tr("Valeur");
      if (idx == 602) return QObject::tr("Aperçu"); // プレビュー
    }

    // 移動ぼかし
    if (cat == 7)
    {
      if (idx == 700) return QObject::tr("Flou de mouvement");
      if (idx == 701) return QObject::tr("Intensité");
      if (idx == 702) return QObject::tr("Aperçu");
    }

    // レンズぼかし
    if (cat == 8)
    {
      if (idx == 800) return QObject::tr("Flou de lentille");
      if (idx == 801) return QObject::tr("Rayon");
      if (idx == 802) return QObject::tr("Aperçu");
      if (idx == 803) return QObject::tr("Luminosité");
    }

    if (cat == 9)
    {
      if (idx == 900) return QObject::tr("Mosaïque");
      if (idx == 901) return QObject::tr("Taille");
    }

     if (cat == 10)
    {
     if (idx == 1000) return QObject::tr("Mer (Seigaiha)");
     if (idx == 1001) return QObject::tr("Mer (Hishi Seigaiha)");
     if (idx == 1002) return QObject::tr("Sippou");
     if (idx == 1003) return QObject::tr("Sippou 2");
     if (idx == 1004) return QObject::tr("Sanki Kuzushi");
     if (idx == 1005) return QObject::tr("Flèche (Yagasuri)");
     if (idx == 1006) return QObject::tr("Flèche 2 (Yagasuri)");
     if (idx == 1007) return QObject::tr("Écaille (Uroko)");
     if (idx == 1008) return QObject::tr("Dent de chien");
     if (idx == 1009) return QObject::tr("Pois");

     if (idx == 1010) return QObject::tr("Suri Bitta");
     if (idx == 1011) return QObject::tr("Tortue (Kikkou)");
     if (idx == 1012) return QObject::tr("Damier");
     if (idx == 1013) return QObject::tr("Asanoha");
     if (idx == 1014) return QObject::tr("Tatewaku");
     if (idx == 1015) return QObject::tr("Hundou Tsunagi");
     if (idx == 1016) return QObject::tr("Sayagata");
     if (idx == 1017) return QObject::tr("Rayures verticales");
     if (idx == 1018) return QObject::tr("Rayures horizontales");

     if (idx == 1050) return QObject::tr("Motif japonais");
     if (idx == 1051) return QObject::tr("Nom");
     if (idx == 1052) return QObject::tr("Taille");
     if (idx == 1053) return QObject::tr("Fond transparent");
     if (idx == 1054) return QObject::tr("Échange de couleurs");
     if (idx == 1055) return QObject::tr("Couleur aléatoire");
     if (idx == 1056) return QObject::tr("Densité");
     if (idx == 1057) return QObject::tr("Angle");
     if (idx == 1058) return QObject::tr("degrés");
    }

     if (cat == 11)
    {
     if (idx == 1100) return QObject::tr("Ligne concentrée");
     if (idx == 1101) return QObject::tr("Longueur");
     if (idx == 1102) return QObject::tr("Longueur (Aléatoire)");
     if (idx == 1103) return QObject::tr("Largeur de ligne");
     if (idx == 1104) return QObject::tr("Densité");
     if (idx == 1105) return QObject::tr("Densité (Aléatoire)");
     if (idx == 1106) return QObject::tr("Rapport d’aspect");
     if (idx == 1107) return QObject::tr("Carré");
     if (idx == 1108) return QObject::tr("Paysage");
     if (idx == 1109) return QObject::tr("Portrait");
     if (idx == 1110) return QObject::tr("Réinitialiser");
     if (idx == 1111) return QObject::tr("Mettre à jour l’aléatoire");
     if (idx == 1112) return QObject::tr("Spécifier le centre");
    }

     if (cat == 12)
    {
     if (idx == 1200) return QObject::tr("Lignes parallèles");
     if (idx == 1201) return QObject::tr("Longueur");
     if (idx == 1202) return QObject::tr("Longueur (Aléatoire)");
     if (idx == 1203) return QObject::tr("Largeur de ligne");
     if (idx == 1204) return QObject::tr("Densité");
     if (idx == 1205) return QObject::tr("Densité (Aléatoire)");
     if (idx == 1206) return QObject::tr("Réinitialiser");
     if (idx == 1207) return QObject::tr("Mettre à jour l’aléatoire");
     if (idx == 1208) return QObject::tr("Spécifier la direction");
    }

     if (cat == 13)
    {
     if (idx == 1300) return QObject::tr("Flash d’oursin de mer");
     if (idx == 1301) return QObject::tr("Longueur");
     if (idx == 1302) return QObject::tr("Longueur (Aléatoire)");
     if (idx == 1303) return QObject::tr("Largeur de ligne");
     if (idx == 1304) return QObject::tr("Densité");
     if (idx == 1305) return QObject::tr("Densité (Aléatoire)");
     if (idx == 1306) return QObject::tr("Décalage (Aléatoire)");
     if (idx == 1307) return QObject::tr("Position d'apparition/disparition");
     if (idx == 1308) return QObject::tr("Réinitialiser");
     if (idx == 1309) return QObject::tr("Mettre à jour l’aléatoire");
    }

     if (cat == 14)
    {
     if (idx == 1400) return QObject::tr("Tourbillon");
    }

     if (cat == 15)
    {
     if (idx == 1500) return QObject::tr("Vague");
    }

     if (cat == 16)
    {
     if (idx == 1600) return QObject::tr("Bruit");
     if (idx == 1601) return QObject::tr("Intensité");
     if (idx == 1602) return QObject::tr("Type");
     if (idx == 1603) return QObject::tr("Couleur");
     if (idx == 1604) return QObject::tr("Monochrome");
    }

     if (cat == 17)
    {
     if (idx == 1700) return QObject::tr("Luminosité/Contraste");
     if (idx == 1701) return QObject::tr("Luminosité");
     if (idx == 1702) return QObject::tr("Contraste");
     if (idx == 1703) return QObject::tr("Aperçu");
    }

     if (cat == 18)
    {
     if (idx == 1800) return QObject::tr("Postérisation");
     if (idx == 1801) return QObject::tr("Étapes");
    }

    if (cat == 19)
    {
      if (idx == 1900) return QObject::tr("Aligner les calques sélectionnés");

      if (idx == 1910) return QObject::tr("Aligner verticalement (fin à gauche)");
      if (idx == 1911) return QObject::tr("Aligner verticalement (centre)");
      if (idx == 1912) return QObject::tr("Aligner verticalement (fin à droite)");
      if (idx == 1913) return QObject::tr("Aligner horizontalement (fin en haut)");
      if (idx == 1914) return QObject::tr("Aligner horizontalement (centre)");
      if (idx == 1915) return QObject::tr("Aligner horizontalement (fin en bas)");

      if (idx == 1920) return QObject::tr("Aligner avec les bords de l'image");
      if (idx == 1921) return QObject::tr("Aligner avec le centre de l'image");

      if (idx == 1930) return QObject::tr("Basé sur une plage de calques sélectionnés");
      if (idx == 1931) return QObject::tr("Basé sur une sélection");
    }
  }


  /////////////////////////////
  /////////////////////////////
  // ロシア語
  /////////////////////////////
  /////////////////////////////
  if (man->TranslateRussian())
  {
    // レベル補正・線画抽出
    if (cat == 0)
    {
      if (idx == 0) return QObject::tr("Вводные");
      if (idx == 1) return QObject::tr("Выводные");
      if (idx == 2) return QObject::tr("Уровни");
      if (idx == 3) return QObject::tr("Извлекаемые линии");
    }

    if (cat == 1)
    {
      if (idx == 100) return QObject::tr("Тон");
      if (idx == 101) return QObject::tr("Насыщенность");
      if (idx == 102) return QObject::tr("Яркость");
      if (idx == 103) return QObject::tr("Тон Насыщенность Яркость");
      //if (idx == 104) return QObject::tr("Color Space"); // 色空間
    }

    if (cat == 2)
    {
      if (idx == 200) return QObject::tr("Тоновая кривая"); // トーンカーブ
      if (idx == 201) return QObject::tr("Сброс"); // リセット
    }

    if (cat == 3)
    {
      if (idx == 300) return QObject::tr("Канал"); // チャンネル操作
      if (idx == 301) return QObject::tr("RGB среднее"); // RGB平均
      if (idx == 302) return QObject::tr("255 - (RGB среднее)"); // 255 - RGB平均
      if (idx == 303) return QObject::tr("Сброс"); // リセット
      if (idx == 304) return QObject::tr("Предустановка"); // プリセット
      if (idx == 310) return QObject::tr("R Компонент"); // R成分
      if (idx == 311) return QObject::tr("G Компонент"); // G成分
      if (idx == 312) return QObject::tr("B Компонент"); // B成分
      if (idx == 313) return QObject::tr("Извлечение линии"); // 線画抽出
      if (idx == 314) return QObject::tr("Монохроматизация"); // モノクロ化
    }

    if (cat == 4)
    {
      if (idx == 400) return QObject::tr("Хроматическая аберрация"); // 色収差/RGBずらし
    }

    if (cat == 5)
    {
      if (idx == 500) return QObject::tr("Нерезкая маска"); // アンシャープマスク
      if (idx == 501) return QObject::tr("Радиус"); // 半径
      if (idx == 502) return QObject::tr("Количество"); // 適用量
      if (idx == 503) return QObject::tr("Просмотр"); // プレビュー
    }

    if (cat == 6)
    {
      if (idx == 600) return QObject::tr("Гауссово размытие");
      if (idx == 601) return QObject::tr("Значение");
      if (idx == 602) return QObject::tr("Просмотр"); // プレビュー
    }
  
    if (cat == 7)
    {
      if (idx == 700) return QObject::tr("Размывание с движением"); // 移動ぼかし
      if (idx == 701) return QObject::tr("Усиление"); // 強度
      if (idx == 702) return QObject::tr("Просмотр"); // プレビュー
    }

    if (cat == 8)
    {
      if (idx == 800) return QObject::tr("Размывание объектива"); // レンズぼかし
      if (idx == 801) return QObject::tr("Радиус"); // 半径
      if (idx == 802) return QObject::tr("Просмотр"); // プレビュー
      if (idx == 803) return QObject::tr("Светлота"); // 明るさ
    }

    if (cat == 9)
    {
      if (idx == 900) return QObject::tr("Мозаика");
      if (idx == 901) return QObject::tr("Размер");
    }

    if (cat == 10)
    {
      if (idx == 1000) return QObject::tr("Море(Сейгайха)"); // 青海波
      if (idx == 1001) return QObject::tr("Море(Хиши-сейгайха)"); // 菱青海波
      if (idx == 1002) return QObject::tr("Шиппо"); // 七宝
      if (idx == 1003) return QObject::tr("Шиппо2"); // 七宝 2
      if (idx == 1004) return QObject::tr("Санги-кудзуши"); // 算木崩し
      if (idx == 1005) return QObject::tr("Стрела(Ягасури)"); // 矢絣
      if (idx == 1006) return QObject::tr("Стрела2(Ягасури)"); // 矢絣 2
      if (idx == 1007) return QObject::tr("Чешуя(Уроко)"); // 鱗
      if (idx == 1008) return QObject::tr("Гусиная лапка"); // 千鳥格子
      if (idx == 1009) return QObject::tr("Узор горошек"); // 水玉
      if (idx == 1010) return QObject::tr("Сури-битта"); // 摺疋田
      if (idx == 1011) return QObject::tr("Черепаха(Кикко)"); // 亀甲
      if (idx == 1012) return QObject::tr("Клетчатый"); // 市松
      if (idx == 1013) return QObject::tr("Асаноха"); // 麻の葉
      if (idx == 1014) return QObject::tr("Татеваку"); // 立涌
      if (idx == 1015) return QObject::tr("Фундо-цунаги"); // 分銅繋ぎ
      if (idx == 1016) return QObject::tr("Саягата"); // 紗綾形
      if (idx == 1017) return QObject::tr("Вертикальная полоса"); // 縦縞
      if (idx == 1018) return QObject::tr("Горизонтальная полоса"); // 横縞

      if (idx == 1050) return QObject::tr("Японский узор"); // 和柄
      if (idx == 1051) return QObject::tr("Название"); // 名称
      if (idx == 1052) return QObject::tr("Размер"); // サイズ
      if (idx == 1053) return QObject::tr("Прозрачный фон"); // 透明背景
      if (idx == 1054) return QObject::tr("Замещение"); // 色の入れ替え
      if (idx == 1055) return QObject::tr("Беспорядочный цвет"); // ランダム色指定
      if (idx == 1056) return QObject::tr("Плотность"); // 密度
      if (idx == 1057) return QObject::tr("Угол"); // 角度
      if (idx == 1058) return QObject::tr("°"); // 度
    }

    if (cat == 11)
    {
      if (idx == 1100) return QObject::tr("Радиальная линия"); // 集中線
      if (idx == 1101) return QObject::tr("Длина"); // 長さ
      if (idx == 1102) return QObject::tr("Длина(беспорядочно)"); // 長さ (ランダム)
      if (idx == 1103) return QObject::tr("Ширина линии"); // 線の太さ
      if (idx == 1104) return QObject::tr("Плотность"); // 密度
      if (idx == 1105) return QObject::tr("Плотность(беспорядочно)"); // 密度 (ランダム)
      if (idx == 1106) return QObject::tr("Пропорция"); // 縦横の比率
      if (idx == 1107) return QObject::tr("Квадрат"); // 正方形
      if (idx == 1108) return QObject::tr("Панорама"); // 横長
      if (idx == 1109) return QObject::tr("Портрет"); // 縦長
      if (idx == 1110) return QObject::tr("Сброс"); // リセット
      if (idx == 1111) return QObject::tr("Обновить"); // ランダム値の更新
      if (idx == 1112) return QObject::tr("Установить центр"); // 中心の指定
    }

    if (cat == 12)
    {
      if (idx == 1200) return QObject::tr("Поток"); // 流線
      if (idx == 1201) return QObject::tr("Длина"); // 長さ
      if (idx == 1202) return QObject::tr("Длина(беспорядочно)"); // 長さ (ランダム)
      if (idx == 1203) return QObject::tr("Ширина линии"); // 線の太さ
      if (idx == 1204) return QObject::tr("Плотность"); // 密度
      if (idx == 1205) return QObject::tr("Плотность(беспорядочно)"); // 密度 (ランダム)
      if (idx == 1206) return QObject::tr("Сброс"); // リセット
      if (idx == 1207) return QObject::tr("Обновить"); // ランダム値の更新
      if (idx == 1208) return QObject::tr("Напавление"); // 方向の指定
    }

    if (cat == 13)
    {
      if (idx == 1300) return QObject::tr("Вспышка"); // ウニフラッシュ
      if (idx == 1301) return QObject::tr("Длина"); // 長さ
      if (idx == 1302) return QObject::tr("Длина(беспорядочно)"); // 長さ (ランダム)
      if (idx == 1303) return QObject::tr("Ширина линии"); // 線の太さ
      if (idx == 1304) return QObject::tr("Плотность"); // 密度
      if (idx == 1305) return QObject::tr("Плотность(беспорядочно)"); // 密度 (ランダム)
      if (idx == 1306) return QObject::tr("Компенсация(беспорядочно)"); // オフセット (ランダム)
      if (idx == 1307) return QObject::tr("Усиление/Затухание"); // 入り抜き位置
      if (idx == 1308) return QObject::tr("Сброс"); // リセット
      if (idx == 1309) return QObject::tr("Обновить"); // ランダム値の更新
    }

    if (cat == 14)
    {
      if (idx == 1400) return QObject::tr("Вихрь"); // 渦巻き
    }

    if (cat == 15)
    {
      if (idx == 1500) return QObject::tr("Волна"); // 波
    }

    if (cat == 16)
    {
      if (idx == 1600) return QObject::tr("Шум"); // ノイズ
      if (idx == 1601) return QObject::tr("Усиление"); // 強さ
      if (idx == 1602) return QObject::tr("Тип"); // タイプ
      if (idx == 1603) return QObject::tr("Цветной"); // カラー
      if (idx == 1604) return QObject::tr("Серый"); // モノクロ
    }

    if (cat == 17)
    {
      if (idx == 1700) return QObject::tr("Яркость/Контраст"); // 明るさ・コントラスト
      if (idx == 1701) return QObject::tr("Яркость"); // 明るさ
      if (idx == 1702) return QObject::tr("Контраст"); // コントラスト
      if (idx == 1703) return QObject::tr("Просмотр"); // プレビュー
    }

    if (cat == 18)
    {
      if (idx == 1800) return QObject::tr("Постеризация"); // ポスタリゼーション
      if (idx == 1801) return QObject::tr("Ступени"); // 段数
    }

    if (cat == 19)
    {
      if (idx == 1900) return QObject::tr("Выравнивание выбранных слоев"); // 選択レイヤーの整列

      if (idx == 1910) return QObject::tr("По вертикали (левый край)"); // 縦に整列 (左端)
      if (idx == 1911) return QObject::tr("По вертикали (центр)"); // 縦に整列 (中央)
      if (idx == 1912) return QObject::tr("По вертикали (правый край)"); // 縦に整列 (右端)
      if (idx == 1913) return QObject::tr("По горизонтали (верхний край)"); // 横に整列 (上端)
      if (idx == 1914) return QObject::tr("По горизонтали (центр)"); // 横に整列 (中央)
      if (idx == 1915) return QObject::tr("По горизонтали (нижний край)"); // 横に整列 (下端)

      if (idx == 1920) return QObject::tr("По краям изображения"); // 画像の端で揃える
      if (idx == 1921) return QObject::tr("По центру изображения"); // 画像の中心で揃える

      if (idx == 1930) return QObject::tr("На основе области выбранного слоя"); // 選択レイヤーの範囲を基準にする
      if (idx == 1931) return QObject::tr("На основе выделения"); // 選択範囲を基準にする
    }
  }

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
  ///////////////////
  // 翻訳がないので英語で
  ///////////////////
  ///////////////////
  if (cat == 0)
  {
    if (idx == 0) return QObject::tr("Input"); // 入力
    if (idx == 1) return QObject::tr("Output"); // 出力
    if (idx == 2) return QObject::tr("Levels"); // レベル補正
    if (idx == 3) return QObject::tr("Extracting Lines"); // 線画抽出
  }

  if (cat == 1)
  {
    if (idx == 100) return QObject::tr("Hue"); // 色相
    if (idx == 101) return QObject::tr("Saturation"); // 彩度
    if (idx == 102) return QObject::tr("Brightness"); // 明度
    if (idx == 103) return QObject::tr("Hue Saturation Brightness"); // 色相・彩度・明度
    if (idx == 104) return QObject::tr("Color Space"); // 色空間
  }

  if (cat == 2)
  {
    if (idx == 200) return QObject::tr("Tone Curve"); // トーンカーブ
    if (idx == 201) return QObject::tr("Reset"); // リセット
  }

  if (cat == 3)
  {
    if (idx == 300) return QObject::tr("Channel Operation"); // チャンネル操作
    if (idx == 301) return QObject::tr("RGB Average"); // RGB平均
    if (idx == 302) return QObject::tr("255 - (RGB Average)"); // 255 - RGB平均
    if (idx == 303) return QObject::tr("Reset"); // リセット
    if (idx == 304) return QObject::tr("Preset"); // プリセット
    if (idx == 310) return QObject::tr("R Component"); // R成分
    if (idx == 311) return QObject::tr("G Component"); // G成分
    if (idx == 312) return QObject::tr("B Component"); // B成分
    if (idx == 313) return QObject::tr("Line Extraction"); // 線画抽出
    if (idx == 314) return QObject::tr("Monochromatization"); // モノクロ化
  }

  if (cat == 4)
  {
    if (idx == 400) return QObject::tr("Chromatic Aberration"); // 色収差/RGBずらし
  }

  if (cat == 5)
  {
    if (idx == 500) return QObject::tr("Unsharp Mask"); // アンシャープマスク
    if (idx == 501) return QObject::tr("Radius"); // 半径
    if (idx == 502) return QObject::tr("Amount"); // 適用量
    if (idx == 503) return QObject::tr("Preview"); // プレビュー
  }

  if (cat == 6)
  {
    if (idx == 600) return QObject::tr("Gaussian Blur"); // ガウスぼかし
    if (idx == 601) return QObject::tr("Value"); // 値
    if (idx == 602) return QObject::tr("Preview"); // プレビュー
  }

  if (cat == 7)
  {
    if (idx == 700) return QObject::tr("Motion Blur"); // 移動ぼかし
    if (idx == 701) return QObject::tr("Intensity"); // 強度
    if (idx == 702) return QObject::tr("Preview"); // プレビュー
  }

  if (cat == 8)
  {
    if (idx == 800) return QObject::tr("Lens Blur"); // レンズぼかし
    if (idx == 801) return QObject::tr("Radius"); // 半径
    if (idx == 802) return QObject::tr("Preview"); // プレビュー
    if (idx == 803) return QObject::tr("Brightness"); // 明るさ
  }

  if (cat == 9)
  {
    if (idx == 900) return QObject::tr("Mosaic"); // モザイク
    if (idx == 901) return QObject::tr("Size"); // サイズ
  }

  if (cat == 10)
  {
    if (idx == 1000) return QObject::tr("Sea (Seigaiha)"); // 青海波
    if (idx == 1001) return QObject::tr("Sea (Hishi Seigaiha)"); // 菱青海波
    if (idx == 1002) return QObject::tr("Sippou"); // 七宝
    if (idx == 1003) return QObject::tr("Sippou 2"); // 七宝 2
    if (idx == 1004) return QObject::tr("Sanki Kuzushi"); // 算木崩し
    if (idx == 1005) return QObject::tr("Arrow (Yagasuri)"); // 矢絣
    if (idx == 1006) return QObject::tr("Arrow 2 (Yagasuri)"); // 矢絣 2
    if (idx == 1007) return QObject::tr("Scale (Uroko)"); // 鱗
    if (idx == 1008) return QObject::tr("Hound's Tooth"); // 千鳥格子
    if (idx == 1009) return QObject::tr("Polka Dot"); // 水玉
    if (idx == 1010) return QObject::tr("Suri Bitta"); // 摺疋田
    if (idx == 1011) return QObject::tr("Turtle (Kikkou)"); // 亀甲
    if (idx == 1012) return QObject::tr("Checker Board"); // 市松
    if (idx == 1013) return QObject::tr("Asanoha"); // 麻の葉
    if (idx == 1014) return QObject::tr("Tatewaku"); // 立涌
    if (idx == 1015) return QObject::tr("Hundou Tsunagi"); // 分銅繋ぎ
    if (idx == 1016) return QObject::tr("Sayagata"); // 紗綾形
    if (idx == 1017) return QObject::tr("Vertical Stripes"); // 縦縞
    if (idx == 1018) return QObject::tr("Horizontal Stripes"); // 横縞

    if (idx == 1050) return QObject::tr("Japanese Pattern"); // 和柄
    if (idx == 1051) return QObject::tr("Name"); // 名称
    if (idx == 1052) return QObject::tr("Size"); // サイズ
    if (idx == 1053) return QObject::tr("Transparent Background"); // 透明背景
    if (idx == 1054) return QObject::tr("Color Exchange"); // 色の入れ替え
    if (idx == 1055) return QObject::tr("Random Color"); // ランダム色指定
    if (idx == 1056) return QObject::tr("Density"); // 密度
    if (idx == 1057) return QObject::tr("Angle"); // 角度
    if (idx == 1058) return QObject::tr("deg"); // 度
  }

  if (cat == 11)
  {
    if (idx == 1100) return QObject::tr("Concentrated Line"); // 集中線
    if (idx == 1101) return QObject::tr("Length"); // 長さ
    if (idx == 1102) return QObject::tr("Length (Random)"); // 長さ (ランダム)
    if (idx == 1103) return QObject::tr("Line Width"); // 線の太さ
    if (idx == 1104) return QObject::tr("Density"); // 密度
    if (idx == 1105) return QObject::tr("Density (Random)"); // 密度 (ランダム)
    if (idx == 1106) return QObject::tr("Aspect Ratio"); // 縦横の比率
    if (idx == 1107) return QObject::tr("Square"); // 正方形
    if (idx == 1108) return QObject::tr("Landscape"); // 横長
    if (idx == 1109) return QObject::tr("Portrait"); // 縦長
    if (idx == 1110) return QObject::tr("Reset"); // リセット
    if (idx == 1111) return QObject::tr("Update Randomness"); // ランダム値の更新
    if (idx == 1112) return QObject::tr("Specify Center"); // 中心の指定
  }

  if (cat == 12)
  {
    if (idx == 1200) return QObject::tr("Parallel Lines"); // 流線
    if (idx == 1201) return QObject::tr("Length"); // 長さ
    if (idx == 1202) return QObject::tr("Length (Random)"); // 長さ (ランダム)
    if (idx == 1203) return QObject::tr("Line Width"); // 線の太さ
    if (idx == 1204) return QObject::tr("Density"); // 密度
    if (idx == 1205) return QObject::tr("Density (Random)"); // 密度 (ランダム)
    if (idx == 1206) return QObject::tr("Reset"); // リセット
    if (idx == 1207) return QObject::tr("Update Randomness"); // ランダム値の更新
    if (idx == 1208) return QObject::tr("Specify Direction"); // 方向の指定
  }

  if (cat == 13)
  {
    if (idx == 1300) return QObject::tr("Sea Urchin Flash"); // ウニフラッシュ
    if (idx == 1301) return QObject::tr("Length"); // 長さ
    if (idx == 1302) return QObject::tr("Length (Random)"); // 長さ (ランダム)
    if (idx == 1303) return QObject::tr("Line Width"); // 線の太さ
    if (idx == 1304) return QObject::tr("Density"); // 密度
    if (idx == 1305) return QObject::tr("Density (Random)"); // 密度 (ランダム)
    if (idx == 1306) return QObject::tr("Offset (Random)"); // オフセット (ランダム)
    if (idx == 1307) return QObject::tr("Fade In/Out Pos"); // 入り抜き位置
    if (idx == 1308) return QObject::tr("Reset"); // リセット
    if (idx == 1309) return QObject::tr("Update Randomness"); // ランダム値の更新
  }

  if (cat == 14)
  {
    if (idx == 1400) return QObject::tr("Swirl"); // 渦巻き
  }

  if (cat == 15)
  {
    if (idx == 1500) return QObject::tr("Wave"); // 波
  }

  if (cat == 16)
  {
    if (idx == 1600) return QObject::tr("Noise"); // ノイズ
    if (idx == 1601) return QObject::tr("Strength"); // 強さ
    if (idx == 1602) return QObject::tr("Type"); // タイプ
    if (idx == 1603) return QObject::tr("Color"); // カラー
    if (idx == 1604) return QObject::tr("Monochrome"); // モノクロ
  }

  if (cat == 17)
  {
    if (idx == 1700) return QObject::tr("Brightness/Contrast"); // 明るさ・コントラスト
    if (idx == 1701) return QObject::tr("Brightness"); // 明るさ
    if (idx == 1702) return QObject::tr("Contrast"); // コントラスト
    if (idx == 1703) return QObject::tr("Preview"); // プレビュー
  }

  if (cat == 18)
  {
    if (idx == 1800) return QObject::tr("Posterization"); // ポスタリゼーション
    if (idx == 1801) return QObject::tr("Steps"); // 段数
  }

  if (cat == 19)
  {
    if (idx == 1900) return QObject::tr("Align Selected Layers"); // 選択レイヤーの整列

    if (idx == 1910) return QObject::tr("Align Vertically (Left End)"); // 縦に整列 (左端)
    if (idx == 1911) return QObject::tr("Align Vertically (Center)"); // 縦に整列 (中央)
    if (idx == 1912) return QObject::tr("Align Vertically (Right End)"); // 縦に整列 (右端)
    if (idx == 1913) return QObject::tr("Align Horizontally (Top End)"); // 横に整列 (上端)
    if (idx == 1914) return QObject::tr("Align Horizontally (Center)"); // 横に整列 (中央)
    if (idx == 1915) return QObject::tr("Align Horizontally (Lower End)"); // 横に整列 (下端)

    if (idx == 1920) return QObject::tr("Align with the edges of an image"); // 画像の端で揃える
    if (idx == 1921) return QObject::tr("Align with the center of the image"); // 画像の中心で揃える

    if (idx == 1930) return QObject::tr("Based on a range of selected layers"); // 選択レイヤーの範囲を基準にする
    if (idx == 1931) return QObject::tr("Based on a selection"); // 選択範囲を基準にする
  }

  return "";
}

QString Trans_StrFilter_Preview()
{
  return Trans_StrFilter( 503 );
}
