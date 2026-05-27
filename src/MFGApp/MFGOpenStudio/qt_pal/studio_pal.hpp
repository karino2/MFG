/* -*- coding: utf-8 -*- マルチバイト */

// Copyright 2026 PGN Inc. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#ifndef QT_STUDIO_PAL_H
#define QT_STUDIO_PAL_H


/*
  MFGStudioとMFGOpenStudioの違いを吸収するPAL
*/

#include <mfg_pal/pal.hpp>
#include <qt_pal/qt_pal.h>
#include <QString>
#include <QImage>
#include "widget_processing.h"

namespace studio_pal {

bool QImageToImage32( QImage& src, mfg_pal::Image32& dest );
bool Image32ToQImage( mfg_pal::Image32& src, QImage& dest );

// widget_processingがポータブルなのでそのまま呼んでも良いのだが、その辺の事情がnamespaceからわかるようにここに定義しておく。
inline bool ProgressImpl( qt_pal::CallbackInfo info ) { return QtProgress( info ); }
inline void CancelProgress() { ::CancelProgress(); }
inline void BeforeProgressCancelableNoDialog() { ::BeforeQtProgressCancelableNoDialog(); }
inline void BeforeProgressCancelable() { ::BeforeQtProgressCancelable(); }
inline void AfterProgress(){ ::AfterQtProgress(); }

}///< studio_pal

#endif // QT_STUDIO_PAL_H
