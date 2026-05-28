/* -*- coding: utf-8 -*- マルチバイト */

// Copyright 2026 PGN Inc. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#include <mfg_pal/pal.hpp>
#include <qt_pal/studio_pal.hpp>

#include <QImage>

namespace studio_pal {
using mfg_pal::TBpp32;

bool QImageToImage32( QImage& src, mfg_pal::Image32& dest )
{
  bool b = dest.Resize( src.width(), src.height() );
  if (!b) return false;

  for (int j=0; j<src.height(); j++)
  {
    uint8_t* line = src.scanLine( j );

    // 32bppの場合
    if (src.depth() == 32)
    {
      for (int i=0; i<src.width(); i++)
      {
        TBpp32* col = (TBpp32*)(line + i*4);
        dest.PixelSet( i,j, *col );
      }
    }

    // 8bppの場合
    if (src.depth() == 8)
    {
      for (int i=0; i<src.width(); i++)
      {
        uint8_t col = *((uint8_t*)(line + i));

        TBpp32 col32;
        col32.A = 255;
        col32.R = col;
        col32.G = col;
        col32.B = col;
        dest.PixelSet( i, j, col32 );
      }
    }
  }
  return true;
}

bool Image32ToQImage( mfg_pal::Image32& src, QImage& dest )
{
  // 大きさは同じにしておいてね (アクセス違反回避)
  if (src.Width() != dest.width()) return false;
  if (src.Height() != dest.height()) return false;

  for (int j=0; j<src.Height(); j++)
  {
    const TBpp32* adr = src.PixelAddress( 0, j );
    for (int i=0; i<src.Width(); i++)
    {
      dest.setPixel( i,j, adr->Value );
      adr++;
    }
  }
  return true;
}

}///< studio_pal

