/* -*- coding: utf-8 -*- マルチバイト */

// Copyright 2026 PGN Inc. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#ifndef WIDGET_PROJECT_EXPLORER_H
#define WIDGET_PROJECT_EXPLORER_H

#include <QFileSystemModel>
#include <QTreeView>
#include <QToolBar>
#include <QAction>

/////////////////////////////////////////////////////////////////////////////
class WidgetProjectExplorer : public QWidget
{
  Q_OBJECT

public:
  explicit WidgetProjectExplorer(QWidget *parent = 0);
  ~WidgetProjectExplorer();
  QSize sizeHint() const;

  void openDir( const QDir& dir );
  void closeDir();

public slots:
  void onSelectionChanged( const QItemSelection &selected, const QItemSelection &deselected );

signals:
  void fileSelected( const QFileInfo& fi );
  void createStringsJson( const QString& jsonPath );
  void importThumbnail();
  void importImage();

private:
  QDir mDir;
  QToolBar mBar;
  QTreeView mTree;
  QFileSystemModel mFModel;

  QAction* mActImportImage;
  QAction* mActRemove;
  QAction* mActNewStrings;
  QAction* mActImportThumb;
};

#endif // WIDGET_REFERENCE_H
