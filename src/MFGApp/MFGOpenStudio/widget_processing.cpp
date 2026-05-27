/* -*- coding: utf-8 -*- マルチバイト */

// Copyright 2026 PGN Inc. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#include "widget_processing.h"
#include <QDialog>

#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QApplication>
#include <QLabel>

#include "trans_str_main.h"
#include <qt_pal/qt_pal.h>

CancelDialog::CancelDialog(QWidget *parent) : QDialog(parent)
{  
  QVBoxLayout* lo = new QVBoxLayout();

  QLabel* label = new QLabel( Trans_StrMain( 605 ) );
  lo->addWidget( label );

  QDialogButtonBox* but = new QDialogButtonBox( QDialogButtonBox::Cancel );
  lo->addWidget( but );


  connect( but, &QDialogButtonBox::rejected, [this]{
    emit canceled();
    hide();
  });

  setLayout( lo );

  setModal( true );
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
WidgetProcessing::WidgetProcessing(QWidget *parent) : QWidget(parent)
{
  mProgress = new QProgressBar();

  QHBoxLayout* lo = new QHBoxLayout();
  lo->addWidget( mProgress );
  lo->setContentsMargins( 0, 0, 0, 0 );
  setLayout( lo );

  mCancelDialog = new CancelDialog(this);
  connect( mCancelDialog, &CancelDialog::canceled, [this]{ mCancelState.DoCancel(); });

  mProcessing = false;
  mProgressBegin = qt_pal::Tick();

}

void WidgetProcessing::StartProcessing()
{
  mProgressBegin = qt_pal::Tick();
  mProcessing = true;
  mCancelState.Clear();
}

// プロセス終わり。自身をhideにしたりフラグを初期化したりする。
void WidgetProcessing::EndProcessing()
{
  mProcessing = false;
  mCancelState.Clear();
  setVisible( false );
  mCancelDialog->hide();
  
}

void WidgetProcessing::MayVisibleLayter()
{
  const int SHOW_PROGRESS_AFTER = 1000;
  const int SHOW_DIALOG_AFTER = 5000;

  if (!isVisible())
  {
    if (mProgressBegin + SHOW_PROGRESS_AFTER < qt_pal::Tick())
    {
      setVisible( true );
    }
  }
  if (mCancelState.MayShowDialog())
  {
    if (mProgressBegin + SHOW_DIALOG_AFTER < qt_pal::Tick())
    {
      mCancelDialog->show();
      mCancelState.DialogShowing();
    }
  }
}

void WidgetProcessing::ProcessEvent()
{
  if (mCancelState.IsModalShown())
  {
    qApp->processEvents( QEventLoop::AllEvents );
  } 
  else
  {
    qApp->processEvents( QEventLoop::ExcludeUserInputEvents );
  }
}

/*
  QtProgress関連

  WidgetCentralは環境により異なるので、WidgetProcessingを返す関数だけはwidget_central.cppで提供する必要がある（冒頭のextern）
*/

extern WidgetProcessing* GetWidgetProcessing();
static qt_pal::cursor_t gOldCursor;

///////////////////////////////////////////////////////////////////////////
// trueで継続
///////////////////////////////////////////////////////////////////////////
bool QtProgress( qt_pal::CallbackInfo info )
{
  // 1000ms以上経って表示されてないなら、表示しよう
  WidgetProcessing* pro = GetWidgetProcessing();
  pro->MayVisibleLayter();

  // キャンセルされた？
  if (pro->IsCanceled())
    return false;

  qt_pal::ApplyCallbackInfo( *pro->mProgress, info );

  // アプリに処理を投げる
  pro->ProcessEvent();  
  return true;
}

// キャンセルフラグを立てる。次回のQtProgressでfalseを返す
void CancelProgress()
{
 WidgetProcessing* pro = GetWidgetProcessing();
 pro->DoCancel();
}

///////////////////////////////////////////////////////////////////////////
void BeforeQtProgress()
{
  gOldCursor = qt_pal::BeginWaitCursor();

  // パネルは最初は隠しておく (一定時間が経過したら、QtProgress で表示)
  WidgetProcessing* pro = GetWidgetProcessing();
  pro->StartProcessing();
}

void BeforeQtProgressCancelable()
{
  BeforeQtProgress();
  WidgetProcessing* pro = GetWidgetProcessing();
  pro->EnableCancel( true );
}

void BeforeQtProgressCancelableNoDialog()
{
  BeforeQtProgress();
  WidgetProcessing* pro = GetWidgetProcessing();
  pro->EnableCancel( false );
}

///////////////////////////////////////////////////////////////////////////
void BeforeQtProgressVisible()
{
  gOldCursor = qt_pal::BeginWaitCursor();

  // 最初からパネルを表示してしまう
  WidgetProcessing* pro = GetWidgetProcessing();
  pro->StartProcessing();
  pro->setVisible( true );
}

///////////////////////////////////////////////////////////////////////////
void AfterQtProgress()
{
  WidgetProcessing* pro = GetWidgetProcessing();
  pro->EndProcessing();
  qt_pal::EndWaitCursor( gOldCursor );
}
