/* -*- coding: utf-8 -*- マルチバイト */

// Copyright 2026 PGN Inc. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#ifndef DIALOG_ABOUT_H
#define DIALOG_ABOUT_H

#include <QDialog>

///////////////////////////////////////////
class DialogAbout : public QDialog
{
  Q_OBJECT

public:
  explicit DialogAbout(QWidget *parent = 0);
  QSize sizeHint() const;
};

#endif // DIALOG_ABOUT_H
