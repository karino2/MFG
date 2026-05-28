/* -*- coding: utf-8 -*- マルチバイト */

// Copyright 2026 PGN Inc. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#include "widget_output.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include "trans_str_mfg.h"
#include "man.h"

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
WidgetOutput::WidgetOutput(QWidget *parent) : QWidget(parent)
{
  QHBoxLayout* lh = new QHBoxLayout();

  mClearButton = new QToolButton();
  mClearButton->setObjectName( "ClearOutput" );
  mClearButton->setDefaultAction( CreateAction( ":/images/icon_trash", "", this ) );
  mClearButton->setToolTip( MStr(26) );
  lh->setContentsMargins( 0, 0, 0, 0 );
  lh->addWidget( mClearButton, 0, Qt::AlignRight );

  mTextEdit = new QTextEdit();
    
  QVBoxLayout* lo = new QVBoxLayout();
  lo->setContentsMargins( 0, 0, 0, 0 );
  lo->addLayout( lh );
  lo->addWidget( mTextEdit, 100 );

  setLayout( lo );
  connect( mClearButton, SIGNAL(clicked()), mTextEdit, SLOT(clear()) );
}

WidgetOutput::~WidgetOutput()
{
}

QString WidgetOutput::MStr( int idx )
{
  return Trans_StrMFG( idx );
}

/////////////////////////////////////////////////////////////////////////////
void WidgetOutput::append( const QString& msg )
{
  mTextEdit->append( msg );
}
