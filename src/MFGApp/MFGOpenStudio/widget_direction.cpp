/* -*- coding: utf-8 -*- マルチバイト */

// Copyright 2026 PGN Inc. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#include "widget_direction.h"
#include <qt_pal/qt_pal.h>
#include <QPainter>
#include <QPen>
#include <QPointF>
#include <QTransform>
#include <QMouseEvent>
#include <cmath> // atan2
#include <QtMath>


static double Atan3( double y, double x )
{
  // xがゼロでもOK
  if ((y == 0) && (x == 0)) return 0;
  if (x == 0)
  {
    if (y > 0) return M_PI/2.0;
    if (y < 0) return -M_PI/2.0;
  }
  return std::atan2( y, x );
};


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
WidgetDirection::WidgetDirection( QWidget *parent) : QWidget(parent)
{
  setMouseTracking( true );

  mMouseDown = false;
  mDir = 0;

  setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
}

///////////////////////////////////////////////////////////////////////////
void WidgetDirection::paintEvent( QPaintEvent* event )
{
  Q_UNUSED(event)
  QPainter painter( this );

  painter.fillRect( rect(), Qt::white );

  double mx = width()/2;
  double my = height()/2;

  painter.setPen( QPen( Qt::white, 0 ) );
  painter.setBrush( QBrush( QColor(0xFFC0C0C0) ) );  

  auto r = 0.45 * width();
  painter.drawEllipse( QPointF(mx, my), r, r );

  double w = 0.35 * width();
  double h = 0.05 * width();

  std::vector<QPointF> p;
  p.push_back( QPointF( 0, -h ) );
  p.push_back( QPointF( +w, 0 ) );
  p.push_back( QPointF( 0, +h ) );
  p.push_back( QPointF( -w, 0 ) );


  painter.save();
  painter.translate( mx, my );
  painter.rotate( qRadiansToDegrees( mDir ) );
  

  painter.setBrush( QBrush( QColor(0xFFFFFFFF) ) );
  painter.drawPolygon( p.data(), (int)p.size() );  

  painter.restore();
}

///////////////////////////////////////////////////////////////////////////
QSize WidgetDirection::sizeHint() const
{
  int w = (int)(qt_pal::UIMag() * 150);
  int h = (int)(qt_pal::UIMag() * 150);
  return QSize( w, h );
}

///////////////////////////////////////////////////////////////////////////
void WidgetDirection::updateDir( QMouseEvent* event )
{
  double mx = event->pos().x() - width()/2;
  double my = event->pos().y() - height()/2;
  mDir = Atan3( my, mx );
}

///////////////////////////////////////////////////////////////////////////
void WidgetDirection::mousePressEvent( QMouseEvent* event )
{
  Q_UNUSED(event)

  updateDir( event );
  repaint();
  mMouseDown = true;
}

///////////////////////////////////////////////////////////////////////////
void WidgetDirection::mouseMoveEvent( QMouseEvent* event )
{
  Q_UNUSED(event)

  if (mMouseDown)
  {
    updateDir( event );
    repaint();
  }
}

void WidgetDirection::setDir( double newVal )
{
  mDir = newVal;
  repaint();
}

void WidgetDirection::mouseReleaseEvent( QMouseEvent* event )
{
  Q_UNUSED(event)
  mMouseDown = false;

  mUpdateListener( mDir );
}
