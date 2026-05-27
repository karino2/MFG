/* -*- coding: utf-8 -*- マルチバイト */

// Copyright 2026 PGN Inc. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#ifndef WIDGET_CENTER_POINTER_H
#define WIDGET_CENTER_POINTER_H

#include <QWidget>


#include <functional>

/////////////////////////////////////////////////////////////////////////////
class WidgetCenterPointer : public QWidget
{
  Q_OBJECT

public:
  explicit WidgetCenterPointer(QWidget *parent = 0);
  QSize sizeHint() const;

  static double mMx;
  static double mMy;
  bool mMouseDown;

  std::function<void()> mUpdateListener;

private:
  void updateMousePos( QMouseEvent* event );

private slots:
  void paintEvent( QPaintEvent *event );

  void mousePressEvent( QMouseEvent* event );
  void mouseReleaseEvent( QMouseEvent* event );
  void mouseMoveEvent( QMouseEvent* event );
};

#endif // WIDGET_LINE_CONCENT_CENTER_H
