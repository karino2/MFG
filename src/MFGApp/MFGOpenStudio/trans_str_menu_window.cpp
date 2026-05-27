/* -*- coding: utf-8 -*- マルチバイト */

// Copyright 2026 PGN Inc. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#if defined(_MSC_VER)
  #pragma execution_character_set("utf-8")
#endif

#include <trans.h>
#include "trans_str_menu_window.h"

///////////////////////////////////////////////////////////////////////////
// - ウィンドウメニューで使用される文字列
// - A string used in the window menu.
///////////////////////////////////////////////////////////////////////////
QString Trans_StrMenuWindow( int idx )
{
  CTranslationManager* man = &Trans();

  //////////////////////////////////
  //////////////////////////////////
  if (man->TranslateJapanese())
  {
    if (idx == 0) return QObject::tr("ウィンドウ(&W)");
    if (idx == 1) return QObject::tr("初期化(&I)...");
    if (idx == 2) return QObject::tr("表示・非表示(&S)");
    if (idx == 3) return QObject::tr("カラー");
    if (idx == 4) return QObject::tr("ブラシプレビュー");
    if (idx == 5) return QObject::tr("ブラシコントロール");
    if (idx == 6) return QObject::tr("ブラシ");
    if (idx == 7) return QObject::tr("ナビゲーター");
    if (idx == 8) return QObject::tr("レイヤー");
    if (idx == 9) return QObject::tr("カラーパレット");
    if (idx == 10) return QObject::tr("ステータスバー");
    if (idx == 11) return QObject::tr("資料");
    if (idx == 13) return QObject::tr("ブラシサイズ");
    if (idx == 14) return QObject::tr("キャンバスの新規ビュー...");
    if (idx == 15) return QObject::tr("スタイル");
    if (idx == 16) return QObject::tr("ライトUI");
    if (idx == 17) return QObject::tr("ダークUI");
    if (idx == 18) return QObject::tr("カラーヒストリー");
    if (idx == 19) return QObject::tr("プロジェクト");
    if (idx == 20) return QObject::tr("キャンバスをタイル状に表示...");
  }

  //////////////////////////////////
  //////////////////////////////////
  if (man->TranslateChineseSimp())
  {
    if (idx == 0) return QObject::tr("窗口(&W)");
    if (idx == 1) return QObject::tr("初始化(&I)...");
    if (idx == 2) return QObject::tr("显示 / 非显示(&S)");
    if (idx == 3) return QObject::tr("颜色");
    if (idx == 4) return QObject::tr("笔刷预览");
    if (idx == 5) return QObject::tr("笔刷控制");
    if (idx == 6) return QObject::tr("笔刷");
    if (idx == 7) return QObject::tr("导览");
    if (idx == 8) return QObject::tr("图层");
    if (idx == 9) return QObject::tr("色板");
    if (idx == 10) return QObject::tr("状态栏");
    if (idx == 11) return QObject::tr("资料");
    if (idx == 13) return QObject::tr("刷子大小");
    if (idx == 14) return QObject::tr("画布的新视图...");
    if (idx == 15) return QObject::tr("风格"); // スタイル
    if (idx == 16) return QObject::tr("明亮介面"); // ライトUI
    if (idx == 17) return QObject::tr("暗色系介面"); // ダークUI
    if (idx == 18) return QObject::tr("色彩历史"); // カラーヒストリー
    if (idx == 19) return QObject::tr("项目"); // プロジェクト
    //if (idx == 20) return QObject::tr("Display the Canvas in a Tiled Pattern..."); // キャンバスをタイル状に表示...
  }

  //////////////////////////////////
  //////////////////////////////////
  if (man->TranslateChineseTrad())
  {
    if (idx == 0) return QObject::tr("視窗(&W)");
    if (idx == 1) return QObject::tr("初始化(&I)...");
    if (idx == 2) return QObject::tr("顯示 / 非顯示(&S)");
    if (idx == 3) return QObject::tr("色彩");
    if (idx == 4) return QObject::tr("筆刷預覽");
    if (idx == 5) return QObject::tr("筆刷控制");
    if (idx == 6) return QObject::tr("筆刷");
    if (idx == 7) return QObject::tr("導覽");
    if (idx == 8) return QObject::tr("圖層");
    if (idx == 9) return QObject::tr("色板");
    if (idx == 10) return QObject::tr("狀態欄");
    if (idx == 11) return QObject::tr("資料");
    if (idx == 13) return QObject::tr("刷子大小");
    if (idx == 14) return QObject::tr("畫布的新視圖...");
    if (idx == 15) return QObject::tr("風格"); // スタイル
    if (idx == 16) return QObject::tr("明亮介面"); // ライトUI
    if (idx == 17) return QObject::tr("暗色系介面"); // ダークUI
    if (idx == 18) return QObject::tr("色彩歷史"); // カラーヒストリー
    if (idx == 19) return QObject::tr("專案"); // プロジェクト
    //if (idx == 20) return QObject::tr("Display the Canvas in a Tiled Pattern..."); // キャンバスをタイル状に表示...
  }

  //////////////////////////////////
  //////////////////////////////////
  if (man->TranslateKorean())
  {
    if (idx == 0) return QObject::tr("창(&W)");
    if (idx == 1) return QObject::tr("초기화(&I)...");
    if (idx == 2) return QObject::tr("표시/비표시(&S)");
    if (idx == 3) return QObject::tr("컬러");
    if (idx == 4) return QObject::tr("브러시 미리보기");
    if (idx == 5) return QObject::tr("브러시 컨트롤");
    if (idx == 6) return QObject::tr("브러시");
    if (idx == 7) return QObject::tr("내비게이터");
    if (idx == 8) return QObject::tr("레이어");
    if (idx == 9) return QObject::tr("팔레트");
    if (idx == 10) return QObject::tr("상태바");
    if (idx == 11) return QObject::tr("참고 자료");
    if (idx == 13) return QObject::tr("브러시 사이즈");
    if (idx == 14) return QObject::tr("캔버스 새로운 뷰...");
    if (idx == 15) return QObject::tr("스타일"); // スタイル
    if (idx == 16) return QObject::tr("라이트 UI"); // ライトUI
    if (idx == 17) return QObject::tr("다크 UI"); // ダークUI
    if (idx == 18) return QObject::tr("컬러 히스토리"); // カラーヒストリー
    if (idx == 19) return QObject::tr("프로젝트"); // プロジェクト
    //if (idx == 20) return QObject::tr("Display the Canvas in a Tiled Pattern..."); // キャンバスをタイル状に表示...
  }

  //////////////////////////////////
  //////////////////////////////////
  if (man->TranslatePortugues())
  {
    if (idx == 0) return QObject::tr("Janela(&W)");
    if (idx == 1) return QObject::tr("Inicializar(&I)...");
    if (idx == 2) return QObject::tr("Mostrar/Ocultar(&S)");
    if (idx == 3) return QObject::tr("Cor");
    if (idx == 4) return QObject::tr("Visualização do Pincel");
    if (idx == 5) return QObject::tr("Controle de Pincel");
    if (idx == 6) return QObject::tr("Pincel");
    if (idx == 7) return QObject::tr("Navegador");
    if (idx == 8) return QObject::tr("Camada");
    if (idx == 9) return QObject::tr("Paleta");
    if (idx == 10) return QObject::tr("Barra de estado");
    if (idx == 11) return QObject::tr("Referência");
    if (idx == 13) return QObject::tr("Tamanho do pincel");
    if (idx == 14) return QObject::tr("Nova visão da tela...");
    if (idx == 15) return QObject::tr("Estilos"); // スタイル
    if (idx == 16) return QObject::tr("Interface Clara"); // ライトUI
    if (idx == 17) return QObject::tr("Interface Escura"); // ダークUI
    if (idx == 18) return QObject::tr("Histórico de cores"); // カラーヒストリー
    if (idx == 19) return QObject::tr("Projeto"); // プロジェクト
    //if (idx == 20) return QObject::tr("Display the Canvas in a Tiled Pattern..."); // キャンバスをタイル状に表示...
  }

  //////////////////////////////////
  //////////////////////////////////
  if (man->TranslateSpanish())
  {
    if (idx == 0) return QObject::tr("Ventana(&W)");
    if (idx == 1) return QObject::tr("Inicializar(&I)...");
    if (idx == 2) return QObject::tr("Mostrar/Ocultar(&S)");
    if (idx == 3) return QObject::tr("Color");
    if (idx == 4) return QObject::tr("Vista previa del pincel");
    if (idx == 5) return QObject::tr("Control de pinceles");
    if (idx == 6) return QObject::tr("Pinceles");
    if (idx == 7) return QObject::tr("Navegador");
    if (idx == 8) return QObject::tr("Capas");
    if (idx == 9) return QObject::tr("Paleta");
    if (idx == 10) return QObject::tr("Barra de estado");
    if (idx == 11) return QObject::tr("Reference"); // 資料
    if (idx == 13) return QObject::tr("Tamaño del pincel"); // ブラシサイズ
    if (idx == 14) return QObject::tr("Nueva vista del lienzo"); // キャンバスの新規ビュー...
    if (idx == 15) return QObject::tr("Estilos"); // スタイル
    if (idx == 16) return QObject::tr("IU clara"); // ライトUI
    if (idx == 17) return QObject::tr("IU oscura"); // ダークUI
    if (idx == 18) return QObject::tr("Historial del colores"); // カラーヒストリー
    if (idx == 19) return QObject::tr("Proyecto"); // プロジェクト
    //if (idx == 20) return QObject::tr("Display the Canvas in a Tiled Pattern..."); // キャンバスをタイル状に表示...
  }

  //////////////////////////////////
  //////////////////////////////////
  if (man->TranslateGerman())
  {
    if (idx == 0) return QObject::tr("Fenster(&W)");
    if (idx == 1) return QObject::tr("Initialisieren(&I)...");
    if (idx == 2) return QObject::tr("Ein-/ausblenden(&S)");
    if (idx == 3) return QObject::tr("Farbe");
    if (idx == 4) return QObject::tr("Pinselprüfung ");
    if (idx == 5) return QObject::tr("Pinselkontrolle");
    if (idx == 6) return QObject::tr("Pinsel");
    if (idx == 7) return QObject::tr("Navigator");
    if (idx == 8) return QObject::tr("Ebene");
    if (idx == 9) return QObject::tr("Palette");
    if (idx == 10) return QObject::tr("Statusleiste");
    if (idx == 11) return QObject::tr("Reference"); // 資料
    if (idx == 13) return QObject::tr("Pinselgröße"); // ブラシサイズ
    if (idx == 14) return QObject::tr("Neue Ansicht der Leinwand"); // キャンバスの新規ビュー...
    if (idx == 15) return QObject::tr("Stile"); // スタイル
    if (idx == 16) return QObject::tr("Helle UI"); // ライトUI
    if (idx == 17) return QObject::tr("Dunkle UI"); // ダークUI
    if (idx == 18) return QObject::tr("Farbgeschichte"); // カラーヒストリー
    if (idx == 19) return QObject::tr("Projekt"); // プロジェクト
    //if (idx == 20) return QObject::tr("Display the Canvas in a Tiled Pattern..."); // キャンバスをタイル状に表示...
  }

  //////////////////////////////////
  //////////////////////////////////
  if (man->TranslateFrench())
  {
    if (idx == 0) return QObject::tr("Fenêtre(&W)");
    if (idx == 1) return QObject::tr("Initialiser(&I)...");
    if (idx == 2) return QObject::tr("Afficher/Masquer(&S)");
    if (idx == 3) return QObject::tr("Couleur");
    if (idx == 4) return QObject::tr("Aperçu du pinceau");
    if (idx == 5) return QObject::tr("Option du pinceau");
    if (idx == 6) return QObject::tr("Pinceau");
    if (idx == 7) return QObject::tr("Navigateur");
    if (idx == 8) return QObject::tr("Masque");
    if (idx == 9) return QObject::tr("Palette");
    if (idx == 10) return QObject::tr("Barre d'état");
    if (idx == 11) return QObject::tr("Reference"); // 資料
    if (idx == 13) return QObject::tr("Taille du pinceau"); // ブラシサイズ
    if (idx == 14) return QObject::tr("Nouvelle vue du canevas"); // キャンバスの新規ビュー...
    if (idx == 15) return QObject::tr("modes"); // スタイル
    if (idx == 16) return QObject::tr("Interface claire"); // ライトUI
    if (idx == 17) return QObject::tr("Interface sombre"); // ダークUI
    if (idx == 18) return QObject::tr("Historique des couleurs"); // カラーヒストリー
    if (idx == 19) return QObject::tr("Projet"); // プロジェクト
    //if (idx == 20) return QObject::tr("Display the Canvas in a Tiled Pattern..."); // キャンバスをタイル状に表示...
  }

  //////////////////////////////////
  //////////////////////////////////
  if (man->TranslateRussian())
  {
    if (idx == 0) return QObject::tr("Окно(&W)");
    if (idx == 1) return QObject::tr("Инициализация(&I)...");
    if (idx == 2) return QObject::tr("Показать/спрятать(&S)");
    if (idx == 3) return QObject::tr("Цвет");
    if (idx == 4) return QObject::tr("Просмотр кисти");
    if (idx == 5) return QObject::tr("Параметры кисти");
    if (idx == 6) return QObject::tr("Кисть");
    if (idx == 7) return QObject::tr("Навигатор");
    if (idx == 8) return QObject::tr("Слой");
    if (idx == 9) return QObject::tr("Палитра");
    if (idx == 10) return QObject::tr("Панель статуса");
    if (idx == 11) return QObject::tr("Справка");
    if (idx == 13) return QObject::tr("Размер кисти"); // ブラシサイズ
    if (idx == 14) return QObject::tr("Новый вид холста"); // キャンバスの新規ビュー...
    if (idx == 15) return QObject::tr("Стили"); // スタイル
    if (idx == 16) return QObject::tr("Светлый интерфейс"); // ライトUI
    if (idx == 17) return QObject::tr("Темный интерфейс"); // ダークUI
    if (idx == 18) return QObject::tr("История цвета"); // カラーヒストリー
    //if (idx == 19) return QObject::tr("Project"); // プロジェクト
    //if (idx == 20) return QObject::tr("Display the Canvas in a Tiled Pattern..."); // キャンバスをタイル状に表示...
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

  ///////////////////////////////////////////////////
  ///////////////////////////////////////////////////
  // 英語は mainwindow.ui が使われるので、下の翻訳は使われない
  ///////////////////////////////////////////////////
  ///////////////////////////////////////////////////
  if (idx == 0) return QObject::tr("Window(&W)"); // ウィンドウ(&W)
  if (idx == 1) return QObject::tr("Initialize(&I)..."); // 初期化(&I)...
  if (idx == 2) return QObject::tr("Show/Hide(&S)"); // 表示・非表示(&S)
  if (idx == 3) return QObject::tr("Color"); // カラー
  if (idx == 4) return QObject::tr("Brush Preview"); // ブラシプレビュー
  if (idx == 5) return QObject::tr("Brush Control"); // ブラシコントロール
  if (idx == 6) return QObject::tr("Brush"); // ブラシ
  if (idx == 7) return QObject::tr("Navigator"); // ナビゲーター
  if (idx == 8) return QObject::tr("Layer"); // レイヤー
  if (idx == 9) return QObject::tr("Palette"); // パレット
  if (idx == 10) return QObject::tr("Status Bar"); // ステータスバー
  if (idx == 11) return QObject::tr("Reference"); // 資料
  if (idx == 13) return QObject::tr("Brush Size"); // ブラシサイズ
  if (idx == 14) return QObject::tr("New View of Canvas"); // キャンバスの新規ビュー...
  if (idx == 15) return QObject::tr("Styles"); // スタイル
  if (idx == 16) return QObject::tr("Light UI"); // ライトUI
  if (idx == 17) return QObject::tr("Dark UI"); // ダークUI
  if (idx == 18) return QObject::tr("Color History"); // カラーヒストリー
  if (idx == 19) return QObject::tr("Project"); // プロジェクト
  if (idx == 20) return QObject::tr("Display the Canvas in a Tiled Pattern..."); // キャンバスをタイル状に表示...

  return "";
}
