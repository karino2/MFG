/* -*- coding: utf-8 -*- マルチバイト */

// Copyright 2026 PGN Inc. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#include "widget_mfg_editor.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFile>
#include <QTextStream>
#include <QFileDialog>
#include <QMessageBox>

#include <qt_pal/qt_pal.h>
#include "app_qt.h"
#include "mfg_samples.h"

#include "trans_str_mfg.h"

using app_qt::AQ;

/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
HighlighterMFG::HighlighterMFG( QTextDocument *parent ) : QSyntaxHighlighter(parent)
{
    setMFGFormat();
}

static void AddRule( QVector<HighlighterMFG::HighlightingRule>& highlightingRules, const QString& pat, const QTextCharFormat& fmt )
{
    HighlighterMFG::HighlightingRule rule;
    rule.pattern = QRegularExpression( pat );
    rule.format = fmt;
    highlightingRules.append( rule );
}

static void AddRules( QVector<HighlighterMFG::HighlightingRule>& highlightingRules, const QStringList& patterns, const QTextCharFormat& fmt )
{
    foreach (const QString &pattern, patterns)
    {
        AddRule( highlightingRules, pattern, fmt );
    }
}

static void SetKeywordRules( QVector<HighlighterMFG::HighlightingRule>& highlightingRules )
{
    QStringList keywordPatterns;
    /*
     * let  def   by mut! fn
     *
    */

    keywordPatterns << "\\blet\\b";
    keywordPatterns << "\\bdef\\b";
    keywordPatterns << "\\bby\\b";
    keywordPatterns << "\\bfn\\b";
    keywordPatterns << "\\bmut!";

    // 登録
    QTextCharFormat keywordFormat;

    keywordFormat.setForeground( Qt::darkMagenta );
    if (qt_pal::DarkMode())
    {
        keywordFormat.setForeground( Qt::magenta );
    }
    keywordFormat.setFontWeight( QFont::Bold );

    AddRules( highlightingRules, keywordPatterns, keywordFormat );
}

static void SetAttrRules( QVector<HighlighterMFG::HighlightingRule>& highlightingRules )
{
    QStringList patterns;
    /*
     @title @param_i32 @param_f32 @bounds @print_expr
     *
    */

    patterns << "@title\\b";
    patterns << "@version\\b";
    patterns << "@param_i32\\b";
    patterns << "@param_f32\\b";
    patterns << "@param_pos\\b";
    patterns << "@param_f32v2\\b";
    patterns << "@param_f32v4\\b";
    patterns << "@bounds\\b";
    patterns << "@print_expr\\b";

    // 登録
    QTextCharFormat format;


    format.setFontItalic(true);
    format.setForeground(Qt::blue);
    if (qt_pal::DarkMode())
    {
      // Qt::blue だと見にくいので、cyan が良いが被るので yellow に
      format.setForeground( Qt::yellow );
    }
    // format.setFontWeight( QFont::Bold );

    AddRules( highlightingRules, patterns, format );
}

static void SetOperatorsRules( QVector<HighlighterMFG::HighlightingRule>& highlightingRules )
{
    QStringList patterns;
    /*
     =
     decorator等と区別がつかないのでやめておく > <
     * + - / ^
     >> >= <= <<
     ==   ..< ...

     *
    */

    patterns << "=";
    patterns << "\\*";
    patterns << "\\+";
    patterns << "-";
    patterns << "/";
    patterns << "^";
    patterns << "==";
    patterns << ">>";
    patterns << "<<";
    patterns << ">=";
    patterns << "<=";
    patterns << "\\|>";
    patterns << "==";
    patterns << "\\.\\.<";
    patterns << "\\.\\.\\.";

    // 登録
    QTextCharFormat format;


    format.setForeground( Qt::darkCyan );
    if (qt_pal::DarkMode())
    {
        format.setForeground( Qt::cyan );
    }

    AddRules( highlightingRules, patterns, format );
}

static void SetParensRules( QVector<HighlighterMFG::HighlightingRule>& highlightingRules )
{
    QStringList patterns;
    /*
     ) (
     ] [
     } {
     *
    */

    patterns << "\\(";
    patterns << "\\)";
    patterns << "\\[";
    patterns << "\\]";
    patterns << "{";
    patterns << "}";

    // 登録
    QTextCharFormat format;


    format.setForeground( Qt::darkBlue );
    if (qt_pal::DarkMode())
    {
        format.setForeground( Qt::blue );
        format.setForeground( Qt::cyan ); // Qt::blue だと見えにくいので、被るけど修正
    }

    AddRules( highlightingRules, patterns, format );
}

/////////////////////////////////////////////////////////////////////
void HighlighterMFG::setMFGFormat()
{
    SetKeywordRules( highlightingRules );
    SetAttrRules( highlightingRules );
    SetOperatorsRules( highlightingRules );
    SetParensRules( highlightingRules );
    /*
     input_u8 result_u8 sampler
     rsum  ifel  elif
     i32 f32 u8
     exp  avg sin cos
   */

    // 文字列
    QTextCharFormat quotationFormat;
    quotationFormat.setForeground(Qt::darkGreen);
    if (qt_pal::DarkMode())
    {
        quotationFormat.setForeground(Qt::green);
    }
    AddRule( highlightingRules, "\".*\"", quotationFormat );

    // # コメント
    QTextCharFormat singleLineCommentFormat;
    singleLineCommentFormat.setForeground( Qt::gray );
    AddRule( highlightingRules,"#[^\n]*", singleLineCommentFormat );
}

/////////////////////////////////////////////////////////////////////
void HighlighterMFG::highlightBlock(const QString &text)
{
    foreach (const HighlightingRule &rule, highlightingRules)
    {
        QRegularExpression expression( rule.pattern );
        QRegularExpressionMatchIterator iterator = expression.globalMatch( text );

        while (iterator.hasNext())
        {
            QRegularExpressionMatch match = iterator.next();
            int length = match.capturedLength();
            setFormat( match.capturedStart(), length, rule.format );
        }
    }
}

using namespace mfg_samples;

static QString FilterPath( size_t index )
{
  return (AQ()->DataFolder() + "filter0%1.mfg").arg( index );
}

static QString LoadFilter( size_t index, const char* defaultContents )
{
  QString path = FilterPath( index );

  QFile qf( path );
  if (!qf.open( QIODevice::ReadOnly| QIODevice::Text ))
    return QString( defaultContents );

  QTextStream stream( &qf );
  stream.setEncoding( QStringConverter::Utf8 );

  return stream.readAll();
}

static bool SaveFilterTo( const QString& path, const QString& contents )
{
  QFile qf( path );
  if (!qf.open( QIODevice::WriteOnly | QIODevice::Text ))
    return false;

  QTextStream stream( &qf );
  stream.setEncoding( QStringConverter::Utf8 );

  stream << contents;

  return true;
}

static bool SaveFilter( size_t index, const QString& contents )
{
  auto path = FilterPath( index );
  return SaveFilterTo( path, contents );
}

const char* g_mfg_init = R"(
@title "New Filter"

def result_u8 |x, y| {
    let bgra = input_u8(x, y)
    let new_bgr = 255-bgra.xyz 
    u8[*new_bgr, bgra.w]
}  
)";
  
  

/////////////////////////////////////////////////////////////////////////////
WidgetMFGEditor::WidgetMFGEditor(QWidget *parent, std::function<const QString&()> getSaveFolder) : QWidget(parent), mFilePath(""), mGetSaveFolder( std::move(getSaveFolder) )
{
  QVBoxLayout* lo = new QVBoxLayout();
  lo->setContentsMargins( 0, 0, 0, 0 );

  // フォントサイズ調整
  QFont font;
  font.setPixelSize( qt_pal::MagInt( 14 ) );

  mEdit = new WidgetCodeEditor();
  // mEdit->setPlainText( mScripts[0] );
  mEdit->setFont( font );
  mHL = new HighlighterMFG( mEdit->document() );
  lo->addWidget( mEdit );

  setLayout( lo );
  setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Preferred );

  connect(mEdit, &WidgetCodeEditor::documentWasModified, this, &WidgetMFGEditor::onDocumentModified );
  connect(mEdit, &WidgetCodeEditor::undoAvailable, this, &WidgetMFGEditor::onUndoStateChanged );
  connect(mEdit, &WidgetCodeEditor::redoAvailable, this, &WidgetMFGEditor::onRedoStateChanged );
}

WidgetMFGEditor::~WidgetMFGEditor()
{
}

void WidgetMFGEditor::SetupDefault()
{
  SetContentFile( QString(g_mfg_init), "" );
  ClearModifiedFlag();
}

QSize WidgetMFGEditor::sizeHint() const
{
  // 狭いとリサイズが走りキャンバスがガクガクする
  // 少し大きめにする。
  return QSize( 560, 120 );
}

void WidgetMFGEditor::SetEdit( QString s )
{
  mEdit->setPlainText( s );
}

void WidgetMFGEditor::ClearModifiedFlag()
{
  mEdit->clearModifiedFlag();
  if (mLastModified != IsModified())
  {
    mLastModified = IsModified();
    UpdateWindowTitle();
  }
}

bool WidgetMFGEditor::IsModified() const
{
  return mEdit->isModified();  
}

bool WidgetMFGEditor::SaveTo( const QString& path )
{
  QString contents = mEdit->toPlainText();
  bool ret = SaveFilterTo( path, contents );  
  if (ret)
  {
    ClearModifiedFlag();
  }
  return ret;
}

bool WidgetMFGEditor::Save( const QString& folder )
{
  QString current = GetFilePath();
  if (current == "")
  {
    return FileSaveAsDialog( folder );
  }

  return SaveTo( current );
}

bool WidgetMFGEditor::FileSaveAsDialog( const QString& saveFolder )
{
  QString folder = saveFolder;
  QString filePath = GetFilePath();
  if (filePath != "")
  {
    // 保存済みのフォルダを選ぶ (2020/07/18～)
    folder = filePath;
  }

  // workaroundGetSaveFileName
  int lastPoint = folder.lastIndexOf(".");
  if (lastPoint != -1) folder = folder.left( lastPoint );
  QString path = QFileDialog::getSaveFileName( this, Str(6), folder, "MFG (*.mfg)" );

  if (path == "")
    return false;

  // MainWindow::fileOpen 時に QDir::toNativeSeparators してるので、合わせる
  path = QDir::toNativeSeparators( path );

  return SaveTo( path );
}

QString WidgetMFGEditor::GetContent() const
{
  return mEdit->toPlainText();
}

QString WidgetMFGEditor::Str( int idx )
{
  return Trans_StrMFG( idx );
}

void WidgetMFGEditor::UpdateWindowTitle()
{
  QString path = GetFilePath();

  // ファイル名取得
  QFileInfo fi;
  fi.setFile( path );
  QString title = fi.fileName();
  if (title == "")
  {
    title = Str(7);
  }

  if (IsModified())
  {
    title = title + " *";
  }

  if (windowTitle() != title)
  {
    // 必要な時だけ更新
    setWindowTitle( title );
  }
}

void WidgetMFGEditor::onDocumentModified()
{
  if (mLastModified == IsModified())
    return;

  mLastModified = IsModified();
  UpdateWindowTitle();
}

void WidgetMFGEditor::closeEvent( QCloseEvent* event )
{
  if (!mCloseCheck)
  {
    event->accept();
    emit closed(this);
    return;
  }

  // 編集されている？
  bool changed = IsModified();

  // 基本は accept (変化が無い)
  event->accept();

  // 変化はあった？
  if (changed)
  {
    // 変化してるけど、保存する？ダイアログ表示
    QString cap = Str(8) + "\n\n";
    cap = cap + GetFilePath();

    QMessageBox::StandardButton res;
    res = QMessageBox::question( this, "", cap, QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel );

    switch( res )
    {
    case QMessageBox::Yes:
      ////////////////////
      // 保存処理
      ////////////////////
      event->ignore();

      if (Save( mGetSaveFolder() ))
      {
        // 受け入れて閉じる
        event->accept();
      }
      break;

    case QMessageBox::No:
      ///////////////////
      // 保存しないまま閉じる
      ///////////////////
      event->accept();

      break;

    case QMessageBox::Cancel:
      ///////////////////////
      // キャンセルして作業に戻る
      ///////////////////////
      event->ignore();
      break;

    default:
      break;
    }
  }

  if (event->isAccepted()) emit closed(this);  
}

void WidgetMFGEditor::onUndoStateChanged( bool canUndo )
{
  mCanUndo = canUndo;
}

void WidgetMFGEditor::onRedoStateChanged( bool canRedo )
{
  mCanRedo = canRedo;
}

void WidgetMFGEditor::Undo()
{
  mEdit->undo();
}

void WidgetMFGEditor::Redo()
{
  mEdit->redo();
}

void WidgetMFGEditor::SetFontSize( int size )
{
  QFont font;
  font.setPixelSize( qt_pal::MagInt( size ) );
  mEdit->setFont( font );
}


/*

WidgetMFGCtrl

*/

/////////////////////////////////////////////////////////////////////////////
WidgetMFGCtrl::WidgetMFGCtrl(QWidget *parent) : QWidget(parent)
{
  QVBoxLayout* lo = new QVBoxLayout();
  lo->setContentsMargins( 0, 0, 0, 0 );

  mScriptComboBox = new QComboBox();
  mScriptComboBox->addItem( "Nega Posi" );
  mScriptComboBox->addItem( "Mosaic" );
  mScriptComboBox->addItem( "Motion Blur" );
  mScriptComboBox->addItem( "Lens Blur" );
  mScriptComboBox->addItem( "Sandstorm" );
  mScriptComboBox->addItem( "Gomashio" );
  mScriptComboBox->addItem( "Median" );
  mScriptComboBox->addItem( "Mirror" );
  mScriptComboBox->addItem( "Cross" );
  lo->addWidget( mScriptComboBox );
  connect( mScriptComboBox, SIGNAL(currentIndexChanged(int)), SLOT(onScriptSelect(int)) );

  mScripts.push_back( LoadFilter( 0, g_mfg_script_negaposi_invert ) );
  mScripts.push_back( LoadFilter( 1, g_mfg_script_mosaic ) ); // 1
  mScripts.push_back( LoadFilter( 2, g_mfg_script_motion_blur ) ); // 2
  mScripts.push_back( LoadFilter( 3, g_mfg_script_lens_blur ) ); // 3
  mScripts.push_back( LoadFilter( 4, g_mfg_script_sand_storm ) ); // 4
  mScripts.push_back( LoadFilter( 5, g_mfg_script_salt_and_pepper ) ); // 5
  mScripts.push_back( LoadFilter( 6, g_mfg_script_median_filter ) ); // 6
  mScripts.push_back( LoadFilter( 7, g_mfg_script_mirror ) ); // 7
  mScripts.push_back( LoadFilter( 8, g_mfg_script_cross_filter ) ); // 8

  mCheckDebug = new QCheckBox( Trans_StrMFG(9) );
  mCheckDebug->setChecked( false );
  lo->addWidget( mCheckDebug );

  lo->setStretch( 1, 30 );
  lo->setStretch( 2, 10 );


  QHBoxLayout* lb = new QHBoxLayout();

  mRunButton = new QPushButton("Run" );
  lb->addWidget( mRunButton );
  connect( mRunButton, SIGNAL(clicked(bool)), this, SLOT(onRunScript(bool)) );

  lo->addLayout( lb );

  setLayout( lo );
  setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Preferred );

  mRunCallback = [](){};
  mSelectScriptCallback = [](const QString&) {};
}

WidgetMFGCtrl::~WidgetMFGCtrl()
{
}


void WidgetMFGCtrl::onScriptSelect( int scriptIndex )
{
  mSelectScriptCallback( mScripts[scriptIndex] );
}


void WidgetMFGCtrl::onRunScript( bool b )
{
  N_UNUSED( b );
  mRunCallback();
}

bool WidgetMFGCtrl::IsDebugEnabled()
{
  return mCheckDebug->isChecked();
}
