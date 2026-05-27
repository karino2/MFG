/* -*- coding: utf-8 -*- マルチバイト */

// Copyright 2026 PGN Inc. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#include "dialog_filter_mfg.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QLabel>
#include <QFrame>
#include <QPushButton>
#include <QTimer>
#include <QTimerEvent>
#include <vector>

#include <mfg_pal/pal.hpp>
#include <qt_pal/qt_pal.h>
#include <qt_pal/mfg_widget_pal.h>

#include "trans_str_filter.h"
#include "trans_str_mfg.h"


using namespace qt_pal;

extern QWidget* MainAsQWidget();

static int MinValue( const mfg::ParamInfo& pinfo )
{
  if (pinfo.GetParamType() == mfg::ParamInfo::SLIDER_INT32)
  {
    return pinfo.GetMin().Value<int>();
  }
  else
  {
    assert( pinfo.GetParamType() == mfg::ParamInfo::SLIDER_FLOAT32);
    return 0;
  }
}

static int MaxValue( const mfg::ParamInfo& pinfo )
{
  if (pinfo.GetParamType() == mfg::ParamInfo::SLIDER_INT32)
  {
    return pinfo.GetMax().Value<int>();
  }
  else
  {
    assert( pinfo.GetParamType() == mfg::ParamInfo::SLIDER_FLOAT32);
    return 200;
  }
}

static mfg_pal::TBpp32 ToBpp32( const std::vector<float>& fvec )
{
  assert(fvec.size() == 4);
  mfg_pal::TBpp32 val;
  val.B = (uint8_t)(255.0F*fvec[0]);
  val.G = (uint8_t)(255.0F*fvec[1]);
  val.R = (uint8_t)(255.0F*fvec[2]);
  val.A = (uint8_t)(255.0F*fvec[3]);
  return val;
}

/*
  WidgetParamXXX
  ParamInfoに対応するWidget。
*/

/*
  WidgetParamSlider
*/
WidgetParamSlider::WidgetParamSlider( QWidget *parent ) : QWidget( parent )
{
  setContentsMargins( 0, 0, 0, 0 );

  QHBoxLayout *lo = new QHBoxLayout();
  lo->setContentsMargins( 0, 0, 0, 0 );

  mParamLabel = new QLabel( "" );
  lo->addWidget( mParamLabel, 1 );
  mSlider = new QSlider();

  mSlider->setOrientation( Qt::Horizontal );
  lo->addWidget( mSlider, 5 );
  mValueLabel = new QLabel( "1" );
  lo->addWidget( mValueLabel, 1 );

  setLayout( lo );
  setMinimumWidth( qt_pal::MagInt( 640 ) );

  connect( mSlider, &QSlider::valueChanged, [this]( int newVal ) {
    SliderPosToLabel( newVal );
  });

}

void WidgetParamSlider::SliderPosToLabel( int pos )
{
  QString newLabel;
  if (mFloatSlider)
  {
    float v = (float)mMapper.GetValue( pos, mSlider->maximum() );

    newLabel = QString::number( v, 'f', 2 );
    if (v >= 10) newLabel = QString::number( (int)v );
  }
  else
  {
    newLabel = QString::number( pos );
  }
  mValueLabel->setText( newLabel );
}

void WidgetParamSlider::SetupInfo( const mfg::ParamInfo& pinfo )
{
  mParamLabel->setText( QString::fromStdString( pinfo.GetLabel() ) );

  mSlider->setMinimum( MinValue( pinfo ) );
  mSlider->setMaximum( MaxValue( pinfo ) );

  int init = 1;
  if (pinfo.GetParamType() == mfg::ParamInfo::SLIDER_FLOAT32)
  {
    mFloatSlider = true;
    wpal::SetupLinearMapper( mMapper, pinfo );
    init = mMapper.GetPos( pinfo.GetInit().Value<float>(), mSlider->maximum() );
  }
  else
  {
    mFloatSlider = false;
    init = pinfo.GetInit().Value<int>();
  }

  mSlider->setValue( init );
  // 初期位置と一致しているときだけvalueChangedが呼ばれないので手で呼ぶ
  if(init == 0)
    SliderPosToLabel( init );
}

void WidgetParamSlider::ApplyTo( mfg::ParamInfo& pinfo )
{
  if( pinfo.GetParamType() == mfg::ParamInfo::SLIDER_INT32 )
  {
    pinfo.Assign( mSlider->value() );
  }
  else
  {
    assert( pinfo.GetParamType() == mfg::ParamInfo::SLIDER_FLOAT32 );
    double v = mMapper.GetValue( mSlider->value(), mSlider->maximum() );
    pinfo.Assign( (float)v );
  }
}

/*
  WidgetParamDirection
*/
WidgetParamDirection::WidgetParamDirection( QWidget *parent ) : QWidget( parent )
{
  QVBoxLayout* lo = new QVBoxLayout();
  mDirection = new wpal::WidgetDirection();
  lo->setContentsMargins( 0, 0, 0, 0 );
  lo->addWidget( mDirection );

  setLayout( lo );
}

void WidgetParamDirection::SetupInfo( const mfg::ParamInfo& pinfo )
{
  mDirection->setDir( (double)pinfo.GetInit().Value<float>() );
}

void WidgetParamDirection::ApplyTo( mfg::ParamInfo& pinfo )
{
  assert( pinfo.GetParamType() == mfg::ParamInfo::DIRECTION_FLOAT32 );
  pinfo.Assign( (float)mDirection->mDir );
}

/*
  WidgetParamCheckBox
*/
WidgetParamCheckBox::WidgetParamCheckBox( QWidget *parent ) : QWidget( parent )
{
  QVBoxLayout* lo = new QVBoxLayout();
  mCheckBox = new QCheckBox( "" );
  lo->addWidget( mCheckBox );

  setLayout( lo );
}

void WidgetParamCheckBox::SetupInfo( const mfg::ParamInfo& pinfo )
{
  mCheckBox->setText( QString::fromStdString( pinfo.GetLabel() ) );
  mCheckBox->setChecked( pinfo.GetInit().Value<int>() == 1 );
}

void WidgetParamCheckBox::ApplyTo( mfg::ParamInfo& pinfo )
{
  assert( pinfo.GetParamType() == mfg::ParamInfo::CHECKBOX_INT32 );
  pinfo.Assign( (int32_t)(mCheckBox->isChecked()) );
}

/*
  WidgetParamDropdown
*/
WidgetParamDropdown::WidgetParamDropdown( QWidget *parent ) : QWidget( parent )
{
  QHBoxLayout* lo = new QHBoxLayout();
  mComboBox = new QComboBox();
  mParamLabel = new QLabel( "" );
  lo->addWidget( mParamLabel, 1 );
  lo->addWidget( mComboBox );

  setLayout( lo );
}

void WidgetParamDropdown::SetupInfo( const mfg::ParamInfo& pinfo )
{
  mParamLabel->setText( QString::fromStdString( pinfo.GetLabel() ) );
  mComboBox->clear();

  for (const auto& item : pinfo.GetItems())
  {
    mComboBox->addItem( QString::fromStdString( item ) );
  }
}

void WidgetParamDropdown::ApplyTo( mfg::ParamInfo& pinfo )
{
  assert( pinfo.GetParamType() == mfg::ParamInfo::DROPDOWN_INT32 );
  pinfo.Assign( (int32_t)(mComboBox->currentIndex()) );
}

/*
  WidgetParamPointer
*/
WidgetParamPointer::WidgetParamPointer( QWidget *parent ) : QWidget( parent )
{
  QVBoxLayout* lo = new QVBoxLayout();
  mCenterPointer = new wpal::WidgetCenterPointer();

  mLabel = new QLabel( "" );
  lo->addWidget( mCenterPointer );
  lo->addWidget( mLabel );
  lo->addStretch();
  setLayout( lo );
}

void WidgetParamPointer::SetupInfo( const mfg::ParamInfo& pinfo )
{
  mLabel->setText( QString::fromStdString( pinfo.GetLabel() ) );
}

void WidgetParamPointer::ApplyTo( mfg::ParamInfo& pinfo )
{
  assert( pinfo.GetParamType() == mfg::ParamInfo::POINTER_FLOAT32_XY );
  pinfo.AssignXY( (float)mCenterPointer->mMx, (float)mCenterPointer->mMy );
}

/*
  WidgetParamColorPicker
*/
WidgetParamColorPicker::WidgetParamColorPicker( QWidget *parent ) : QWidget( parent )
{
  QVBoxLayout* lo = new QVBoxLayout();
  mColorPicker = new wpal::WidgetColorPicker();
  lo->addWidget( mColorPicker );

  setLayout( lo );
}

void WidgetParamColorPicker::SetupInfo( const mfg::ParamInfo& pinfo )
{
  mColorPicker->SetLabel( QString::fromStdString( pinfo.GetLabel() ) );
  mColorPicker->SetColor( ToBpp32( pinfo.GetBGRA() ) );
}

void WidgetParamColorPicker::ApplyTo( mfg::ParamInfo& pinfo )
{
  assert( pinfo.GetParamType() == mfg::ParamInfo::COLOR_PICKER_FLOAT32_V4 );

  mfg_pal::TBpp32 col = mColorPicker->Color();
  float b = ((float)col.B)/255.0F;
  float g = ((float)col.G)/255.0F;
  float r = ((float)col.R)/255.0F;
  float a = ((float)col.A)/255.0F;
  pinfo.AssignBGRA( b, g, r, a );
}

/*
  WidgetMFG関連
*/


WidgetMFG::WidgetMFG( MFGTarget&& target, QWidget *parent) : QWidget(parent), mTarget( std::move(target) ), mBinary( nullptr ), mNextBinary( nullptr )
{
  Init();
}

void WidgetMFG::LoadBinary( mfg::MFGBinary&& binary )
{
  if (mProcessing)
  {
    // 現在処理中。処置が終わったあとにセットする。
    mNextBinary = std::move(binary);
  }
  else
  {
    mBinary = std::move(binary);
    ApplyBin();
  }
}

void WidgetMFG::LoadScript(  const std::string& script, std::unique_ptr<mfg_parser::ResourceLoader> loader, mfg::ResStringMap &&smap )
{
  LoadBinary( mTarget.CompileScript( script, std::move(loader), std::move(smap) ) );
}

void WidgetMFG::LoadMAR( const QString& zipPath )
{
  LoadBinary( mTarget.CompileZip( zipPath ) );
}

void WidgetMFG::InvalidatePreview()
{
  mStalePreview = true;
  mLastChange = mfg_pal::Tick();
}

void WidgetMFG::Init()
{
  // レイアウトを初期化する。
  // ただしこの時点ではMFGBinaryに依存しない共通の初期化だけにし、
  // MFGBinaryに依存するものは初期化しない。
  // MFGBinaryに依存するものはmParamHLayoutの下に追加される。

  mParamHLayout = new QHBoxLayout();
  // Show Preview
  mCheckPreview = new QCheckBox( Str(3) );
  mCheckPreview->setChecked( true );

  mButtonUpdateRand = new QPushButton( MStr(57) );
  connect( mButtonUpdateRand, &QPushButton::clicked, [this](){
    if (mBinary.IsEmpty())
      return;
    mBinary._irBinary->NewSeed();
    InvalidatePreview();
  } );

  connect( mCheckPreview, SIGNAL(clicked(bool)), this, SLOT(onPreviewChecked(bool)) );

  // Layout

  QVBoxLayout* lor = new QVBoxLayout();

  lor->addLayout( mParamHLayout );

  // セパレータ
  QFrame *horizontalLine = new QFrame();
  horizontalLine->setFrameShape(QFrame::HLine); // 水平線
  horizontalLine->setFrameShadow(QFrame::Sunken); // 凹んだ見た目
  lor->addWidget( horizontalLine );

  QHBoxLayout* loDefWidget = new QHBoxLayout();

  loDefWidget->addWidget( mCheckPreview );
  loDefWidget->addWidget( mButtonUpdateRand );
  lor->addLayout( loDefWidget );

  setLayout( lor );

  mFirstShow = true;
  mProcessing = false;
  mStalePreview = true;

  // MFG_Preview 用意
  mTarget.PreviewInit();
}

static void ClearChildren( QLayout* layout )
{
  while(true)
  {
    // takeAtはunchainもするっぽい。自分でdeleteせい、との事。
    QLayoutItem *item = layout->takeAt( 0 );
    if (item == nullptr)
      return;
    
    if (item->layout() != nullptr)
    {
      ClearChildren( item->layout() );
    }
    else if(item->widget() != nullptr)
    {
      // item->layoutはキャストだがitem->widget()は別のポインタで別々にdeleteしないといけないっぽい。
      // https://doc.qt.io/qt-6/qlayoutitem.html#widget
      delete item->widget();
    }
    delete item;
  }
}

template<typename T>
void SetupAndAdd( QVBoxLayout* vbox, std::vector<IWidgetParam*>& widgetVec,  T* pwidget, size_t pidx, const mfg::ParamInfo& pinfo, int& minHeight )
{
  pwidget->SetupInfoAndIndex( pidx, pinfo );
  widgetVec.push_back( pwidget );
  vbox->addWidget( pwidget );
  minHeight += pwidget->sizeHint().height();
}

void WidgetMFG::ApplyBin()
{
  ClearChildren( mParamHLayout );
  mParamWidgets.clear();

  QVBoxLayout* vbox = new QVBoxLayout();

  auto& pinfo = mBinary.GetParamInfo();
  const int vspacing = vbox->spacing();
  int minHeight = vspacing;

  for (auto pidx : mfg_pal::NRange(pinfo.size()))
  {
    auto& one = pinfo[pidx];

    switch(one.GetParamType())
    {
      case mfg::ParamInfo::SLIDER_INT32:
      case mfg::ParamInfo::SLIDER_FLOAT32:
      {
        auto slider = new WidgetParamSlider();
        connect( slider->mSlider, &QSlider::valueChanged, [this](int) {
          InvalidatePreview();
        });
        SetupAndAdd( vbox, mParamWidgets, slider, pidx, one, minHeight );
        break;
      }
      case mfg::ParamInfo::CHECKBOX_INT32:
      {
        auto checkBox = new WidgetParamCheckBox();
        connect( checkBox->mCheckBox, &QCheckBox::clicked, [this](){
          InvalidatePreview();
        });
        SetupAndAdd( vbox, mParamWidgets, checkBox, pidx, one, minHeight );
        break;
      }
      case mfg::ParamInfo::DROPDOWN_INT32:
      {
        auto dropdown = new WidgetParamDropdown();
        connect( dropdown->mComboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [this](int){
          InvalidatePreview();
        });
        SetupAndAdd( vbox, mParamWidgets, dropdown, pidx, one, minHeight );
        break;
      }
      case mfg::ParamInfo::COLOR_PICKER_FLOAT32_V4:
      {
        auto picker = new WidgetParamColorPicker();
        connect( picker->mColorPicker,  &wpal::WidgetColorPicker::colorChanged, [this](){
          InvalidatePreview();
        });
        SetupAndAdd( vbox, mParamWidgets, picker, pidx, one, minHeight );
        break;
      }
      case mfg::ParamInfo::DIRECTION_FLOAT32:
      {
        auto direction = new WidgetParamDirection();

        // なぜかDialogMFGListでは80くらい足りない。（なんで？） #1751
        minHeight = std::max( minHeight, vspacing+direction->sizeHint().height() + qt_pal::MagInt(80) );
        direction->SetupInfoAndIndex( pidx, one );
        direction->mDirection->mUpdateListener = [this]( double ) { InvalidatePreview(); };

        mParamWidgets.push_back( direction );
        mParamHLayout->addWidget( direction );
        break;
      }
      case mfg::ParamInfo::POINTER_FLOAT32_XY:
      {
        auto pointer = new WidgetParamPointer();
        minHeight = std::max( minHeight, vspacing+pointer->sizeHint().height() );
        pointer->SetupInfoAndIndex( pidx, one );
        pointer->mCenterPointer->mUpdateListener = [this]() { InvalidatePreview(); };

        mParamWidgets.push_back( pointer );
        mParamHLayout->addWidget( pointer );
        break;
      }
    }
  }
  mParamHLayout->addLayout( vbox );
  setMinimumHeight( minHeight );

  InvalidatePreview();
}

void WidgetMFG::InitAndApply()
{
  Init();
  ApplyBin();
}

QString WidgetMFG::GetTitle() const
{
  return QString::fromStdString( mBinary.GetTitle() );
}

QString WidgetMFG::Str( int idx ) const
{
  // 翻訳済みの文字列を取得
  return Trans_StrFilter( 500 + idx );
}

// MFGのtransファイルから。
QString WidgetMFG::MStr( int idx ) const
{
  return Trans_StrMFG( idx );
}

void WidgetMFG::ShowEvent()
{
  if (mFirstShow)
  {
    mFirstShow = false;
    mTimer = startTimer( 100 );

    // 初回だけはすぐに更新されるように100msec待たずにBuildPreviewを走らせる。
    QTimer::singleShot(0, [this]{
      mStalePreview = false;
      qt_pal::cursor_t old = qt_pal::BeginWaitCursor();
      BuildPreview();
      qt_pal::EndWaitCursor( old );
    });
  }
  else
  {
    emit updatePreview();
  }
}

void WidgetMFG::HideEvent()
{
  // プレビューの処理の途中に閉じられた、みたいな異常事態
  if (mProcessing) {
    MFGTarget::CancelProgress();
  }
}

void WidgetMFG::timerEvent( QTimerEvent* event )
{
  if (event->timerId() == mTimer)
  {
    size_t dif = mfg_pal::Tick() - mLastChange;
    if ((dif > 500) && mStalePreview)
    {
      mStalePreview = false;
      qt_pal::cursor_t old = qt_pal::BeginWaitCursor();
      BuildPreview();
      qt_pal::EndWaitCursor( old );
    }
  }
}

void WidgetMFG::onPreviewChecked( bool b )
{
  Q_UNUSED(b)

  InvalidatePreview();
}

void WidgetMFG::ApplyToPInfos()
{
  auto& pinfo = mBinary.GetParamInfo();

  for( auto iw : mParamWidgets )
  {
    iw->ApplyTo( pinfo[iw->GetParamIndex()] );
  }
}

void WidgetMFG::BuildPreview()
{
  if (mBinary.IsEmpty()) return;
  if (!mCheckPreview->isChecked())
  {
    mTarget.RevertToOriginalPreview();
    emit updatePreview();
    return;
  }

  // この中のQtProgressの中からprocessEventが呼ばれてまたここに来る可能性がある。
  // 以下で再入を防止。スレッド自体はGUIスレッドなのでこれで十分でしょう。
  if (mProcessing)
    return;
 
  // 処理中にOKボタンを押されないようにする。キャンセルはできる。
  mProcessing = true;
  emit startProcessing();

  auto clearGuard = mfg_pal::ScopeGuard([&]{
    mProcessing = false;
    emit endProcessing();

    // 処理中に新しいバイナリがセットされた。やり直し。
    if (!mNextBinary.IsEmpty())
    {
      mBinary = std::move(mNextBinary);
      mNextBinary.Clear();
      ApplyBin();
    }
  });

  ///////////////////////////////
  // 元レイヤーへの MFGプレビュー
  ///////////////////////////////

  ApplyToPInfos();

  // 縮小サイズじゃないのでProgressも処理しておく。
  MFGTarget::BeforeProgressCancelableNoDialog();

  bool success = mTarget.ApplyPreview( mBinary );

  MFGTarget::AfterProgress();
  emit updatePreview();

  if (!success)
  {
    emit canceled();
  }
}

void WidgetMFG::Filter()
{
  if (mBinary.IsEmpty()) return;

  ///////////////////////////////////////////////
  // FireAlpaca の処理
  ///////////////////////////////////////////////
  ApplyToPInfos();

  cursor_t old = BeginWaitCursor();
  MFGTarget::BeforeProgressCancelable();

  mTarget.ApplyFilter( mBinary );

  MFGTarget::AfterProgress();
  EndWaitCursor( old );
}

void WidgetMFG::Connect( QDialog* dialog, QDialogButtonBox* buttonBox )
{
  auto buttonOk = buttonBox->button( QDialogButtonBox::Ok );

  connect( this, &WidgetMFG::startProcessing, this, [buttonOk, this]() {
    buttonOk->setEnabled(false);
  });
  connect( this, &WidgetMFG::endProcessing, this, [buttonOk, this]() {
    buttonOk->setEnabled(true);
  });
  connect( this, &WidgetMFG::canceled, this, [dialog, this]() {
    dialog->reject();
  });
}

/*
  DialogMFG関連
*/

DialogMFG::DialogMFG( MFGTarget&& target, QWidget *parent ) : QDialog(parent), mWidgetMFG( new WidgetMFG( std::move(target) , nullptr ) )
{
  Init();
}

void DialogMFG::Init()
{
  // ？を表示しない
  qt_pal::RemoveQuestion( this );
  QVBoxLayout* lo = new QVBoxLayout();

  lo->addWidget( mWidgetMFG );

  // Ok/Cancel
  QHBoxLayout* lok = new QHBoxLayout();
  QDialogButtonBox* but = new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel );
  lok->addWidget( but );
  lo->addLayout( lok );

  mWidgetMFG->Connect( this, but );
  connect( mWidgetMFG, &WidgetMFG::updatePreview, [this]{
    emit updatePreview();
  } );

  connect( but, SIGNAL(accepted()), this, SLOT(accept()) );
  connect( but, SIGNAL(rejected()), this, SLOT(reject()) );

  connect( this, &QDialog::rejected, [this]{ MFGTarget::CancelProgress(); });
  
  setLayout( lo );
}

void DialogMFG::LoadScript( const std::string& script, std::unique_ptr<mfg_parser::ResourceLoader> loader, mfg::ResStringMap &&smap )
{ 
  mWidgetMFG->LoadScript( script, std::move(loader), std::move(smap) );
  setWindowTitle( mWidgetMFG->GetTitle() );
}

void DialogMFG::LoadMAR( const QString& zipPath )
{ 
  mWidgetMFG->LoadMAR( zipPath );
  setWindowTitle( mWidgetMFG->GetTitle() );
}

void DialogMFG::showEvent( QShowEvent* event )
{
  Q_UNUSED(event)
  qt_pal::InitDialogPos( this, MainAsQWidget() );

  mWidgetMFG->ShowEvent();

}

void DialogMFG::hideEvent( QHideEvent* event )
{
  Q_UNUSED(event)

  mWidgetMFG->HideEvent();
}




