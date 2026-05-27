/* -*- coding: utf-8 -*- マルチバイト */

// Copyright 2026 Kazuma Arino. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#ifndef MAR_UTIL_HPP_
#define MAR_UTIL_HPP_

/*
  アプリ側で使う便利クラスのあるヘッダ。
  コア側からincludeはされていない。

  MARファイルをVerifyしたりタイトルを取り出したりなどするのに使う。
*/

#include <mfg_pal/pal.hpp>
#include "mfg_facade.hpp" // EndsWith, FEReadText, etc.
#include "cppunzip.hpp"
#include "mfg_resource_expander.hpp"
#include <memory>
#include <string>
#include <stdexcept>

namespace mar_util
{
namespace uz = cppunzip;
using namespace mfg_pal;
using mfg_resource_expander::AttrScanner;
using mfg_resource_expander::MFGString;

using mfg::EndsWith;
using mfg::FEReadText;

template<typename T>
uz::file_entry_iterator FindFile( uz::FileEntryLister& lister,  T pred )
{
  auto iter = lister.begin();
  while( iter != lister.end() )
  {
    if (pred( *iter ))
      return iter;
    ++iter;
  }
  return lister.end();
}

inline uz::file_entry_iterator FindFirstSuffixFile( uz::FileEntryLister& lister,  const std::string& suffix )
{
  return FindFile( lister, [&suffix]( uz::FileEntry& fent ){ return EndsWith( fent.fileName(), suffix );} );
}

inline uz::file_entry_iterator FindFileByName( uz::FileEntryLister& lister,  const std::string& fname )
{
  // ファイル名の前にディレクトリ名がつく事があるので、結局FindFirstSuffixFileと同じになってしまった。
  return FindFirstSuffixFile( lister, fname );
}

/*
  strings.jsonなどが無くても初期化は可能。
  リソースのlookupが必要な時に初めてファイルがあるかをチェックする。
*/
struct MFGStringResolver
{
  uz::FileEntryLister& _lister;
  // resMapはlazyにloadする。
  // strings.jsonは無い場合も多いので、ある事を前提とはしない。
  bool _mapLoaded = false;
  mfg_parser::ResStringMap _resMap;

  MFGStringResolver( uz::FileEntryLister& lister ) : _lister( lister ) {}

  // これはリソースが使われた場合のみ呼ばれる。
  void ensureLoad()
  {
    if (_mapLoaded)
      return;
    
    _mapLoaded = true;
    auto jsoniter = FindFileByName( _lister, mfg::RES_STRING_FNAME );

    // リソースが使われているがリソースのjsonが入っていない。
    // これはlookupと同じexception のmsg idを使っているが、まぁいいだろう。
    if (jsoniter == _lister.end())
      throw mfg_internal::FmtUserError( mfg_internal::ResId::RESOURCE_NOT_FOUND, mfg::RES_STRING_FNAME );


    auto jsonstr = FEReadText( *jsoniter );
    _resMap = mfg::JsonToResStringMap( jsonstr );
  }

  std::string Resolve( const MFGString& mstring )
  {
    switch(mstring._type)
    {
      case MFGString::STRING:
        return std::move(mstring._stringVal);
      case MFGString::RESID:
      {
        ensureLoad();
        mfg_parser::LocalResMap resolver( _resMap );

        std::string res;
        if (resolver.Lookup( mstring._stringVal, res))
          return res;
        // 見つからない。
        throw mfg_internal::FmtUserError( mfg_internal::ResId::RESOURCE_NOT_FOUND, mstring._stringVal );
      }
      break;
      case MFGString::NOT_FOUND:
        return "";
    }
  }

  // UserErrorのexceptionは""を返すバージョン。
  std::string ResolveNoEx( const MFGString& mstring )
  {
    try
    {
      return Resolve( mstring );
    }
    catch ( const mfg_internal::UserError& )
    {
      return "";
    }
  }
};

struct MARFile
{
  std::unique_ptr<uz::File> _file;
  uz::UnZipper _unzipper;
  uz::FileEntryLister _lister;
  MFGStringResolver _resolver;

  // listFilesはbeginの状態でiteratorを初期化するだけなのでzipが壊れていなければ無害。
  MARFile( const nstring& path ) : _file( mfg_pal::GetZipFile( path ) ), _unzipper( *_file ), _lister( _unzipper.listFiles() ), _resolver( _lister ) {}

  uz::file_entry_iterator FindMFGFile() { return FindFirstSuffixFile( _lister, ".mfg" ); }
  uz::file_entry_iterator FindThumbnailFile() { return FindFileByName( _lister, mfg::THUMBNAIL_FNAME ); }

  uz::file_entry_iterator FileEnd() const { return _lister.end(); }
  std::string ResolveNoEx( const MFGString& mstr ) { return _resolver.ResolveNoEx( mstr ); }
};

/*
  UIで使うMARアーカイブの情報。
  @versionや@titleなどをリソースを解決した状態で取り出したもの。
*/
struct MARInfo
{
  std::string _title;
  std::string _version;
};

/*
  アプリから使うMARファイルを扱う便利クラス。
  Verifyしたりタイトルを取り出したり。
*/
struct MARUtil
{
  /*
    パスで示されているmarファイルが壊れていないかの確認。

    とりあえず以下くらいをチェック。
    - zipとしてfile entryの列挙が出来る
    - その中にmfgファイルが入っている

    mfgファイルの中はチェックしない。
  */
  static bool Verify( const nstring& path )
  {
    try
    {
      MARFile mfile( path );

      // mfgのファイルがあればvalid
      auto iter = mfile.FindMFGFile();
      return iter != mfile.FileEnd();
    }
    catch( uz::UnZipError& )
    {
      return false;
    }
  }

  /*
    UIから使いそうなMARのattrから作る情報をまとめて返す。
    エラーの場合は空文字を入れておく。    
  */
  static MARInfo LookupInfo( const nstring& path )
  {
    MARInfo info;
    try
    {
      MARFile mfile( path );

      auto iter = mfile.FindMFGFile();

      // verifyしていれば無いはずだが一応
      if (iter == mfile.FileEnd())
        return info;

      std::string source = FEReadText( *iter );

      AttrScanner scanner(source);
      std::vector<std::string> attrNames { "title", "version" };

      auto attrMap = scanner.ScanStringAttrList( attrNames );
      for( const auto& pair: attrMap )
      {
        if(pair.first == "title")
        {
          info._title = mfile.ResolveNoEx( pair.second );
        }
        else if(pair.first == "version")
        {
          info._version = mfile.ResolveNoEx( pair.second );
        }
      }

      return info;
    }
    catch( uz::UnZipError& )
    {
      // Verifyされているならここには来ないはず。
      // どこかでファイルが壊れた時などの特殊なケースのみ。
      return info;
    }
    catch( std::runtime_error& )
    {
      // スキャン失敗。この場合お扱いも悩ましいがとりあえず空タイトルとしておく。
      return info;
    }
    
    return info;
  }

  // タイトルをとりだす、
  // エラーの場合は""を返す事にする。
  static std::string LookupTitle( const nstring& path )
  {
    auto info = LookupInfo( path );
    return info._title;
  }

  // marに含まれているサムネイルのpngファイル（thumbnail.png）のバイト列を返す
  // なければreturnのvectoryは空。
  static std::vector<uint8_t> LookupThumbnail( const nstring& path )
  {
    try
    {
      MARFile mfile( path );

      auto iter = mfile.FindThumbnailFile();

      // verifyしていれば無いはずだが一応
      if (iter == mfile.FileEnd())
        return {};

      return (*iter).readContent();
    }
    catch( uz::UnZipError& )
    {
      // Verifyされているならここには来ないはず。
      // どこかでファイルが壊れた時などの特殊なケースのみ。
      return {};
    }
    catch( std::runtime_error& )
    {
      // スキャン失敗。この場合お扱いも悩ましいがとりあえず空タイトルとしておく。
      return {};
    }
  }

};



}///< mar_util
#endif

