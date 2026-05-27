// Copyright 2026 PGN Inc. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

/* -*- coding: utf-8 -*- マルチバイト */

#include <mfg_pal/pal.hpp>
#include <QString>
#include <QFile>


namespace mfg_pal
{

/*
  GetZipFileの実装
*/

struct QFileAdapter : public uz::File
{
  QFile _qf;

  QFileAdapter( const nstring& filepath ) : File( 0 ), _qf( QString::fromStdWString( filepath ) )
  {
    if (!_qf.open( QIODevice::ReadOnly ))
      throw uz::UnZipError("Fail to open.");
    
      _size = _qf.size();
  }
  ~QFileAdapter() {}

protected:
  int readAtImpl(size_t pos, uint8_t* dst, size_t size)
  {
    if (!_qf.seek( pos ) )
      throw uz::UnZipError("Fail to seek.");

    auto adjusted = std::min(size, _size-pos);
    auto readActual = _qf.read( (char*)dst, adjusted );
    if (adjusted != readActual)
      throw uz::UnZipError("Fail to read.");
    
    return (int)adjusted;
  }

};


std::unique_ptr<uz::File> GetZipFile( const nstring& filepath )
{
  return std::unique_ptr<uz::File>( new QFileAdapter( filepath ) );
} 

} ///<mfg_pal
