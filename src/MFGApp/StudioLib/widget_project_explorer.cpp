/* -*- coding: utf-8 -*- マルチバイト */

// Copyright 2026 PGN Inc. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#include "widget_project_explorer.h"
#include <qt_pal/qt_pal.h>
#include "man.h"
#include "trans_str_mfg.h"

#include <QVBoxLayout>
#include <QUrl>
#include <QScroller>
#include <QDesktopServices>

static constexpr const char* RES_STRING_FNAME = "strings.json";
static constexpr const char* THUMBNAIL_FNAME = "thumbnail.png";

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
WidgetProjectExplorer::WidgetProjectExplorer(QWidget *parent) : QWidget(parent), mDir("")
{
  mBar.setIconSize( QSize( qt_pal::MagInt( 20 ), qt_pal::MagInt( 20 ) ) );

  mActImportImage = CreateAction( ":/images/icon_file", tr(""), this );
  mBar.addAction( mActImportImage );
  connect( mActImportImage, &QAction::triggered, this, [this](bool){ emit importImage(); } );

  mActRemove = CreateAction( ":/images/icon_trash", tr(""), this );
  mBar.addAction( mActRemove );
  connect( mActRemove, &QAction::triggered, this, [this](bool){
    auto selected = mTree.selectionModel()->selectedRows();
    if (!selected.isEmpty())
    {
      QModelIndex index = selected.first();
      QFileInfo fi = mFModel.fileInfo( index );
      if (fi.suffix() == "png")
      {
        QFile::remove( fi.absoluteFilePath());
      }

    }
  } );
  mActRemove->setEnabled( false );

  mActNewStrings = CreateAction( ":/images/icon_strings", tr(""), this );
  mBar.addAction( mActNewStrings );
  mActNewStrings->setEnabled( false );
  connect( mActNewStrings, &QAction::triggered, this, [this](bool){
    emit createStringsJson( mDir.filePath(RES_STRING_FNAME) );
    mActNewStrings->setEnabled( !mDir.exists( RES_STRING_FNAME ) );
  });

  mActImportThumb = CreateAction(":/images/thumb_import", tr(""), this );
  mBar.addAction( mActImportThumb );
  mActImportThumb->setEnabled( false );
  connect( mActImportThumb, &QAction::triggered, this, [this](bool){
    emit importThumbnail();
    mActImportThumb->setEnabled( !mDir.exists( THUMBNAIL_FNAME ) );
  });

  mActImportThumb->setToolTip( Trans_StrMFG( 61 ) );



  mFModel.setFilter( QDir::Files );

  mTree.setModel( nullptr );
  QScroller::grabGesture(&mTree, QScroller::TouchGesture);
  mTree.setSelectionMode( QAbstractItemView::SingleSelection );

  QVBoxLayout* lo = new QVBoxLayout();
  lo->setContentsMargins( 0, 0, 0, 0 );
  lo->addWidget( &mBar );
  lo->addWidget( &mTree );

  setLayout( lo );
  setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Preferred );
}

WidgetProjectExplorer::~WidgetProjectExplorer()
{
}

QSize WidgetProjectExplorer::sizeHint() const
{
  return QSize( 260, 400 );
}

/////////////////////////////////////////////////////////////////////////////
void WidgetProjectExplorer::openDir( const QDir& dir )
{
  mDir = dir;

  mFModel.setRootPath( mDir.absolutePath() );
  mTree.setModel( &mFModel );
  mTree.setRootIndex( mFModel.index(mDir.absolutePath()) );
  // name, size, type, modified date.
  // hide size, type, mod date for a while.
  mTree.hideColumn(1);
  mTree.hideColumn(2);
  mTree.hideColumn(3);

  connect( mTree.selectionModel(), &QItemSelectionModel::selectionChanged, this, &WidgetProjectExplorer::onSelectionChanged );
  connect( &mTree, &QTreeView::doubleClicked, this, [this](const QModelIndex &index) {
    QFileInfo fi = mFModel.fileInfo( index );
    if(fi.suffix() == "png" || fi.suffix() == "json")
    {
      QUrl imageUrl = QUrl::fromLocalFile(fi.absoluteFilePath());
      QDesktopServices::openUrl(imageUrl);
    }
  });

  mActRemove->setEnabled( false );

  mActNewStrings->setEnabled( !mDir.exists( RES_STRING_FNAME ) );
  mActImportThumb->setEnabled( true );
}

/////////////////////////////////////////////////////////////////////////////
void WidgetProjectExplorer::closeDir()
{
  mDir = QDir("");

  mFModel.setRootPath( "" );
  mTree.setModel( nullptr );
  mActRemove->setEnabled( false );
  mActNewStrings->setEnabled( false );
  mActImportThumb->setEnabled( false );
}

void WidgetProjectExplorer::onSelectionChanged( const QItemSelection &selected, const QItemSelection &deselected )
{
  if (!selected.isEmpty())
  {
    QModelIndex index = selected.indexes().first();
    QFileInfo fi = mFModel.fileInfo( index );
    if (fi.suffix() == "mfg")
    {
      // qDebug() << fi.absoluteFilePath();
      emit fileSelected( fi );
      // mfgファイルは削除出来てもいい気がするが、現状作り直す方法が無いので削除出来ない事にしておく。
      mActRemove->setEnabled( false );
      return;
    }
    else if(fi.suffix() == "png")
    {
      mActRemove->setEnabled( true );
      return;
    }
  }
  mActRemove->setEnabled( false );
}
