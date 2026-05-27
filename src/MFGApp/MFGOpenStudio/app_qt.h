/* -*- coding: utf-8 -*- マルチバイト */

// Copyright 2026 PGN Inc. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#ifndef ALPACA_QT_H_
#define ALPACA_QT_H_

#include <QAction>
#include <QString>
#include <QDir>
#include <QSettings>
#include <vector>

namespace app_qt
{

/////////////////////////////////////////////////////////////////////////////
// アプリ全体のutility。データフォルダのパスやMRUなどのアプリワイドなデータを保持。
// 社内ライブラリでは他の用途もいろいろあり、MFGOpenStudioでは名前を揃えて共有する都合でこうなっている。
// 作成する必要はなく、AQ() 経由でアクセス。
/////////////////////////////////////////////////////////////////////////////
class AppQt
{
private:
  QString _dataFolder; // 設定フォルダ
  QString _iccProfileFolder;  // ICCプロファイルフォルダ (/なし)

  QString _tmpFolder;  // Tmpフォルダ
  QString _debugFolder;  // Debugフォルダ

  void AddMRU_File( QString filepath );

public:
  AppQt();

  QString _statusMessage; // StatusBarのテキスト
  bool _statusMessageChanged; // StatusBarのテキスト、変更あった？

  // Path
  QString DataFolder() const { return _dataFolder; }
  QString ICCProfileFolder() const { return _iccProfileFolder; }

  QString TmpFolder() const { return _tmpFolder; }
  QString DebugFolder() const { return _debugFolder; }

  // MRU
  static const size_t MAX_RECENT = 8;

  // MRU Files
  std::vector<QString> _recentFiles;
  QAction* _recentAction[MAX_RECENT];

  // MRU Folders
  std::vector<QString> _recentProjects;
  QAction* _recentProjectAction[MAX_RECENT];

  void AddMRU( QString filepath );
  void AddMRU_Project( QDir projDir );
  void SaveMRU( QSettings* stg );
  void ReadMRU( QSettings* stg );

  // ステータスバーのキャプション
  void updateStatusTitle( QString str );

  bool StatusMessageChanged() const { return _statusMessageChanged; }
  void StatusMessageShown() { _statusMessageChanged = false; }
  const QString& StatusMessage() const { return _statusMessage; }

  // ショートカット文字列
  QString removeMenuShortcut( QString s );
  QString r4s( QString s );
  void rMenu( QMenu* menu );
  void rAction( QAction* action );
};

/////////////////////////////////////////////////////////////////////////////
// AppQt (グローバル変数) を取得
/////////////////////////////////////////////////////////////////////////////
AppQt* AQ();

} ///< app_qt

#endif


