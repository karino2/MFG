/* -*- coding: utf-8 -*- マルチバイト */

// Copyright 2026 PGN Inc. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#include "widget_simple_color_picker.h"
#include <QPaintEvent>
#include <QPainter>
#include <qt_pal/qt_pal.h>
#include <QColorDialog>

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
WidgetSimpleColorPicker::WidgetSimpleColorPicker( QWidget *parent ) : QWidget(parent)
{
  _color.Value = 0xFFFFFFFF;
}

QSize WidgetSimpleColorPicker::sizeHint() const
{
  int s = qt_pal::MagInt( 60 );
  return QSize( s, s );
}

void WidgetSimpleColorPicker::paintEvent( QPaintEvent *event )
{
  QPainter painter( this );
  painter.setRenderHint(QPainter::Antialiasing);

  painter.fillRect(rect(), QColor(_color.Value) );

  // 枠線
  painter.setPen(QPen(Qt::gray, 1));
  painter.drawRect(rect().adjusted(0, 0, -1, -1));
}

void WidgetSimpleColorPicker::mousePressEvent( QMouseEvent *event )
{
  if (event->button() == Qt::LeftButton)
  {
    QColor newColor = QColorDialog::getColor(QColor(_color.Value), this, "Select Color");

    if (newColor.isValid())
    {
        _color.Value = newColor.rgba();
        repaint();
        emit colorChanged( _color );
    }
  }
}
