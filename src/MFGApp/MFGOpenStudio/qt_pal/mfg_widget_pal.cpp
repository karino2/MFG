/* -*- coding: utf-8 -*- マルチバイト */

// Copyright 2026 PGN Inc. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#include "mfg_widget_pal.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <vector>

#include <qt_pal/qt_pal.h>
#include "widget_simple_color_picker.h"


using namespace qt_pal;

namespace mfg_widget_pal {

WidgetColorPicker::WidgetColorPicker( QWidget *parent ) : QWidget( parent )
{
  mColorPicker = new WidgetSimpleColorPicker();

  QHBoxLayout* lh = new QHBoxLayout();
  lh->addWidget( mColorPicker, 0, Qt::AlignTop );
  lh->addStretch();

  QVBoxLayout* lo = new QVBoxLayout();
  mLabel = new QLabel("");
  mLabel->setAlignment(Qt::AlignCenter);
  QFont font;
  font.setBold( true );
  mLabel->setFont( font );

  lo->addWidget( mLabel, 0, Qt::AlignLeft | Qt::AlignTop );
  lo->addLayout( lh );
  lo->addStretch();

  setLayout( lo );
  lo->setContentsMargins( 2, 2, 2, 2 );

  connect( mColorPicker, &WidgetSimpleColorPicker::colorChanged, this, [this]( mfg_pal::TBpp32 color ){
    emit colorChanged( color );
  } );


  // 小さくなりすぎないよう
  setMinimumHeight( qt_pal::MagInt( 90 ) );
  setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Preferred );
}

mfg_pal::TBpp32 WidgetColorPicker::Color()
{ 
  return mColorPicker->Color();
}

void WidgetColorPicker::SetColor( mfg_pal::TBpp32 pcolor )
{
  mColorPicker->SetColor( pcolor );
  emit colorChanged( pcolor );
}


///////////////////////////////////////////////////////////////////////////
double LinearMapper::GetPos( double value, int w ) const
{
  // 値を、画面上の位置に
  if (!_pairPos.empty())
  {
    ////////////////////////
    // 上限より大きいなら、その値を
    ////////////////////////
    const size_t s = _pairValue.size();
    const double max = _pairValue[s-1];
    if (value >= max)
    {
      return _pairPos[s-1] * w;
    }

    ////////////////////////
    // 下限より小さいなら、その値を
    ////////////////////////
    if (value < _pairValue[0])
    {
      return _pairPos[0] * w;
    }

    ////////////////////////
    // 下回るレベルを調べる
    ////////////////////////
    const int s2 = (int)s - 2;
    for (int i=s2; i>=0; i--)
    {
      const double v1 = _pairValue[i+1];
      const double v0 = _pairValue[i];

      if (value >= v0)
      {
        const double p = (value - v0) / (v1 - v0);
        const double pos = _pairPos[i] + p * (_pairPos[i+1] - _pairPos[i]);

        // 正規化されてるので、幅考慮
        return pos * w;
      }
    }
  }

  return value;
}

///////////////////////////////////////////////////////////////////////////
double LinearMapper::GetValue( double pos, int w ) const
{
  // 画面上の位置を、値に
  if (!_pairPos.empty())
  {
    // 正規化する
    const double n = pos / w;
    const size_t s = _pairPos.size();

    // 上限より大きいなら、その値を
    const double max = _pairPos[s-1];
    if (n >= max) return _pairValue[s-1];

    // 下限より小さいなら、その値を
    if (n < _pairPos[0]) return _pairValue[0];

    // 下回るレベルを調べる
    const int s2 = (int)s - 2;
    for (int i=s2; i>=0; i--)
    {
      const double p1 = _pairPos[i+1];
      const double p0 = _pairPos[i];

      if (n >= p0)
      {
        const double v = (n - p0) / (p1 - p0);
        const double value = _pairValue[i] + v * (_pairValue[i+1] - _pairValue[i]);
        return value;
      }
    }
  }

  return pos;
}


void SetupLinearMapper( LinearMapper& mapper, const mfg::ParamInfo& info )
{
  double dmin = (double)info.GetMin().Value<float>();
  double dmax = (double)info.GetMax().Value<float>();
  double interval = dmax - dmin;

  mapper.Add( 0.0, dmin );

  if (interval > 100.0)
  {
    // 大きい値の時は小さい方を広くする感じに振る舞う
    mapper.Add( 0.4, dmin + interval/30 );
    mapper.Add( 0.60, dmin + interval/6 );
    mapper.Add( 0.85, dmin + interval/3 );
  }
  else
  {
    // 角度など小さい範囲では線形で
    mapper.Add( 0.3, dmin + 0.3*interval );
    mapper.Add( 0.6, dmin + 0.6*interval );
    mapper.Add( 0.9, dmin + 0.9*interval );
  }
  mapper.Add( 1.0, dmax );
}

}///< mfg_widget_pal