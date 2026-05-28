/* -*- coding: utf-8 -*- マルチバイト */

// Copyright 2026 PGN Inc. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#include "widget_preview.h"

#include <QVBoxLayout>
#include <qt_pal/studio_pal.hpp>
#include <QPainter>
#include <QFileInfo>
#include "trans_str_mfg.h"

#include "man.h"


/*
  WidgetPreview
*/
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
WidgetPreview::WidgetPreview(QWidget *parent, const std::vector<QString>& mdzPaths) : QWidget(parent)
{
  mTargetPaths = mdzPaths;

  setContentsMargins( 0,0,0,0 );

  mTargetComboBox = new QComboBox();
  for (auto& path : mTargetPaths )
  {
    mTargetComboBox->addItem( QFileInfo( path ).fileName() );
  }
  mResetButton = new QPushButton( UIIcon(":/images/icon_reset"), MStr(25) );
  mTargetView = new WidgetTargetView();

  LoadTarget( mTargetPaths[0] );  

  QVBoxLayout* lo = new QVBoxLayout();
  lo->addWidget( mTargetComboBox, 1 );
  lo->addWidget( mResetButton, 1 );
  lo->addWidget( mTargetView, 100 );
  lo->setContentsMargins( 0, 0, 0, 0 );
  lo->setSpacing( 0 );
  setLayout( lo );

  connect( mTargetComboBox, &QComboBox::currentIndexChanged, this, [this]( int index ) {
    LoadTarget( mTargetPaths[index] );
  });
  connect( mResetButton, SIGNAL(clicked(bool)), this, SLOT(resetCanvas()) );

  setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Minimum );
}

///////////////////////////////////////////////////////////////////////////
WidgetPreview::~WidgetPreview()
{
}

void WidgetPreview::LoadTarget( const QString& mdzPath )
{
  mCurrentPath = mdzPath;
  resetCanvas();
}



void WidgetPreview::resetCanvas()
{
  mTargetView->LoadTarget( mCurrentPath );
}

QString WidgetPreview::MStr( int idx )
{
  return Trans_StrMFG( idx );
}

QImage WidgetPreview::AsThumbnail()
{
  const int THUMB_SIZE = 512;

  auto& srcPal = mTargetView->GetPalImage();

  QImage original( srcPal.Width(), srcPal.Height(), QImage::Format_ARGB32 );
  studio_pal::Image32ToQImage( srcPal, original );

  QImage qthumb( THUMB_SIZE, THUMB_SIZE, QImage::Format_ARGB32 );

  // originalからqthumbにストレッチ
  QPainter painter( &qthumb );
  // スムージングする（バイリニアとか）
  painter.setRenderHint(QPainter::SmoothPixmapTransform);

  painter.drawImage( qthumb.rect(), original, original.rect() );

  return qthumb;
}

qt_pal::MFGTarget WidgetPreview::GetTarget( std::function<void(const std::string& log)> logger )
{
  return qt_pal::MFGTarget( *mTargetView, std::move(logger) );
}

