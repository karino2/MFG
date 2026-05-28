/* -*- coding: utf-8 -*- マルチバイト */

// Copyright 2026 PGN Inc. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#ifndef PROJECT_H
#define PROJECT_H

#include <mfg_pal/pal.hpp>
#include <vector>
#include <string>

#include <QDir>
#include <QString>
#include <QFileInfo>

/*
  指定したディレクトリをプロジェクトディレクトリとして扱うためのクラス。
*/
struct Project
{
  QDir _dir;
  QString _filterPath; // mfg path, full path.
  bool _isSelected;

  Project() : _dir(), _filterPath(""), _isSelected(false) {}

  void SetProject(const QDir& dir, const QString& filterPath )
  {
    _dir = dir;
    _filterPath = filterPath;
    _isSelected = true;
  }

  bool IsProjectSelected() const { return _isSelected; }

  const QString& FilterPath() const { return _filterPath; }
  QString ProjectName() const { return _dir.dirName(); }
  QString ProjectPath() const { return _dir.absolutePath(); }
  const QDir ProjectDir() const { return _dir; }
  QFileInfo DestArchiveFI( const QDir& dir )
  {
    return QFileInfo( dir, ProjectName() + ".mar" );
  }

  QString GetFilePath( QString fileName ){ return QFileInfo( _dir, fileName ).absoluteFilePath(); }
  bool FileExists( const QString& fileName ) const { return _dir.exists( fileName ); }

  void Close()
  {
    _dir = QDir();
    _isSelected = false;
  }
};

/*
  zipに加えるファイルの情報。
  pathがstd::stringなのはZipWriterのコメント参照。
*/
struct ArchiveEntryInfo
{
  std::string _path;
  std::string _name;
  ArchiveEntryInfo( const std::string &path, const std::string& name ) : _path( path ), _name( name ) {}
};

struct ProjectArchiver
{
  QDir _dir;
  ProjectArchiver( const QDir& dir ) : _dir( dir )
  {    
  }

  std::vector<ArchiveEntryInfo> ListEntries()
  {
    std::vector<ArchiveEntryInfo> ret;

    QStringList filters;
    filters << "*.mfg" << "*.png" << "*.json";
    _dir.setNameFilters( filters );
    _dir.setFilter( QDir::Files | QDir::NoDotAndDotDot );
    QFileInfoList list = _dir.entryInfoList();

    for (int i=0; i < list.count(); i++)
    {
      const QFileInfo fi( list.at( i ) );
      std::string path = fi.absoluteFilePath().toStdString();
      std::string name = fi.fileName().toStdString();
      ret.emplace_back( path, name );
    }
    return ret;
  }

  bool ArchiveTo( const QFileInfo& destZip );
};

/*
  プロジェクト用のResourceLoader。
*/
#include "mfg.hpp"

struct ProjectResourceLoader: mfg_parser::ResourceLoader
{
  QDir _dir;
  ProjectResourceLoader( const QDir& dir ) : _dir( dir )
  {
  }

  bool LoadImage( const std::string& name, mfg_pal::Image32& dest ) override;
};


#endif // PROJECT_H
