// Copyright 2026 PGN Inc. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#include "window_state_manager.h"

#include <QtCore/QEvent>
#include <QtCore/QSettings>
#include <QtGui/QGuiApplication>
#include <QtGui/QScreen>

#include "mainwindow.h"

namespace {
const QString GeometryTag = QStringLiteral( "main/geometry2" );
const QString StateTag = QStringLiteral( "main/windowState2" );
const QString InitVersionTag = QStringLiteral( "main/version" );
const QString InitGeometryTag = QStringLiteral( "geometry2" );
const QString InitStateTag = QStringLiteral( "windowState2" );
QString subGroup() {
  auto screen = qApp->primaryScreen();
  if ( !screen ) {
    return QString{};
  }
  auto size = screen->availableSize();
  return QStringLiteral( "%1x%2_%3" ).arg( size.width() )
      .arg( size.height() )
      .arg( screen->logicalDotsPerInch() );
}
bool load( QSettings &settings, QByteArray *geometory, QByteArray *state ) {
  Q_ASSERT( geometory );
  Q_ASSERT( state );
  auto group = subGroup();
  if ( group.isEmpty() ) {
    return false;
  }
  auto availableGroups = settings.childGroups();
  availableGroups.removeOne( "main" );
  if (availableGroups.isEmpty()) {
    return false;
  }
  if (availableGroups.contains( group )) {
    settings.beginGroup( group );
  } else {
    settings.beginGroup( availableGroups.at(0) );
  }
  *geometory = settings.value( InitGeometryTag ).toByteArray();
  *state = settings.value( InitStateTag ).toByteArray();

  return !geometory->isEmpty() && !state->isEmpty();
}
} // unnamed namespace

WindowStateManager::WindowStateManager( QSettings* settings , QMainWindow* window , const QString &initialSettingsPath )
  : QObject( window )
  , mSettings( settings )
  , mWindow( window )
  , mInitialSettingsPath( initialSettingsPath )
{
  Q_ASSERT( settings );
  Q_ASSERT( window );
  QSettings initialSettings( mInitialSettingsPath, QSettings::IniFormat );
  if (initialSettings.value( InitVersionTag ).toString() != STUDIO_APP_VERSION) {
    initialSettings.clear();
    initialSettings.setValue( InitVersionTag, STUDIO_APP_VERSION );
  } else {
    initialSettings.beginGroup( subGroup() );
    setHasInitialState( !initialSettings.value( InitGeometryTag ).isNull()
                     && !initialSettings.value( InitStateTag ).isNull() );
  }
}

bool WindowStateManager::hasInitialState() const
{
  return mHasInitialState;
}

void WindowStateManager::save()
{
  mSettings->setValue( GeometryTag, mWindow->saveGeometry() );
  mSettings->setValue( StateTag, mWindow->saveState() );
}

void WindowStateManager::restore()
{
  loadState();
  emit restoreFinished();
}

void WindowStateManager::saveInitialState()
{
  QSettings initialSettings( mInitialSettingsPath, QSettings::IniFormat );
  initialSettings.beginGroup( subGroup() );
  initialSettings.setValue( InitGeometryTag, mWindow->saveGeometry() );
  initialSettings.setValue( InitStateTag, mWindow->saveState() );
  setHasInitialState( true );
}

void WindowStateManager::restoreInitialState()
{
  if (!mHasInitialState) {
    return;
  }
  QSettings initialSettings( mInitialSettingsPath, QSettings::IniFormat );
  QByteArray geometry;
  QByteArray windowState;
  if (!load( initialSettings, &geometry, &windowState )) {
    return;
  }
  mWindow->restoreGeometry( geometry );
  mWindow->restoreState( windowState );
}

void WindowStateManager::loadState()
{
  const QVariant geometry = mSettings->value( GeometryTag );
  const QVariant windowState = mSettings->value( StateTag );
  if (!geometry.isNull() && !windowState.isNull()) {
    mWindow->restoreGeometry( geometry.toByteArray() );
    mWindow->restoreState( windowState.toByteArray() );
  }
}

void WindowStateManager::setHasInitialState(bool hasInitialState)
{
  if (mHasInitialState == hasInitialState) {
    return;
  }
  mHasInitialState = hasInitialState;
  emit hasInitialStateChanged( mHasInitialState );
}
