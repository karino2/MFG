/* -*- coding: utf-8 -*- マルチバイト */

// Copyright 2026 PGN Inc. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#ifndef MAN_H
#define MAN_H

#include <QAction>
#include <QIcon>
#include <QObject>
#include <QString>


namespace windowmanager {
  const int LEFT_PANEL_WIDTH = 240;
  const int RIGHT_PANEL_WIDTH = 260; // macを考慮 (240->260)
}

QAction *CreateAction( const QString &iconPath, const QString &text, QObject *parent, bool useUIIcon = true );
void UpdateIcons( const QString &styleName );
QIcon UIIcon( QString path );

#endif // MAN_H
