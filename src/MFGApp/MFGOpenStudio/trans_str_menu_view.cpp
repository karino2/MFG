/* -*- coding: utf-8 -*- マルチバイト */

// Copyright 2026 PGN Inc. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#if defined(_MSC_VER)
  #pragma execution_character_set("utf-8")
#endif

#include <trans.h>
#include "trans_str_menu_view.h"

///////////////////////////////////////////////////////////////////////////
// - 表示メニューで使用される文字列
// - A string used in the view menu.
///////////////////////////////////////////////////////////////////////////
QString Trans_StrMenuView( int idx )
{
  CTranslationManager* man = &Trans();

  //////////////////////////////////
  //////////////////////////////////
  if (man->TranslateJapanese())
  {
    if (idx == 0) return QObject::tr("表示(&V)"); // View
    if (idx == 1) return QObject::tr("拡大表示(&I)"); // Zoom In
    if (idx == 2) return QObject::tr("縮小表示(&O)"); // Zoom Out
    if (idx == 3) return QObject::tr("ウィンドウサイズに合わせる(&F)"); // Fit to Window Size
    if (idx == 4) return QObject::tr("左回転(&L)"); // Rotate Left
    if (idx == 5) return QObject::tr("回転・反転を解除(&S)"); // Release Rotate/Flip
    if (idx == 6) return QObject::tr("右回転(&R)"); // Rotate Right
    if (idx == 7) return QObject::tr("左右反転(&H)"); // Flip Horizontally
    if (idx == 8) return QObject::tr("グリッド(&G)"); // Grid
    if (idx == 9) return QObject::tr("透明背景(&T)"); // Transparent Background
    if (idx == 10) return QObject::tr("ブラシサイズカーソル(&B)"); // Brush Size Cursor
    if (idx == 11) return QObject::tr("ピクセル単位のグリッド(&P)"); // Pixel Grid
    if (idx == 12) return QObject::tr("背景色の指定(&D)..."); // Background Color
    if (idx == 13) return QObject::tr("漫画原稿ガイドの表示(&M)"); // Show Comic Guide Lines
    if (idx == 14) return QObject::tr("漫画原稿ガイド設定(&S)..."); // Comic Guide Lines Settings
    if (idx == 21) return QObject::tr("カラープロファイルを適用して表示"); // Display with Applying Color Profile
    if (idx == 22) return QObject::tr("CMYKソフトプルーフ"); // CMYK Soft Proof
    if (idx == 23) return QObject::tr("カラーマネジメント設定..."); // Color Management Settings...
    if (idx == 24) return QObject::tr("ガイド"); // Guides
    if (idx == 25) return QObject::tr("ガイド編集"); // Edit Guides
    if (idx == 27) return QObject::tr("グリッド設定..."); // Grid Settings
    if (idx == 28) return QObject::tr("100%"); // 100%
    if (idx == 29) return QObject::tr("200%"); // 200%
    if (idx == 30) return QObject::tr("アクティブレイヤーのみ表示(&A)"); // Show only Active Layer
    if (idx == 31) return QObject::tr("暗い市松模様"); // Dark Checkered Pattern
    if (idx == 32) return QObject::tr("ルーラー"); // Ruler
    if (idx == 33) return QObject::tr("ガイドを追加..."); // Add Guide
    if (idx == 34) return QObject::tr("ガイドをクリア"); // Clear Guides
    if (idx == 35) return QObject::tr("ルーラー設定"); // Ruler Settings
    if (idx == 36) return QObject::tr("ブラシカーソルを隠す(&C)"); // Hide Brush Cursor
    if (idx == 40) return QObject::tr("優先度設定"); // Priority Settings
    if (idx == 41) return QObject::tr("レスポンス (遅延更新)"); // Response (Delayed update)
    if (idx == 42) return QObject::tr("パフォーマンス (一気に更新)"); // Performance (Update at once)
    if (idx == 43) return QObject::tr("グレースケール表示"); // Grayscale Display
  }

  //////////////////////////////////
  //////////////////////////////////
  if (man->TranslateChineseSimp())
  {
    if (idx == 0) return QObject::tr("显示(&V)"); // View
    if (idx == 1) return QObject::tr("放大(&I)"); // Zoom In
    if (idx == 2) return QObject::tr("缩小(&O)"); // Zoom Out
    if (idx == 3) return QObject::tr("配合窗口尺寸(&F)"); // Fit to Window Size
    if (idx == 4) return QObject::tr("左旋转(&L)"); // Rotate Left
    if (idx == 5) return QObject::tr("旋转初始化(&S)"); // Release Rotate/Flip
    if (idx == 6) return QObject::tr("右旋转(&R)"); // Rotate Right
    if (idx == 7) return QObject::tr("左右反转(&H)"); // Flip Horizontally
    if (idx == 8) return QObject::tr("网格(&G)"); // Grid
    if (idx == 9) return QObject::tr("透明背景(&T)"); // Transparent Background
    if (idx == 10) return QObject::tr("笔刷尺寸(&B)"); // Brush Size
    if (idx == 11) return QObject::tr("像素网格(&P)"); // Pixel Grid
    if (idx == 12) return QObject::tr("背景色(&D)..."); // Background Color
    if (idx == 13) return QObject::tr("显示漫画原稿指南(&M)"); // Show Comic Guide Lines
    if (idx == 14) return QObject::tr("漫画原稿指南设定(&S)..."); // Comic Guide Lines Settings
    if (idx == 21) return QObject::tr("显示要使用的颜色"); // カラープロファイルを適用して表示
    if (idx == 22) return QObject::tr("CMYK软式打样"); // CMYKソフトプルーフ
    if (idx == 23) return QObject::tr("颜色管理设定"); // カラーマネジメント設定...
    if (idx == 24) return QObject::tr("指引线"); // ガイド線
    if (idx == 25) return QObject::tr("辅助线编辑"); // ガイド編集
    if (idx == 27) return QObject::tr("格线设定"); // グリッド設定...
    if (idx == 28) return QObject::tr("100%"); // 100%
    if (idx == 29) return QObject::tr("200%"); // 200%
    if (idx == 30) return QObject::tr("仅显示活动层(&A)"); // アクティブレイヤーのみ表示(&A)
    if (idx == 31) return QObject::tr("黯淡市松模样"); // 暗い市松模様
    if (idx == 32) return QObject::tr("标尺"); // ルーラー
    if (idx == 33) return QObject::tr("添加辅助线..."); // ガイドを追加...
    if (idx == 34) return QObject::tr("清除辅助线"); // ガイドをクリア
    if (idx == 35) return QObject::tr("标尺设置"); // ルーラー設定
    if (idx == 36) return QObject::tr("隐藏画笔光标(&C)"); // ブラシカーソルを隠す(&C)
    if (idx == 40) return QObject::tr("优先级设置"); // Priority Settings
    if (idx == 41) return QObject::tr("响应 (延迟更新）"); // Response (Delayed update)
    if (idx == 42) return QObject::tr("性能 (集中更新)"); // Performance (Update at once)
    if (idx == 43) return QObject::tr("灰度显示"); // Grayscale Display
  }

  //////////////////////////////////
  //////////////////////////////////
  if (man->TranslateChineseTrad())
  {
    if (idx == 0) return QObject::tr("顯示(&V)"); // View
    if (idx == 1) return QObject::tr("放大(&I)"); // Zoom In
    if (idx == 2) return QObject::tr("縮小(&O)"); // Zoom Out
    if (idx == 3) return QObject::tr("配合視窗尺寸(&F)"); // Fit to Window Size
    if (idx == 4) return QObject::tr("向左旋轉(&L)"); // Rotate Left
    if (idx == 5) return QObject::tr("解除旋轉(&S)"); // Release Rotate/Flip
    if (idx == 6) return QObject::tr("向右旋轉(&R)"); // Rotate Right
    if (idx == 7) return QObject::tr("水平翻轉(&H)"); // Flip Horizontally
    if (idx == 8) return QObject::tr("網格(&G)"); // Grid
    if (idx == 9) return QObject::tr("透明背景(&T)"); // Transparent Background
    if (idx == 10) return QObject::tr("筆刷大小(&B)"); // Brush Size
    if (idx == 11) return QObject::tr("像素網格(&P)"); // Pixel Grid
    if (idx == 12) return QObject::tr("背景顏色(&D)..."); // Background Color
    if (idx == 13) return QObject::tr("顯示漫畫原稿導覽(&M)"); // Show Comic Guide Lines
    if (idx == 14) return QObject::tr("漫畫原稿導覽設定(&S)..."); // Comic Guide Lines Settings
    if (idx == 21) return QObject::tr("顯示要使用的顏色"); // カラープロファイルを適用して表示
    if (idx == 22) return QObject::tr("CMYK軟式打樣"); // CMYKソフトプルーフ
    if (idx == 23) return QObject::tr("顏色管理設定"); // カラーマネジメント設定...
    if (idx == 24) return QObject::tr("指引線"); // ガイド線
    if (idx == 25) return QObject::tr("輔助線編輯"); // ガイド編集
    if (idx == 27) return QObject::tr("格線設定"); // グリッド設定...
    if (idx == 28) return QObject::tr("100%"); // 100%
    if (idx == 29) return QObject::tr("200%"); // 200%
    if (idx == 30) return QObject::tr("僅顯示活動層(&A)"); // アクティブレイヤーのみ表示(&A)
    if (idx == 31) return QObject::tr("黯淡市松模樣"); // 暗い市松模様
    if (idx == 32) return QObject::tr("尺規"); // ルーラー
    if (idx == 33) return QObject::tr("新增輔助線..."); // ガイドを追加...
    if (idx == 34) return QObject::tr("清除輔助線"); // ガイドをクリア
    if (idx == 35) return QObject::tr("尺規設定"); // ルーラー設定
    if (idx == 36) return QObject::tr("隱藏畫筆游標(&C)"); // ブラシカーソルを隠す(&C)
    if (idx == 40) return QObject::tr("優先順序設定"); // Priority Settings
    if (idx == 41) return QObject::tr("回應 (延遲更新)"); // Response (Delayed update)
    if (idx == 42) return QObject::tr("效能 (集中更新)"); // Performance (Update at once)
    if (idx == 43) return QObject::tr("灰階顯示"); // Grayscale Display
  }

  //////////////////////////////////
  //////////////////////////////////
  if (man->TranslateKorean())
  {
    if (idx == 0) return QObject::tr("표시(&V)"); // View
    if (idx == 1) return QObject::tr("확대표시(&I)"); // Zoom In
    if (idx == 2) return QObject::tr("축소표시(&O)"); // Zoom Out
    if (idx == 3) return QObject::tr("창 사이즈에 맞추기(&F)"); // Fit to Window Size
    if (idx == 4) return QObject::tr("좌회전(&L)"); // Rotate Left
    if (idx == 5) return QObject::tr("회전/반전 초기화(&S)"); // Release Rotate/Flip
    if (idx == 6) return QObject::tr("우회전(&R)"); // Rotate Right
    if (idx == 7) return QObject::tr("좌우 반전(&H)"); // Flip Horizontally
    if (idx == 8) return QObject::tr("그리드(&G)"); // Grid
    if (idx == 9) return QObject::tr("투명색 배경(&T)"); // Transparent Background
    if (idx == 10) return QObject::tr("브러시 사이즈(&B)"); // Brush Size
    if (idx == 11) return QObject::tr("픽셀 단위 그리드(&P)"); // Pixel Grid
    if (idx == 12) return QObject::tr("배경색 지정(&D)..."); // Background Color
    if (idx == 13) return QObject::tr("만화원고 가이드 표시(&M)"); // Show Comic Guide Lines
    if (idx == 14) return QObject::tr("만화원고 가이드 설정(&S)..."); // Comic Guide Lines Settings
    if (idx == 21) return QObject::tr("컬러 프로파일 적용하여 표시"); // カラープロファイルを適用して表示
    if (idx == 22) return QObject::tr("CMYK 소프트 프루프"); // CMYKソフトプルーフ
    if (idx == 23) return QObject::tr("컬러 매니지먼트 세팅"); // カラーマネジメント設定...
    if (idx == 24) return QObject::tr("가이드선"); // ガイド線
    if (idx == 25) return QObject::tr("가이드 편집"); // ガイド編集
    if (idx == 27) return QObject::tr("그리드 설정"); // グリッド設定...
    if (idx == 28) return QObject::tr("100%"); // 100%
    if (idx == 29) return QObject::tr("200%"); // 200%
    if (idx == 30) return QObject::tr("활성 레이어만 표시(&A)"); // アクティブレイヤーのみ表示(&A)
    if (idx == 31) return QObject::tr("어두운 체크 무늬"); // 暗い市松模様
    if (idx == 32) return QObject::tr("자 rulers"); // ルーラー
    if (idx == 33) return QObject::tr("가이드 추가..."); // ガイドを追加...
    if (idx == 34) return QObject::tr("가이드 지우기"); // ガイドをクリア
    if (idx == 35) return QObject::tr("자 설정"); // ルーラー設定
    if (idx == 36) return QObject::tr("브러시 커서 숨기기(&C)"); // ブラシカーソルを隠す(&C)
    if (idx == 40) return QObject::tr("우선순위 설정"); // Priority Settings
    if (idx == 41) return QObject::tr("응답 (지연 업데이트)"); // Response (Delayed update)
    if (idx == 42) return QObject::tr("성능(일괄 업데이트)"); // Performance (Update at once)
    if (idx == 43) return QObject::tr("그레이스케일 표시"); // Grayscale Display
  }

  //////////////////////////////////
  //////////////////////////////////
  if (man->TranslatePortugues())
  {
    if (idx == 0) return QObject::tr("Visualizar(&V)"); // View
    if (idx == 1) return QObject::tr("Aumentar Zoom(&I)"); // Zoom In
    if (idx == 2) return QObject::tr("Diminuir Zoom(&O)"); // Zoom Out
    if (idx == 3) return QObject::tr("Ajustar ao Tamanho da Janela(&F)"); // Fit to Window Size
    if (idx == 4) return QObject::tr("Girar à Esquerda(&L)"); // Rotate Left
    if (idx == 5) return QObject::tr("Liberar Girar/Inverter(&S)"); // Release Rotate/Flip
    if (idx == 6) return QObject::tr("Girar à Direita(&R)"); // Rotate Right
    if (idx == 7) return QObject::tr("Inverter Horizontalmente(&H)"); // Flip Horizontally
    if (idx == 8) return QObject::tr("Grade(&G)"); // Grid
    if (idx == 9) return QObject::tr("Plano de Fundo Transparente(&T)"); // Transparent Background
    if (idx == 10) return QObject::tr("Tamanho do pincel(&B)"); // Brush Size
    if (idx == 11) return QObject::tr("Grade de pincel(&P)"); // Pixel Grid
    if (idx == 12) return QObject::tr("Cor de plano de fundo(&D)..."); // Background Color
    if (idx == 13) return QObject::tr("Mostrar linhas-guia cômicas(&M)"); // Show Comic Guide Lines
    if (idx == 14) return QObject::tr("Configurações do guia de quadrinhos(&S)..."); // Comic Guide Lines Settings
    if (idx == 21) return QObject::tr("Aplicar e Exibir Perfil de Cor"); // カラープロファイルを適用して表示
    if (idx == 22) return QObject::tr("Prova Suave CMYK"); // CMYKソフトプルーフ
    if (idx == 23) return QObject::tr("Configurações de Gerenciamento de Cores"); // カラーマネジメント設定...
    if (idx == 24) return QObject::tr("Linhas de Guia"); // ガイド
    if (idx == 25) return QObject::tr("Editar Guias"); // ガイド編集
    if (idx == 27) return QObject::tr("Configurações de Grade"); // グリッド設定...
    if (idx == 28) return QObject::tr("100%"); // 100%
    if (idx == 29) return QObject::tr("200%"); // 200%
    if (idx == 30) return QObject::tr("Exibir Apenas Camada Ativa(&A)"); // アクティブレイヤーのみ表示(&A)
    if (idx == 31) return QObject::tr("Xadrez Escuro"); // 暗い市松模様
    if (idx == 32) return QObject::tr("Régua"); // ルーラー
    if (idx == 33) return QObject::tr("Adicionar Guia..."); // ガイドを追加...
    if (idx == 34) return QObject::tr("Limpar Guias"); // ガイドをクリア
    if (idx == 35) return QObject::tr("Configurações de Régua"); // ルーラー設定
    if (idx == 36) return QObject::tr("Ocultar Cursor do Pincel (&C)"); // ブラシカーソルを隠す(&C)
    if (idx == 40) return QObject::tr("Configuração de prioridade"); // Priority Settings
    if (idx == 41) return QObject::tr("Resposta (atualização atrasada)"); // Response (Delayed update)
    if (idx == 42) return QObject::tr("Performance (atualização em lote)"); // Performance (Update at once)
    if (idx == 43) return QObject::tr("Exibir em escala de cinza"); // Grayscale Display
  }

  //////////////////////////////////
  //////////////////////////////////
  if (man->TranslateSpanish())
  {
    if (idx == 0) return QObject::tr("Visualizaciön(&V)"); // View
    if (idx == 1) return QObject::tr("Aumentar(&I)"); // Zoom In
    if (idx == 2) return QObject::tr("Reducir(&O)"); // Zoom Out
    if (idx == 3) return QObject::tr("Ajustar al tamaño de la ventana(&F)"); // Fit to Window Size
    if (idx == 4) return QObject::tr("Rotar hacia la izquierda(&L)"); // Rotate Left
    if (idx == 5) return QObject::tr("Eliminar rotación/volteo(&S)"); // Release Rotate/Flip
    if (idx == 6) return QObject::tr("Rotar hacia la derecha(&R)"); // Rotate Right
    if (idx == 7) return QObject::tr("Voltear horizontalmente(&H)"); // Flip Horizontally
    if (idx == 8) return QObject::tr("Cuadrícula(&G)"); // Grid
    if (idx == 9) return QObject::tr("Fondo transparente(&T)"); // Transparent Background
    if (idx == 10) return QObject::tr("Tamaño del pincel(&B)"); // Brush Size
    if (idx == 11) return QObject::tr("Cuadrícula de píxeles(&P)"); // Pixel Grid
    if (idx == 12) return QObject::tr("Especificar color de fondo(&D)..."); // Background Color
    if (idx == 13) return QObject::tr("Mostrar líneas guía para cómic(&M)"); // Show Comic Guide Lines
    if (idx == 14) return QObject::tr("Ajustes de líneas guía gula cómic(&S)..."); // Comic Guide Lines Settings
    if (idx == 21) return QObject::tr("Aplicar y visualizar perfiles de color"); // カラープロファイルを適用して表示
    if (idx == 22) return QObject::tr("Prueba en pantalla CMYK"); // CMYKソフトプルーフ
    if (idx == 23) return QObject::tr("Configuración de gestión del color..."); // カラーマネジメント設定...
    if (idx == 24) return QObject::tr("Líneas guía"); // ガイド
    if (idx == 25) return QObject::tr("Editar guías"); // ガイド編集
    if (idx == 27) return QObject::tr("Configuración de cuadrícula..."); // グリッド設定...
    if (idx == 28) return QObject::tr("100%"); // 100%
    if (idx == 29) return QObject::tr("200%"); // 200%
    if (idx == 30) return QObject::tr("Mostrar sólo la capa activa(&A)"); // アクティブレイヤーのみ表示(&A)
    if (idx == 31) return QObject::tr("Patrón de cuadros oscuros"); // 暗い市松模様
    if (idx == 32) return QObject::tr("Regla"); // ルーラー
    if (idx == 33) return QObject::tr("Añadir guía..."); // ガイドを追加...
    if (idx == 34) return QObject::tr("Guías claras"); // ガイドをクリア
    if (idx == 35) return QObject::tr("Configuración de la regla"); // ルーラー設定
    if (idx == 36) return QObject::tr("Ocultar cursor de pincel(&C)"); // ブラシカーソルを隠す(&C)
    if (idx == 40) return QObject::tr("Configuración de prioridad"); // Priority Settings
    if (idx == 41) return QObject::tr("Respuesta (actualización retardada)"); // Response (Delayed update)
    if (idx == 42) return QObject::tr("Rendimiento (actualización en bloque)"); // Performance (Update at once)
    if (idx == 43) return QObject::tr("Mostrar en escala de grises"); // Grayscale Display
  }

  //////////////////////////////////
  //////////////////////////////////
  if (man->TranslateGerman())
  {
    if (idx == 0) return QObject::tr("Ansicht(&V)"); // View
    if (idx == 1) return QObject::tr("Vergrößern(&I)"); // Zoom In
    if (idx == 2) return QObject::tr("Verkleinern(&O)"); // Zoom Out
    if (idx == 3) return QObject::tr("An Fenstergröße anpassen(&F)"); // Fit to Window Size
    if (idx == 4) return QObject::tr("Nach links drehen(&L)"); // Rotate Left
    if (idx == 5) return QObject::tr("Drehung freigeben/umdrehen(&S)"); // Release Rotate/Flip
    if (idx == 6) return QObject::tr("Rechtsdrehung(&R)"); // Rotate Right
    if (idx == 7) return QObject::tr("Umdrehen(&H)"); // Flip Horizontally
    if (idx == 8) return QObject::tr("Raster(&G)"); // Grid
    if (idx == 9) return QObject::tr("Transparenter Hintergrund(&T)"); // Transparent Background
    if (idx == 10) return QObject::tr("Pinselgröße(&B)"); // Brush Size
    if (idx == 11) return QObject::tr("Pixelgitter(&P)"); // Pixel Grid
    if (idx == 12) return QObject::tr("Hintergrundfarbe(&D)..."); // Background Color
    if (idx == 13) return QObject::tr("Comic-Guides anzeigen(&M)"); // Show Comic Guide Lines
    if (idx == 14) return QObject::tr("Einstellung der Comic-Richtlinien(&S)..."); // Comic Guide Lines Settings
    if (idx == 21) return QObject::tr("Farbmanagement-Einstellungen"); // カラープロファイルを適用して表示
    if (idx == 22) return QObject::tr("CMYK-Weichzeichner"); // CMYKソフトプルーフ
    if (idx == 23) return QObject::tr("Color Management Settings..."); // カラーマネジメント設定...
    if (idx == 24) return QObject::tr("Hilfslinien"); // ガイド
    if (idx == 25) return QObject::tr("Hilfslinien bearbeiten"); // ガイド編集
    if (idx == 27) return QObject::tr("Raster-Einstellungen"); // グリッド設定...
    if (idx == 28) return QObject::tr("100%"); // 100%
    if (idx == 29) return QObject::tr("200%"); // 200%
    if (idx == 30) return QObject::tr("Nur aktive Ebene anzeigen(&A)"); // アクティブレイヤーのみ表示(&A)
    if (idx == 31) return QObject::tr("Dunkles Schachbrettmuster"); // 暗い市松模様
    if (idx == 32) return QObject::tr("Lineal"); // ルーラー
    if (idx == 33) return QObject::tr("Hilfslinie hinzufügen..."); // ガイドを追加...
    if (idx == 34) return QObject::tr("Hilfslinien löschen"); // ガイドをクリア
    if (idx == 35) return QObject::tr("Lineal-Einstellungen"); // ルーラー設定
    if (idx == 36) return QObject::tr("Pinsel-Cursor ausblenden(&C)(&C)"); // ブラシカーソルを隠す(&C)
    if (idx == 40) return QObject::tr("Prioritätseinstellungen"); // Priority Settings
    if (idx == 41) return QObject::tr("Reaktion (verzögerte Aktualisierung)"); // Response (Delayed update)
    if (idx == 42) return QObject::tr("Performance (gesamte Aktualisierung auf einmal)"); // Performance (Update at once)
    if (idx == 43) return QObject::tr("Graustufenanzeige"); // Grayscale Display
  }

  //////////////////////////////////
  //////////////////////////////////
  if (man->TranslateFrench())
  {
    if (idx == 0) return QObject::tr("Afficher(&V)"); // View
    if (idx == 1) return QObject::tr("Zoom avant(&I)"); // Zoom In
    if (idx == 2) return QObject::tr("Zoom arrière(&O)"); // Zoom Out
    if (idx == 3) return QObject::tr("Ajuster à la fenêtre(&F)"); // Fit to Window Size
    if (idx == 4) return QObject::tr("Faire pivoter à gauche(&L)"); // Rotate Left
    if (idx == 5) return QObject::tr("Libérer la rotation/Faire basculer(&S)"); // Release Rotate/Flip
    if (idx == 6) return QObject::tr("Faire pivoter à droite(&R)"); // Rotate Right
    if (idx == 7) return QObject::tr("Faire basculer(&H)"); // Flip Horizontally
    if (idx == 8) return QObject::tr("Grille(&G)"); // Grid
    if (idx == 9) return QObject::tr("Arrière-plan transparent(&T)"); // Transparent Background
    if (idx == 10) return QObject::tr("Taille du pinceau(&B)"); // Brush Size
    if (idx == 11) return QObject::tr("Grille de pixels(&P)"); // Pixel Grid
    if (idx == 12) return QObject::tr("Couleur d'arrière-plan(&D)..."); // Background Color
    if (idx == 13) return QObject::tr("Afficher les lignes du guide manga(&M)"); // Show Comic Guide Lines
    if (idx == 14) return QObject::tr("Paramètres lignes du guide manga(&S)..."); // Comic Guide Lines Settings
    if (idx == 21) return QObject::tr("Affichage avec profil colorimétrique appliqué"); // カラープロファイルを適用して表示
    if (idx == 22) return QObject::tr("Previsualisation douce CMJN"); // CMYKソフトプルーフ
    if (idx == 23) return QObject::tr("Paramètres de gestion des couleurs"); // カラーマネジメント設定...
    if (idx == 24) return QObject::tr("Repères"); // ガイド
    if (idx == 25) return QObject::tr("Modifier les repères"); // ガイド編集
    if (idx == 27) return QObject::tr("Paramètres de grille"); // グリッド設定...
    if (idx == 28) return QObject::tr("100%"); // 100%
    if (idx == 29) return QObject::tr("200%"); // 200%
    if (idx == 30) return QObject::tr("SAfficher uniquement le calque actif(&A)"); // アクティブレイヤーのみ表示(&A)
    if (idx == 31) return QObject::tr("Motif à carreaux sombre"); // 暗い市松模様
    if (idx == 32) return QObject::tr("Règle"); // ルーラー
    if (idx == 33) return QObject::tr("Ajouter un repère..."); // ガイドを追加...
    if (idx == 34) return QObject::tr("Supprimer les repères"); // ガイドをクリア
    if (idx == 35) return QObject::tr("Paramètres de la règle"); // ルーラー設定
    if (idx == 36) return QObject::tr("Masquer le curseur de pinceau (&C)"); // ブラシカーソルを隠す(&C)
    if (idx == 40) return QObject::tr("Paramètres de priorité"); // Priority Settings
    if (idx == 41) return QObject::tr("Réponse (mise à jour différée)"); // Response (Delayed update)
    if (idx == 42) return QObject::tr("Performance (mise à jour groupée)"); // Performance (Update at once)
    if (idx == 43) return QObject::tr("Affichage en niveaux de gris"); // Grayscale Display
  }

  //////////////////////////////////
  //////////////////////////////////
  if (man->TranslateRussian())
  {
    if (idx == 0) return QObject::tr("Вид(&V)"); // View
    if (idx == 1) return QObject::tr("Увеличить(&I)"); // Zoom In
    if (idx == 2) return QObject::tr("Уменьшить(&O)"); // Zoom Out
    if (idx == 3) return QObject::tr("Подогнать к размеру окна(&F)"); // Fit to Window Size
    if (idx == 4) return QObject::tr("Повернуть влево(&L)"); // Rotate Left
    if (idx == 5) return QObject::tr("Отменить поворот/отражение(&S)"); // Release Rotate/Flip
    if (idx == 6) return QObject::tr("Повернуть вправо(&R)"); // Rotate Right
    if (idx == 7) return QObject::tr("Отразить(&H)"); // Flip Horizontally
    if (idx == 8) return QObject::tr("Сетка(&G)"); // Grid
    if (idx == 9) return QObject::tr("Прозрачный фон(&T)"); // Transparent Background
    if (idx == 10) return QObject::tr("Размер кисти(&B)"); // Brush Size
    if (idx == 11) return QObject::tr("Пиксельная сетка(&P)"); // Pixel Grid
    if (idx == 12) return QObject::tr("Цвет фона(&D)..."); // Background Color
    if (idx == 13) return QObject::tr("Показать разметочные линии комикса(&M)"); // Show Comic Guide Lines
    if (idx == 14) return QObject::tr("Настройки направляющих линий комикса(&S)..."); // Comic Guide Lines Settings
    if (idx == 21) return QObject::tr("Отображение с применением цветового профиля"); // カラープロファイルを適用して表示
    if (idx == 22) return QObject::tr("CMYK цветпроба"); // CMYKソフトプルーフ
    if (idx == 23) return QObject::tr("Управление цветом..."); // カラーマネジメント設定...
    if (idx == 24) return QObject::tr("Гиды"); // ガイド
    if (idx == 25) return QObject::tr("Настройки гида"); // ガイド編集
    if (idx == 27) return QObject::tr("Параметры сетки..."); // グリッド設定...
    if (idx == 28) return QObject::tr("100%"); // 100%
    if (idx == 29) return QObject::tr("200%"); // 200%
    if (idx == 30) return QObject::tr("Показать только текущий слой(&A)"); // アクティブレイヤーのみ表示(&A)
    if (idx == 31) return QObject::tr("Темный клетчатый узор"); // 暗い市松模様
    if (idx == 32) return QObject::tr("Линейка"); // ルーラー
    if (idx == 33) return QObject::tr("Добавить гид......"); // ガイドを追加...
    if (idx == 34) return QObject::tr("Очистить гиды"); // ガイドをクリア
    if (idx == 35) return QObject::tr("Параметры линейки"); // ルーラー設定
    if (idx == 36) return QObject::tr("Скрыть курсор кисти (&C)"); // ブラシカーソルを隠す(&C)
    //if (idx == 40) return QObject::tr("優先度設定"); // Priority Settings
    //if (idx == 41) return QObject::tr("レスポンス (遅延更新)"); // Response (Delayed update)
    //if (idx == 42) return QObject::tr("パフォーマンス (一気に更新)"); // Performance (Update at once)
    //if (idx == 43) return QObject::tr("グレースケール表示"); // Grayscale Display
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
  if (idx == 0) return QObject::tr("View(&V)"); // 表示(&V)
  if (idx == 1) return QObject::tr("Zoom In(&I)"); // 拡大表示(&I)
  if (idx == 2) return QObject::tr("Zoom Out(&O)"); // 縮小表示(&O)
  if (idx == 3) return QObject::tr("Fit to Window Size(&F)"); // ウィンドウサイズに合わせる(&F)
  if (idx == 4) return QObject::tr("Rotate Left(&L)"); // 左回転(&L)
  if (idx == 5) return QObject::tr("Release Rotate/Flip(&S)"); // 回転・反転を解除(&S)"
  if (idx == 6) return QObject::tr("Rotate Right(&R)"); // 右回転(&R)
  if (idx == 7) return QObject::tr("Flip Horizontally(&H)"); // 左右反転(&H)
  if (idx == 8) return QObject::tr("Grid(&G)"); // グリッド(&G)
  if (idx == 9) return QObject::tr("Transparent Background(&T)"); // 透明背景(&T)
  if (idx == 10) return QObject::tr("Brush Size Cursor(&B)"); // ブラシサイズカーソル(&B)
  if (idx == 11) return QObject::tr("Pixel Grid(&P)"); // ピクセル単位のグリッド(&P)
  if (idx == 12) return QObject::tr("Background Color(&D)..."); // 背景色の指定(&D)...
  if (idx == 13) return QObject::tr("Show Comic Guide Lines(&M)"); // 漫画原稿ガイドの表示(&M)
  if (idx == 14) return QObject::tr("Comic Guide Lines Settings(&S)..."); // 漫画原稿ガイド設定(&S)...
  if (idx == 21) return QObject::tr("Display with Applying Color Profile"); // カラープロファイルを適用して表示
  if (idx == 22) return QObject::tr("CMYK Soft Proof"); // CMYKソフトプルーフ
  if (idx == 23) return QObject::tr("Color Management Settings..."); // カラーマネジメント設定...
  if (idx == 24) return QObject::tr("Guides"); // ガイド
  if (idx == 25) return QObject::tr("Edit Guides"); // ガイド編集
  if (idx == 27) return QObject::tr("Grid Settings..."); // グリッド設定...
  if (idx == 28) return QObject::tr("100%"); // 100%
  if (idx == 29) return QObject::tr("200%"); // 200%
  if (idx == 30) return QObject::tr("Show only Active Layer(&A)"); // アクティブレイヤーのみ表示(&A)
  if (idx == 31) return QObject::tr("Dark Checkered Pattern"); // 暗い市松模様
  if (idx == 32) return QObject::tr("Ruler"); // ルーラー
  if (idx == 33) return QObject::tr("Add Guide..."); // ガイドを追加...
  if (idx == 34) return QObject::tr("Clear Guides"); // ガイドをクリア
  if (idx == 35) return QObject::tr("Ruler Settings"); // ルーラー設定
  if (idx == 36) return QObject::tr("Hide Brush Cursor(&C)"); // ブラシカーソルを隠す(&C)
  if (idx == 40) return QObject::tr("Priority Settings"); // 優先度設定
  if (idx == 41) return QObject::tr("Response (Delayed update)"); // レスポンス (遅延更新)
  if (idx == 42) return QObject::tr("Performance (Update at once)"); // パフォーマンス (一気に更新)
  if (idx == 43) return QObject::tr("Grayscale Display"); // グレースケール表示

  return "";
}
