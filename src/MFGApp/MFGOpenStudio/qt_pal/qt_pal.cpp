/* -*- coding: utf-8 -*- マルチバイト */

// Copyright 2026 PGN Inc. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#include "qt_pal.h"
#include <cmath>

#include <QString>
#include <QDialog>
#include <QWidget>
#include <QStandardPaths>
#include <QApplication>

namespace qt_pal {

static double g_uiMag = 1.0;

void SetUIMag( double mag )
{
  if (mag < 0.1) mag = 0.1;
  if (mag > 3) mag = 3;
  g_uiMag = mag;
}

double UIMag()
{
  return g_uiMag;
}

//////////////////////////////////////////////////////////////////////////////
int MagInt( int value )
{
  const double d = g_uiMag * value;
  return (int)std::ceil( d );
}


double MagDouble( double value )
{
  const double d = g_uiMag * value;
  return d;
}


QString GetQtDesktopPath()
{
  const QString desktopPath = QStandardPaths::standardLocations( QStandardPaths::DesktopLocation ).at( 0 );
  return desktopPath;
}

///////////////////////////////////////////////////////////////////////////
void RemoveQuestion( QDialog* dialog )
{
#if defined(_WIN32)
  // Windowsの時だけ、[？]フラグを消す
  Qt::WindowFlags flags = dialog->windowFlags();
  flags &= ~Qt::WindowContextHelpButtonHint;
  dialog->setWindowFlags( flags );
#else
  N_UNUSED(dialog)
#endif
}

///////////////////////////////////////////////////////////////////////////
// ダイアログの初期位置設定 (コンストラクタで呼んではいけない)
///////////////////////////////////////////////////////////////////////////
void InitDialogPos( QDialog* dialog, QWidget* mainWindow )
{
  // メインウィンドウ基準で中央に
  QRect scr = mainWindow->geometry();
  dialog->move( scr.center() - dialog->rect().center() );
}

cursor_t BeginWaitCursor()
{
  QApplication::setOverrideCursor( Qt::WaitCursor );
  return (cursor_t)0;
}

void EndWaitCursor( cursor_t old )
{
  N_UNUSED(old);
  QApplication::restoreOverrideCursor();
}

}///<qt_pal
