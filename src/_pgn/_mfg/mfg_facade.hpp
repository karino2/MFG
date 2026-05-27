/* -*- coding: utf-8 -*- マルチバイト */

// Copyright 2026 Kazuma Arino. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#ifndef MFG_FACADE_HPP_
#define MFG_FACADE_HPP_

#include <mfg_pal/pal.hpp>
#include "mfg_runtime_common.hpp"
#include "mfg_parser.hpp"

#include "cppunzip.hpp"
#include <istream>

//
// インテグレート用facadeなどを置く場所。
// 外部に公開するAPIはここに置く。
//
// BACKENDはD3DBackendかMetalBackend。
//
namespace mfg {
namespace uz = cppunzip;
using namespace mfg_pal;

inline bool EndsWith( const std::string& target, const std::string& suffix )
{
  if (target.length() < suffix.length())
    return false;

  return target.rfind( suffix ) == (target.length() - suffix.length());
}

inline std::string FEReadText( uz::FileEntry& fe )
{
  std::vector<uint8_t> utf8source = fe.readContent();
  return std::string( (char*)utf8source.data(), utf8source.size() );
}


struct ZipLoader: mfg_parser::ResourceLoader
{
  std::unique_ptr<uz::File> _zipFile;
  uz::UnZipper _unzipper;
  std::map<std::string, uz::FileEntry> _fileMap;

  // Windows対応がいるかも。
  std::string EraseDirName( std::string fname )
  {
    auto pos = fname.find('/');
    if (pos != fname.npos)
    {
      fname.erase(0,  pos+1);
    }
    return fname;
  }

  ZipLoader( std::unique_ptr<uz::File>&& zipFile ) : _zipFile(std::move(zipFile)), _unzipper(*_zipFile)
  {
    for( const auto& entry : _unzipper.listFiles() )
    {
      if (!entry.isDir())
      {
        auto fname = EraseDirName( entry.fileName() );
        _fileMap.insert( { fname, entry } );
      }
    }
  }

  bool FileExists( const std::string& name )
  {
    return _fileMap.end() != _fileMap.find( name );
  }

  std::vector<uint8_t> LoadContent( const std::string& name )
  {
    auto iter = _fileMap.find( name );
    if( iter == _fileMap.end() )
      throw mfg_internal::FmtUserError( mfg_internal::ResId::RESOURCE_NOT_FOUND, name );

    return iter->second.readContent();
  }

  std::string LoadText( const std::string& name )
  {
    std::vector<uint8_t> buf = LoadContent( name );
    return std::string( (char*)buf.data(), buf.size() );
  }

  std::string LoadFirstScript()
  {
    for( auto& kv: _fileMap )
    {
      if(EndsWith( kv.first, ".mfg" ))
      {
        return FEReadText( kv.second );
      }
    }
    throw mfg_internal::FmtUserError( mfg_internal::ResId::SCRIPT_NOT_FOUND );
  }

  bool LoadImage( const std::string& name, mfg_pal::Image32& dest ) override
  {
    try
    {
      std::vector<uint8_t> buf = LoadContent( name );
      return mfg_pal::DecodePng32( buf, dest );
    }
    catch( mfg_internal::UserError& )
    {
      return false;
    }
  }
};

/*
  "ja"などのlocale stringからNLanguageを返す。UnitTestのためpublicにしている。
  本来はlibneet側に入れるべきだがここでしか使ってないのでここに置いておく。
*/
mfg_pal::NLanguage StringToNLanguage( const std::string& loc );

using mfg_parser::ResStringMap;
/*
  MEP 25:  mar内での国際化
  https://github.com/karino2/MFG/blob/main/docs/ja/MEP/25.md

  のフォーマットのjsonをResStringMapにして返す。
  知らないロケールは例外にする。
*/
ResStringMap JsonToResStringMap( const std::string& json );


/*
  MFGBinary関連
*/
namespace internal = mfg_internal;
namespace common = mfg_runtime_common;

using common::IRBinary;
using common::ParamInfo;
using common::SParamInfo;
using common::RVal;

using internal::InputsInfo;
using common::InputTiles;
using common::TileReference;

using common::PerfInfo;

// パースした結果のバイナリ。
// ほとんどIRBinaryと同じだが、外部とのAPI向けのfacade。
struct MFGBinary
{
  std::unique_ptr<IRBinary> _irBinary;

  MFGBinary( IRBinary* binary ) : _irBinary( binary ) {}
  MFGBinary( MFGBinary&& ) = default;
  MFGBinary &operator=( MFGBinary&& ) = default;

  bool IsEmpty() const { return _irBinary.get() == nullptr; }
  void Clear() { _irBinary.reset( nullptr ); }

  // フィルタのタイトル
  const std::string& GetTitle() const
  {
    return _irBinary->_title;
  }

  // ダイアログなどでユーザーに設定してもらうパラメータ
  std::vector<ParamInfo>& GetParamInfo()
  {
    return _irBinary->_params;
  }

  // EngineのステータスからUIに依存せずに設定できるパラメータ
  std::vector<SParamInfo>& GetSParamInfo()
  {
    return _irBinary->_sparams;
  }


  // Objective-Cなどからのラッパーのために、void*として持ち回るためのAPI。
  // この戻りの結果はdeleteする必要がある。
  MFGBinary* DetachAsNew()
  {
    return new MFGBinary( _irBinary.release() );
  }
};

constexpr const char* RES_STRING_FNAME = "strings.json";
constexpr const char* THUMBNAIL_FNAME = "thumbnail.png";

template<typename BACKEND>
struct MFGFacade
{
  std::ostringstream _logStream;
  bool _debug = false;
  mfg::PerfInfo _perfInfo;
  const mfg::PerfInfo& GetPerfInfo() const { return _perfInfo; }

  void EnableDebugLog( bool enable ) { _debug = enable; }

  ResStringMap LoadResString( ZipLoader& loader )
  {
    if (!loader.FileExists( RES_STRING_FNAME ))
      return ResStringMap();

    std::string jsonstr = loader.LoadText( RES_STRING_FNAME );

    try
    {
      return JsonToResStringMap( jsonstr );
    }
    catch ( const mfg_internal::UserError& jerr )
    {
      throw mfg_internal::FmtUserError( mfg_resource::ResId::STRINGS_JSON_PARSE_ERROR, std::string( jerr.what() ) );
    }
  }

  MFGBinary Compile( std::istream& zipFile )
  {
    std::unique_ptr<ZipLoader> loader( new ZipLoader( std::unique_ptr<uz::File>( new uz::IStreamFile( zipFile ) ) ) );
    return Compile( loader );
  }

  MFGBinary CompileZip( const nstring& filepath )
  {
    std::unique_ptr<ZipLoader> loader( new ZipLoader( mfg_pal::GetZipFile( filepath ) ) );
    return Compile( loader );
  }


  MFGBinary Compile( std::unique_ptr<ZipLoader>& loader )
  {
    auto source = loader->LoadFirstScript();

    auto rmap = LoadResString( *loader );

    internal::ResetUniqueName();
    mfg_parser::TreeBuilder builder;
    builder.SetLoader( std::move(loader) );
    mfg_parser::Parser parser( builder, source );
    parser._localStrMap = std::move(rmap);

    parser.ParseAll();
    builder._binary->LowerAndAnalyze();
    return MFGBinary( builder.DetachEntityHolder() );    
  }

  MFGBinary Compile( std::string source, std::unique_ptr<mfg_parser::ResourceLoader> loader, ResStringMap&& smap )
  {
    internal::ResetUniqueName();
    mfg_parser::TreeBuilder builder;
    if (loader.get() != nullptr)
      builder.SetLoader( std::move(loader) );
    mfg_parser::Parser parser( builder, source );
    parser._localStrMap = std::move(smap);

    parser.ParseAll();
    builder._binary->LowerAndAnalyze();
    return MFGBinary( builder.DetachEntityHolder() );    
  }

  MFGBinary Compile( std::string source )
  {
    return Compile( source, {}, {} );
  }

  // コンパイルしたバイナリを実行するメソッド。
  // キャンセルされたらfalse、正常終了すればtrueを返す。
  bool Run( MFGBinary& bin, InputTiles& inputTiles, TileReference& resRef, mfg_pal::TProgressCallback callback = nullptr )
  {
    // 現状は毎回作る。
    BACKEND backend;
    backend._debug = _debug;
    SetupParams( backend, bin.GetParamInfo() );
    SetupParams( backend, bin.GetSParamInfo() );

    bool ret = backend.Run( *bin._irBinary.get(), inputTiles, resRef, callback );

    _logStream << backend.DrainLogs();
    _perfInfo = backend.GetPerfInfo();

    return ret;
  }
  /*
    実行時のログ、つまり@print_expr でログされた場合にその文字列が返る。
    このメソッドを呼ぶと内部のログはクリアされる。
  */
  std::string DrainLogs()
  {
    auto ret = _logStream.str();
    _logStream.str( std::string() );
    return ret;
  }

private:

  // PARAM_TYPEはPramInfoかSParamInfo
  template<typename PARAM_TYPE>
  void SetupParams( BACKEND& backend, std::vector<PARAM_TYPE>& pinfo )
  {
    for (auto& one : pinfo)
    {
      one.SetupEnv( backend._renv );
    }
  }
};

}///< mfg
#endif

