/* -*- coding: utf-8 -*- マルチバイト */

// Copyright 2026 PGN Inc. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#include "widget_target_view.h"

#include <QPainter>
#include <QVBoxLayout>
#include "man.h"
#include <qt_pal/studio_pal.hpp>


static bool LoadPng( const QString& path, mfg_pal::Image32& dest )
{
  QImage img;
  if (img.load( path ))
  {
    return studio_pal::QImageToImage32( img, dest );
  }
  return false;
}

/*
  WidgetTargetView
*/

WidgetTargetView::WidgetTargetView( QWidget* parent ) : QWidget( parent )
{
  // setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
  // setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
  setSizePolicy( QSizePolicy::Ignored, QSizePolicy::Ignored );
}

void WidgetTargetView::paintEvent( QPaintEvent* event )
{
  Q_UNUSED(event)
  QPainter painter( this );

  // Widgetの全領域を灰色で塗りつぶす
  painter.fillRect(rect(), QColor(200, 200, 200)); 

  if (_qImage.isNull())
    return;

  qreal dpr = devicePixelRatioF();
  int x = (width() - _qImage.width()/dpr) / 2;
  int y = (height() - _qImage.height()/dpr) / 2;
  painter.drawImage( x, y, _qImage );
}

void WidgetTargetView::resizeEvent( QResizeEvent* event )
{
  QWidget::resizeEvent( event );
  UpdateOffscreen();
}

void WidgetTargetView::UpdateOffscreen()
{
  QImage tmp( _targetImage.Width(), _targetImage.Height(), QImage::Format_ARGB32 );
  studio_pal::Image32ToQImage( _targetImage, tmp );

  qreal dpr = devicePixelRatioF();
  _qImage = tmp.scaled(size() * dpr, 
                        Qt::KeepAspectRatio, 
                        Qt::SmoothTransformation);
  _qImage.setDevicePixelRatio( dpr );
}

void WidgetTargetView::LoadTarget( const QString& pngPath )
{
  LoadPng( pngPath, _targetImage );

  UpdateOffscreen();

  update();
}
