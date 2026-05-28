// Copyright 2026 PGN Inc. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#include "uiiconmanager.h"

#include <QtGui/QAction>

UIIconManager::UIIconManager(const QString &styleName, QObject *parent)
  : QObject( parent )
{
  selector.setExtraSelectors( {styleName} );
}

void UIIconManager::setIcon(QAction *action, const QString &key)
{
  const auto path = key + ".png";
  connect( action, SIGNAL(destroyed()), this, SLOT(onActionDestroyed()) );
  iconMap.insert( action, path );
  action->setIcon( QIcon( selector.select( path ) ) );
}

void UIIconManager::updateIcons( const QString &styleName )
{
  selector.setExtraSelectors( {styleName} );
  QHashIterator<QAction *, QString> i( iconMap );
  while (i.hasNext()) {
    i.next();
    i.key()->setIcon( QIcon( selector.select( i.value() ) ) );
  }
}

void UIIconManager::onActionDestroyed()
{
  QAction *action = qobject_cast<QAction*>( sender() );
  if (action) {
    iconMap.remove( action );
  }
}
