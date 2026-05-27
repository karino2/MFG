// Copyright 2026 PGN Inc. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#ifndef UIICONMANAGER_H
#define UIICONMANAGER_H

#include <QtCore/QObject>
#include <QtCore/QFileSelector>
#include <QtCore/QHash>

class QAction;

class UIIconManager : public QObject
{
  Q_OBJECT
public:
  explicit UIIconManager( const QString &styleName, QObject *parent = nullptr );

  void setIcon( QAction *action, const QString &key );

public slots:
  void updateIcons( const QString &styleName );

private slots:
  void onActionDestroyed();

private:
  QFileSelector selector;
  QHash<QAction *, QString> iconMap;
};

#endif // UIICONMANAGER_H
