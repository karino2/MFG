// Copyright 2026 PGN Inc. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#ifndef WINDOWSTATEMANAGER_H
#define WINDOWSTATEMANAGER_H

#include <QtCore/QObject>

class QSettings;
class QMainWindow;

class WindowStateManager : public QObject
{
  Q_OBJECT
  Q_PROPERTY(bool hasInitialState READ hasInitialState NOTIFY hasInitialStateChanged FINAL)
public:
  WindowStateManager( QSettings* settings, QMainWindow* window, const QString &initialSettingsPath );
  bool hasInitialState() const;

signals:
  void restoreFinished();
  void hasInitialStateChanged( bool hasInitialState );

public slots:
  void save();
  void restore();
  void saveInitialState();
  void restoreInitialState();

private slots:
  void loadState();
  void setHasInitialState( bool hasInitialState );

private:
  QSettings* mSettings = nullptr;
  QMainWindow* mWindow = nullptr;
  QString mInitialSettingsPath;
  bool mHasInitialState = false;
};

#endif // WINDOWSTATEMANAGER_H
