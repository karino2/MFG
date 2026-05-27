/* -*- coding: utf-8 -*- マルチバイト */

// Copyright 2026 PGN Inc. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#ifndef DIALOG_NEW_PROJECT_H
#define DIALOG_NEW_PROJECT_H

#include <QDialog>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <vector>
#include <string>

struct ExampleFilterEntry
{
  QString _fileName;
  QString _label;
  std::string _script;
  bool _isTemplate;
  ExampleFilterEntry( const QString& fileName, const QString& label, std::string script, bool isTemplate ) : _fileName( fileName ), _label( label ), _script( std::move(script) ), _isTemplate( isTemplate ) {}
};
bool EnsureExamplesProject( const QString& projDirPath );

/////////////////////////////////////////////////////////////////////
class DialogNewProject : public QDialog
{
  Q_OBJECT
public:
  // Dialog位置保存 (static)
  static int gPosX;
  static int gPosY;
  static bool gPosInit;

  explicit DialogNewProject( QWidget *parent, const QString& dir );
  QSize sizeHint() const;

  QString getDir() { return mDirEdit->text(); }
  QString getName() { return mNameEdit->text(); }
  const std::string& getSelectedTemplate();

private:
  QString Str( int idx );

  QLineEdit* mDirEdit;
  QPushButton* mBrowse;

  QLineEdit* mNameEdit;
  QComboBox* mTemplateCombo;
  std::vector<std::string> mTemplateContents;
  void AddTemplate( const QString& label, std::string script );

public slots:
  void onBrowse();
  void showEvent( QShowEvent* event );
  void hideEvent( QHideEvent* event );
};

#endif // DIALOG_NEW_PROJECT_H
