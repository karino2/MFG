/* -*- coding: utf-8 -*- マルチバイト */

// Copyright 2026 PGN Inc. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>


#include <vector>
#include <string>

class WidgetCanvas;
class WidgetCentral;
class WidgetMFGEditor;

// MainWindowのインターフェース。
struct IMainWindow
{
  // インターフェース
  virtual void AddOutput( QString s ) = 0;
  virtual std::vector<WidgetMFGEditor*> EditorChildren() = 0;
  virtual WidgetCentral* Central() = 0;
  virtual void onFileOpen() = 0;
  virtual void openScriptFile( QString filePath ) = 0;
  virtual QString getSaveFolder() = 0;
  virtual QStatusBar* GetStatusBar() const = 0;
};

IMainWindow* Main();

///////////////////////////////////////////////////////////////////////////
// devicePixelRatioF 管理
///////////////////////////////////////////////////////////////////////////
template <class T>
double DprF( const T* widget )
{
  const double dprf = widget->devicePixelRatioF();
  return dprf;
}


#endif // MAINWINDOW_H
