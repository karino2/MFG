/* -*- coding: utf-8 -*- マルチバイト */

// Copyright 2026 PGN Inc. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#ifndef MAINWINDOW_UTIL_H
#define MAINWINDOW_UTIL_H

/*
  mainwindow.cppはOpen版とMFGStudioでかなりの部分重複しているが、違う部分もあるため一つには出来ない。
  そこでここに両者に共通のコードを置く。
  ほとんどはプロジェクトの作成関連のコードだが、それ以外のちょっとしたutilityもここに置くのでmainwindow_util.hという名前にしておく。
  基本的にはmainwindow.cppでしか使わないコード。
*/

#include <vector>
#include <string>

#include <QDir>
#include <QString>
#include <QFileInfo>
#include <QFile>
#include <QSettings>
#include <mfg_pal/pal.hpp>

namespace mainwindow_util
{
using mfg_pal::nstring;

inline std::vector<QString> MapPrefix( const QString& prefix, const std::vector<QString>& paths )
{
  std::vector<QString> ret;
  for (auto& path : paths)
  {
    ret.emplace_back( prefix + path );
  }
  return ret;
}

inline void CopyResFile( const QString& folder, const QString& fileName )
{
  QFile file( fileName );
  if (!file.exists()) return;

  QFileInfo fi( fileName );
  QString copyName = fi.fileName();

  // コピーする
  QString destName = folder + copyName;
  file.copy( destName );
}


/*
  プロジェクト関連クラス
*/

/*
  指定されたディレクトリの下に、projNameのディレクトリを作り、その中に初期コンテンツを埋める。
  projNameのディレクトリは存在してない事が前提（ProjDirExists()で呼び出し側がチェックする）。
*/
struct ProjectCreator
{
  QDir _dir;
  QString _projName;
  ProjectCreator( const QDir& dir, const QString& projName ) : _dir( dir ), _projName( projName ) {}

  /*
    すでにプロジェクトのディレクトリが存在するか？
    （すでに存在している場合はエラー）
  */
  bool ProjDirExists()
  {
    return _dir.exists( _projName );
  }

  QString ProjPath() const { return _dir.filePath( _projName ); }

  nstring ToNativePath( QString qpath )
  {
    return QDir::toNativeSeparators( qpath ).toStdWString();
  }

  bool CreateFile( const QString& path, const std::string& content )
  {
    QFile qf( path );
    if (!qf.open( QIODevice::WriteOnly ))
      return false;

    return content.size() == qf.write( content.c_str(), content.size() );
  }

  QDir ProjDir() { return QDir( ProjPath() ); }

  QString FilterFilePath()
  { 
    QDir dir = ProjDir();
    return dir.filePath( "filter.mfg" );
  }

  /*
    _projNameのディレクトリを作り、filter.mfgというファイルを作り初期コンテンツを埋める。
  */
  bool CreateProj( const std::string& initialContent )
  {
    assert( !ProjDirExists() );
    if (!_dir.mkdir( _projName ))
      return false;

    QString mfgPath = FilterFilePath();
    if (!CreateFile(mfgPath, initialContent))
      return false;

    return true;
  }

};

/*
  指定されたディレクトリがMFGのプロジェクトディレクトリかをチェック。
  ディレクトリが存在し、拡張子がmfgのファイルが一つでもあればvalidとみなす。
  最初に見つかったmfgファイルを返す。
*/
struct ProjectValidator
{
  QDir _projDir;
  ProjectValidator( const QDir& projDir ) : _projDir( projDir ) {}

  /*
    MFGファイルのリストを返す。
    ディレクトリ名は含めない。
    isEmpty()でファイルが無いかをチェックする。
  */
  QStringList ListMFG()
  {
    return _projDir.entryList( { "*.mfg" }, QDir::Files, QDir::Name );
  }

  bool IsValidProject()
  {
    return !ListMFG().isEmpty();
  }

  // 最初のMFGファイルをフルパスで返す。
  QString FirstMFGPath()
  {
    // すでにIsVliadPorjectはチェック済みのはずなので
    // 以下は失敗しない。
    return _projDir.filePath( ListMFG().at( 0 ) );
  }

};


inline QSettings* NewSettings( const QString& fileName )
{
  return new QSettings( fileName, QSettings::IniFormat );
}


}///<mainwindow_util


#endif // MAINWINDOW_UTIL_H
