/* -*- coding: utf-8 -*- マルチバイト */

// Copyright 2026 PGN Inc. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#ifndef WIDGET_TARGET_VIEW_H
#define WIDGET_TARGET_VIEW_H

#include <QWidget>
#include <QImage>
#include <vector>
#include <mfg_pal/pal.hpp>

class WidgetTargetView: public QWidget
{
  Q_OBJECT

public:
  explicit WidgetTargetView( QWidget *parent = 0 );

  void LoadTarget( const QString& pngPath );
  void UpdateOffscreen();

  mfg_pal::Image32& GetPalImage() { return _targetImage; }

  mfg_pal::Image32 _targetImage;
  QImage _qImage;

protected:
  void paintEvent( QPaintEvent* event ) override;
  void resizeEvent( QResizeEvent* event ) override;
};

#endif // WIDGET_TARGET_VIEW_H
