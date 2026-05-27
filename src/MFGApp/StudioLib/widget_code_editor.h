/* -*- coding: utf-8 -*- マルチバイト */

// Copyright 2026 PGN Inc. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#ifndef WIDGET_CODE_EDITOR_H
#define WIDGET_CODE_EDITOR_H

#include <QDialog>
#include <QSyntaxHighlighter>
#include <QTextEdit>
#include <QPushButton>
#include <QAction>

#include <QSlider>
#include <QCheckBox>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPlainTextEdit>

/////////////////////////////////////////////////////////////////////
class LineNumberArea;

class WidgetCodeEditor : public QPlainTextEdit
{
  Q_OBJECT

public:
  WidgetCodeEditor(QWidget *parent = 0);

  void lineNumberAreaPaintEvent(QPaintEvent *event);
  int lineNumberAreaWidth();

  bool isModified() const { return document()->isModified(); }
  void clearModifiedFlag() { document()->setModified( false ); }

signals:
  void documentWasModified();

protected:
    void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;

private slots:
  void updateLineNumberAreaWidth(int newBlockCount);
  void highlightCurrentLine();
  void updateLineNumberArea(const QRect &, int);
  void onDocumentModified();

private:
  QWidget *lineNumberArea;
};


#endif // WIDGET_CODE_EDITOR_H
