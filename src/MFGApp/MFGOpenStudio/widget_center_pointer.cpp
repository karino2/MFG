/* -*- coding: utf-8 -*- マルチバイト */

// Copyright 2026 PGN Inc. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#include "widget_center_pointer.h"
#include <qt_pal/qt_pal.h>
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QtMinMax>
#include <QMouseEvent>

// 中心設定部
double WidgetCenterPointer::mMx = 0.5;
double WidgetCenterPointer::mMy = 0.5;


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
WidgetCenterPointer::WidgetCenterPointer(QWidget *parent) : QWidget(parent)
{
  mMouseDown = false;

  setMouseTracking( true );
  setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
}

///////////////////////////////////////////////////////////////////////////
void WidgetCenterPointer::paintEvent( QPaintEvent *event )
{
  N_UNUSED( event )
  QPainter painter(this);

  painter.fillRect(rect(), Qt::white);
  painter.setRenderHint(QPainter::Antialiasing);

  // 十字
  int w = width();
  int h = height();
  painter.setPen( QPen( QColor(0xFFA0A0A0), 1 ) );
  painter.drawLine( w/2, 0, w/2, h );
  painter.drawLine( 0, h/2, w, h/2 );

  // 中心点
  int dx = (int)(mMx * w );
  int dy = (int)(mMy * h );

  painter.setPen( QPen( QColor(0xFF000000), 1 ) );
  auto r = qt_pal::MagDouble(5);
  painter.drawEllipse( QPointF(dx, dy), r, r );

  QColor highlight( 0xFF80C4F7 );
  auto r2 = qt_pal::MagDouble(3);
  painter.setPen( QPen( highlight, 1 ) );
  painter.drawEllipse( QPointF(dx, dy), r2, r2 );
}

///////////////////////////////////////////////////////////////////////////
QSize WidgetCenterPointer::sizeHint() const
{
  int s = qt_pal::MagInt( 150 );
  return QSize( s, s );
}

///////////////////////////////////////////////////////////////////////////
void WidgetCenterPointer::mousePressEvent( QMouseEvent* event )
{
  mMouseDown = true;
  updateMousePos( event );
  repaint();
}

///////////////////////////////////////////////////////////////////////////
void WidgetCenterPointer::mouseReleaseEvent( QMouseEvent* event )
{
  mMouseDown = false;
  updateMousePos( event );
  repaint();

  mUpdateListener();
}

///////////////////////////////////////////////////////////////////////////
void WidgetCenterPointer::mouseMoveEvent( QMouseEvent* event )
{
  if (mMouseDown)
  {
    updateMousePos( event );
    repaint();
  }
}

///////////////////////////////////////////////////////////////////////////
void WidgetCenterPointer::updateMousePos( QMouseEvent* event )
{
  // 0 .. 1
  mMx = (double)event->pos().x() / width();
  mMy = (double)event->pos().y() / height();

  mMx = qBound( 0.0, mMx, 1.0 );
  mMy = qBound( 0.0, mMy, 1.0 );
}
