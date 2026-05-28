/* -*- coding: utf-8 -*- マルチバイト */

// Copyright 2026 PGN Inc. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#include <mfg_pal/pal.hpp>
#include <qt_pal/studio_pal.hpp>
#include <QImage>

namespace mfg_pal {


bool DecodePng32( const std::vector<uint8_t>& buf, mfg_pal::Image32& dest )
{
  QImage qimg = QImage::fromData(buf.data(), static_cast<int>(buf.size()));

  // 読み込み失敗（データが壊れているか、pngじゃないパス）
  if (qimg.isNull())
    return false;

  if(!dest.Resize( qimg.width(), qimg.height() ))
    return false;

  if(!studio_pal::QImageToImage32( qimg, dest ))
    return false;

  return true;
}


}///< mfg_pal

