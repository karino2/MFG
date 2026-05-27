/* -*- coding: utf-8 -*- マルチバイト */

// Copyright 2026 PGN Inc. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#include "widget_code_editor.h"
#include <qt_pal/qt_pal.h>

#include <QPainter>

class LineNumberArea : public QWidget
{
public:
    LineNumberArea(WidgetCodeEditor *editor) : QWidget(editor) {
        codeEditor = editor;
    }

    QSize sizeHint() const Q_DECL_OVERRIDE {
        return QSize(codeEditor->lineNumberAreaWidth(), 0);
    }

protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE {
        codeEditor->lineNumberAreaPaintEvent(event);
    }

private:
    WidgetCodeEditor *codeEditor;
};

/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
WidgetCodeEditor::WidgetCodeEditor(QWidget *parent) : QPlainTextEdit(parent)
{
  lineNumberArea = new LineNumberArea(this);

  connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateLineNumberAreaWidth(int)));
  connect(this, SIGNAL(updateRequest(QRect,int)), this, SLOT(updateLineNumberArea(QRect,int)));
  connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(highlightCurrentLine()));
  connect(document(), &QTextDocument::contentsChanged, this, &WidgetCodeEditor::onDocumentModified );

  updateLineNumberAreaWidth(0);
  highlightCurrentLine();
}

/////////////////////////////////////////////////////////////////////
int WidgetCodeEditor::lineNumberAreaWidth()
{
  int digits = 1;
  int max = qMax(1, blockCount());
  while (max >= 10) {
    max /= 10;
    ++digits;
  }

#if QT_VERSION < QT_VERSION_CHECK(5, 11, 0)
  int space = 3 + fontMetrics().width(QLatin1Char('9')) * digits;
#else
  int space = 3 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;
#endif

  return space;
}

/////////////////////////////////////////////////////////////////////
void WidgetCodeEditor::updateLineNumberAreaWidth(int /* newBlockCount */)
{
  setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

/////////////////////////////////////////////////////////////////////
void WidgetCodeEditor::updateLineNumberArea( const QRect &rect, int dy )
{
  if (dy)
  {
    lineNumberArea->scroll(0, dy);
  }
  else
  {
    lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());
  }

  if (rect.contains(viewport()->rect())) updateLineNumberAreaWidth(0);
}

/////////////////////////////////////////////////////////////////////
void WidgetCodeEditor::resizeEvent( QResizeEvent *e )
{
  QPlainTextEdit::resizeEvent( e );

  QRect cr = contentsRect();
  lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

/////////////////////////////////////////////////////////////////////
void WidgetCodeEditor::highlightCurrentLine()
{
  QList<QTextEdit::ExtraSelection> extraSelections;

  if (!isReadOnly())
  {
    QTextEdit::ExtraSelection selection;
    QColor lineColor = QColor( 240, 240, 240, 255 );
    if (qt_pal::DarkMode())
    {
      lineColor = QColor( 80, 80, 80, 255 );
    }

    selection.format.setBackground(lineColor);
    selection.format.setProperty(QTextFormat::FullWidthSelection, true);
    selection.cursor = textCursor();
    selection.cursor.clearSelection();
    extraSelections.append(selection);
  }

  setExtraSelections(extraSelections);
}

/////////////////////////////////////////////////////////////////////
void WidgetCodeEditor::lineNumberAreaPaintEvent(QPaintEvent *event)
{
  QPainter painter(lineNumberArea);
  painter.fillRect(event->rect(), Qt::lightGray);

  QTextBlock block = firstVisibleBlock();
  int blockNumber = block.blockNumber();
  int top = (int) blockBoundingGeometry(block).translated(contentOffset()).top();
  int bottom = top + (int) blockBoundingRect(block).height();

  while (block.isValid() && top <= event->rect().bottom())
  {
    if (block.isVisible() && bottom >= event->rect().top())
    {
      QString number = QString::number(blockNumber + 1);
      painter.setPen(Qt::black);
      painter.drawText( 0, top, lineNumberArea->width(), fontMetrics().height(), Qt::AlignRight, number );
    }

    block = block.next();
    top = bottom;
    bottom = top + (int) blockBoundingRect(block).height();
    ++blockNumber;
  }
}

void WidgetCodeEditor::onDocumentModified()
{
  emit documentWasModified();
}