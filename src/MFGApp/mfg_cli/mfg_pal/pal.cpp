// Copyright 2026 PGN Inc. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

/* -*- coding: utf-8 -*- マルチバイト */

#include <mfg_pal/pal.hpp>
#include <fstream>

namespace mfg_pal
{

/*
  GetZipFileの実装
*/

// cppunzipのIStreamFileとほとんど同じだがifstreamを保持するバージョン。
struct IStreamFileAdapter : public uz::File
{
  std::ifstream _istream;

  IStreamFileAdapter( const nstring& filepath) : File(0), _istream( filepath, std::ios::binary )
  {
    _istream.seekg(0, std::istream::end);
    _size = _istream.tellg();
    _istream.seekg(0);
  }
  virtual ~IStreamFileAdapter() {}

protected:
  int readAtImpl(size_t pos, uint8_t* dst, size_t size) {
    _istream.clear();
    _istream.seekg(pos);
    _istream.read((char*)dst, size);
    return (int)_istream.gcount();
  }
};

std::unique_ptr<uz::File> GetZipFile( const nstring& filepath )
{
  return std::unique_ptr<uz::File>( new IStreamFileAdapter( filepath ) );
  // return std::make_unique<IStreamFileAdapter>( filepath );
} 

} ///<mfg_pal
