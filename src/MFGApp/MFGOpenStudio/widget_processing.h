/* -*- coding: utf-8 -*- マルチバイト */

// Copyright 2026 PGN Inc. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#ifndef WIDGET_PROCESSING_H
#define WIDGET_PROCESSING_H

#include <QWidget>
#include <QProgressBar>
#include <QDialog>

#include <qt_pal/qt_pal.h>

/*
  時間がたったら出す時に、キャンセラブルにするために出すダイアログ。
  ユーザーインプットはブロックしたいのでダイアログにする必要がある。
*/
class CancelDialog : public QDialog
{
  Q_OBJECT
public:
  explicit CancelDialog(QWidget *parent = 0);
signals:
  void canceled();  
};

/*
  キャンセラブルの何種類かの組み合わせの管理

  - Cancelableかどうか
  - 時間がたったらダイアログを自動で出すか出さないか
  - すでにダイアログを出したか
  - キャンセルが選ばれたか
*/
struct CancelState
{
  bool mCancelable = false;
  bool mWithDialog = false;
  bool mDialogShowing = false;
  bool mCanceled = false;

  void Clear()
  {
    mCancelable = false;
    mWithDialog = false;
    mDialogShowing = false;
    mCanceled = false;
  }

  void Enable( bool withDialog )
  {
    mCancelable = true;
    mWithDialog = withDialog;
  }

  bool MayShowDialog()
  {
    return mWithDialog && !mDialogShowing;
  }

  void DialogShowing()
  {
    mDialogShowing = true;
  }

  bool IsCanceled() const { return mCanceled; }
  void DoCancel() { mCanceled = true; }

  bool IsModalShown() const
  {
    if (!mCancelable)
      return false;

    // ダイアログは呼び出し側が出しているケース
    if (!mWithDialog)
      return true;
    
    // WidgetProcessingがキャンセルダイアログを出しているケース
    if (mDialogShowing)
      return true;

    return false;
  }
};


class WidgetProcessing : public QWidget
{
  Q_OBJECT
public:
  explicit WidgetProcessing(QWidget *parent = 0);

  QProgressBar* mProgress;
  CancelDialog* mCancelDialog;
  

  size_t mProgressBegin; // 開始時間
  bool mProcessing; // 処理中？

  // キャンセラブル関連
  CancelState mCancelState;
  bool IsCanceled() const { return mCancelState.IsCanceled(); }  
  void DoCancel() { mCancelState.DoCancel(); }
  void EnableCancel( bool withDialog ) { mCancelState.Enable(withDialog); }

  // Processing関連
  void StartProcessing();
  void EndProcessing();
  // 1000ms以上経って表示されてないなら、表示しよう
  void MayVisibleLayter();

  void ProcessEvent();

signals:
public slots:
};

// プログレス表示
bool QtProgress( qt_pal::CallbackInfo info );

void BeforeQtProgress();
void BeforeQtProgressVisible();

/*
  キャンセル可能なプログレス関連。
  モーダルなダイアログをすでに出していて裏で回しているケースと、ダイアログを出していないケースの二種類がある。

  キャンセルは次回のQtProgressがfalseを返すようになるだけなので、それを見てちゃんと終了するのはTPgoressCallbackを処理している側の責任。
*/
// 時間が来たらキャンセルダイアログを自動で出す
void BeforeQtProgressCancelable();
// キャンセルダイアログはProgressは出さない（呼び出し元がすでにモーダルダイアログを出しているケース）
void BeforeQtProgressCancelableNoDialog();
// プログレスの途中で外部からキャンセルしたい場合に呼ぶ。自前のモーダルダイアログのキャンセルボタンなどが押された場合の処理。
void CancelProgress();


void AfterQtProgress();

#endif // WIDGET_PROCESSING_H
