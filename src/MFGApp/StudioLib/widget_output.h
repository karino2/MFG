/* -*- coding: utf-8 -*- マルチバイト */

// Copyright 2026 PGN Inc. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#ifndef WIDGET_OUTPUT_H
#define WIDGET_OUTPUT_H

#include <QString>
#include <QTextEdit>
#include <QToolButton>


/////////////////////////////////////////////////////////////////////////////
class WidgetOutput : public QWidget
{
  Q_OBJECT

public:
  explicit WidgetOutput(QWidget *parent = 0);
  ~WidgetOutput();
  QString MStr( int idx );

  void append( const QString& msg );

private:
  QTextEdit* mTextEdit;
  QToolButton* mClearButton;

};

#endif // WIDGET_OUTPUT_H
