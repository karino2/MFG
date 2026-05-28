/* -*- coding: utf-8 -*- マルチバイト */

// Copyright 2026 PGN Inc. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

/*
  このファイルはMFGOpenStudioとMFGStudioのmainwindow.cppの共通メソッドを置くファイルです。
  MFGStudioとMFGOpenStudioのMainWindowはプロジェクト関連の処理などはコードレベルでは全く同一ですが、
  クラスとしては異なります。そこでコードレベルで同一のメソッドをここに置く事で重複を除去します。
*/


#include "mainwindow_impl.h"
#include "ui_mainwindow.h"

#include "studio_conf.h"
#include "trans.h"
#include "window_state_manager.h"

#include "dialog_about.h"
#include "dialog_new_project.h"
#include "dialog_filter_mfg.h"

#include "widget_mfg_editor.h"
#include "widget_central.h"
#include "widget_project_explorer.h"
#include "widget_preview.h"
#include "widget_output.h"

#include <QClipboard>
#include <QDesktopServices>
#include <QFileDialog>
#include <QLibraryInfo>
#include <QMessageBox>
#include <QTranslator>


#include "mainwindow_util.h"
using namespace mainwindow_util;

#include <qt_pal/qt_pal.h>
using namespace mfg_pal;
using qt_pal::GetQtDesktopPath;

#include "app_qt.h"
using app_qt::AQ;
using app_qt::AppQt;

using qt_pal::BeginWaitCursor;
using qt_pal::EndWaitCursor;
using qt_pal::cursor_t;

void MainWindow::InitSamples( const std::vector<QString>& samples )
{
  // サンプルターゲットをコピーしておく
  auto resNames = MapPrefix( ":/sample_targets/", samples );
  for (auto& res : resNames)
  {
    CopyResFile( PreviewTargetFolder(), res );
  }
}

void MainWindow::InitLanguage()
{
  // デフォはOS言語で、自動選択
  mLanguageIndex = 0;

  QLocale lc;
  QLocale::Language lang = lc.language();

  // 言語設定は保存されてる？
  mSettingsLanguage = NewSettings( mLanguagePath );
  QVariant v = mSettingsLanguage->value( "language/index" );
  if (!v.isNull()) mLanguageIndex = v.toInt();

  bool englishMode = false;
  if (mLanguageIndex == 1) englishMode = true; // 英語が選ばれた
  if ((mLanguageIndex == 0) && (lang == QLocale::English)) englishMode = true; // 自動かつ英語

  if (englishMode)
  {
    // 英語なら切り替えはしない
    Trans().SetDoTranslate( false );
  }
  else
  {
    if (mLanguageIndex == 0)
    {
      /////////////////////////////
      // 自動選択
      /////////////////////////////
      if (lang == QLocale::English) Trans().SetDoTranslate( false );
      if (lang == QLocale::Japanese) Trans().SetLanguage( NL_JAPANESE );
      if (lang == QLocale::Chinese)
      {
        QLocale::Territory territory = lc.territory();
        if (territory == QLocale::HongKong || territory == QLocale::Taiwan)  // FIXME: 本当に香港と台湾だけ？
          Trans().SetLanguage( NL_CHINESE_TRAD );
        else
          Trans().SetLanguage( NL_CHINESE_SIMP );
      }
      if (lang == QLocale::Korean) Trans().SetLanguage( NL_KOREAN );
      if (lang == QLocale::Portuguese) Trans().SetLanguage( NL_PORTUGUESE );
      if (lang == QLocale::Spanish) Trans().SetLanguage( NL_SPANISH );
      if (lang == QLocale::German) Trans().SetLanguage( NL_GERMAN );
      if (lang == QLocale::French) Trans().SetLanguage( NL_FRENCH );
      if (lang == QLocale::Russian) Trans().SetLanguage( NL_RUSSIAN );
      if (lang == QLocale::Hindi) Trans().SetLanguage( NL_HINDI );
      if (lang == QLocale::Bengali) Trans().SetLanguage( NL_BENGALI );
      if (lang == QLocale::Polish) Trans().SetLanguage( NL_POLISH );
    }
    else
    {
      /////////////////////////////
      // Indexから選択 (英語以外)
      /////////////////////////////
      if (mLanguageIndex == 1) Trans().SetDoTranslate( false );
      if (mLanguageIndex == 2) Trans().SetLanguage( NL_JAPANESE );
      if (mLanguageIndex == 3) Trans().SetLanguage( NL_CHINESE_SIMP );
      if (mLanguageIndex == 4) Trans().SetLanguage( NL_CHINESE_TRAD );
      if (mLanguageIndex == 5) Trans().SetLanguage( NL_KOREAN );
      if (mLanguageIndex == 6) Trans().SetLanguage( NL_PORTUGUESE );
      if (mLanguageIndex == 7) Trans().SetLanguage( NL_SPANISH );
      if (mLanguageIndex == 8) Trans().SetLanguage( NL_GERMAN );
      if (mLanguageIndex == 9) Trans().SetLanguage( NL_FRENCH );
      if (mLanguageIndex == 10) Trans().SetLanguage( NL_RUSSIAN );
      if (mLanguageIndex == 11) Trans().SetLanguage( NL_HINDI );
      if (mLanguageIndex == 12) Trans().SetLanguage( NL_BENGALI );
      if (mLanguageIndex == 13) Trans().SetLanguage( NL_POLISH );
    }

    // Mac の Edit メニューのみ (Mountain Lion 不具合対応)
#if defined(__APPLE__)
    ui->menu_Edit->setTitle( " Edit" );
#endif

    // 必要なら (英語じゃないなら) メニュー切り替え
    if (Trans().Language() != NL_ENGLISH) TranslateActions();
  }
}

void MainWindow::InitTranslation()
{
  // Qtの翻訳ファイル読み込み
  QTranslator* translator = new QTranslator( this );
  if (translator->load( Trans().Locale(), "qt", "_", QLibraryInfo::path( QLibraryInfo::TranslationsPath ) ))
  {
    QCoreApplication::installTranslator( translator );
  }
  else
  {
    delete translator;
  }
}

///////////////////////////////////////////////////////////////////////////
// データフォルダを用意しておく
///////////////////////////////////////////////////////////////////////////
void MainWindow::initDataFolder()
{
  /////////////////
  // 設定ファイルパス
  /////////////////
  QString dataFolder = AQ()->DataFolder();
  QString iccFolder = AQ()->ICCProfileFolder();

  // 基本設定
  mConfigPath = dataFolder + "Config.ini";   // 全体コンフィグ

  // その他、言語設定やショートカット
  mInitPath = dataFolder + "Initialize.ini"; // ウィンドウ初期化用
  mLanguagePath = dataFolder + "Language.ini";

  mPreviewTargetFolder = dataFolder + "preview_targets";
  mExamplesProjectFolder = dataFolder + "examples_project";
  mDebugFolder = dataFolder + "debug";

  /////////////////
  // フォルダを掘る
  /////////////////
  QDir dir2;
  dir2.mkpath( iccFolder );
  dir2.mkpath( mPreviewTargetFolder );
#if defined(QT_DEBUG)
  dir2.mkpath( mDebugFolder );
#endif

  /////////////////
  // フォルダ名の調整
  /////////////////
  mPreviewTargetFolder = mPreviewTargetFolder + "/";
  mDebugFolder = mDebugFolder + "/";
}

///////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
QString MainWindow::getSaveFolder()
{
  if (_project.IsProjectSelected())
    return _project.ProjectPath();

  const QString s = settings()->value( "main/saveFolder" ).toString();
  if (s != "") return s;

  return GetQtDesktopPath();
}

///////////////////////////////////////////////////////////////////////////
void MainWindow::setSaveFolder( QString folderPath )
{
  settings()->setValue( "main/saveFolder", folderPath );
}

///////////////////////////////////////////////////////////////////////////
void MainWindow::setSavePath( QString fullPath )
{
  const QFileInfo fi( fullPath );
  const QString folderPath = fi.absoluteDir().absolutePath();

  setSaveFolder( folderPath );
}

///////////////////////////////////////////////////////////////////////////
QString MainWindow::getOpenFolder()
{
  if (_project.IsProjectSelected())
    return _project.ProjectPath();

  const QString s = settings()->value( "main/openFolder" ).toString();
  if (s != "") return s;

  return GetQtDesktopPath();
}

///////////////////////////////////////////////////////////////////////////
void MainWindow::setOpenFolder( QString folderPath )
{
  settings()->setValue( "main/openFolder", folderPath );
}

///////////////////////////////////////////////////////////////////////////
QString MainWindow::getOpenProjectFolder()
{
  const QString s = settings()->value( "main/openProjFolder" ).toString();
  if (s != "") return s;

  return GetQtDesktopPath();
}

///////////////////////////////////////////////////////////////////////////
void MainWindow::setOpenProjectFolder( QString folderPath )
{
  settings()->setValue( "main/openProjFolder", folderPath );
}

void MainWindow::initDefaultShortcut()
{
  ui->action_FileSave->setShortcut( QKeySequence::Save );
  ui->action_FileClose->setShortcut( QKeySequence::Close );
  ui->action_ViewZoomIn->setShortcut( QKeySequence::ZoomIn );
  ui->action_ViewZoomOut->setShortcut( QKeySequence::ZoomOut );
}

WidgetMFGEditor* MainWindow::NewEditor()
{
  auto editor = new WidgetMFGEditor(0, [this]() { return getSaveFolder(); });
  editor->SetFontSize( mFontSize );
  return editor;
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void MainWindow::sMenu( QMenu* menu, QString text )
{
  // リソースがなかった差し替えない
  if (text == "") return;
  menu->setTitle( text );
}

///////////////////////////////////////////////////////////////////////////
void MainWindow::sAction( QAction* action, QString text )
{
  // リソースがなかった差し替えない
  if (text == "") return;
  action->setText( text );
}

std::vector<QString> MainWindow::TargetList( const std::vector<QString>& samples, QStringList exts )
{
  auto predefined = MapPrefix( PreviewTargetFolder(), samples );
  std::set<QString> preNameSet( samples.begin(), samples.end() );

  QDir targetFolder( PreviewTargetFolder() );
  targetFolder.setNameFilters( exts );
  targetFolder.setFilter( QDir::Files | QDir::NoDotAndDotDot );

  QFileInfoList list = targetFolder.entryInfoList();
  std::vector<QFileInfo> filtered;
  std::copy_if(list.begin(), list.end(), std::back_inserter(filtered),
             [&preNameSet](const QFileInfo &fi) {
              return preNameSet.find(fi.fileName()) == preNameSet.end();
             });

  std::vector<QString> custom = MapFn(filtered, [](const QFileInfo& fi) { return fi.absoluteFilePath(); } );

  // customが先、predefinedがあと。
  std::copy( predefined.begin(), predefined.end(), back_inserter(custom) );
  return custom;
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void MainWindow::InitMRU()
{
  auto aq = AQ();

  // Recent Files
  aq->_recentAction[0] = ui->action_File0;
  aq->_recentAction[1] = ui->action_File1;
  aq->_recentAction[2] = ui->action_File2;
  aq->_recentAction[3] = ui->action_File3;
  aq->_recentAction[4] = ui->action_File4;
  aq->_recentAction[5] = ui->action_File5;
  aq->_recentAction[6] = ui->action_File6;
  aq->_recentAction[7] = ui->action_File7;

  connect( aq->_recentAction[0], SIGNAL(triggered()), this, SLOT(onFileMRU0()) );
  connect( aq->_recentAction[1], SIGNAL(triggered()), this, SLOT(onFileMRU1()) );
  connect( aq->_recentAction[2], SIGNAL(triggered()), this, SLOT(onFileMRU2()) );
  connect( aq->_recentAction[3], SIGNAL(triggered()), this, SLOT(onFileMRU3()) );
  connect( aq->_recentAction[4], SIGNAL(triggered()), this, SLOT(onFileMRU4()) );
  connect( aq->_recentAction[5], SIGNAL(triggered()), this, SLOT(onFileMRU5()) );
  connect( aq->_recentAction[6], SIGNAL(triggered()), this, SLOT(onFileMRU6()) );
  connect( aq->_recentAction[7], SIGNAL(triggered()), this, SLOT(onFileMRU7()) );

  // Recent Projects
  aq->_recentProjectAction[0] = ui->action_FileProject0;
  aq->_recentProjectAction[1] = ui->action_FileProject1;
  aq->_recentProjectAction[2] = ui->action_FileProject2;
  aq->_recentProjectAction[3] = ui->action_FileProject3;
  aq->_recentProjectAction[4] = ui->action_FileProject4;
  aq->_recentProjectAction[5] = ui->action_FileProject5;
  aq->_recentProjectAction[6] = ui->action_FileProject6;
  aq->_recentProjectAction[7] = ui->action_FileProject7;

  connect( aq->_recentProjectAction[0], SIGNAL(triggered()), this, SLOT(onFileMRUProject0()) );
  connect( aq->_recentProjectAction[1], SIGNAL(triggered()), this, SLOT(onFileMRUProject1()) );
  connect( aq->_recentProjectAction[2], SIGNAL(triggered()), this, SLOT(onFileMRUProject2()) );
  connect( aq->_recentProjectAction[3], SIGNAL(triggered()), this, SLOT(onFileMRUProject3()) );
  connect( aq->_recentProjectAction[4], SIGNAL(triggered()), this, SLOT(onFileMRUProject4()) );
  connect( aq->_recentProjectAction[5], SIGNAL(triggered()), this, SLOT(onFileMRUProject5()) );
  connect( aq->_recentProjectAction[6], SIGNAL(triggered()), this, SLOT(onFileMRUProject6()) );
  connect( aq->_recentProjectAction[7], SIGNAL(triggered()), this, SLOT(onFileMRUProject7()) );
}

void MainWindow::AdjustInitialToolBar()
{
  // macOSなら ツールバーのボタン間隔を小さくする
#if defined(__APPLE__)
  for (auto bar : findChildren<QToolBar*>())
  {
    bar->layout()->setSpacing( 0 );
  }
#endif
}

void MainWindow::InitInitialProject()
{
  const QString lastProj = settings()->value("main/currentProject").toString();
  if (lastProj != "")
  {
    openProject( lastProj, true );
  }
  else
  {
    setWindowTitle( STUDIO_APP_NAME );
  }
}

///////////////////////////////////////////////////////////////////////////
void MainWindow::setSmallIcon( bool smallIcon )
{
  if (!mAdjustIconSize || mSmallIcon == smallIcon) return;

  static const auto iconSizes = [this] {
    QHash<QToolBar*, QSize> sizes;
    for (auto bar : findChildren<QToolBar*>())
    {
      sizes.insert( bar, bar->iconSize() );
    }
    return sizes;
  }();
  mSmallIcon = smallIcon;
  const double scale = smallIcon ? 0.75 : 1;
  for (auto bar : findChildren<QToolBar*>())
  {
    auto size = iconSizes.value( bar ) * scale;
    if (size.width() < 18)
    {
      size = QSize( 18, 18 );
    }
    bar->setIconSize( size );
  }
}

/////////////////////////////////////////////////////////////////////////////
void MainWindow::updateAllChildTitle()
{
  // 全てのタブのタイトルを更新
  std::vector<WidgetMFGEditor*> ch = EditorChildren();
  for (size_t i=0; i<ch.size(); i++)
  {
    WidgetMFGEditor* e = ch[i];
    e->UpdateWindowTitle();
  }
}

void MainWindow::updateFontSize()
{
  // 全てのエディタを更新
  std::vector<WidgetMFGEditor*> ch = EditorChildren();
  for (size_t i=0; i<ch.size(); i++)
  {
    WidgetMFGEditor* e = ch[i];
    e->SetFontSize( mFontSize );
  }
}


WidgetMFGEditor* MainWindow::AE()
{
  QMdiSubWindow* sub = Central()->MdiArea()->currentSubWindow();
  if (sub == NULL) return NULL;

  return dynamic_cast<WidgetMFGEditor*>(sub->widget());
}

/////////////////////////////////////////////////////////////////////////////
std::vector<WidgetMFGEditor*> MainWindow::EditorChildren()
{
  std::vector<WidgetMFGEditor*> res;

  QList<QMdiSubWindow*> children = Central()->MdiArea()->subWindowList();
  for (QList<QMdiSubWindow*>::iterator i = children.begin(); i != children.end(); ++i)
  {
    QMdiSubWindow *subWindow = (*i);
    WidgetMFGEditor *child = dynamic_cast<WidgetMFGEditor*>(subWindow->widget());
    if (child == NULL) continue;

    res.push_back( child );
  }
  return res;
}

/////////////////////////////////////////////////////////////////////////////
WidgetCentral* MainWindow::Central()
{
  return mWidgetCentral;
}

/////////////////////////////////////////////////////////////////////////////
void MainWindow::AddOutput( QString s )
{
  mOutput->append( s );
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void MainWindow::resizeEvent( QResizeEvent* event )
{
  QMainWindow::resizeEvent( event );
  setSmallIcon( event->size().height() < 800 );
}

void MainWindow::ApplyStatusTitle()
{
  if (AQ()->StatusMessageChanged())
  {
    if (GetStatusBar()->isVisible())
    {
      GetStatusBar()->showMessage( AQ()->StatusMessage() );
    }
    AQ()->StatusMessageShown();
  }
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// メニューの更新
///////////////////////////////////////////////////////////
void MainWindow::updateFileMenu()
{
  WidgetMFGEditor* ae = AE();

  bool hasChild = (ae != NULL);

  ui->action_FileSave->setEnabled( hasChild );
  ui->action_FileSaveAs->setEnabled( hasChild );
  ui->action_FileClose->setEnabled( hasChild );

  // project
  bool projSelected = _project.IsProjectSelected();
  ui->action_FileProjectOpen->setEnabled( true );
  ui->action_FileProjectNew->setEnabled( true );
  ui->action_FileExamplesProject->setEnabled( true );
  ui->action_FileProjectClose->setEnabled( projSelected );

  // MRU更新 (Files)
  size_t s = AQ()->_recentFiles.size();
  for (size_t i=0; i<AppQt::MAX_RECENT; i++)
  {
    AQ()->_recentAction[i]->setVisible( s > i );
    if (s > i)
    {
      // ファイル名設定
      AQ()->_recentAction[i]->setText( AQ()->_recentFiles[i] );
    }
  }
  ui->menu_FileRecent->setEnabled( s > 0 );

  // MRU更新 (Projects)
  s = AQ()->_recentProjects.size();
  for (size_t i=0; i<AppQt::MAX_RECENT; i++)
  {
    AQ()->_recentProjectAction[i]->setVisible( s > i );
    if (s > i)
    {
      // ファイル名設定
      AQ()->_recentProjectAction[i]->setText( AQ()->_recentProjects[i] );
    }
  }
  ui->menu_FileRecentProject->setEnabled( (s > 0) );
}

///////////////////////////////////////////////////////////
void MainWindow::updateEditMenu()
{
  ui->action_EditUndo->setEnabled( false );
  ui->action_EditRedo->setEnabled( false );

  ui->action_EditCut->setEnabled( false );
  ui->action_EditCopy->setEnabled( false );
  ui->action_EditPaste->setEnabled( false );

  WidgetMFGEditor* ae = AE();
  if (ae == NULL) return;

  auto te = ae->GetTextEdit();
  if (te->textCursor().hasSelection())
  {
    ui->action_EditCut->setEnabled( true );
    ui->action_EditCopy->setEnabled( true );  
  }
  ui->action_EditUndo->setEnabled( ae->CanUndo() );
  ui->action_EditRedo->setEnabled( ae->CanRedo() );
  
  const QClipboard *clipboard = QApplication::clipboard();
  QString subtype("plain");
  if (clipboard->text(subtype) != "")
  {
    ui->action_EditPaste->setEnabled( true );
  }
}


///////////////////////////////////////////////////////////
void MainWindow::updateViewMenu()
{
  ui->action_ViewZoomIn->setEnabled( false );
  ui->action_ViewZoomOut->setEnabled( false );

  WidgetMFGEditor* ae = AE();
  if (ae != NULL)
  {
    // ズーム設定
    ui->action_ViewZoomIn->setEnabled( true );
    ui->action_ViewZoomOut->setEnabled( true );
  }
}

///////////////////////////////////////////////////////////
void MainWindow::updateHelpMenu()
{
  const bool canTrigger = true;

  ui->menu_HelpLanguage->setEnabled( canTrigger );

  ui->action_HelpAuto->setChecked( mLanguageIndex == 0 );
  ui->action_HelpEnglish->setChecked( mLanguageIndex == 1 );
  ui->action_HelpJapanese->setChecked( mLanguageIndex == 2 );
  ui->action_HelpChineseSimp->setChecked( mLanguageIndex == 3 );
  ui->action_HelpChineseTrad->setChecked( mLanguageIndex == 4 );
  ui->action_HelpKorean->setChecked( mLanguageIndex == 5 );
  ui->action_HelpPortugues->setChecked( mLanguageIndex == 6 );
  ui->action_HelpSpanish->setChecked( mLanguageIndex == 7 );
  ui->action_HelpGerman->setChecked( mLanguageIndex == 8 );
  ui->action_HelpFrench->setChecked( mLanguageIndex == 9 );
  ui->action_HelpRussian->setChecked( mLanguageIndex == 10 );
  ui->action_HelpHindi->setChecked( mLanguageIndex == 11 );
  ui->action_HelpBengali->setChecked( mLanguageIndex == 12 );
  ui->action_HelpPolish->setChecked( mLanguageIndex == 13 );

  // 非対応言語
  ui->action_HelpHindi->setVisible( false );
  ui->action_HelpBengali->setVisible( false );
  ui->action_HelpPolish->setVisible( false );

}

///////////////////////////////////////////////////////////
// メニュー (File,Edit...) が押された時のイベント
///////////////////////////////////////////////////////////
void MainWindow::onFileMenu()
{
  updateFileMenu();
}

///////////////////////////////////////////////////////////////////////////
void MainWindow::onEditMenu()
{
  updateEditMenu();
}

void MainWindow::onProjectMenu()
{
  updateProjectMenu();
}

///////////////////////////////////////////////////////////////////////////
void MainWindow::onViewMenu()
{
  updateViewMenu();
}

///////////////////////////////////////////////////////////////////////////
void MainWindow::onWindowMenu()
{
  updateWindowMenu();
}

///////////////////////////////////////////////////////////////////////////
void MainWindow::onHelpMenu()
{
  updateHelpMenu();
}

void MainWindow::AfterInitChild( WidgetMFGEditor* child )
{
  QMdiSubWindow *sub = Central()->MdiArea()->addSubWindow( child, Qt::Window );
  sub->showMaximized();


  // たぶんchildだけで十分だけど一応元のコードを真似しておく。
  updateAllChildTitle();  
}

///////////////////////////////////////////////////////////////////////////
// File メニュー
///////////////////////////////////////////////////////////////////////////
QString MainWindow::fileOpenDialog( QString folder)
{
  const QString cap = "Open MFG Script";
  const QString filter = "MFG (*.mfg)";

  return QFileDialog::getOpenFileName( this, cap, folder, filter );
}

QString MainWindow::fileOpenDialog()
{
  // 初期フォルダ
  QString folder = getOpenFolder();
  return fileOpenDialog( folder );
}

void MainWindow::setProject( const QDir& dir, const QString& filterPath  )
{
  _project.SetProject( dir, filterPath );
  openScriptFile( _project.FilterPath() );
  setWindowTitle( _project.ProjectName() );
  mWidgetProjectExplorer->openDir( dir );
  settings()->setValue( "main/currentProject", _project.ProjectPath() );
}

void MainWindow::setProjectWithMRU( const QDir& dir, const QString& filterPath  )
{
  setProject( dir, filterPath );

  QDir parent(dir);
  parent.cdUp();
  setOpenProjectFolder( parent.absolutePath() );

  AQ()->AddMRU_Project( _project._dir );
}

///////////////////////////////////////////////////////////////////////////
void MainWindow::openProject( QString dirPath, bool internalProject )
{
  QDir dir( dirPath );
  ProjectValidator validator( dir );
  if (!validator.IsValidProject())
  {
    QMessageBox::warning( this, "", MStr(11), QMessageBox::Ok );  
    return;
  }

  if (internalProject)
  {
    setProject( dir, validator.FirstMFGPath() );
  }
  else
  {
    setProjectWithMRU( dir, validator.FirstMFGPath() );
  }
}

static QMdiSubWindow* FindAlreadyOpened( const QString& fpath, QList<QMdiSubWindow*> subList )
{
  for (int i=0; i<subList.size(); i++)
  {
    QMdiSubWindow* w = subList[i];
    WidgetMFGEditor* e = dynamic_cast<WidgetMFGEditor*>( w->widget() );
    if (e != nullptr)
    {
      if(fpath == e->GetFilePath())
      {
        return w;
      }
    }
  }
  return nullptr;
}

void MainWindow::openScriptFile( QString filePath_ )
{
  // ファイルパスの区切り文字を OS ネイティブに変換する。（開き方によって受け取るパスの区切り文字が違うことがあるので）
  QString filePath = QDir::toNativeSeparators(filePath_);
  nstring fn = filePath.toStdWString();

  QMdiSubWindow* e = FindAlreadyOpened( filePath, Central()->MdiArea()->subWindowList() );
  if (e != nullptr)
  {
    // すでに開かれている。それをactiveに。
    Central()->MdiArea()->setActiveSubWindow( e );
    return;
  }

  // ファイルが存在しない
  if (!QFileInfo::exists( filePath ))
  {
    // この文字列は変えないと駄目かも
    QString cap = Str(51) + "\n\n" + filePath;
    QMessageBox::warning( this, "", cap, QMessageBox::Ok );
    return;
  }

  // 読み込む
  QFileInfo fi;
  fi.setFile( filePath );

  // 次はここからOpenDialogを開く
  setOpenFolder( fi.dir().path() );

  QString content;

  // 空のファイルはcontentが空として正常とみなす。
  if( fi.size() != 0 )
  {
    // 中身があれば
    QFile fr( filePath );
    if (!fr.open( QIODevice::ReadOnly | QIODevice::Text))
    {
      const QString cap = Str(67) + "\n\n" + filePath;
      QMessageBox::warning( this, "", cap, QMessageBox::Ok );
      return;
    }

    // UTF8として読み込む
    QTextStream stream( &fr );
    stream.setEncoding( QStringConverter::Utf8 );

    content = stream.readAll();
  }

  AQ()->AddMRU( filePath );

  auto child = NewEditor();
  child->SetContentFile( content, filePath );

  AfterInitChild( child );
}

bool MainWindow::ensureProjectClose()
{
  if (!_project.IsProjectSelected())
    return true;

  if (QMessageBox::Ok != QMessageBox::question( this, "", MStr(46), QMessageBox::Ok | QMessageBox::Cancel))
    return false;

  onFileProjectClose();
  return true;
}


/////////////////////////////////////////////////////////////////////////
void MainWindow::onFileProjectNew()
{
  endFloating();

  if (!ensureProjectClose())
    return;

  QString dir = getOpenProjectFolder();
  DialogNewProject nd( this, dir );
  if (nd.exec())
  {
    ProjectCreator creator( nd.getDir(), nd.getName() );
    if (creator.ProjDirExists())
    {
      QMessageBox::warning( this, "", MStr(1), QMessageBox::Ok );
      return;
    }    

    if (!creator.CreateProj( nd.getSelectedTemplate() ))
    {
      QMessageBox::warning( this, "", MStr(2), QMessageBox::Ok );
      return;
    }

    setProjectWithMRU( creator.ProjDir(), creator.FilterFilePath() );
  }
}

/////////////////////////////////////////////////////////////////////////
void MainWindow::onFileExamplesProject()
{
  endFloating();

  if (!ensureProjectClose())
    return;

  QString examplesProjDir = ExamplesProjectFolder();
  if (!EnsureExamplesProject( examplesProjDir ))
  {
    // 作るの失敗
    QMessageBox::warning( this, "", MStr(47), QMessageBox::Ok );
    return ;
  }
  openProject( examplesProjDir, true );
}

/////////////////////////////////////////////////////////////////////////
void MainWindow::onFileProjectOpen()
{
  endFloating();

  if (!ensureProjectClose())
    return;

  QFileDialog fd( this );
  fd.setFileMode( QFileDialog::Directory );
  fd.setOption( QFileDialog::ShowDirsOnly, true );
  fd.setDirectory( getOpenProjectFolder() );
  if (fd.exec())
  {
    openProject( fd.selectedFiles().at( 0 ) );
  }
}

void MainWindow::closeSciprtIfExist( QString filePath )
{
  QList<QMdiSubWindow*> children = Central()->MdiArea()->subWindowList();
  for (QList<QMdiSubWindow*>::iterator i = children.begin(); i != children.end(); ++i)
  {
    QMdiSubWindow *subWindow = (*i);
    WidgetMFGEditor *child = dynamic_cast<WidgetMFGEditor*>(subWindow->widget());
    if (child == NULL) continue;

    if (child->GetFilePath() == filePath)
    {
      subWindow->close();
    }
  }
}

void MainWindow::closeAllTab()
{
  QList<QMdiSubWindow*> children = Central()->MdiArea()->subWindowList();
  for (QList<QMdiSubWindow*>::iterator i = children.begin(); i != children.end(); ++i)
  {
    QMdiSubWindow *subWindow = (*i);
    subWindow->close();
  }
}


///////////////////////////////////////////////////////////////////////////
void MainWindow::onFileProjectClose()
{
  endFloating();

  closeAllTab();

  if (HasOpenChild())
  {
    // dirtyなエディタなどを閉じなかったケース。プロジェクトは閉じなかった。
    return;
  }

  _project.Close();
  mWidgetProjectExplorer->closeDir();
  settings()->setValue( "main/currentProject", "" );
  setWindowTitle( STUDIO_APP_NAME );
}


/////////////////////////////////////////////////////////////////////////
void MainWindow::onFileNew()
{
  auto child = NewEditor();
  child->SetupDefault();
  AfterInitChild( child );
}


/////////////////////////////////////////////////////////////////////////
void MainWindow::onFileOpen()
{
  endFloating();

  const QString filePath = fileOpenDialog();
  if (filePath == "") return;

  openScriptFile( filePath );
}

///////////////////////////////////////////////////////////////////////////
void MainWindow::onFileMRU( int index )
{
  endFloating();

  if (index < 0) return;
  if (index >= (int)AQ()->_recentFiles.size()) return;

  openScriptFile( AQ()->_recentFiles[index] );
}


///////////////////////////////////////////////////////////////////////////
void MainWindow::onFileMRUProject( int index )
{
  endFloating();

  if (index < 0) return;
  if (index >= (int)AQ()->_recentProjects.size()) return;

  if (!ensureProjectClose())
    return;

  openProject( AQ()->_recentProjects[index] );
}

///////////////////////////////////////////////////////////////////////////
void MainWindow::onFileSave()
{
  endFloating();

  WidgetMFGEditor* ae = AE();
  if (ae == NULL) return;

  ae->Save( getSaveFolder() );

  AfterShowDialog();
}

///////////////////////////////////////////////////////////////////////////
void MainWindow::onFileSaveAs()
{
  endFloating();

  auto ae = AE();
  if (ae == NULL)
    return;
  
  QString folder = getSaveFolder();
  ae->FileSaveAsDialog( folder );

  AfterShowDialog();
}

bool MainWindow::LoadTarget( const QString& path )
{
  mWidgetPreview->LoadTarget( path );
  return true;
}

bool MainWindow::HasOpenChild()
{
  std::vector<WidgetMFGEditor*> list = EditorChildren();
  return !list.empty();
}

///////////////////////////////////////////////////////////////////////////
void MainWindow::onFileClose()
{
  endFloating();

  QMdiSubWindow* sub = Central()->MdiArea()->activeSubWindow();
  if (sub == NULL) return;

  sub->close();
}


///////////////////////////////////////////////////////////////////////////
// Edit メニュー
///////////////////////////////////////////////////////////////////////////
void MainWindow::onEditUndo()
{
  auto ae = AE();
  if (ae == NULL)
    return;
  
  ae->Undo();
}

///////////////////////////////////////////////////////////////////////////
void MainWindow::onEditRedo()
{
  auto ae = AE();
  if (ae == NULL)
    return;
  
  ae->Redo();
}

///////////////////////////////////////////////////////////////////////////
void MainWindow::onEditCut()
{
  WidgetMFGEditor* ae = AE();
  if (ae == NULL) return;

  ae->GetTextEdit()->cut();
}

///////////////////////////////////////////////////////////////////////////
void MainWindow::onEditCopy()
{
  WidgetMFGEditor* ae = AE();
  if (ae == NULL) return;

  ae->GetTextEdit()->copy();
}

///////////////////////////////////////////////////////////////////////////
void MainWindow::onEditPaste()
{
  WidgetMFGEditor* ae = AE();
  if (ae == NULL) return;

  ae->GetTextEdit()->paste();
}

///////////////////////////////////////////////////////////////////////////
// MFG関連
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
static void MFGDebugLog( const std::string& msg )
{
  auto main = Main();
  if (main == NULL)
    return;

  main->AddOutput( QString::fromStdString( msg ).trimmed() );
}

static constexpr const char* RES_STRING_FNAME = "strings.json";
static constexpr const char* THUMBNAIL_FNAME = "thumbnail.png";

std::unique_ptr<DialogMFG> MainWindow::CreateDialogMFG( const std::string& scriptPath )
{
  std::unique_ptr<mfg_parser::ResourceLoader> loader;
  mfg::ResStringMap smap;

  if (_project.IsProjectSelected()) {
    loader.reset( new ProjectResourceLoader( _project.ProjectDir() ) );
    if (_project.FileExists( RES_STRING_FNAME ))
    {
      auto resPath = _project.GetFilePath( RES_STRING_FNAME );

      QFile qf( resPath );

      // failの場合はどうしよう？とりあえず無視。
      if (qf.open( QIODevice::ReadOnly ))
      {
        auto data = qf.readAll();
        std::string s( (char*)data.constData(), data.size() );
        smap = mfg::JsonToResStringMap( s );

      }
    }
  }

  auto ret = std::unique_ptr<DialogMFG>(
            new DialogMFG( mWidgetPreview->GetTarget( MFGDebugLog ) )
        );
  connect( ret.get(), &DialogMFG::updatePreview, [this]{
    mWidgetPreview->TargetUpdated();
  });
  ret->LoadScript( scriptPath, std::move(loader), std::move(smap) );
  return ret;
}

void MainWindow::RunScript( const std::string& script )
{
  bool enableDebug = mMFGDebug->isChecked();
  try {
    onFilterMFGScript( script, enableDebug );
  }
  catch(const mfg_parser::ParseError& err)
  {
    AddOutput( QString("ParseError:") + QString::number( err.GetLineNum() ) + QString( ":" ) + QString::number( err.GetColumn() ) + QString( ": " ) + QString::fromStdString( err.what() ) );
  }
  catch(const mfg_internal::UserError& err)
  {
    AddOutput( QString("UserError: ") + QString::fromStdString( err.what() ) );
  }
  catch(const mfg_internal::Error& err)
  {
    AddOutput( QString("Error: ") + QString::fromStdString( err.what() ) );
  }
}

void MainWindow::onRunScript()
{
  auto ae = AE();
  if (ae != NULL)
    RunScript( ae->GetContent().toStdString() );
}

///////////////////////////////////////////////////////////////////////////
void MainWindow::onProjectPackMAR()
{
  QString lastPath = settings()->value( "main/marFolder" ).toString();
  if (lastPath == "")
  {
    lastPath = GetQtDesktopPath();
  }
  QFileDialog fd( this );
  fd.setFileMode( QFileDialog::Directory );
  fd.setOption( QFileDialog::ShowDirsOnly, true );
  fd.setDirectory( lastPath );
  fd.setLabelText( QFileDialog::Accept, MStr(53) ); 
  if (fd.exec())
  {
    QString curPath = fd.selectedFiles().at( 0 );
    QDir dir( curPath );
    ProjectArchiver archiver( _project.ProjectDir() );
    QFileInfo archiveFI = _project.DestArchiveFI( dir );
    if (archiver.ArchiveTo( archiveFI ))
    {
      settings()->setValue( "main/marFolder", curPath );

      QMessageBox::information( this, "", MStr(22) + archiveFI.absoluteFilePath(), QMessageBox::Ok );
    }
    else
    {
      QMessageBox::information( this, "", MStr(52) + archiveFI.absoluteFilePath(), QMessageBox::Ok );
    }
  }
}

void MainWindow::onNewStringsJson( const QString& jsonPath )
{
  std::string defJson = R"({
"ja": {"TITLE_SAMPLE": "タイトルのサンプル", "LABEL_SAMPLE": "ラベルのサンプル"},
"en": {"TITLE_SAMPLE": "English or Default Title Sample", "LABEL_SAMPLE": "Sample Label"}
}
)";
  QFile qf( jsonPath );
  if (!qf.open( QIODevice::WriteOnly ))
  {
    QMessageBox::warning( this, "", MStr(55), QMessageBox::Ok );
    return;
  }

  if (defJson.size() != qf.write( defJson.c_str(), defJson.size() ))
  {
    QMessageBox::warning( this, "", MStr(55), QMessageBox::Ok );
    return;
  }
}

///////////////////////////////////////////////////////////////////////////
// View メニュー
///////////////////////////////////////////////////////////////////////////
void MainWindow::onViewZoomIn()
{
  if (isMenuForbidden()) return;

  mFontSize = (mFontSize*12)/10;
  updateFontSize();
}

///////////////////////////////////////////////////////////////////////////
void MainWindow::onViewZoomOut()
{
  if (isMenuForbidden()) return;

  mFontSize = (mFontSize*833)/1000;
  if (mFontSize < 1) mFontSize = 1;
  updateFontSize();
}

///////////////////////////////////////////////////////////////////////////
void MainWindow::onWindowInitialize()
{
  // 一応尋ねてから
  QMessageBox::StandardButton res;
  res = QMessageBox::question( this, "", Str(62), QMessageBox::Ok | QMessageBox::Cancel );
  if (res == QMessageBox::Ok)
  {
    // ウィンドウ位置の初期化
    initWindowSettings();
  }
}

void MainWindow::onImportImageToProject()
{
  // 無いはずだが念の為
  if (!_project.IsProjectSelected())
    return;

  // とりあえずDesktop。
  QString folder = GetQtDesktopPath();

  QString filter = "PNG (*.png)";

  QString filePath = QFileDialog::getOpenFileName( this, "", folder, filter );
  if (filePath == "") return;

  // import処理
  QFileInfo source( filePath );
  QString destPath = _project.GetFilePath( source.fileName() );

  if (!QFile::copy(filePath,  destPath))
  {
    QMessageBox::warning( this, "", MStr(54), QMessageBox::Ok );
  }
}

/*
  previewのキャンバスをthumbnail.pngとしてimport。
  512x512にする。
*/
void MainWindow::onImportAsThumbnail()
{
  // 無いはずだが念の為
  if (!_project.IsProjectSelected())
    return;

  cursor_t old = BeginWaitCursor();
  auto guard = ScopeGuard( [&](){ EndWaitCursor( old );  });

  QImage qimg = mWidgetPreview->AsThumbnail();
  
  auto qpath = _project.GetFilePath( THUMBNAIL_FNAME );
  if(qimg.save( qpath, "png" ))
  {
    QMessageBox::information( this, "", MStr(62), QMessageBox::Ok );
  }
  else
  {
    QMessageBox::information( this, "", MStr(63), QMessageBox::Ok );
  }
}


///////////////////////////////////////////////////////////////////////////
void MainWindow::onWindowPreview()
{
  mDockPreview->setVisible( !mDockPreview->isVisible() );
}

///////////////////////////////////////////////////////////////////////////
void MainWindow::onWindowOutput()
{
  mDockOutput->setVisible( !mDockOutput->isVisible() );
}

///////////////////////////////////////////////////////////////////////////
void MainWindow::onWindowStatus()
{
  statusBar()->setVisible( !statusBar()->isVisible() );
}

///////////////////////////////////////////////////////////////////////////
// Help メニュー
///////////////////////////////////////////////////////////////////////////
void MainWindow::onHelpOfficial()
{
  cursor_t old = BeginWaitCursor();

  const QString s("https://modernfilterlanguageforgpu.org/");
  const QUrl url = QUrl( s );
  QDesktopServices::openUrl( url );

  EndWaitCursor( old );
}

///////////////////////////////////////////////////////////////////////////
void MainWindow::onHelpAbout()
{
  DialogAbout* dialog = new DialogAbout( this );
  dialog->exec();
  delete dialog;
}

///////////////////////////////////////////////////////////////////////////
void MainWindow::onHelpConfig()
{
  cursor_t old = BeginWaitCursor();

  QUrl url = QUrl::fromLocalFile( AQ()->DataFolder() );
  QDesktopServices::openUrl( url );

  EndWaitCursor( old );
}

///////////////////////////////////////////////////////////////////////////
void MainWindow::onHelpLangIndex( int index )
{
  // 不要
  if (mLanguageIndex == index) return;

  // 警告して閉じる
  QMessageBox::information( this, "", Str(66), QMessageBox::Ok );
  mLanguageIndex = index;
  close();
}

///////////////////////////////////////////////////////////////////////////
void MainWindow::onHelpLangAuto()
{
  onHelpLangIndex( 0 );
}

///////////////////////////////////////////////////////////////////////////
void MainWindow::onHelpLangEnglish()
{
  onHelpLangIndex( 1 );
}

///////////////////////////////////////////////////////////////////////////
void MainWindow::onHelpLangJapanese()
{
  onHelpLangIndex( 2 );
}

///////////////////////////////////////////////////////////////////////////
void MainWindow::onHelpLangChineseSimp()
{
  onHelpLangIndex( 3 );
}

///////////////////////////////////////////////////////////////////////////
void MainWindow::onHelpLangChineseTrad()
{
  onHelpLangIndex( 4 );
}

///////////////////////////////////////////////////////////////////////////
void MainWindow::onHelpLangKorean()
{
  onHelpLangIndex( 5 );
}

///////////////////////////////////////////////////////////////////////////
void MainWindow::onHelpLangPortugues()
{
  onHelpLangIndex( 6 );
}

///////////////////////////////////////////////////////////////////////////
void MainWindow::onHelpLangSpanish()
{
  onHelpLangIndex( 7 );
}

///////////////////////////////////////////////////////////////////////////
void MainWindow::onHelpLangGerman()
{
  onHelpLangIndex( 8 );
}

///////////////////////////////////////////////////////////////////////////
void MainWindow::onHelpLangFrench()
{
  onHelpLangIndex( 9 );
}

///////////////////////////////////////////////////////////////////////////
void MainWindow::onHelpLangRussian()
{
  onHelpLangIndex( 10 );
}

///////////////////////////////////////////////////////////////////////////
void MainWindow::onHelpLangHindi()
{
  onHelpLangIndex( 11 );
}

///////////////////////////////////////////////////////////////////////////
void MainWindow::onHelpLangBengali()
{
  onHelpLangIndex( 12 );
}

///////////////////////////////////////////////////////////////////////////
void MainWindow::onHelpLangPolish()
{
  onHelpLangIndex( 13 );
}

