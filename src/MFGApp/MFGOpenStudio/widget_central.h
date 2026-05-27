// Copyright 2026 PGN Inc. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

/* -*- coding: utf-8 -*- */

#ifndef WIDGET_CENTRAL_H
#define WIDGET_CENTRAL_H

#include <QWidget>
#include <QMdiArea>
#include <QMdiSubWindow>

#include "widget_processing.h"

class WidgetCentral : public QWidget
{
  Q_OBJECT

private:
  QMdiArea* _mdiArea; // MDIエリア

public:
  explicit WidgetCentral(QWidget *parent = 0);
  virtual ~WidgetCentral();

  WidgetProcessing* _widgetProcessing; // プログレス

  QMdiArea* MdiArea(){ return _mdiArea; }

  // 重い処理の進捗中？
  bool Processing();

signals:
public slots:
  void dropEvent( QDropEvent* event );
  void dragEnterEvent( QDragEnterEvent *event );
private:
};

#endif // WIDGET_CENTRAL_H
