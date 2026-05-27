/* -*- coding: utf-8 -*- マルチバイト */

// Copyright 2026 PGN Inc. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#include <qt_pal/mfg_target.h>
#include <qt_pal/studio_pal.hpp> // ProgressImplのため

namespace qt_pal {

MFGTarget::MFGTarget( WidgetTargetView& widgetTarget, std::function<void(const std::string& log)> logger ) : _widgetTarget( widgetTarget ), mShowLog( std::move(logger) ), _original( new mfg_pal::Image32() )
{
}

void MFGTarget::PreviewInit()
{
  _original->Copy( _widgetTarget._targetImage );
  _commit = false;
}

void MFGTarget::CheckLogs()
{
  std::string logs = mMFG.DrainLogs();
  if (logs.size() != 0)
  {
    mShowLog( logs );
  }
}

using namespace mfg;

void MFGTarget::RevertToOriginalPreview()
{
  _widgetTarget._targetImage.Copy( *_original );
  _widgetTarget.UpdateOffscreen();
}

bool MFGTarget::ApplyToTarget( mfg::MFGBinary& binary )
{
  bool success = true;

  InputTiles tiles;
  tiles.Insert( 0, _original.get() );
  TileReference resultRef( &_widgetTarget._targetImage );


  mMFG.EnableDebugLog( mDebug );
    
  try
  {
    success = mMFG.Run( binary, tiles, resultRef, studio_pal::ProgressImpl );
    CheckLogs();
  }
  catch(const mfg_internal::Error& err)
  {
    mShowLog( std::string("Error: ") + err.what());
    success = false;
  }
  if (success)
  {
    _widgetTarget.UpdateOffscreen();
  }

  return success;
}

bool MFGTarget::ApplyPreview( mfg::MFGBinary& binary )
{
  return ApplyToTarget( binary );
}

void MFGTarget::ApplyFilter( mfg::MFGBinary& binary )
{
  _commit = true;
  ApplyToTarget( binary );
}

void MFGTarget::EndFilter()
{
  if (!_commit)
  {
    RevertToOriginalPreview();
  }
}

mfg::MFGBinary MFGTarget::CompileZip( QString zipPath )
{
  nstring npath = zipPath.toStdWString();
  return mMFG.CompileZip( npath );
}

mfg::MFGBinary MFGTarget::CompileScript( const std::string& script, std::unique_ptr<mfg_parser::ResourceLoader> loader, mfg::ResStringMap &&smap )
{
  return mMFG.Compile( script, std::move(loader), std::move(smap) );
}

void MFGTarget::CancelProgress() { studio_pal::CancelProgress();}
void MFGTarget::BeforeProgressCancelableNoDialog() { studio_pal::BeforeProgressCancelableNoDialog(); }
void MFGTarget::BeforeProgressCancelable() { studio_pal::BeforeProgressCancelable(); }
void MFGTarget::AfterProgress() { studio_pal::AfterProgress(); }

}///<qt_pal
