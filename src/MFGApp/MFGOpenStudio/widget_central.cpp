// Copyright 2026 PGN Inc. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

/* -*- coding: utf-8 -*- */

#include "widget_central.h"

#include <QVBoxLayout>
#include <QUrl>
#include <QMimeData>
#include <QDropEvent>

#include "mainwindow.h"

WidgetProcessing* GetWidgetProcessing()
{
  return Main()->Central()->_widgetProcessing;
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
WidgetCentral::WidgetCentral(QWidget *parent) : QWidget(parent)
{
  // MDI (コンストラクタでaddSubWindowしちゃダメ)
  _mdiArea = new QMdiArea( this );
  _mdiArea->setViewMode( QMdiArea::TabbedView );

  // プログレス
  _widgetProcessing = new WidgetProcessing();
  _widgetProcessing->setVisible( false );

  QVBoxLayout* lo = new QVBoxLayout();
  lo->addWidget( _mdiArea, 100 );
  lo->addWidget( _widgetProcessing, 1 );
  lo->setContentsMargins( 0, 0, 0, 0 );

  setLayout( lo );

  // Drag and Drop有効
  setAcceptDrops( true );
}

///////////////////////////////////////////////////////////////////////////
WidgetCentral::~WidgetCentral()
{
}

///////////////////////////////////////////////////////////////////////////
bool WidgetCentral::Processing()
{
  return _widgetProcessing->mProcessing;
}

///////////////////////////////////////////////////////////////////////////
void WidgetCentral::dropEvent( QDropEvent* event )
{
  const QMimeData* mime = event->mimeData();

  // ファイルを開く Drag & Drop
  if (mime->hasUrls())
  {
    const QList<QUrl> urls = mime->urls();
    for (int i=0; i<urls.size(); i++)
    {
      // パス取得
      const QUrl u = urls.at( i );
      const QString path = u.path();
      const QString localFile = u.toLocalFile();

      // 開く
      Main()->openScriptFile( localFile );
    }
  }
}

///////////////////////////////////////////////////////////////////////////
void WidgetCentral::dragEnterEvent( QDragEnterEvent *event )
{
  event->accept();
}
