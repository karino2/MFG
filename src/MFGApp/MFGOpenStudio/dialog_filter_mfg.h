/* -*- coding: utf-8 -*- マルチバイト */

// Copyright 2026 PGN Inc. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#ifndef DIALOG_FILTER_MFG_H
#define DIALOG_FILTER_MFG_H

#include <QDialog>
#include <QSlider>
#include <QLabel>
#include <QCheckBox>
#include <QComboBox>
#include <QHBoxLayout>
#include <string>
#include <functional>
#include <vector>
#include <memory>

#include "mfg.hpp"

#include <qt_pal/mfg_target.h>
#include <qt_pal/mfg_widget_pal.h>

namespace wpal = mfg_widget_pal;

/*
  WidgetParamXXX
  ParamInfoに対応するwidgetのインターフェース。
*/
struct IWidgetParam
{
  virtual QWidget* AsWidget() = 0;
  virtual void SetParamIndex( size_t pidx ) = 0;
  virtual size_t GetParamIndex() = 0;
  virtual void SetupInfo( const mfg::ParamInfo& pinfo ) = 0;  
  virtual void ApplyTo( mfg::ParamInfo& pinfo ) = 0;

  void SetupInfoAndIndex( size_t pidx, const mfg::ParamInfo& pinfo )
  {
    SetParamIndex( pidx );
    SetupInfo( pinfo );
  }
};


class WidgetParamSlider : public QWidget, public IWidgetParam
{
  Q_OBJECT

public:
  size_t mPinfoIndex = 0;
  QSlider* mSlider;

  explicit WidgetParamSlider( QWidget *parent = 0 );

  void SetParamIndex( size_t pidx ) override { mPinfoIndex = pidx; }
  size_t GetParamIndex() override { return mPinfoIndex; }
  QWidget* AsWidget() override { return this; }

  void SetupInfo( const mfg::ParamInfo& pinfo ) override;
  void ApplyTo( mfg::ParamInfo& pinfo ) override ;

private:
  QLabel* mParamLabel;
  QLabel* mValueLabel;

  wpal::LinearMapper mMapper;
  bool mFloatSlider = false;

  void SliderPosToLabel( int pos );
};

class WidgetParamDirection : public QWidget, public IWidgetParam
{
  Q_OBJECT

public:
  size_t mPinfoIndex = 0;
  wpal::WidgetDirection* mDirection;

  explicit WidgetParamDirection( QWidget *parent = 0 );

  void SetParamIndex( size_t pidx ) override { mPinfoIndex = pidx; }
  size_t GetParamIndex() override { return mPinfoIndex; }
  QWidget* AsWidget() override { return this; }

  // DO nothing
  void SetupInfo( const mfg::ParamInfo& pinfo ) override ;
  void ApplyTo( mfg::ParamInfo& pinfo ) override ;
};

class WidgetParamCheckBox : public QWidget, public IWidgetParam
{
  Q_OBJECT

public:
  size_t mPinfoIndex = 0;
  QCheckBox* mCheckBox;

  explicit WidgetParamCheckBox( QWidget *parent = 0 );

  void SetParamIndex( size_t pidx ) override { mPinfoIndex = pidx; }
  size_t GetParamIndex() override { return mPinfoIndex; }
  QWidget* AsWidget() override { return this; }

  void SetupInfo( const mfg::ParamInfo& pinfo ) override;
  void ApplyTo( mfg::ParamInfo& pinfo ) override ;

};

class WidgetParamDropdown : public QWidget, public IWidgetParam
{
  Q_OBJECT

public:
  size_t mPinfoIndex = 0;
  QComboBox* mComboBox;

  explicit WidgetParamDropdown( QWidget *parent = 0 );

  void SetParamIndex( size_t pidx ) override { mPinfoIndex = pidx; }
  size_t GetParamIndex() override { return mPinfoIndex; }
  QWidget* AsWidget() override { return this; }

  void SetupInfo( const mfg::ParamInfo& pinfo ) override;
  void ApplyTo( mfg::ParamInfo& pinfo ) override ;

  private:
  QLabel* mParamLabel;

};


class WidgetParamPointer : public QWidget, public IWidgetParam
{
  Q_OBJECT

public:
  size_t mPinfoIndex = 0;
  wpal::WidgetCenterPointer* mCenterPointer;

  explicit WidgetParamPointer( QWidget *parent = 0 );

  void SetParamIndex( size_t pidx ) override { mPinfoIndex = pidx; }
  size_t GetParamIndex() override { return mPinfoIndex; }
  QWidget* AsWidget() override { return this; }

  void SetupInfo( const mfg::ParamInfo& pinfo ) override;
  void ApplyTo( mfg::ParamInfo& pinfo ) override ;

private:
  QLabel* mLabel;
};

class WidgetParamColorPicker : public QWidget, public IWidgetParam
{
  Q_OBJECT

public:
  size_t mPinfoIndex = 0;
  wpal::WidgetColorPicker* mColorPicker;

  explicit WidgetParamColorPicker( QWidget *parent = 0 );

  void SetParamIndex( size_t pidx ) override { mPinfoIndex = pidx; }
  size_t GetParamIndex() override { return mPinfoIndex; }
  QWidget* AsWidget() override { return this; }

  void SetupInfo( const mfg::ParamInfo& pinfo ) override;
  void ApplyTo( mfg::ParamInfo& pinfo ) override ;
};


class QPushButton;
class QDialogButtonBox;

/*
  MFG実行時のparamなどを全て管理するwidget。
*/
class WidgetMFG : public QWidget
{
  Q_OBJECT
public:
  explicit WidgetMFG( qt_pal::MFGTarget&& target, QWidget *parent = 0);

  void LoadBinary( mfg::MFGBinary&& binary );

  void LoadScript( const std::string& script, std::unique_ptr<mfg_parser::ResourceLoader> loader, mfg::ResStringMap &&smap );
  void LoadMAR( const QString& zipPath );

  const mfg::PerfInfo& GetPerfInfo() const { return mTarget.mMFG.GetPerfInfo(); }
  QString GetTitle() const ;

  void ShowEvent();
  void HideEvent();

  void Filter();
  void SetDebug( bool isDebug ) { mTarget.mDebug = isDebug; }

  QString Str( int idx ) const ;
  QString MStr( int idx ) const ;

  void Connect( QDialog* dialog, QDialogButtonBox* buttonBox );

  qt_pal::MFGTarget& GetTarget() { return mTarget; }

private:
  std::vector<IWidgetParam*> mParamWidgets;

  qt_pal::MFGTarget mTarget;

  mfg::MFGBinary mBinary;

  /*
    BuildPreviewの間に新しいバイナリがロードされた場合、
    その場でmBinaryを変更すると作業中のBuildPreviewの中で不整合が起きてしまう。
    そのため、mProcessingの間はmNextBinaryにセットしておいて、mProcessingが終わった段階でmBinaryを更新する。
  */
  mfg::MFGBinary mNextBinary;
  
  /*
    ParamWidgetの親レイアウト。
    ポインターとかDIRECTIONとか左に置きたいものは直接これに追加し、
    それ以外のものはVBoxに追加してそのVBoxを最後にこのレイアウトに追加する。
  */
  QHBoxLayout* mParamHLayout;

  // Preview
  QCheckBox* mCheckPreview;
  QPushButton* mButtonUpdateRand;

  bool mFirstShow;
  int mTimer;
  /*
    mStalePreviewとmProcessingの関係は繊細なのでここにコメントしてく。

    mStalePreviewは、基本的にはタイマー時にBuildPreviewを呼ぶかどうかを判定するだけで、BuildPreviewの中ではこのフラグは見ない。
    一方、mProcessingはBuildPreviewの先頭でtrueになり、終わりでfalseになる。
    基本的にこれらのフラグはGUIスレッドだけで操作されるのだが、
    BuildPreviewの中でProgressを回すので他のGUIスレッドの関数が再入される可能性はある。
    だから排他制御はいらないが、mProcessingの間に他の関数が呼ばれる可能性は考慮する必要がある。

    mStalePreviewはタイマーの中でBuildPreviewが呼ばれる事が確定した段階でfalseとなる。
    これはBuildPreviewの間に作られる予定のPreviewがstaleになる可能性があり、
    その場合はBuildPreviewの間にmStalePreviewがtrueにする事が出来るようにするため。
  */
  bool mProcessing;
  bool mStalePreview;
  size_t mLastChange;


  void Init();
  void ApplyBin();
  void InitAndApply();
  void InvalidatePreview();
  void BuildPreview();
  void CheckLogs() { mTarget.CheckLogs(); }
  void ApplyToPInfos();


signals:
  void startProcessing();
  void endProcessing();
  void canceled();
  void updatePreview();

public slots:
  void timerEvent( QTimerEvent* event ) override;
  void onPreviewChecked( bool b );

};

class DialogMFG : public QDialog
{
    Q_OBJECT
public:

  explicit DialogMFG( qt_pal::MFGTarget&& target, QWidget *parent = 0);

  void LoadScript( const std::string& script, std::unique_ptr<mfg_parser::ResourceLoader> loader, mfg::ResStringMap &&smap );
  void LoadMAR( const QString& zipPath );

  void Filter() { mWidgetMFG->Filter(); }
  const mfg::PerfInfo& GetPerfInfo() const { return mWidgetMFG->GetPerfInfo(); }
  void SetDebug( bool isDebug ) { mWidgetMFG->SetDebug( isDebug ); }
  qt_pal::MFGTarget& GetTarget() { return mWidgetMFG->GetTarget(); }

private:
  WidgetMFG* mWidgetMFG;

  void Init();

public slots:
  void showEvent( QShowEvent* event );
  void hideEvent( QHideEvent* event );
signals:
  void updatePreview();
};

#endif // DIALOG_FILTER_MFG_H
