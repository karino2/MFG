/* -*- coding: utf-8 -*- マルチバイト */

// Copyright 2026 PGN Inc. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#include "widget_resource.h"

#include <QPainter>
#include <qt_pal/qt_pal.h>
#include <qt_pal/studio_pal.hpp>

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
WidgetResource::WidgetResource( QString resourceStr, QWidget *parent) : QWidget(parent)
{
  QPixmap pm( resourceStr );
  mImg = pm.toImage();

  mSize.setWidth( qt_pal::MagInt( mImg.width() ) );
  mSize.setHeight( qt_pal::MagInt( mImg.height() ) );

  setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
}

///////////////////////////////////////////////////////////////////////////
void WidgetResource::paintEvent( QPaintEvent* event )
{
  Q_UNUSED(event)

  QRect rd( 0, 0, width(), height() );
  QRect rs( 0, 0, mImg.width(), mImg.height() );

  QPainter painter( this );
  painter.setRenderHint( QPainter::Antialiasing, true );
  painter.drawImage( rd, mImg, rs );
}

///////////////////////////////////////////////////////////////////////////
QSize WidgetResource::sizeHint() const
{
  return mSize;
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
WidgetImage32::WidgetImage32( mfg_pal::Image32* src, QWidget *parent) : QWidget(parent)
{
  N_UNUSED( parent );

  setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
  updateImage( src );
}

///////////////////////////////////////////////////////////////////////////
QSize WidgetImage32::sizeHint() const
{
  return mSize;
}

///////////////////////////////////////////////////////////////////////////
void WidgetImage32::updateImage( mfg_pal::Image32* src )
{
  if (src == NULL) return;

  // 画像更新
  QImage tmp( src->Width(), src->Height(), QImage::Format_ARGB32 );
  
  studio_pal::Image32ToQImage( *src, tmp );
  mImg = tmp;

  // sizeHint更新
  mSize.setWidth( src->Width() );
  mSize.setHeight( src->Height() );

  // サイズ調整
  adjustSize();
}

///////////////////////////////////////////////////////////////////////////
void WidgetImage32::paintEvent( QPaintEvent* event )
{
  Q_UNUSED(event)

  QPainter painter( this );
  painter.setRenderHint( QPainter::Antialiasing, true );
  painter.drawImage( 0, 0, mImg );
}

///////////////////////////////////////////////////////////////////////////
void WidgetImage32::resizeEvent( QResizeEvent* event )
{
  update();
}
