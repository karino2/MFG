/* -*- coding: utf-8 -*- マルチバイト */

// Copyright 2026 PGN Inc. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#ifndef OPEN_TRANS_H
#define OPEN_TRANS_H

#include <mfg_pal/pal.hpp>
#include <QLocale>

///////////////////////////////////////////////////////////////////////////
class CTranslationManager
{
public:
  static QLocale LanguageToLocale( mfg_pal::NLanguage lang );

public:
  CTranslationManager();

  QString StrParam();

  // 各言語
  bool TranslateJapanese();
  bool TranslateChineseSimp();
  bool TranslateChineseTrad();
  bool TranslateKorean();
  bool TranslatePortugues();
  bool TranslateSpanish();
  bool TranslateGerman();
  bool TranslateFrench();
  bool TranslateRussian();
  bool TranslateHindi();
  bool TranslateBengali();
  bool TranslatePolish();

  // 切り替える？
  bool DoTranslate();
  void SetDoTranslate( bool b );

  // 言語設定 (デフォは英語)
  mfg_pal::NLanguage Language();
  QString LocaleStr();
  QLocale Locale();
  void SetLanguage( mfg_pal::NLanguage lang );

private:
  mfg_pal::NLanguage mLanguage;
  bool mDoTranslate;
  QString mParamStr;
};

CTranslationManager& Trans();

#endif // OPEN_TRANS_H
