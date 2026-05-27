/* -*- coding: utf-8 -*- マルチバイト */

// Copyright 2026 PGN Inc. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#ifndef WIDGET_DIRECTION_H
#define WIDGET_DIRECTION_H

#include <QWidget>
#include <functional>

/////////////////////////////////////////////////////////////////////
class WidgetDirection : public QWidget
{
    Q_OBJECT
public:
  explicit WidgetDirection( QWidget *parent = 0 );
  QSize sizeHint() const;

  double mDir;
  std::function<void(double)> mUpdateListener;
  void setDir( double newVal );

private:
  bool mMouseDown;

  void updateDir( QMouseEvent* event );

public slots:
  void paintEvent( QPaintEvent* event );

  void mousePressEvent( QMouseEvent* event );
  void mouseMoveEvent( QMouseEvent* event );
  void mouseReleaseEvent( QMouseEvent* event );
};

#endif // WIDGET_DIRECTION_H
