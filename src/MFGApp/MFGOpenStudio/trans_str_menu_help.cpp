/* -*- coding: utf-8 -*- マルチバイト */

// Copyright 2026 PGN Inc. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#if defined(_MSC_VER)
  #pragma execution_character_set("utf-8")
#endif

#include <trans.h>
#include "trans_str_menu_help.h"

///////////////////////////////////////////////////////////////////////////
// - ヘルプメニューで使用される文字列
// - A string used in the help menu.
///////////////////////////////////////////////////////////////////////////
QString Trans_StrMenuHelp( int idx )
{
  CTranslationManager* man = &Trans();

  //////////////////////////////////
  //////////////////////////////////
  if (man->TranslateJapanese())
  {
    if (idx == 0) return QObject::tr("設定フォルダを開く(&C)...");
    if (idx == 1) return QObject::tr("プリセットブラシを復元...");
    if (idx == 2) return QObject::tr("マルチスレッド処理による高速化");
    if (idx == 3) return QObject::tr("ブラシ処理を独立したスレッドで行う");
    if (idx == 4) return QObject::tr("ブラシ描画時の表示更新の最適化");
    if (idx == 5) return QObject::tr("MDPファイルの高速保存");
    if (idx == 6) return QObject::tr("データの初期化");
    if (idx == 7) return QObject::tr("グラデーションの初期化...");
    if (idx == 8) return QObject::tr("全てのレイヤーの親子関係を解除");
    if (idx == 9) return QObject::tr("全てのブラシを削除...");

    if (idx == 10) return QObject::tr("FireAlpaca (無料版) の設定を使用する...");
    if (idx == 11) return QObject::tr("旧バージョンの設定を引き継ぐ...");
    if (idx == 12) return QObject::tr("設定を書き出す...");
    if (idx == 13) return QObject::tr("設定を取り込む...");

    if (idx == 200) return QObject::tr("公式サイト...");
    if (idx == 201) return QObject::tr("ライセンス認証...");
  }

  //////////////////////////////////
  //////////////////////////////////
  if (man->TranslateChineseSimp())
  {
    if (idx == 0) return QObject::tr("打开设定档案(&C)...");
    if (idx == 1) return QObject::tr("恢复笔刷默认值...");
    if (idx == 2) return QObject::tr("透过多执行处理进行最佳化"); // マルチスレッド処理による高速化
    if (idx == 3) return QObject::tr("笔刷座标"); // ブラシ処理を独立したスレッドで行う
    if (idx == 4) return QObject::tr("优化笔刷的更新"); // ブラシ描画時の表示更新の最適化
    if (idx == 5) return QObject::tr("高速储存MDP档案"); // MDPファイルの高速保存
    if (idx == 6) return QObject::tr("档案初始化"); // データの初期化
    if (idx == 7) return QObject::tr("渐层初始化"); // グラデーションの初期化...
    if (idx == 8) return QObject::tr("解除所有图层的父子关系"); // 全てのレイヤーの親子関係を解除

    if (idx == 10) return QObject::tr("使用fireAlpaca (免费版)的设定..."); // FireAlpaca (無料版) の設定を使用する...
    if (idx == 11) return QObject::tr("继承旧版本设置..."); // 旧バージョンの設定を引き継ぐ
    if (idx == 12) return QObject::tr("导出设置..."); // 設定を書き出す
    if (idx == 13) return QObject::tr("导入设置..."); // 設定を取り込む

    if (idx == 200) return QObject::tr("官方网站"); // 公式サイト...
    if (idx == 201) return QObject::tr("许可证认证..."); // ライセンス認証...
  }

  //////////////////////////////////
  //////////////////////////////////
  if (man->TranslateChineseTrad())
  {
    if (idx == 0) return QObject::tr("打開設定檔案(&C)...");
    if (idx == 1) return QObject::tr("復原為筆刷預設值...");
    if (idx == 2) return QObject::tr("透過多執行處理進行最佳化"); // マルチスレッド処理による高速化
    if (idx == 3) return QObject::tr("筆刷座標"); // ブラシ処理を独立したスレッドで行う
    if (idx == 4) return QObject::tr("優化筆刷的更新"); // ブラシ描画時の表示更新の最適化
    if (idx == 5) return QObject::tr("高速儲存MDP檔案"); // MDPファイルの高速保存
    if (idx == 6) return QObject::tr("檔案初始化"); // データの初期化
    if (idx == 7) return QObject::tr("漸層初始化"); // グラデーションの初期化...
    if (idx == 8) return QObject::tr("解除所有圖層的父子關係"); // 全てのレイヤーの親子関係を解除

    if (idx == 10) return QObject::tr("使用fireAlpaca (免費版)的設定..."); // FireAlpaca (無料版) の設定を使用する...
    if (idx == 11) return QObject::tr("繼承舊版本設定..."); // 旧バージョンの設定を引き継ぐ
    if (idx == 12) return QObject::tr("匯出設定..."); // 設定を書き出す
    if (idx == 13) return QObject::tr("匯入設定..."); // 設定を取り込む

    if (idx == 200) return QObject::tr("官方網站"); // 公式サイト...
    if (idx == 201) return QObject::tr("許可證認證..."); // ライセンス認証...
  }

  //////////////////////////////////
  //////////////////////////////////
  if (man->TranslateKorean())
  {
    if (idx == 0) return QObject::tr("설정 폴더 열기(&C)...");
    if (idx == 1) return QObject::tr("프리셋 브러시 복원...");
    if (idx == 2) return QObject::tr("멀티 스레드 처리로 속도 향상"); // マルチスレッド処理による高速化
    if (idx == 3) return QObject::tr("별도 스레드에서 브러시 처리하기"); // ブラシ処理を独立したスレッドで行う
    if (idx == 4) return QObject::tr("브러시 디스플레이 업데이트 최적화"); // ブラシ描画時の表示更新の最適化
    if (idx == 5) return QObject::tr("MDP 파일 빠르게 저장"); // MDPファイルの高速保存
    if (idx == 6) return QObject::tr("데이터 초기화"); // データの初期化
    if (idx == 7) return QObject::tr("그라데이션 초기화..."); // グラデーションの初期化...
    if (idx == 8) return QObject::tr("모든 레이어의 부모-자식 관계 해제"); // 全てのレイヤーの親子関係を解除

    if (idx == 10) return QObject::tr("FireAlpaca(무료판) 설정 사용..."); // FireAlpaca (無料版) の設定を使用する...
    if (idx == 11) return QObject::tr("이전 버전 설정 가져오기..."); // 旧バージョンの設定を引き継ぐ
    if (idx == 12) return QObject::tr("설정 내보내기..."); // 設定を書き出す
    if (idx == 13) return QObject::tr("설정 가져오기..."); // 設定を取り込む

    if (idx == 200) return QObject::tr("공식 사이트..."); // 公式サイト...
    if (idx == 201) return QObject::tr("라이선스 인증..."); // ライセンス認証...
  }

  //////////////////////////////////
  //////////////////////////////////
  if (man->TranslatePortugues())
  {
    if (idx == 0) return QObject::tr("Abrir Pasta de Config(&C)...");
    if (idx == 1) return QObject::tr("Restaurar Pincéis Predefinidos..."); // プリセットブラシを復元...
    if (idx == 2) return QObject::tr("Aceleração por Processamento Multithread"); // マルチスレッド処理による高速化
    if (idx == 3) return QObject::tr("Processamento de Pincel em Thread Separado"); // ブラシ処理を独立したスレッドで行う
    if (idx == 4) return QObject::tr("Otimização de Atualização de Exibição durante a Pintura com Pincel"); // ブラシ描画時の表示更新の最適化
    if (idx == 5) return QObject::tr("Salvamento Rápido de Arquivo MDP"); // MDPファイルの高速保存
    if (idx == 6) return QObject::tr("Inicializar Dados"); // データの初期化
    if (idx == 7) return QObject::tr("Redefinir Gradiente..."); // グラデーションの初期化...
    if (idx == 8) return QObject::tr("Remover todas as relações de parentesco entre camadas"); 

    if (idx == 10) return QObject::tr("Usar Configurações do FireAlpaca (Versão Gratuita)..."); // FireAlpaca (無料版) の設定を使用する...
    if (idx == 11) return QObject::tr("Importar configurações da versão anterior..."); // 旧バージョンの設定を引き継ぐ
    if (idx == 12) return QObject::tr("Exportar configurações..."); // 設定を書き出す
    if (idx == 13) return QObject::tr("Importar configurações..."); // 設定を取り込む

    if (idx == 200) return QObject::tr("Site Oficial..."); // 公式サイト...
    if (idx == 201) return QObject::tr("Ativação de Licença..."); // ライセンス認証...
  }

  //////////////////////////////////
  //////////////////////////////////
  if (man->TranslateSpanish())
  {
    if (idx == 0) return QObject::tr("Abrir carpeta de configuración(&C)...");
    if (idx == 1) return QObject::tr("Restaurar pinceles preestablecidos..."); // プリセットブラシを復元...
    if (idx == 2) return QObject::tr("Optimizar con procesamiento multihilo"); // マルチスレッド処理による高速化
    if (idx == 3) return QObject::tr("Procesamiento de pinceles en hilos independientes"); // ブラシ処理を独立したスレッドで行う
    if (idx == 4) return QObject::tr("Optimización de las actualizaciones de visualización al dibujar con pinceles"); // ブラシ描画時の表示更新の最適化
    if (idx == 5) return QObject::tr("Guardado rápido de archivos MDP"); // MDPファイルの高速保存
    if (idx == 6) return QObject::tr("Inicialización de datos"); // データの初期化
    if (idx == 7) return QObject::tr("Inicialización de degradados..."); // グラデーションの初期化...
    if (idx == 8) return QObject::tr("Eliminar todas las relaciones de parentesco entre capas"); // 全てのレイヤーの親子関係を解除

    if (idx == 10) return QObject::tr("Uso de la configuración de FireAlpaca (versión gratuita)..."); // FireAlpaca (無料版) の設定を使用する...
    if (idx == 11) return QObject::tr("Importar la configuración de la versión anterior..."); // 旧バージョンの設定を引き継ぐ
    if (idx == 12) return QObject::tr("Exportar configuración..."); // 設定を書き出す
    if (idx == 13) return QObject::tr("Importar configuración..."); // 設定を取り込む

    if (idx == 200) return QObject::tr("Página web oficial..."); // 公式サイト...
    if (idx == 201) return QObject::tr("Activación de Licencia..."); // ライセンス認証...
  }

  //////////////////////////////////
  //////////////////////////////////
  if (man->TranslateGerman())
  {
    if (idx == 0) return QObject::tr("Einstellungsdatei öffnen(&C)...");
    if (idx == 1) return QObject::tr("Voreingestellte Pinsel wiederherstellen..."); // プリセットブラシを復元...
    if (idx == 2) return QObject::tr("Optimieren mit Multi-Thread-Verarbeitung"); // マルチスレッド処理による高速化
    if (idx == 3) return QObject::tr("Die Pinselverarbeitung wird in einem separaten Thread ausgeführt."); // ブラシ処理を独立したスレッドで行う
    if (idx == 4) return QObject::tr("Optimierung der Pinselanzeige-Updates "); // ブラシ描画時の表示更新の最適化
    if (idx == 5) return QObject::tr("Schnelles Speichern von MDP-Dateien"); // MDPファイルの高速保存
    if (idx == 6) return QObject::tr("Daten initialisieren"); // データの初期化
    if (idx == 7) return QObject::tr("Gradienten initialisieren..."); // グラデーションの初期化...
    if (idx == 8) return QObject::tr("FireAlpaca verwenden (kostenlose Version) Einstellungen"); // 全てのレイヤーの親子関係を解除

    if (idx == 10) return QObject::tr("UFireAlpaca verwenden (kostenlose Version) Einstellungen"); // FireAlpaca (無料版) の設定を使用する...
    if (idx == 11) return QObject::tr("Einstellungen der vorherigen Version übernehmen..."); // 旧バージョンの設定を引き継ぐ
    if (idx == 12) return QObject::tr("Einstellungen exportieren..."); // 設定を書き出す
    if (idx == 13) return QObject::tr("Einstellungen importieren..."); // 設定を取り込む

    if (idx == 200) return QObject::tr("Offizielle Website..."); // 公式サイト...
    if (idx == 201) return QObject::tr("Lizenzaktivierung..."); // ライセンス認証...
  }

  //////////////////////////////////
  //////////////////////////////////
  if (man->TranslateFrench())
  {
    if (idx == 0) return QObject::tr("Ouvrir le dossier de configuration(&C)...");
    if (idx == 1) return QObject::tr("Restaurer les pinceaux prédéfinis..."); // プリセットブラシを復元...
    if (idx == 2) return QObject::tr("Optimiser avec le traitement multi-thread"); // マルチスレッド処理による高速化
    if (idx == 3) return QObject::tr("Le traitement des pinceaux est effectué dans un fil séparé."); // ブラシ処理を独立したスレッドで行う
    if (idx == 4) return QObject::tr("Optimisation des mises à jour d'affichage des pinceaux"); // ブラシ描画時の表示更新の最適化
    if (idx == 5) return QObject::tr("Sauvegarde rapide des fichiers MDP"); // MDPファイルの高速保存
    if (idx == 6) return QObject::tr("Initialiser les données"); // データの初期化
    if (idx == 7) return QObject::tr("Initialisation des dégradés..."); // グラデーションの初期化...
    if (idx == 8) return QObject::tr("Annuler les relations parent-enfant de tous les calques"); // 全てのレイヤーの親子関係を解除

    if (idx == 10) return QObject::tr("Utiliser les paramètres de FireAlpaca (version gratuite)"); // FireAlpaca (無料版) の設定を使用する...
    if (idx == 11) return QObject::tr("Importer les paramètres de la version précédente..."); // 旧バージョンの設定を引き継ぐ
    if (idx == 12) return QObject::tr("Exporter les paramètres..."); // 設定を書き出す
    if (idx == 13) return QObject::tr("Importer les paramètres..."); // 設定を取り込む

    if (idx == 200) return QObject::tr("Site officiel..."); // 公式サイト...
    if (idx == 201) return QObject::tr("Activation de Licence..."); // ライセンス認証...
  }

  //////////////////////////////////
  //////////////////////////////////
  if (man->TranslateRussian())
  {
    if (idx == 0) return QObject::tr("Открыть папку с настройками(&C)...");
    if (idx == 1) return QObject::tr("Восстановить предустановленные кисти..."); // プリセットブラシを復元...
    if (idx == 2) return QObject::tr("Оптимизация с помощью многопоточной обработки"); // マルチスレッド処理による高速化
    if (idx == 3) return QObject::tr("Обработка кисти выполняется в отдельном потоке"); // ブラシ処理を独立したスレッドで行う
    if (idx == 4) return QObject::tr("Оптимизация обновлений отображения кисти"); // ブラシ描画時の表示更新の最適化
    if (idx == 5) return QObject::tr("Быстрое сохранение файла MDP"); // MDPファイルの高速保存
    if (idx == 6) return QObject::tr("Инициализация данных"); // データの初期化
    if (idx == 7) return QObject::tr("Инициализация градиентов..."); // グラデーションの初期化...
    if (idx == 8) return QObject::tr("Отменить родительско-дочерная связь всех слоев"); // 全てのレイヤーの親子関係を解除

    if (idx == 10) return QObject::tr("Использовать FireAlpaca (бесплатная версия) настройки..."); // FireAlpaca (無料版) の設定を使用する...
    //if (idx == 11) return QObject::tr("Carrying over settings from previous version..."); // 旧バージョンの設定を引き継ぐ
    //if (idx == 12) return QObject::tr("Export Settings..."); // 設定を書き出す
    //if (idx == 13) return QObject::tr("Import Settings..."); // 設定を取り込む

    if (idx == 200) return QObject::tr("Официальный сайт..."); // 公式サイト...
    //if (idx == 201) return QObject::tr("License Verification..."); // ライセンス認証...
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
  if (idx == 0) return QObject::tr("Open Config Folder(&C)..."); // 設定フォルダを開く(&C)...
  if (idx == 1) return QObject::tr("Restore Preset Brushes..."); // プリセットブラシを復元...
  if (idx == 2) return QObject::tr("Optimize with Multi-Thread Processing"); // マルチスレッド処理による高速化
  if (idx == 3) return QObject::tr("Brush Processing is Done in a Separate Thread."); // ブラシ処理を独立したスレッドで行う
  if (idx == 4) return QObject::tr("Optimizing Brush Display Updates"); // ブラシ描画時の表示更新の最適化
  if (idx == 5) return QObject::tr("Fast Saving of MDP Files"); // MDPファイルの高速保存
  if (idx == 6) return QObject::tr("Initialize Data"); // データの初期化
  if (idx == 7) return QObject::tr("Initializing Gradients..."); // グラデーションの初期化...
  if (idx == 8) return QObject::tr("Cancel Parent-Child Relationships of All Layers"); // 全てのレイヤーの親子関係を解除
  if (idx == 9) return QObject::tr("Delete All Brushes..."); // 全てのブラシを削除...

  if (idx == 10) return QObject::tr("Use FireAlpaca (free version) Settings..."); // FireAlpaca (無料版) の設定を使用する...
  if (idx == 11) return QObject::tr("Carrying over settings from previous version..."); // 旧バージョンの設定を引き継ぐ
  if (idx == 12) return QObject::tr("Export Settings..."); // 設定を書き出す
  if (idx == 13) return QObject::tr("Import Settings..."); // 設定を取り込む

  if (idx == 200) return QObject::tr("Official Website..."); // 公式サイト...
  if (idx == 201) return QObject::tr("License Verification..."); // ライセンス認証...

  return "";
}
