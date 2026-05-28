/* -*- coding: utf-8 -*- マルチバイト */

// Copyright 2026 PGN Inc. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#ifndef QT_PAL_H
#define QT_PAL_H

#ifndef N_UNUSED
#define N_UNUSED(x) (void)x;
#endif

class QDialog;
class QWidget;
#include <QString>
#include <mfg_pal/pal.hpp>
#include <QProgressBar>

namespace qt_pal {

inline bool DarkMode() { return false; }
void SetUIMag( double mag );
double UIMag();
int MagInt( int value );
double MagDouble( double value );

// デスクトップのパスを取得
QString GetQtDesktopPath();

// ダイアログの「？」を取り除く
void RemoveQuestion( QDialog* dialog );

// ダイアログの初期位置
void InitDialogPos( QDialog* dialog, QWidget* mainWindow );

using cursor_t = int; // qtでは不要
cursor_t BeginWaitCursor();
void EndWaitCursor( cursor_t old );

using mfg_pal::Tick;

using mfg_pal::CallbackInfo;

inline void ApplyCallbackInfo( QProgressBar& pbar, const CallbackInfo& info )
{
    pbar.setFormat( "%p%" );

    pbar.setMinimum( 0 );
    pbar.setMaximum( 100 );
    pbar.setValue( (int)info._percent );
}

}///< qt_pal

#endif // QT_PAL_H
