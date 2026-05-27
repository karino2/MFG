/* -*- coding: utf-8 -*- マルチバイト */

// Copyright 2026 PGN Inc. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#include "dialog_about.h"

#include <QVBoxLayout>
#include <QTableWidget>
#include <QScrollArea>
#include <QPainter>
#include <QLabel>
#include <QListWidget>
#include <QDialogButtonBox>

#include "widget_resource.h"
#include <qt_pal/qt_pal.h>

using namespace qt_pal;

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
DialogAbout::DialogAbout(QWidget *parent) :
    QDialog(parent)
{
  // ？を表示しない
  RemoveQuestion( this );

  // 可能な限り小さく
  setSizePolicy( QSizePolicy::Maximum, QSizePolicy::Maximum );

  // バージョン表記
  QString cap = "ver ";
  cap = cap + STUDIO_APP_VERSION;
  setWindowTitle( cap );

  // 中身
  QVBoxLayout* lo = new QVBoxLayout();
  lo->setContentsMargins( 2, 2, 2, 2 );

  // Image
  // アイコン
  WidgetResource* a = new WidgetResource( ":/images/about_mfg.png" );
  lo->addWidget( a, 0, Qt::AlignCenter );

  // Library
  QLabel* label = new QLabel( "We use the following library" );
  label->setContentsMargins( 2,2,2,2 );
  lo->addWidget( label );

  QListWidget* list = new QListWidget();
  list->addItem( "Qt - https://www.qt.io/jp" );
  list->addItem( "MFG - https://github.com/karino2/MFG" );
  list->addItem( "PicoJSON - https://github.com/kazuho/picojson" );
  list->addItem( "minizip-ng - https://github.com/zlib-ng/minizip-ng" );

  lo->addWidget( list );
  list->setFocusPolicy( Qt::NoFocus );

  // OkCancel
  QDialogButtonBox* but = new QDialogButtonBox( QDialogButtonBox::Ok );
  lo->addWidget( but );

  lo->addStretch();

  lo->setContentsMargins( 2, 2, 2, 2 );
  setLayout( lo );

  // Connect
  connect( but, SIGNAL(accepted()), this, SLOT(accept()) );
  connect( but, SIGNAL(rejected()), this, SLOT(reject()) );
}

///////////////////////////////////////////////////////////////////////////
QSize DialogAbout::sizeHint() const
{
  // Library が全部見える感じで
  return QSize( 1, qt_pal::MagInt( 600 ) );
}

