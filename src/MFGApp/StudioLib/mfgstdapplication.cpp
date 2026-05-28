/* -*- coding: utf-8 -*- マルチバイト */

// Copyright 2026 PGN Inc. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#include "mfgstdapplication.h"

#include <QtGui>

#include "mainwindow.h"

#if defined(__APPLE__)
// Macの場合の関連付けパラメータ
QString gMacOpenParam;
QString GetMacOpenParam()
{
  return gMacOpenParam;
}
#endif

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
MFGStdApplication::MFGStdApplication( int &argv, char **args )
    : QApplication( argv, args )
{
  mMain = NULL;
}

///////////////////////////////////////////////////////////////////////////
bool MFGStdApplication::event(QEvent *event)
{

#if defined(__APPLE__)
  /////////////////////////
  // 関連付け対応 (Macのみ)
  /////////////////////////
  if (event->type() == QEvent::FileOpen)
  {
    QFileOpenEvent* ev = static_cast<QFileOpenEvent*>(event);
    if (mMain != NULL)
    {
      // 起動済みなら、開く
      mMain->openScriptFile( ev->file() );
    }
    else
    {
      // 起動済みでないなら、保存しておく (後で開く)
      gMacOpenParam = ev->file();
    }
    return true;
  }
#endif

  return QApplication::event(event);
}
