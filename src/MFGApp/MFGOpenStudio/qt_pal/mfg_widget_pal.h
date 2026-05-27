/* -*- coding: utf-8 -*- マルチバイト */

// Copyright 2026 PGN Inc. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#ifndef MFG_WIDGET_PAL_H
#define MFG_WIDGET_PAL_H

#include <QLabel>

#include "mfg.hpp"
#include "widget_center_pointer.h"
#include "widget_direction.h"

#include <mfg_pal/pal.hpp>

class WidgetSimpleColorPicker;

namespace mfg_widget_pal {

class WidgetColorPicker : public QWidget
{
  Q_OBJECT

public:
  explicit WidgetColorPicker( QWidget *parent = 0 );
  ~WidgetColorPicker(){}
  mfg_pal::TBpp32 Color();
  void SetColor( mfg_pal::TBpp32 color );
  void SetLabel( const QString& label ) { mLabel->setText( label ); }

signals:
  void colorChanged( mfg_pal::TBpp32 color );

private:
  QLabel* mLabel;
  WidgetSimpleColorPicker* mColorPicker;
};


using ::WidgetDirection;
using ::WidgetCenterPointer;

class LinearMapper
{
private:
  // 位置と値のペア (変換用)
  std::vector<double> _pairPos;
  std::vector<double> _pairValue;

public:
  LinearMapper() = default;

  // ペアを管理 (posは正規化された値を指定)
  void Clear()
  {
    _pairPos.clear();
    _pairValue.clear();
  }
  void Add( double pos, double value )
  {
    _pairPos.push_back( pos );
    _pairValue.push_back( value );
  }

  // 値 (value) からUI位置を取得 (例えば、w は 0..w の値を取るスライダーの最大値)
  double GetPos( double value, int w ) const;

  // UI位置 (pos) からマッピングされた値を取得 (例えば、pos,w はスライダー位置と最大値)
  double GetValue( double pos, int w ) const;
};


void SetupLinearMapper( LinearMapper& mapper, const mfg::ParamInfo& info );


}///< mfg_widget_pal

#endif // MFG_WIDGET_PAL_H
