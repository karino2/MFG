/* -*- coding: utf-8 -*- マルチバイト */

// Copyright 2026 PGN Inc. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#ifndef WIDGET_SIMPLE_COLOR_PICKER_H
#define WIDGET_SIMPLE_COLOR_PICKER_H

#include <QWidget>

#include <mfg_pal/pal.hpp>

/*
  オープンソース用に単なるColorDialogを呼び出すだけのカラーピッカーを作る。
  色を選びつつプレビューを確認する事が出来ないが、まずはシンプルな実装でオープンソース化を目指す。
*/

class QPaintEvent;
class QMouseEvent;

class WidgetSimpleColorPicker : public QWidget
{
  Q_OBJECT
public:
  explicit WidgetSimpleColorPicker( QWidget *parent = 0);
  QSize sizeHint() const;

  mfg_pal::TBpp32 _color;

  mfg_pal::TBpp32 Color() { return _color; }
  void SetColor( mfg_pal::TBpp32 color )
  { 
    _color = color;
    repaint();
  }

private slots:
    void paintEvent( QPaintEvent *event );
    void mousePressEvent( QMouseEvent *event );

signals:
  void colorChanged( mfg_pal::TBpp32 color );

};

#endif // WIDGET_SIMPLE_COLOR_PICKER_H
