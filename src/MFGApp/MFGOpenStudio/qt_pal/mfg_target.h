/* -*- coding: utf-8 -*- マルチバイト */

// Copyright 2026 PGN Inc. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#ifndef MFG_OPEN_TARGET_H
#define MFG_OPEN_TARGET_H

#include <QString>
#include <string>
#include <functional>
#include <memory>

#include <mfg_pal/pal.hpp>
#include "mfg.hpp"

#if defined(MFG_BACKEND_METAL)
#include "mfg_runtime_metal.hpp"
#elif defined(MFG_BACKEND_D3D)
#include "mfg_runtime_d3d.hpp"
#else
#error Only support Metal now.
#endif

#include "widget_target_view.h"


namespace qt_pal {

/*
  MFGの対象となるオブジェクトをラップするクラス。

  WidgetMFGのViewModelに近い役割を果たす。
*/
struct MFGTarget
{
  WidgetTargetView& _widgetTarget;

  mfg::MFG mMFG;
  std::function<void(const std::string& log)> mShowLog;
  bool mDebug = false;
  
  // moveしたいのでunique_ptrにしておく。
  std::unique_ptr<mfg_pal::Image32> _original;
  bool _commit = false;

  explicit MFGTarget( WidgetTargetView& widgetTarget,  std::function<void(const std::string& log)> logger );

  void PreviewInit();
  void CheckLogs();

  // 途中でキャンセルされるとfalse, それ以外はtrueを返す。
  bool ApplyToTarget( mfg::MFGBinary& binary );
  bool ApplyPreview( mfg::MFGBinary& binary );
  // プレビューのチェックがキャンセルされた時。
  void RevertToOriginalPreview();
  void ApplyFilter( mfg::MFGBinary& binary );
  void EndFilter();

  mfg::MFGBinary CompileZip( QString zipPath );
  mfg::MFGBinary CompileScript( const std::string& script, std::unique_ptr<mfg_parser::ResourceLoader> loader, mfg::ResStringMap &&smap );

  static void CancelProgress();
  static void BeforeProgressCancelableNoDialog();
  static void BeforeProgressCancelable();
  static void AfterProgress();
};

}///< qt_pal

#endif // MFG_OPEN_TARGET_H
