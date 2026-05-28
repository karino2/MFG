/* -*- coding: utf-8 -*- マルチバイト */

// Copyright 2026 PGN Inc. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#include "mainwindow_impl.h"
#include "mfgstdapplication.h"
#include <qt_pal/qt_pal.h>

#include <QtWidgets>

#include <QScreen>


int main(int argc, char *argv[])
{
  MFGStdApplication a( argc, argv );

  ////////////////////////////////////////
  // 高dpi対応 (MFGStdApplication 作成後)
  ////////////////////////////////////////

  // 基本dpi
  int baseDpi = 96;
#if defined(__APPLE__)
  baseDpi = 72;
#endif

#if 1
  const int dpi = qApp->primaryScreen()->logicalDotsPerInchX(); // 常に基本dpi (96/72dpi) しか取得できないが、pixmapが高密度化しないので、下じゃなくてコッチを使わないとバグる
#else
  const int dpi = qApp->primaryScreen()->handle()->logicalDpi().first; // 192dpiとか正しい値を取得できるけど、Pixmapが高密度化しないのでダメ
#endif
  const double uiMag = (double)dpi / baseDpi;
  qt_pal::SetUIMag( uiMag );

  MainWindow w;
  a.setMain( &w );

  w.show();

  return a.exec();
}
