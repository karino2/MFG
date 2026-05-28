/* -*- coding: utf-8 -*- マルチバイト */

// Copyright 2026 PGN Inc. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#ifndef WIDGET_PREVIEW_H
#define WIDGET_PREVIEW_H

#include <QWidget>
#include <QComboBox>
#include <QPushButton>
#include <QImage>
#include <vector>
#include <functional>

#include <qt_pal/mfg_target.h>
#include "widget_target_view.h"


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
class WidgetPreview : public QWidget
{
  Q_OBJECT

public:
  explicit WidgetPreview(QWidget *parent, const std::vector<QString>& pngPaths);
  ~WidgetPreview();

  QComboBox* mTargetComboBox;
  QPushButton* mResetButton;
  WidgetTargetView* mTargetView;

  std::vector<QString> mTargetPaths;
  QString mCurrentPath; // リロードのために現在表示しているmdzのパスは覚えておく。

  void LoadTarget( const QString& pngPath );

  QString MStr( int idx );
  QImage AsThumbnail();
  qt_pal::MFGTarget GetTarget( std::function<void(const std::string& log)> logger );
  void TargetUpdated() { mTargetView->update(); }

public slots:
  void resetCanvas();
};

#endif // WIDGET_PREVIEW_H
