/* -*- coding: utf-8 -*- マルチバイト */

// Copyright 2026 PGN Inc. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#include "project.h"

#include <mz.h>
#include <mz_strm.h>
#include <mz_zip.h>
#include <mz_zip_rw.h>


/*
  minizipのwrapper。

  ファイルのパスは本来はnstringにすべきだが、下がUTF8のc_strを前提としていて、
  上はどうせQStringなので今は直接std::stringを渡す事にする。
  将来Qt以外からも使う日が来たらnstringに変更する。
*/
struct ZipWriter
{
  void* _zipWriter;
  std::string _zipName;
  bool _opened = false;

  ZipWriter()
  {
    _zipWriter = mz_zip_writer_create();
  }

  /*
    destZipPathは生成するzipファイルのパス。
    zipNameはzipファイルのbasenameの拡張子をとったもの。
    セパレータ周りの処理はOS依存なので上でやってもらうため、別々に渡す。
  */
  bool Create( const std::string& destZipPath, const std::string& zipName )
  {
    assert( !_opened );
    _zipName = zipName;
    if(MZ_OK == mz_zip_writer_open_file(_zipWriter, destZipPath.c_str(), 0, 0))
    {
      _opened = true;
    }
    return _opened;
  }

  /*
    filePathにあるファイルを ${_zipName}/${name} としてzipファイルに追加。${_zipName}/ は自動で追加される。
  */
  bool AddFile( const std::string& filePath, const std::string& name )
  {
    assert( _opened );
    std::string entryName = _zipName + "/" + name;
    return MZ_OK == mz_zip_writer_add_file( _zipWriter, filePath.c_str(), entryName.c_str() );
  }

  void Close()
  {
    if (_opened)
    {
      mz_zip_writer_close( _zipWriter );
      _opened = false;
    }
  }

  ~ZipWriter()
  {
    Close();

    if (_zipWriter != nullptr)
      mz_zip_writer_delete( &_zipWriter );
  }
};

bool ProjectArchiver::ArchiveTo( const QFileInfo& destZip )
{
  if (destZip.exists())
  {
    QFile::remove( destZip.absoluteFilePath() );
  }

  std::string name = destZip.baseName().toStdString();
  ZipWriter writer;

  if(!writer.Create( destZip.absoluteFilePath().toStdString(), name ))
    return false;
  
  auto entryList = ListEntries(); 
  for (const auto& entry : entryList)
  {
    if (!writer.AddFile( entry._path, entry._name ))
      return false;
  }
  // ZipWriterのcloseはデストラクタで呼ばれる。
  return true;
}

/*
  ProjectResourceLoader。ひとまずここに置くが将来移動するかもしれないのでincludeはここに置く。
*/

#include <qt_pal/studio_pal.hpp>

bool ProjectResourceLoader::LoadImage( const std::string& name, mfg_pal::Image32& dest )
{
  // return LoadPngAsTile( "test_data/" + name, dest );
  QString fname = QString::fromStdString( name );
  if (!_dir.exists( fname ))
    return false;


  QImage qimg;
  if(!qimg.load( _dir.filePath( fname ), "PNG" ))
    return false;

  return studio_pal::QImageToImage32( qimg, dest );
}