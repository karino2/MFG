/* -*- coding: utf-8 -*- マルチバイト */

// Copyright 2026 Kazuma Arino. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#include "mfg.hpp"
#include "mfg_resource.hpp"
#include <memory>
#include <tuple>
#include <typeinfo>
#include <typeindex>
#include <map>
#include <functional>

/*
  mfg処理系はだいたいはヘッダオンリーだが、まれにcppが必要になるものはここに置く。
  数が少ないのでファイルは分けない。
*/
using namespace mfg_internal;

namespace mfg_internal
{

//
// UniqueName関連
//

// グローバル変数なのでzero初期化される
static std::map<char, int> g_UniqNameCounters;

static int UniqueCount(char prefix )
{
  auto iter = g_UniqNameCounters.find( prefix );
  if (iter == g_UniqNameCounters.end())
  {
    // 初回。0を返して次は1
    g_UniqNameCounters[prefix] = 1;
    return 0;
  }
  return g_UniqNameCounters[prefix]++;
}

std::string UniqueName( char prefix )
{
  return prefix + std::to_string( UniqueCount( prefix ) );
}

void ResetUniqueName()
{
  g_UniqNameCounters.clear();
}

//
// そのほかのstatic
//
int g_irelem_alloc_count = 0;

} ///<mfg_internal

namespace mfg_forest {
int g_node_alloc_count = 0; // これはforestだが、どうせデバッグ目的なのでいいでしょう。
}

// 文字列リソース関連。
// とりあえずここに置く。
namespace mfg_resource
{

static mfg_pal::NLanguage g_currentLang = mfg_pal::NL_ENGLISH;

void SetLanguage( mfg_pal::NLanguage lang )
{
  g_currentLang = lang;
}

mfg_pal::NLanguage GetCurrentLanguage()
{
  return g_currentLang;
}

extern const char* ResourceStringGen( ResId rid, mfg_pal::NLanguage lang );
extern const char* UIResourceStringGen( UIResId rid, mfg_pal::NLanguage lang );

const char* ResourceString( ResId rid )
{
  return ResourceStringGen( rid, g_currentLang );
}

const char* UIResourceString( UIResId rid )
{
  return UIResourceStringGen( rid, g_currentLang );
}


}///< mfg_resource

/*
  外部ライブラリに依存するものを以下に置く。
  picojosn関連(JsonToResStringMap)
*/


/*
  JsonToResStringMap関連
  picojsonはこのレポジトリに含まれているが、コアには依存を含めない、という事でここに隔離。
*/
#include "picojson.h"
#include <map>
#include <string>

namespace mfg {

static std::map<std::string, mfg_pal::NLanguage> g_locLangMap{
  {"zh_Hans", mfg_pal::NL_CHINESE_SIMP},
  {"zh_Hant", mfg_pal::NL_CHINESE_TRAD},
  {"en", mfg_pal::NL_ENGLISH},
  {"fr", mfg_pal::NL_FRENCH},
  {"de", mfg_pal::NL_GERMAN},
  {"ja", mfg_pal::NL_JAPANESE},
  {"ko", mfg_pal::NL_KOREAN},
  {"pt", mfg_pal::NL_PORTUGUESE},
  {"ru", mfg_pal::NL_RUSSIAN},
  {"es", mfg_pal::NL_SPANISH},
  {"hi", mfg_pal::NL_HINDI},
  {"bn", mfg_pal::NL_BENGALI},
  {"pl", mfg_pal::NL_POLISH},
};

/*
  Localeの文字列（"ja"とか)からNLanguageを返す。
  NLanguageStringの反対。
  存在しない場合は例外をthrow
*/
mfg_pal::NLanguage StringToNLanguage( const std::string& loc )
{
  auto iter = g_locLangMap.find(loc);
  if (iter == g_locLangMap.end())
    throw FmtUserError(ResId::UNKNOWN_LOCALE, loc );
  return iter->second;
}

/*
  MEP 25:  mar内での国際化
  https://github.com/karino2/MFG/blob/main/docs/ja/MEP/25.md

  のフォーマットのjsonをResStringMapにして返す。
  知らないロケールは例外にする。
*/
ResStringMap JsonToResStringMap( const std::string& json )
{
  picojson::value v;
  const std::string err = picojson::parse( v, json );

  if (!err.empty())
    throw FmtUserError( ResId::INVALID_JSON, err );

  if (!v.is<picojson::object>())
    throw FmtUserError( ResId::ROOT_IS_NOT_OBJECT );

  picojson::object& root = v.get<picojson::object>();

  ResStringMap ret;
  for (picojson::object::const_iterator iter = root.begin(); iter != root.end(); ++iter)
  {
    /*
      以下を期待

      iter->first: "ja"
      iter->second: {"MY_TITLE": "日本語タイトル", "STRENGTH_LABEL": "強さ"}
    */
    auto nlang = StringToNLanguage( iter->first );
    if (!iter->second.is<picojson::object>())
      throw FmtUserError( ResId::NONE_OBJECT_FOR_EACH_LANG );

    const picojson::object& smapObj = iter->second.get<picojson::object>();
    std::map<std::string, std::string> smap;
    for (picojson::object::const_iterator siter = smapObj.begin(); siter != smapObj.end(); ++siter)
    {
      /*
        siter->first: キー
        siter->second: 文字列
      */
      if (!siter->second.is<std::string>())
        throw FmtUserError( ResId::VALUE_OF_KEYVALUE_IS_NOT_STRING );

      smap.insert( {siter->first, siter->second.get<std::string>()} );
    }
    ret.insert( { nlang, std::move(smap) });
  }
  return ret;
}

}///< mfg_resource