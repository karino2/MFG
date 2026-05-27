/* -*- coding: utf-8 -*- マルチバイト */

// Copyright 2026 PGN Inc. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#include "app_qt.h"

#include "studio_conf.h"

#include <QStandardPaths>
#include <QMenu>
#include <QApplication>

namespace app_qt
{

////////////////////////////////////////////////////////////////////////////
AppQt* gAppQt = NULL;
AppQt* AQ()
{
  if (gAppQt == NULL)
  {
    // 使われた時だけ初期化する
    gAppQt = new AppQt();
  }
  return gAppQt;
}

////////////////////////////////////////////////////////////////////////////
AppQt::AppQt()
{
  // アプリ設定 (MainWindowでも設定しているが、その前に呼ばれると困るので先に設定)
  qApp->setOrganizationName( STUDIO_ORG_NAME );
  qApp->setOrganizationDomain( STUDIO_ORG_DOMAIN );
  qApp->setApplicationName( STUDIO_APP_NAME );

  _statusMessageChanged = false;

  // 設定フォルダ周り
  const QString dataPath = QStandardPaths::standardLocations( QStandardPaths::AppLocalDataLocation ).at( 0 );

  // Win/Mac共通パスのため
  QFileInfo dataInfo( dataPath );

  // ファイル作成時に勝手に掘るけど、念のため先に掘っておく
  QDir dir;

  // AppDataで使うので設定しておく
  _dataFolder = dataInfo.absoluteFilePath();
  dir.mkpath( _dataFolder );
  _dataFolder = _dataFolder + "/";

  // ICC
  _iccProfileFolder = _dataFolder + "icc_profile";

  // Tmp
  _tmpFolder = _dataFolder + "tmp";
  dir.mkpath( _tmpFolder );
  _tmpFolder = _tmpFolder + "/";

  // Debug
  _debugFolder = _dataFolder + "debug";
  dir.mkpath( _debugFolder );
  _debugFolder = _debugFolder + "/";
}

////////////////////////////////////////////////////////////////////////////
void AppQt::AddMRU_File( QString filepath )
{
  // 既存かチェック
  bool exists = false;
  size_t index = 0;
  for (size_t i=0; i<_recentFiles.size(); i++)
  {
    if (_recentFiles[i] == filepath)
    {
      exists = true;
      index = i;
    }
  }

  // 既存なので、insert
  if (exists)
  {
    QString tmp = _recentFiles[index];
    _recentFiles.erase( _recentFiles.begin() + index );
    _recentFiles.insert( _recentFiles.begin(), tmp );
    return;
  }

  // 新規なので先頭に追加
  _recentFiles.insert( _recentFiles.begin(), filepath );

  // 最大数以上いってたら、カット
  if (_recentFiles.size() > MAX_RECENT)
  {
    _recentFiles.erase( _recentFiles.begin() + MAX_RECENT );
  }
}

////////////////////////////////////////////////////////////////////////////
void AppQt::AddMRU_Project( QDir projDir )
{
  QString filepath = projDir.absolutePath();
  
  // 既存かチェック
  bool exists = false;
  size_t index = 0;
  for (size_t i=0; i<_recentProjects.size(); i++)
  {
    if (_recentProjects[i] == filepath)
    {
      exists = true;
      index = i;
    }
  }

  // 既存なので、insert
  if (exists)
  {
    QString tmp = _recentProjects[index];
    _recentProjects.erase( _recentProjects.begin() + index );
    _recentProjects.insert( _recentProjects.begin(), tmp );
    return;
  }

  // 新規なので先頭に追加
  _recentProjects.insert( _recentProjects.begin(), filepath );

  // 最大数以上いってたら、カット
  if (_recentProjects.size() > MAX_RECENT)
  {
    _recentProjects.erase( _recentProjects.begin() + MAX_RECENT );
  }
}

////////////////////////////////////////////////////////////////////////////
void AppQt::AddMRU( QString filepath )
{
  // MRUに保存するのは必ず "/" 区切りのパス (fileOpen時に、toNativeSeparators され、最近使ったファイルが重複するバグ対応)
  filepath = QDir::fromNativeSeparators( filepath );

  AddMRU_File( filepath );
}

////////////////////////////////////////////////////////////////////////////
void AppQt::SaveMRU( QSettings* stg )
{
  // Save Files
  for (size_t i=0; i<_recentFiles.size(); i++)
  {
    QString s = "mru/file" + QString::number( i );
    stg->setValue( s, _recentFiles[i] );
  }

  // Save Projects
  for (size_t i=0; i<_recentProjects.size(); i++)
  {
    QString s = "mru/project" + QString::number( i );
    stg->setValue( s, _recentProjects[i] );
  }
}

////////////////////////////////////////////////////////////////////////////
void AppQt::ReadMRU( QSettings* stg )
{
  // Read Files
  for (size_t i=0; i<MAX_RECENT; i++)
  {
    QString s = "mru/file" + QString::number( i );
    QVariant v = stg->value( s );
    if (!v.isNull()) _recentFiles.push_back( v.toString() );
  }

  // Read Projects
  for (size_t i=0; i<MAX_RECENT; i++)
  {
    QString s = "mru/project" + QString::number( i );
    QVariant v = stg->value( s );
    if (!v.isNull()) _recentProjects.push_back( v.toString() );
  }
}

////////////////////////////////////////////////////////////////////////////
void AppQt::updateStatusTitle( QString str )
{
  // ステータスのテキストは変更あった？
  if (str != _statusMessage)
  {
    // timerで負荷の掛からないように更新
    _statusMessage = str;
    _statusMessageChanged = true;
  }
}

////////////////////////////////////////////////////////////////////////////
QString AppQt::removeMenuShortcut( QString s )
{
  int i0 = s.lastIndexOf( "(&" );
  int i1 = s.lastIndexOf( ")" );

  if (i0 == -1) return s;
  if (i1 == -1) return s;
  if ((i0 + 3) != i1) return s;

  // (&F) とかを取り除く
  s.remove( i0, 4 );
  return s;
}

////////////////////////////////////////////////////////////////////////////
QString AppQt::r4s( QString s )
{
  // (&F) とかを取り除く
  s = removeMenuShortcut( s );

  int idx = s.lastIndexOf( "..." );
  if (idx == -1) return s;

  // ... を取り除く
  s.remove( idx, 3 );
  return s;
}

////////////////////////////////////////////////////////////////////////////
void AppQt::rMenu( QMenu* menu )
{
  QString s = menu->title();
  QString s2 = removeMenuShortcut( s );

  if (s != s2) menu->setTitle( s2 );
}

////////////////////////////////////////////////////////////////////////////
void AppQt::rAction( QAction* action )
{
  QString s = action->text();
  QString s2 = removeMenuShortcut( s );

  if (s != s2) action->setText( s2 );
}


} ///< neet

