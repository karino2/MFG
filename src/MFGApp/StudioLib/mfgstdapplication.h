/* -*- coding: utf-8 -*- マルチバイト */

// Copyright 2026 PGN Inc. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#ifndef MFGSTDAPPLICATION_H
#define MFGSTDAPPLICATION_H

#include <QApplication>

#if defined(__APPLE__)
// Macの場合の関連付けパラメータ
QString GetMacOpenParam();
#endif

/*
  MFGStudio用のQApplicationクラス。
  Mac用のドラッグ&ドロップ処理
*/
///////////////////////////////////////////////////////////////////////////
struct IMainWindow;
class MFGStdApplication
    : public QApplication
{
  Q_OBJECT
public:
  MFGStdApplication( int &argv, char **args );
  void setMain( IMainWindow* main ){ mMain = main; }
  bool event(QEvent *event);

private:
  IMainWindow* mMain;
};

#endif
