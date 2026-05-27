/* -*- coding: utf-8 -*- マルチバイト */

// Copyright 2026 PGN Inc. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#if defined(_MSC_VER)
  #pragma execution_character_set("utf-8")
#endif

#include <trans.h>

using namespace mfg_pal;

///////////////////////////////////////////////////////////
CTranslationManager* gTrans = NULL;
CTranslationManager& Trans()
{
  if (gTrans == NULL)
  {
    // 最初に使う時だけ初期化
    gTrans = new CTranslationManager();
  }

  return *gTrans;
}

///////////////////////////////////////////////////////////
CTranslationManager::CTranslationManager()
{
  // 基本は切り替える
  mDoTranslate = true;

  // 基本は英語
  mLanguage = NL_ENGLISH;
}

static std::string NLanguageString( NLanguage lang )
{
  std::string s;
  switch(lang)
  {
  case NL_CHINESE_SIMP: s = "zh_Hans"; break;
  case NL_CHINESE_TRAD: s = "zh_Hant"; break;
  case NL_ENGLISH: s = "en"; break;
  case NL_FRENCH: s = "fr"; break;
  case NL_GERMAN: s = "de"; break;
  case NL_JAPANESE: s = "ja"; break;
  case NL_KOREAN: s = "ko"; break;
  case NL_PORTUGUESE: s = "pt"; break;
  case NL_RUSSIAN: s = "ru"; break;
  case NL_SPANISH: s = "es"; break;
  case NL_HINDI: s = "hi"; break;
  case NL_BENGALI: s = "bn"; break;
  case NL_POLISH: s = "pl"; break;
  }
  return s;
}

QLocale CTranslationManager::LanguageToLocale( NLanguage lang )
{
  std::string s = NLanguageString( lang );
  QString qs = QString::fromStdString( s );
  QLocale l( qs );
  return l;
}

bool CTranslationManager::TranslateJapanese()
{
  if (!DoTranslate()) return false;
  if (mLanguage == NL_JAPANESE) return true;
  return false;
}

bool CTranslationManager::TranslateChineseSimp()
{
  if (!DoTranslate()) return false;
  if (mLanguage == NL_CHINESE_SIMP) return true;
  return false;
}

bool CTranslationManager::TranslateChineseTrad()
{
  if (!DoTranslate()) return false;
  if (mLanguage == NL_CHINESE_TRAD) return true;
  return false;
}

bool CTranslationManager::TranslateKorean()
{
  if (!DoTranslate()) return false;
  if (mLanguage == NL_KOREAN) return true;
  return false;
}

bool CTranslationManager::TranslateSpanish()
{
  if (!DoTranslate()) return false;
  if (mLanguage == NL_SPANISH) return true;
  return false;
}

bool CTranslationManager::TranslatePortugues()
{
  if (!DoTranslate()) return false;
  if (mLanguage == NL_PORTUGUESE) return true;
  return false;
}

bool CTranslationManager::TranslateGerman()
{
  if (!DoTranslate()) return false;
  if (mLanguage == NL_GERMAN) return true;
  return false;
}

bool CTranslationManager::TranslateFrench()
{
  if (!DoTranslate()) return false;
  if (mLanguage == NL_FRENCH) return true;
  return false;
}

bool CTranslationManager::TranslateRussian()
{
  if (!DoTranslate()) return false;
  if (mLanguage == NL_RUSSIAN) return true;
  return false;
}

bool CTranslationManager::TranslateHindi()
{
  if (!DoTranslate()) return false;
  if (mLanguage == NL_HINDI) return true;
  return false;
}

bool CTranslationManager::TranslateBengali()
{
  if (!DoTranslate()) return false;
  if (mLanguage == NL_BENGALI) return true;
  return false;
}

bool CTranslationManager::TranslatePolish()
{
  if (!DoTranslate()) return false;
  if (mLanguage == NL_POLISH) return true;
  return false;
}

///////////////////////////////////////////////////////////
bool CTranslationManager::DoTranslate()
{
  return mDoTranslate;
}

void CTranslationManager::SetDoTranslate( bool b )
{
  mDoTranslate = b;
}

///////////////////////////////////////////////////////////
NLanguage CTranslationManager::Language()
{
  return mLanguage;
}

QString CTranslationManager::LocaleStr()
{
  std::string s = NLanguageString( mLanguage );
  return QString::fromStdString( s );
}

QLocale CTranslationManager::Locale()
{
  return LanguageToLocale( mLanguage );
}

void CTranslationManager::SetLanguage( NLanguage lang )
{
  mLanguage = lang;
}

QString CTranslationManager::StrParam()
{
  return mParamStr;
}
