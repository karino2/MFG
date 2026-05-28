/* -*- coding: utf-8 -*- マルチバイト */

// Copyright 2026 PGN Inc. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#if defined(_MSC_VER)
#pragma execution_character_set("utf-8")
#endif

#include "man.h"
#include <qt_pal/qt_pal.h>
#include "uiiconmanager.h"


namespace {
UIIconManager iconManager("");
} // unnamed namespace

///////////////////////////////////////////////////////////////////////////
// UI用のアイコン取得
///////////////////////////////////////////////////////////////////////////
QIcon UIIcon( QString path )
{
  if (qt_pal::UIMag() >= 2.0) path = path + "_x2";
  //if (qt_pal::UIMag() > 1.0) path = path + "_x2";

  path = path + ".png";

  QIcon icon( path );
  return icon;
}

///////////////////////////////////////////////////////////////////////////
QAction *CreateAction( const QString &iconPath, const QString &text, QObject *parent, bool useUIIcon )
{
  QString path = iconPath;
  if (useUIIcon && qt_pal::UIMag() >= 2.0) path = path + "_x2";

  QAction *action = new QAction( text, parent );
  iconManager.setIcon( action, path );
  return action;
}

///////////////////////////////////////////////////////////////////////////
void UpdateIcons( const QString &styleName )
{
  iconManager.updateIcons( styleName );
}