/* -*- coding: utf-8 -*- マルチバイト */

// Copyright 2026 PGN Inc. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#ifndef WIDGET_RESOURCE_H
#define WIDGET_RESOURCE_H

#include <QWidget>
#include <QImage>
#include <mfg_pal/pal.hpp>

/////////////////////////////////////////////////////////////////////
// リソース画像を元にしたウィジェット
/////////////////////////////////////////////////////////////////////
class WidgetResource : public QWidget
{
    Q_OBJECT
public:
  explicit WidgetResource( QString resourceStr, QWidget *parent = 0 );
  QSize sizeHint() const;

private:
  QImage mImg;
  QSize mSize;

signals:
public slots:
  void paintEvent( QPaintEvent* event );
};

/////////////////////////////////////////////////////////////////////
// mfg_pal::Image32画像を元にしたウィジェット
/////////////////////////////////////////////////////////////////////
class WidgetImage32 : public QWidget
{
    Q_OBJECT
public:
  explicit WidgetImage32( mfg_pal::Image32* src, QWidget *parent = 0 );
  QSize sizeHint() const;

  void updateImage( mfg_pal::Image32* src );

private:
  QImage mImg;
  QSize mSize;

public slots:
  void paintEvent( QPaintEvent* event );
  void resizeEvent( QResizeEvent* event );
};

#endif // WIDGET_RESOURCE_H
