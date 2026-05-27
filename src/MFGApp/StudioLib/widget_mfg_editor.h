/* -*- coding: utf-8 -*- マルチバイト */

// Copyright 2026 PGN Inc. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
#ifndef WIDGET_MFG_EDITOR_H
#define WIDGET_MFG_EDITOR_H

#include <QTextEdit>
#include <QLabel>
#include <QPushButton>
#include <QCheckBox>
#include <QComboBox>
#include <QSyntaxHighlighter>
#include <vector>
#include <functional>
#include "widget_code_editor.h"

/////////////////////////////////////////////////////////////////////
class HighlighterMFG : public QSyntaxHighlighter
{
    Q_OBJECT
public:
    HighlighterMFG( QTextDocument *parent = 0 );

    struct HighlightingRule
    {
        QRegularExpression pattern;
        QTextCharFormat format;
    };

protected:
    void highlightBlock( const QString &text ) Q_DECL_OVERRIDE;

private:
    QVector<HighlightingRule> highlightingRules;

    void setMFGFormat();
};

/////////////////////////////////////////////////////////////////////////////
class WidgetMFGCtrl : public QWidget
{
  Q_OBJECT

public:
  explicit WidgetMFGCtrl(QWidget *parent = 0);
  ~WidgetMFGCtrl();

  std::function<void()> mRunCallback;
  std::function<void(const QString&)> mSelectScriptCallback;

  bool IsDebugEnabled();

private:
  QComboBox* mScriptComboBox;
  QCheckBox* mCheckDebug;
  QPushButton* mRunButton;

  std::vector<QString> mScripts;

public slots:
  void onScriptSelect( int scriptIndex );
  void onRunScript( bool b );
};

/////////////////////////////////////////////////////////////////////////////
class WidgetMFGEditor : public QWidget
{
  Q_OBJECT

public:
  explicit WidgetMFGEditor(QWidget *parent, std::function<const QString&()> getSaveFolder);
  ~WidgetMFGEditor();
  QSize sizeHint() const;
  bool mCloseCheck = true;

  void SetFilePath( const QString& name ) { mFilePath = name; }
  void SetEdit( QString s );
  // ファイルの中身とパスをセット。
  void SetContentFile( QString content, const QString& fpath )
  {
    SetEdit( std::move(content) );
    SetFilePath( fpath );
    mEdit->clearModifiedFlag();
    mLastModified = false;
  }

  bool SaveTo( const QString& path );
  bool Save( const QString& folder );

  bool FileSaveAsDialog( const QString& saveFolder );
  void UpdateWindowTitle();

  const QString& GetFilePath() const { return mFilePath; }
  QString GetContent() const ;

  void ClearModifiedFlag();
  bool IsModified() const;
  void SetupDefault();

  WidgetCodeEditor* GetTextEdit() { return mEdit; }

  bool CanUndo() const { return mCanUndo; }
  bool CanRedo() const { return mCanRedo; }
  void Undo();
  void Redo();

  void SetFontSize( int size );

signals:
  void closed( WidgetMFGEditor* );

private:
  WidgetCodeEditor* mEdit;
  HighlighterMFG* mHL;

  QString Str( int idx );

  QString mFilePath;
  bool mLastModified = false;
  bool mCanUndo = false;
  bool mCanRedo = false;
  std::function<const QString&()> mGetSaveFolder;

private slots:
  void onDocumentModified();
  void closeEvent( QCloseEvent* event );
  void onUndoStateChanged( bool canUndo );
  void onRedoStateChanged( bool canRedo );
};

#endif // WIDGET_MFG_EDITOR_H
