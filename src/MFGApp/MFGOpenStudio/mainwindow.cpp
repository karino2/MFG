/* -*- coding: utf-8 -*- マルチバイト */

// Copyright 2026 PGN Inc. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#include "mainwindow_impl.h"
#include "ui_mainwindow.h"
#include "studio_conf.h"

#include "man.h"
#include "trans.h"
#include "trans_str_menu_file.h"
#include "trans_str_menu_edit.h"
#include "trans_str_menu_view.h"
#include "trans_str_menu_window.h"
#include "trans_str_menu_help.h"
#include "trans_str_main.h"
#include "trans_str_mfg.h"

#include "app_qt.h"

#include <qt_pal/qt_pal.h>

#include "dialog_filter_mfg.h"

#include "widget_central.h"
#include "widget_preview.h"
#include "widget_project_explorer.h"
#include "widget_output.h"

#include "window_state_manager.h"

#include <QCloseEvent>
#include <QFileInfo>
#include <QTimer>

#include "mainwindow_util.h"
using namespace mainwindow_util;

using namespace mfg_pal;

using qt_pal::GetQtDesktopPath;
using app_qt::AQ;

using qt_pal::BeginWaitCursor;
using qt_pal::EndWaitCursor;
using qt_pal::cursor_t;

///////////////////////////////////////////////////////////////////////////
static MainWindow* gMainWindow = NULL;
IMainWindow* Main(){ return gMainWindow; }
// mainwindow.hをincludeしなくてもInitDialogPosを使えるように。
QWidget* MainAsQWidget() { return gMainWindow; }

// ベンチマークはMFGOpenStudioでは空実装
void MainBenchBegin() {}
void MainBenchEnd( QString ){}
void MainBenchAdd( QString ){}

static std::vector<QString> g_sampleNames {
  "colorful_baloon.png",
  "riverside_photo.png",
  "girl001.png",
  "man_lineart.png",
  "nightcity_photo.png",
  "room_photo.png",
  "grid.png",
  "color_grid.png",
  "noise_riverside_photo.png",
  "girl001_lineart.png",
  "non_aa_line.png",
  "non_aa_chara.png",
  "non_aa_color_shape.png",
  "non_aa_silhouette.png",
  "thumbnail_base.png",
};

void MainWindow::InitAll()
{
  // データフォルダを用意しておく
  initDataFolder();

  InitSamples( g_sampleNames );

  InitLanguage();

  initDefaultShortcut();

  InitTranslation();

  // Mac用 (メニューの括弧を削る)
#if defined(__APPLE__)
  RemoveActionsKakkoMac();
#endif

  // Central
  mWidgetCentral = new WidgetCentral();
  setCentralWidget( mWidgetCentral );

  // Window初期化
  InitToolbar();
  InitWidgets();
  InitMRU();
  InitConnect();

  // 設定読み込み (環境・ブラシ・パレット)
  openSettings();

  AdjustInitialToolBar();
  InitInitialProject();

  updateFontSize();
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
  mWidgetPreview = nullptr; // 初期化中はInitWidgetsを呼ばれるまでAC()がnullptrを返すようにmWidgetPreviewをnullptrに初期化しておく。
  ui->setupUi(this);

  gMainWindow = this;

  // 変数初期化
  mFirstShow = true;

  // 言語設定など
  setContextMenuPolicy( Qt::NoContextMenu );

  // アプリ設定
  qApp->setOrganizationName( STUDIO_ORG_NAME );
  qApp->setOrganizationDomain( STUDIO_ORG_DOMAIN );
  qApp->setApplicationName( STUDIO_APP_NAME );

  InitAll();
}

///////////////////////////////////////////////////////////////////////////
MainWindow::~MainWindow()
{
  delete ui;
}


///////////////////////////////////////////////////////////////////////////
void MainWindow::TranslateActions()
{
  // メニュー翻訳にどれだけ掛かってるか調べる
  mTranslateMenuMs = mfg_pal::Tick();

  // ファイルメニュー
  sMenu( ui->menu_File, Trans_StrMenuFile(0) );
  sAction( ui->action_FileProjectNew, MStr(0) );
  sAction( ui->action_FileProjectOpen, MStr(10) );
  sAction( ui->action_FileOpen, MStr(15) );  
  sMenu( ui->menu_FileRecent, Trans_StrMenuFile(4) );
  sMenu( ui->menu_FileRecentProject, MStr(12) );
  sAction( ui->action_FileSave, MStr(16) );
  sAction( ui->action_FileSaveAs, MStr(17) );
  sAction( ui->action_FileExamplesProject, MStr(48) );
  sAction( ui->action_FileClose, MStr(56) );
  sAction( ui->action_FileProjectClose, MStr(13) );

  // 編集メニュー
  sMenu( ui->menu_Edit, Trans_StrMenuEdit(0) );
  sAction( ui->action_EditUndo, Trans_StrMenuEdit(1) );
  sAction( ui->action_EditRedo, Trans_StrMenuEdit(2) );
  sAction( ui->action_EditCut, Trans_StrMenuEdit(3) );
  sAction( ui->action_EditCopy, MStr(24) );
  sAction( ui->action_EditPaste, Trans_StrMenuEdit(5) );

  // プロジェクトメニュー
  sMenu( ui->menu_Project, MStr(20) );
  sAction( ui->action_ProjectPackMAR, MStr(21) );

  // 表示メニュー
  sMenu( ui->menu_View, Trans_StrMenuView(0) );
  sAction( ui->action_ViewZoomIn, Trans_StrMenuView(1) );
  sAction( ui->action_ViewZoomOut, Trans_StrMenuView(2) );
 
  // ウィンドウメニュー
  sMenu( ui->menu_Window, Trans_StrMenuWindow(0) );
  sAction( ui->action_WindowInitialize, Trans_StrMenuWindow(1) );
  sAction( ui->action_WindowShowHide, Trans_StrMenuWindow(2) );
  sAction( ui->action_WindowPreview, MStr(23) );
  sAction( ui->action_WindowOutput, MStr(27) );
  sAction( ui->action_WindowStatus, Trans_StrMenuWindow(10) );

  // ヘルプメニュー
  sAction( ui->action_HelpOfficial, Trans_StrMenuHelp(200) );
  sAction( ui->action_HelpConfig, Trans_StrMenuHelp(0) );

  // 計測完了
  mTranslateMenuMs = mfg_pal::Tick() - mTranslateMenuMs;
}

///////////////////////////////////////////////////////////////////////////
QString MainWindow::Str( int idx )
{
  // 翻訳済みの文字列を取得
  return Trans_StrMain( idx );
}

// MFG用翻訳文字列取得。
// FA3のリソースも使うので別インデックスとして関数を分ける。
QString MainWindow::MStr( int idx )
{
  // 翻訳済みの文字列を取得
  return Trans_StrMFG( idx );
}


///////////////////////////////////////////////////////////////////////////
void MainWindow::RemoveActionsKakkoMac()
{
  auto aq = AQ();

  // ファイルメニュー
  aq->rMenu( ui->menu_File );
  aq->rAction( ui->action_FileOpen );
  aq->rMenu( ui->menu_FileRecent );
  aq->rMenu( ui->menu_FileRecentProject );
  aq->rAction( ui->action_FileSave );
  aq->rAction( ui->action_FileSaveAs );
  aq->rAction( ui->action_FileClose );
  aq->rAction( ui->action_FileProjectClose );

  // 編集メニュー
  aq->rMenu( ui->menu_Edit );
  aq->rAction( ui->action_EditUndo );
  aq->rAction( ui->action_EditRedo );
  aq->rAction( ui->action_EditCut );
  aq->rAction( ui->action_EditCopy );
  aq->rAction( ui->action_EditPaste );

  // 表示メニュー
  aq->rMenu( ui->menu_View );
  aq->rAction( ui->action_ViewZoomIn );
  aq->rAction( ui->action_ViewZoomOut );

  // ウィンドウメニュー
  aq->rMenu( ui->menu_Window );
  aq->rAction( ui->action_WindowInitialize );
  aq->rAction( ui->action_WindowShowHide );
  aq->rAction( ui->action_WindowPreview );
  aq->rAction( ui->action_WindowOutput );

  // ヘルプメニュー
  aq->rAction( ui->action_HelpConfig );
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void MainWindow::initWindowSettings()
{
  // ウィンドウの状態を復元
  mWindowStateManager->restoreInitialState();
  // その後、最大化する
  showMaximized();

  // ステータスバーを表示
  statusBar()->setVisible( true );
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void MainWindow::openSettings()
{
  // 既存の状態を読み込む
  mSettings = NewSettings( mConfigPath );

  //////////////////////////
  // アイコンサイズを調整するかどうか
  //////////////////////////
#if defined(__APPLE__)
  mAdjustIconSize = mSettings->value( "main/adjustIconSize", true ).toBool();
#else
  mAdjustIconSize = mSettings->value( "main/adjustIconSize" ).toBool();
#endif
  // 全てのStateと、各ウィンドウのGeometry (main だけ保存すれば、子は勝手に保存されるっぽい)
  mWindowStateManager = new WindowStateManager( mSettings, this, mInitPath );
  connect( mWindowStateManager, &WindowStateManager::restoreFinished, this, [this] {

    // Windowの表示・非表示 (Tab状態)
    QVariant v;

    v = mSettings->value( "statusBar/visible" );
    if (!v.isNull()) statusBar()->setVisible( v.toBool() );
  } );
  if (mWindowStateManager->hasInitialState()) {
    mWindowStateManager->restore();
  }

  //////////////////////
  // 初回起動時は、最大化
  //////////////////////
  if (mSettings->value( "main/windowState2" ).isNull())
  {
    setWindowState( Qt::WindowMaximized );
  }

  QVariant v;

  ///////////////////
  // 表示オプション状態
  ///////////////////
  v = mSettings->value( "view/fontSize" );
  if (!v.isNull()) mFontSize = v.toInt();

  // MRU読み込み
  AQ()->ReadMRU( mSettings );
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void MainWindow::saveSettings()
{
  // 言語
  mSettingsLanguage->setValue( "language/index", QVariant::fromValue(mLanguageIndex) );

  // 最小化してる？
  Qt::WindowStates ws = this->windowState();
  bool winMinimized = (ws & Qt::WindowMinimized);

  // 最小化してる時に保存しちゃダメ (Widgetsが消えてしまう)
  if (!winMinimized)
  {
    // 全てのStateと、各ウィンドウのGeometry
    mWindowStateManager->save();

    // Windowの表示・非表示
    mSettings->setValue( "statusBar/visible", QVariant::fromValue( statusBar()->isVisible() ) );
  }

  // 表示オプション保存
  mSettings->setValue( "view/fontSize", QVariant::fromValue( mFontSize ) );

  // MRU保存
  AQ()->SaveMRU( mSettings );
}



///////////////////////////////////////////////////////////////////////////
void MainWindow::InitToolbar()
{
  /////////////
  // MFG関連ToolBar (Run, デバッグかどうかなど)
  /////////////
  mMFGBar = new QToolBar();
  mMFGBar->setIconSize( QSize( qt_pal::MagInt( 24 ), qt_pal::MagInt( 24 ) ) );
  mMFGBar->setObjectName( "MFGBar" );
  
  mMFGRun = new QToolButton();
  mMFGRun->setObjectName( "MFGRun" );
  mMFGRun->setDefaultAction( CreateAction( ":/images/icon_run", "", this ) );
  mMFGRun->setFocusPolicy( Qt::NoFocus ); // フォーカスあたってもいいかも？
  mMFGRun->setToolTip( MStr(19) );

  mMFGDebug = new QCheckBox( MStr(9) ); // 図形

  mMFGBar->addWidget( mMFGRun );
  mMFGBar->addWidget( mMFGDebug );

  addToolBar( Qt::TopToolBarArea, mMFGBar );
  connect( mMFGRun, SIGNAL(clicked()), this, SLOT(onRunScript()) );
}

std::vector<QString> MainWindow::TargetList()
{
  QStringList exts;
  exts << "*.png";

  return TargetList( g_sampleNames, exts );
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void MainWindow::InitWidgets()
{
  // 左右のみDock
  Qt::DockWidgetAreas lr = Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea;

  //////////////////////
  // [左 Widgets]
  //////////////////////
  mWidgetProjectExplorer = new WidgetProjectExplorer();
  connect( mWidgetProjectExplorer, &WidgetProjectExplorer::fileSelected, this, [this]( const QFileInfo& fi ) {
    openScriptFile( fi.absoluteFilePath() );
  });
  connect( mWidgetProjectExplorer, &WidgetProjectExplorer::importImage, this, [this]() {
    // ダイアログを出してインポート
    onImportImageToProject();
  });
  connect( mWidgetProjectExplorer, &WidgetProjectExplorer::createStringsJson, this, [this]( const QString& jsonPath ) {
    onNewStringsJson( jsonPath );
  });
  connect( mWidgetProjectExplorer, &WidgetProjectExplorer::importThumbnail, this, [this]() {
    onImportAsThumbnail();
  });
  mDockProjectExplorer = new QDockWidget();
  mDockProjectExplorer->setObjectName( "DockProjextExplorer" );
  mDockProjectExplorer->setWindowTitle( "Project Explorer" );
  mDockProjectExplorer->setWidget( mWidgetProjectExplorer );  
  mDockProjectExplorer->setAllowedAreas( lr );
  mDockProjectExplorer->setVisible( true );
  addDockWidget( Qt::LeftDockWidgetArea, mDockProjectExplorer );

  //////////////////////
  // [右 Widgets]
  //////////////////////

  // Preview
  auto targetPaths = TargetList();
  mWidgetPreview = new WidgetPreview( this, targetPaths );

  mDockPreview = new QDockWidget();
  mDockPreview->setObjectName( "DockPreview" );
  mDockPreview->setWindowTitle( MStr(23) );
  mDockPreview->setWidget( mWidgetPreview );
  mDockPreview->setAllowedAreas( lr );
  addDockWidget( Qt::RightDockWidgetArea, mDockPreview );

  // Bottom.
  mOutput = new WidgetOutput();

  mDockOutput = new QDockWidget();
  mDockOutput->setObjectName( "DockOutput" );
  mDockOutput->setWindowTitle( MStr(27) );
  mDockOutput->setWidget( mOutput );
  mDockOutput->setAllowedAreas( lr | Qt::BottomDockWidgetArea );
  addDockWidget( Qt::BottomDockWidgetArea, mDockOutput );

  // マルチカラム化
  setDockNestingEnabled( true );
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void MainWindow::InitConnect()
{
  // メニューのconnect
  connect( ui->action_FileProjectNew, SIGNAL(triggered()), this, SLOT(onFileProjectNew()) );
  connect( ui->action_FileExamplesProject, SIGNAL(triggered()), this, SLOT(onFileExamplesProject()) );
  connect( ui->action_FileProjectOpen, SIGNAL(triggered()), this, SLOT(onFileProjectOpen()) );
  connect( ui->action_FileOpen, SIGNAL(triggered()), this, SLOT(onFileOpen()) );
  connect( ui->action_FileSave, SIGNAL(triggered()), this, SLOT(onFileSave()) );
  connect( ui->action_FileSaveAs, SIGNAL(triggered()), this, SLOT(onFileSaveAs()) );
  connect( ui->action_FileClose, SIGNAL(triggered()), this, SLOT(onFileClose()) );
  connect( ui->action_FileProjectClose, SIGNAL(triggered()), this, SLOT(onFileProjectClose()) );

  connect( ui->action_EditUndo, SIGNAL(triggered()), this, SLOT(onEditUndo()) );
  connect( ui->action_EditRedo, SIGNAL(triggered()), this, SLOT(onEditRedo()) );
  connect( ui->action_EditCut, SIGNAL(triggered()), this, SLOT(onEditCut()) );
  connect( ui->action_EditCopy, SIGNAL(triggered()), this, SLOT(onEditCopy()) );
  connect( ui->action_EditPaste, SIGNAL(triggered()), this, SLOT(onEditPaste()) );

  connect( ui->action_ProjectPackMAR, SIGNAL(triggered()), this, SLOT(onProjectPackMAR()) );

  connect( ui->action_ViewZoomIn, SIGNAL(triggered()), this, SLOT(onViewZoomIn()) );
  connect( ui->action_ViewZoomOut, SIGNAL(triggered()), this, SLOT(onViewZoomOut()) );

  connect( ui->action_WindowInitialize, SIGNAL(triggered()), this, SLOT(onWindowInitialize()) );
  connect( ui->action_WindowPreview, SIGNAL(triggered()), this, SLOT(onWindowPreview()) );
  connect( ui->action_WindowOutput, SIGNAL(triggered()), this, SLOT(onWindowOutput()) );
  connect( ui->action_WindowStatus, SIGNAL(triggered()), this, SLOT(onWindowStatus()) );

  connect( ui->action_HelpOfficial, SIGNAL(triggered()), this, SLOT(onHelpOfficial()) );
  connect( ui->action_HelpAbout, SIGNAL(triggered()), this, SLOT(onHelpAbout()) );
  connect( ui->action_HelpConfig, SIGNAL(triggered()), this, SLOT(onHelpConfig()) );
  connect( ui->action_HelpAuto, SIGNAL(triggered()), this, SLOT(onHelpLangAuto()) );
  connect( ui->action_HelpEnglish, SIGNAL(triggered()), this, SLOT(onHelpLangEnglish()) );
  connect( ui->action_HelpJapanese, SIGNAL(triggered()), this, SLOT(onHelpLangJapanese()) );
  connect( ui->action_HelpChineseSimp, SIGNAL(triggered()), this, SLOT(onHelpLangChineseSimp()) );
  connect( ui->action_HelpChineseTrad, SIGNAL(triggered()), this, SLOT(onHelpLangChineseTrad()) );
  connect( ui->action_HelpKorean, SIGNAL(triggered()), this, SLOT(onHelpLangKorean()) );
  connect( ui->action_HelpPortugues, SIGNAL(triggered()), this, SLOT(onHelpLangPortugues()) );
  connect( ui->action_HelpSpanish, SIGNAL(triggered()), this, SLOT(onHelpLangSpanish()) );
  connect( ui->action_HelpGerman, SIGNAL(triggered()), this, SLOT(onHelpLangGerman()) );
  connect( ui->action_HelpFrench, SIGNAL(triggered()), this, SLOT(onHelpLangFrench()) );
  connect( ui->action_HelpRussian, SIGNAL(triggered()), this, SLOT(onHelpLangRussian()) );
  connect( ui->action_HelpHindi, SIGNAL(triggered()), this, SLOT(onHelpLangHindi()) );
  connect( ui->action_HelpBengali, SIGNAL(triggered()), this, SLOT(onHelpLangBengali()) );
  connect( ui->action_HelpPolish, SIGNAL(triggered()), this, SLOT(onHelpLangPolish()) );

  ui->action_HelpQt->setStatusTip( tr("Show the Qt library's About box") );
  connect( ui->action_HelpQt, SIGNAL(triggered()), qApp, SLOT(aboutQt()) );

  // メニューが表示される前の更新
  connect( ui->menu_File, SIGNAL(aboutToShow()), this, SLOT(onFileMenu()) );
  connect( ui->menu_Edit, SIGNAL(aboutToShow()), this, SLOT(onEditMenu()) );
  connect( ui->menu_Project, SIGNAL(aboutToShow()), this, SLOT(onProjectMenu()) );
  connect( ui->menu_View, SIGNAL(aboutToShow()), this, SLOT(onViewMenu()) );
  connect( ui->menu_Window, SIGNAL(aboutToShow()), this, SLOT(onWindowMenu()) );
  connect( ui->menu_Help, SIGNAL(aboutToShow()), this, SLOT(onHelpMenu()) );

  // ロールを設定する場所がほかにないのでここで設定しておく。
  ui->action_HelpQt->setMenuRole( QAction::AboutQtRole );
  ui->action_HelpAbout->setMenuRole( QAction::AboutRole );
}

/////////////////////////////////////////////////////////////////////////////
bool MainWindow::event( QEvent* event )
{
  //////////////
  // イベント処理
  //////////////

  if (event->type() == QEvent::LayoutRequest) {
    // 初回のみ、起動状態の保存と前回のウィンドウ状態の復元
    static std::once_flag once;
    std::call_once(once, [this] {
      if (!mWindowStateManager->hasInitialState()) {
        QTimer::singleShot( 0, this, [this] {
          mWindowStateManager->saveInitialState();
          mWindowStateManager->restore();
        } );
      }
    } );
  }

  return QMainWindow::event( event );
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
int gAntsCount = 0;

///////////////////////////////////////////////////////////////////////////
void MainWindow::timerEvent( QTimerEvent* event )
{
  // 各種タイマー処理
  // MFGOpenStudioでは一種類しか無いので無駄な間接になっているがMFGStudioとのコード共有のためこうなっている。
  if (event->timerId() == mTimerAnts)
  {
    timerAnts(); // 100[ms]
  }
}



///////////////////////////////////////////////////////////////////////////
void MainWindow::timerAnts()
{
  // statusBarのメッセージ更新 (150msに一度)
  ApplyStatusTitle();

  // 蟻処理が呼ばれすぎないよう
  size_t dif = mfg_pal::Tick() - mLastAntsArrive;
  if (dif < mAntsInterval) return;

  mLastAntsArrive = mfg_pal::Tick();
}

///////////////////////////////////////////////////////////////////////////
void MainWindow::showEvent( QShowEvent* event )
{
  Q_UNUSED(event);

  // 一回だけ
  if (!mFirstShow) return;
  mFirstShow = false;

  // タイマー初期化
  mLastAntsArrive = 0;
  mAntsInterval = 150;
  mTimerAnts = startTimer( 100 );
}

///////////////////////////////////////////////////////////////////////////
void MainWindow::closeEvent( QCloseEvent* event )
{
  // 何か処理中だったら、すぐ抜ける
  WidgetProcessing* wp = Central()->_widgetProcessing;
  if (wp->mProcessing)
  {
    event->ignore();
    return;
  }

  // 基本的に閉じる
  bool ignored = false;
  event->accept();

  // 全て閉じるのを試みる
  Central()->MdiArea()->closeAllSubWindows();

  if (HasOpenChild())
  {
    // 子ウィンドウが残ってるので、閉じない
    event->ignore();
    ignored = true;
  }

  // 全て閉じられた？
  if (!ignored)
  {
    // 念のためタイマーを殺す
    killTimer( mTimerAnts );

    cursor_t old = BeginWaitCursor();

    // ウィンドウを閉じる前に状態保存
    saveSettings();

    delete mSettingsLanguage;
    delete mSettings;

    EndWaitCursor( old );
  }
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// メニューの更新
///////////////////////////////////////////////////////////

void MainWindow::updateProjectMenu()
{
  ui->action_ProjectPackMAR->setEnabled( _project.IsProjectSelected() );
}

///////////////////////////////////////////////////////////
void MainWindow::updateWindowMenu()
{
  bool b = true;
  ui->action_WindowInitialize->setEnabled( b );

  ui->action_WindowPreview->setEnabled( b );
  ui->action_WindowOutput->setEnabled( b );

  ui->action_WindowPreview->setChecked( mDockPreview->isVisible() );
  ui->action_WindowOutput->setChecked( mDockOutput->isVisible() );

  ui->action_WindowP0->setVisible( false );
  ui->action_WindowP1->setVisible( false );

  ui->action_WindowStatus->setChecked( statusBar()->isVisible() );
}

///////////////////////////////////////////////////////////////////////////
void MainWindow::onFilterMFGScript( const std::string& data, bool enableDebug )
{
  if (!checkBeforeFilter()) return;
  mfg_resource::SetLanguage( Trans().Language() );

  {
    auto dialog = CreateDialogMFG( data );
    // パースエラーなどのexceptionがあるかもしれないのでScopeGuardで呼ぶ。
    auto guard = ScopeGuard( [&](){
      dialog->GetTarget().EndFilter();
    });

    // 今の所コンストラクタで設定するのと変わらないが、
    // 将来的にあとから変えるかもしれないのでこうなっている。
    // コンストラクタにしたほうがいいかも
    dialog->SetDebug( enableDebug );
    if (dialog->exec() == QDialog::Accepted)
    {
      dialog->Filter();

      auto info = dialog->GetPerfInfo();
      AddOutput( "InputSetup: " + QString::number(info._inputCopy) + " [ms]" );
      AddOutput( "Kernel: " + QString::number(info._kernelRun) + " [ms]" );
      AddOutput( "ResultCopy: " + QString::number(info._resultCopy) + " [ms]" );
      AddOutput( "Total: " + QString::number(info.Total()) + " [ms]" );
      AddOutput( "" );
    }
  }
}
