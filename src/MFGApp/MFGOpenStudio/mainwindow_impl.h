/* -*- coding: utf-8 -*- マルチバイト */

// Copyright 2026 PGN Inc. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#ifndef MAINWINDOW_IMPL_H
#define MAINWINDOW_IMPL_H

#include <QMainWindow>

#include <QToolBar>
#include <QAction>
#include <QCheckBox>
#include <QSlider>
#include <QComboBox>
#include <QDockWidget>
#include <QLabel>
#include <QToolButton>
#include <QSettings>
#include <QSpinBox>
#include <QMdiSubWindow>
#include <QTextEdit>
#include <vector>


#include "mainwindow.h"
#include <memory> // unique_ptr

#include "project.h"


/////////////////////////////////////////////
namespace Ui {
  class MainWindow;
}

class WindowStateManager;
class WidgetMFGEditor;
class WidgetProjectExplorer;
class WidgetOutput;
class DialogMFG;
class WidgetCentral;
class WidgetPreview;
class QSliderAlpha;

/////////////////////////////////////////////
// MainWindow
/////////////////////////////////////////////
class MainWindow : public QMainWindow, public IMainWindow
{
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = 0);
  ~MainWindow();

  WidgetMFGEditor* AE(); // アクティブな子ウィンドウのエディタ。

  std::vector<WidgetMFGEditor*> EditorChildren() override; // 全ての子ウィンドウ
  WidgetMFGEditor* NewEditor();
  WidgetCentral* Central() override;

  QSettings* settings(){ return mSettings; }

  // プレビューのターゲットのリスト
  std::vector<QString> TargetList( const std::vector<QString>& samples, QStringList exts );
  std::vector<QString> TargetList();


  // ファイル関連
  QString fileOpenDialog();
  QString fileOpenDialog( QString folder );
  void openProject( QString folderPath, bool internalProject = false );

  // プロジェクト関連
  void setProject(const QDir& dir, const QString& filterPath );
  void setProjectWithMRU(const QDir& dir, const QString& filterPath );


  // MFGのスクリプトのファイル関連
  void openScriptFile( QString filePath ) override;
  void closeSciprtIfExist( QString filePath );
  void closeAllTab();

  // 何かをした後の処理
  void AfterInitChild( WidgetMFGEditor* child );

public:
  // 保存されるコンフィグ設定
  int mLanguageIndex; // 言語選択 0:自動 1:English 2:Japanese ...

  bool textAA(){ return false; }
  size_t TranslateMenuMs() const { return mTranslateMenuMs; }

  const QString& PreviewTargetFolder() const { return mPreviewTargetFolder; }
  const QString& ExamplesProjectFolder() const { return mExamplesProjectFolder; }

  // ファイルダイアログ
  QString getSaveFolder() override;
  void setSaveFolder( QString folderPath );
  void setSavePath( QString fullPath );

  QString getOpenFolder();
  void setOpenFolder( QString folderPath );

  QString getOpenProjectFolder();
  void setOpenProjectFolder( QString folderPath );

  void AddOutput( QString s ) override;

  QStatusBar* GetStatusBar() const override { return statusBar(); }

  bool ensureProjectClose();

private:
  Ui::MainWindow *ui;
  Project _project;

  // 各種 Settings
  QSettings* mSettings; // 設定ファイル
  QSettings* mSettingsLanguage; // 言語設定ファイル

  // 各種フォルダー
  QString mPreviewTargetFolder;  // プレビューのターゲットを置くフォルダ
  QString mExamplesProjectFolder;  // サンプルのフィルタ一覧のプロジェクトのフォルダ
  QString mDebugFolder; // デバッグ出力フォルダ

  // 各種パス
  QString mInitPath; // コンフィグ設定 (Window初期化用)
  QString mConfigPath; // コンフィグ設定
  QString mLanguagePath;  // 言語設定

  // MFGツールバー
  QToolBar* mMFGBar;
  QToolButton* mMFGRun;
  QCheckBox* mMFGDebug;

  QSliderAlpha* mWandTolerance;
  QLabel* mWandToleranceValue;

  // タイマー
  int mTimerAnts;

  // 色々
  bool mFirstShow;
  size_t mLastAntsArrive;
  size_t mAntsInterval;
  WindowStateManager *mWindowStateManager;
  bool mSmallIcon = false;
  bool mAdjustIconSize = false;
  size_t mTranslateMenuMs;
  int mFontSize = 14;

  // 各種タイマー処理
  void timerAnts();

  std::unique_ptr<DialogMFG> CreateDialogMFG( const std::string& script );

  void onFilterMFGScript( const std::string& script, bool enableDebug );

  void RunScript( const std::string& script );

  WidgetPreview* mWidgetPreview;
  QDockWidget* mDockPreview;

  WidgetProjectExplorer* mWidgetProjectExplorer;
  QDockWidget* mDockProjectExplorer;

  WidgetOutput* mOutput;
  QDockWidget* mDockOutput;

  // CentralWidget
  WidgetCentral* mWidgetCentral;

  bool LoadTarget( const QString& path );

  bool HasOpenChild();
  void ApplyStatusTitle();

  // 色々な初期化
  void InitSamples( const std::vector<QString>& );
  void InitLanguage();
  void InitToolbar();
  void InitWidgets();
  void InitMRU();
  void InitConnect();
  void InitTranslation();
  void AdjustInitialToolBar();
  void InitInitialProject();

  // いろいろな初期化のルート。
  void InitAll();

  // 設定フォルダを初期化
  void initDataFolder();


  // Window等の設定
  void initWindowSettings();
  void openSettings();
  void saveSettings();

  // メニュー状態の更新
  void updateFileMenu();
  void updateEditMenu();
  void updateProjectMenu();
  void updateViewMenu();
  void updateWindowMenu();
  void updateHelpMenu();

public slots:
  // ファイルメニュー
  void onFileProjectNew();
  void onFileProjectOpen();
  void onFileProjectClose();
  void onFileNew();
  void onFileOpen() override;
  void onFileMRU( int index );
  void onFileMRU0(){ onFileMRU(0); }
  void onFileMRU1(){ onFileMRU(1); }
  void onFileMRU2(){ onFileMRU(2); }
  void onFileMRU3(){ onFileMRU(3); }
  void onFileMRU4(){ onFileMRU(4); }
  void onFileMRU5(){ onFileMRU(5); }
  void onFileMRU6(){ onFileMRU(6); }
  void onFileMRU7(){ onFileMRU(7); }
  void onFileMRUProject( int index );
  void onFileMRUProject0(){ onFileMRUProject(0); }
  void onFileMRUProject1(){ onFileMRUProject(1); }
  void onFileMRUProject2(){ onFileMRUProject(2); }
  void onFileMRUProject3(){ onFileMRUProject(3); }
  void onFileMRUProject4(){ onFileMRUProject(4); }
  void onFileMRUProject5(){ onFileMRUProject(5); }
  void onFileMRUProject6(){ onFileMRUProject(6); }
  void onFileMRUProject7(){ onFileMRUProject(7); }
  void onFileSave();
  void onFileSaveAs();
  void onFileExamplesProject();
  void onFileClose();

  // 編集メニュー
  void onEditUndo();
  void onEditRedo();
  void onEditCut();
  void onEditCopy();
  void onEditPaste();

  // プロジェクトメニュー
  void onProjectPackMAR();

  // 表示メニュー
  void onViewZoomIn();
  void onViewZoomOut();

  // ウィンドウメニュー
  void onWindowInitialize();
  void onWindowPreview();
  void onWindowOutput();
  void onWindowStatus();

  // ヘルプメニュー
  void onHelpOfficial();
  void onHelpAbout();
  void onHelpConfig();
  void onHelpLangIndex( int index );
  void onHelpLangAuto();
  void onHelpLangEnglish();
  void onHelpLangJapanese();
  void onHelpLangChineseSimp();
  void onHelpLangChineseTrad();
  void onHelpLangKorean();
  void onHelpLangPortugues();
  void onHelpLangSpanish();
  void onHelpLangGerman();
  void onHelpLangFrench();
  void onHelpLangRussian();
  void onHelpLangHindi();
  void onHelpLangBengali();
  void onHelpLangPolish();

  void initDefaultShortcut();

protected:
  void resizeEvent( QResizeEvent* event ) override;

private slots:
  bool event( QEvent* event ) override;

  void timerEvent( QTimerEvent* event ) override;
  void showEvent( QShowEvent* event ) override;
  void closeEvent( QCloseEvent* event ) override;

public:

  void updateAllChildTitle();

  // 多言語対応
  void sMenu( QMenu* menu, QString text );
  void sAction( QAction* action, QString text );
  void TranslateActions();
  QString Str( int idx );
  // MFG用リソースは別インデックスとするためMStrと呼ぶ事に。
  QString MStr( int idx );


  // Mac用のショートカット外し
  void RemoveActionsKakkoMac();

public slots:
  // 各メニューがクリックされた
  void onFileMenu();
  void onEditMenu();
  void onProjectMenu();
  void onViewMenu();
  void onWindowMenu();
  void onHelpMenu();
  void onRunScript();

  void setSmallIcon( bool smallIcon );

private:

  void updateFontSize();
  void onNewStringsJson( const QString& jsonPath );
  void onImportImageToProject();
  void onImportAsThumbnail();

  // MFGStudioとソースを共有するためのダミー実装
  void endFloating() {}
  void AfterShowDialog() {}
  bool isMenuForbidden() { return false; }
  bool checkBeforeFilter() { return true; }  
};


#endif // MAINWINDOW_IMPL_H
