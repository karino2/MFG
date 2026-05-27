/* -*- coding: utf-8 -*- マルチバイト */

// Copyright 2026 Kazuma Arino. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#ifndef MFG_BINARY_HPP_
#define MFG_BINARY_HPP_

#include <mfg_pal/pal.hpp>

#include "mfg_elem.hpp"
#include "mfg_printer.hpp"
#include "mfg_tensor.hpp"

#include <ostream>

/*
  IRBinaryとその周辺のコードを置く場所。
  IRBinaryはパーサーが組み立てて、外部からも参照する。
  パーサーは内部でしか使わないので外部は知る必要が無いがIRBinaryは知る必要がある。
*/
namespace mfg_binary
{
using namespace mfg_internal;
using namespace mfg_pal;

// 32bitのRuntimeValue、略してRVal32。
// 型情報は持たない。
struct RVal32
{
  union
  {
    int32_t _ival = 0;
    uint32_t _uival;
    float _fval;
  } u;

  template<typename T>
  RVal32( T value ) { Set( value ); }
  RVal32() = default;

  // RVal32( const RVal32& ) = default;

  template<typename T> T Value() const;
  template<typename T> void Set( T value );
};

template<> inline int32_t RVal32::Value() const { return u._ival; }
template<> inline uint32_t RVal32::Value() const { return u._uival; }
template<> inline float RVal32::Value() const { return u._fval; }
template<> inline void RVal32::Set( int32_t value ) { u._ival = value; }
template<> inline void RVal32::Set( uint32_t value ) { u._uival = value; }
template<> inline void RVal32::Set( float value ) { u._fval = value; }

// RuntimeEnv、略してREnv。
// 変数名とRVal32のmapを持つ。
struct REnv
{
  std::map<std::string, RVal32> _valMap;

  void Assign( std::string name, RVal32 val )
  {
    _valMap[ std::move(name) ] = val;
  }

  RVal32 GetRVal32( const std::string& name ) const
  {
    return _valMap.at( name );
  }

  template<typename T>
  T GetValue( const std::string& name )
  {
    return GetRVal32( name ).Value<T>();
  }

  void AddIOExtent( const InputsInfo& iinfo, const TLTensor& result, size_t width, size_t height )
  {
    std::vector<size_t> bounds { width, height };

    iinfo.ForEachInput( [&]( InputIndex, OneInputInfo& one ) {
      AddExtent( one.GetTensorInfo(), bounds );
    });
    AddExtent( result.GetTensorInfo(), bounds );
  }

  void AddExtent( const TensorInfo& tsinfo, const std::vector<size_t>& bounds )
  {
    for (auto dim : NRange( bounds.size() ))
    {
      std::string extentName = tsinfo.GetExtentName( dim );
      Assign( extentName, { static_cast<uint32_t>( bounds[dim] ) } );
    }
  }

  void AddConstTSExtent( const ConstTensor& tsinfo )
  {
    AddExtent( tsinfo.GetTensorInfo(), tsinfo.GetExtents() );
  }

  void LogVars( std::ostream& ostream, const std::vector<LogEntry>& logs )
  {
    for( auto& entry: logs )
    {
      if (entry._vtype.IsInt())
      {
        ostream << GetValue<int32_t>( entry._varName ) << std::endl;
      }
      else if(entry._vtype.IsUInt())
      {
        ostream << GetValue<uint32_t>( entry._varName ) << std::endl;
      }
      else if(entry._vtype.IsFloat())
      {
        ostream << GetValue<float>( entry._varName ) << std::endl;
      }
      else
      {
        // AddLogEntryでチェックしているのでここには来ないはず。
        assert(false);
      }
    }
  }
};

/*
  RuntiemにCImageTileXXを渡すためのInputTiles関連のコード。
  CMangaLayerには依存しないが、CMangaLayersから組み立てられる前提。
*/

/*
  mfg_pal::Image32かmfg_pal::Image64のポインタを持つ。
  TileTypeはCMangaLayerの対応する型を表す（OneInputInfoの方では無い事に注意）
  これ以外のレイヤーの場合は存在しないかのように扱う（シェーダーとしては0で埋められた配列が来る）
*/
struct TileReference
{
  enum TileType
  {
    TILE32,
    TILE64
  };

  TileType _tileType;
  union
  {
    mfg_pal::Image32 *_tile32;
    mfg_pal::Image64 *_tile64;
  } u;

  TileReference( mfg_pal::Image64* tile64 ) : _tileType( TILE64 ) { u._tile64 = tile64; }
  TileReference( mfg_pal::Image32* tile32 ) : _tileType( TILE32 ) { u._tile32 = tile32; }

  // uは初期化しない特殊ケース
  TileReference( TileType tt ) : _tileType( tt ) {}

  void SetReference( mfg_pal::Image64* tile64 )
  {
    _tileType = TILE64;
    u._tile64 = tile64;
  }

  void SetReference( mfg_pal::Image32* tile32 )
  {
    _tileType = TILE32;
    u._tile32 = tile32;
  }

  /*
  Previewの終わりなどで使う時用に、参照してない方はnullを返すptr取得関数
  */
  mfg_pal::Image32* GetTile32Ptr()
  {
    if (_tileType == TILE32)
      return u._tile32;
    return NULL;
  }

  mfg_pal::Image64* GetTile64Ptr()
  {
    if (_tileType == TILE64)
      return u._tile64;
    return NULL;
  }

  int Width() const
  {
    if (_tileType == TILE64) {
      return u._tile64->Width(); 
    }
    return u._tile32->Width();
  }

  int Height() const
  {
    if (_tileType == TILE64) {
      return u._tile64->Height(); 
    }
    return u._tile32->Height();
  }

};

/*
  TensorのBound関係
*/

struct Bound
{
  enum InfoType { IMM, VARIABLE };

  InfoType _btype;
  size_t _imm;
  std::string _varName;

  Bound( size_t imm ) : _btype( IMM ), _imm( imm ) {}
  Bound( const std::string& varName ) : _btype( VARIABLE ), _imm( 0 ), _varName( varName ) {}
  Bound( const Bound& ) = default;
  Bound( Bound&& ) = default;

  bool IsImm() const { return _btype == IMM; }
  bool IsVariable() const { return _btype == VARIABLE; }
};

struct BoundsInfo
{
  std::map<std::string, std::vector<Bound>> _tensorBounds;

  void RegisterTensorBoundByImm(const std::string& tensorName, const std::vector<size_t>& dims )
  {
    auto bounds = MapFn( dims, [](size_t imm) { return Bound( imm ); } );
    _tensorBounds[ tensorName ] = std::move( bounds );
  }

  // dimsはi32かu32のImmElem, Variableのどれか
  void RegisterTensorBound(const std::string& tensorName, const std::vector<BaseExprElem*>& dims )
  {
    auto bounds = MapFn( dims, [](const BaseExprElem* bexpr)
      {
        auto etype = bexpr->_elemType;
        if (etype == IRElemType::ImmElem )
        {
          return Bound( (size_t)bexpr->AsInt() );
        } 
        else if (etype == IRElemType::Variable)
        {
          auto vari = (Variable*)bexpr;
          return Bound( vari->_name ); 
        }
        else
        {
          assert(false);
          return Bound( 0 );
        }
      } );
    RegisterTensorBound( tensorName, std::move(bounds) );
  }

  void RegisterTensorBound(const std::string& tensorName, std::vector<Bound>&& bounds )
  {
    _tensorBounds[ tensorName ] = std::move( bounds );
  }

  // 手作業でやっていた時代のコードの移行用ワークアラウンド
  std::vector<size_t> ResolveImmOnly( const std::string& tensorName )
  {
    auto& bounds = _tensorBounds[ tensorName ];
    auto ret = MapFn( bounds, []( const Bound& binfo ){ 
      assert( binfo.IsImm() );
      return binfo._imm;
    });
    return ret;
  }

  std::vector<size_t> Resolve( const std::string& tensorName, REnv& renv )
  {
    auto& bounds = _tensorBounds[ tensorName ];
    auto ret = MapFn( bounds, [&renv]( const Bound& binfo ){ 
      if (binfo.IsImm())
      {
        return binfo._imm;        
      }
      else
      {
        assert( binfo.IsVariable() );
        return (size_t)renv.GetValue<uint32_t>( binfo._varName );
      }
    });
    return ret;
  }
};

// 流れるインターフェース用アクセサ
struct BoundsInfoRegisterer
{
  BoundsInfo& _info;

  BoundsInfoRegisterer( BoundsInfo& info ) : _info( info ) {}

  BoundsInfoRegisterer& OfTensor( TLTensor& tensor, size_t xdim )
  {
    _info.RegisterTensorBoundByImm( tensor.Name(), { xdim } );
    return *this;
  }

  BoundsInfoRegisterer& OfTensor( TLTensor& tensor, const Variable* vari )
  {
    _info.RegisterTensorBound( tensor.Name(), { (BaseExprElem*)vari } );
    return *this;
  }

  BoundsInfoRegisterer& OfTensor( TLTensor& tensor, std::vector<Bound>&& bounds )
  {
    _info.RegisterTensorBound( tensor.Name(), std::move(bounds) );
    return *this;
  }

  BoundsInfoRegisterer& OfTensor( TLTensor& tensor, size_t xdim, size_t ydim )
  {
    _info.RegisterTensorBoundByImm( tensor.Name(), { xdim, ydim } );
    return *this;
  }

  BoundsInfoRegisterer& OfTensor( TLTensor& tensor, const std::vector<size_t>& dims )
  {
    _info.RegisterTensorBoundByImm( tensor.Name(), dims );
    return *this;
  }

  BoundsInfoRegisterer& OfTileInfo( InputsInfo& iinfo, TLTensor& result, const TileReference& resultTile )
  {
    std::vector<size_t> dims { (size_t)resultTile.Width(), (size_t)resultTile.Height() };

    iinfo.ForEachInput([&]( InputIndex, OneInputInfo& one ){
      _info.RegisterTensorBoundByImm( one.TsName(), dims );
    });

    _info.RegisterTensorBoundByImm( result.Name(), dims );
    return *this;

  }
};

//
// TopLevelEntity、略してTLEntity関連
// TLEntityはテンソルかTLBlockで、カーネルに対応するなにか。
//


// Token用String。
// 文字列の先頭のポインタと長さを持つ。
// NULLターミネートされてない場合がある。
//
// 文字列リテラルの場合、TokenをToStringした場合とは中身が違うことがあるが、
// TStringをToStringした場合は同じものを返す（ダブルクオートやエスケープの処理をせずそのまま返す）
//
// IRBinaryのlookupなどで使いたいからここに置く。
struct TString
{
  std::string _store; // 部分文字列じゃない時にstringをここに保持してそこへの参照とする場合がある。extentなど動的に生成される識別子用
  const char* _str;
  size_t _len;

  TString( const char* str, size_t len ) : _str( str ), _len( len ) {}
  TString( const char* str ) : TString( str, std::strlen(str) ) {}
  TString( const TString& src ) : TString( src._str, src._len )
  {
    if (src._str == src._store.c_str() )
    {
      _store = src._store;
      _str = _store.c_str();
    }
  }
  TString& operator=( const TString& ) = default;

private:
  // 誤用を防ぐため、特別なファクトリ経由のみ許す
  TString( const std::string& str ) : _store( str ), _str( _store.c_str() ), _len ( str.size() ) {}

public:
  static TString Create( const std::string& str )
  {
    return TString(str);
  }

  int compare( const char* dest_str, size_t dest_len ) const
  {
    int res = std::strncmp( _str, dest_str, std::min( _len, dest_len ) );
    if (res != 0)
      return res;
    if (_len < dest_len)
      return -1;
    if (_len > dest_len)
      return 1;
    return 0;
  }

  int compare( const TString& dest ) const
  {
    return compare( dest._str, dest._len );
  }

  bool operator< ( const TString& dest ) const
  {
    return compare( dest ) < 0;
  }

  bool operator== ( const TString& dest ) const
  {
    return compare( dest ) == 0;
  }

  bool operator== ( const std::string& dest ) const
  {
    return compare( dest.c_str(), dest.size() ) == 0;
  }

  bool IsEqual( const char* pat, size_t len ) const 
  {
    // ショートカット
    if (len != _len)
      return false;
    return (*this) == TString( pat, len );
  }

  bool IsCharAt( size_t offset, char expect ) const
  {
    if (offset >= _len)
      return false;

    return _str[offset] == expect;
  }

  bool IsStrAt( size_t offset, const char* pat, size_t len ) const
  {
    if (offset+len > _len)
      return false;

    return 0 == std::strncmp( _str+offset, pat, len );
  }

  std::string ToString() const
  {
    return std::string(_str, _len );
  }

  char operator[]( size_t i ) const
  {
    assert( i < _len );
    return _str[i];
  }
  
};

// std::stringのcomparatorとしても使える
struct TStringComparator
{
  using is_transparent = std::true_type;

  // less
  bool operator() ( const std::string& left, const TString& right ) const
  {
    return right.compare( left.c_str(), left.size() ) > 0;
  }
  
  bool operator() ( const TString& left, const std::string& right ) const
  {
    return left.compare( right.c_str(), right.size() ) < 0;
  }

  template<typename T>
  bool operator() (const T& l, const T& r ) const
  {
    return l < r;
  }
};

// 
// TopLevelEntityを保持する構造体。
// ポインタと種類を保持するのみで、実体は別の人が管理
// TLEとTLEの間の何も無い、を表す為にUNBOUNDという状態も持つ
// 
struct TLEntityRef
{
  union TLEPtr {
    TLTensor* _tensor;
    TLBlock* _tblock;

    TLEPtr( TLTensor* t ) : _tensor(t) {}
    TLEPtr( TLBlock* tb ) : _tblock(tb) {}    
  } u;

  enum TLEType
  {
    UNBOUND,
    TENSOR,
    TOP_LEVEL_BLOCK
  };

  TLEType _etype;

  TLEntityRef() : u( (TLTensor*)nullptr ), _etype( UNBOUND ) {}
  TLEntityRef( TLTensor* tensor ) : u( tensor ), _etype( TENSOR ) {}
  TLEntityRef( TLBlock* tblock ) : u( tblock ), _etype( TOP_LEVEL_BLOCK ) {}
  TLEntityRef( const TLEntityRef& src ) : u( (TLTensor*)nullptr ), _etype( src._etype )
  {
    switch (src._etype)
    {
      case UNBOUND:
        break;
      case TENSOR:
        u._tensor = src.u._tensor;
        break;
      case TOP_LEVEL_BLOCK:
        u._tensor = src.u._tensor;
        break;
    }
  }

  void Assign( TLTensor* tensor )
  {
    _etype = TENSOR;
    u._tensor = tensor;
  }

  void Assign( TLBlock* tblock )
  {
    _etype = TOP_LEVEL_BLOCK;
    u._tblock = tblock;
  }

  TLRoot* AsTopLevel() const
  {
    switch (_etype)
    {
      case TLEntityRef::UNBOUND:
        return nullptr;
      case TLEntityRef::TENSOR:
        return u._tensor;
      case TLEntityRef::TOP_LEVEL_BLOCK:
        return u._tblock;
    }
  }

  // TはReferenceで対応しているどれか
  template<typename T>
  void AddReference( T& input )
  {
    auto tl = AsTopLevel();
    if (tl == nullptr)
    {
      // UNBOUNDの場合。
      // UnitTest以外ではエラーなのだが、
      // パーサー単体でのテストは重要なのでこのケースは正式に何もしないとする
      return;
    }
    return tl->AddReference( input );
  }

  void CallTensorReferenced( size_t refIndex )
  {
    auto tl = AsTopLevel();
    if (tl == nullptr)
    {
      // AddReferenceのコメント参照
      return;
    }
    tl->CallTensorReferenced( refIndex );    
  }

  void Lower( InlineFuncRegistry& ifr )
  {
    switch (_etype)
    {
      case TLEntityRef::UNBOUND:
        break;
      case TLEntityRef::TENSOR:
        u._tensor->Lower( ifr );
        break;
      case TLEntityRef::TOP_LEVEL_BLOCK:
        u._tblock->Lower( ifr );
        break;
    }
  }

  void CollectFreeRef(  std::function<GlobalTensorLike*(const std::string&)> tslookup )
  {
    switch (_etype)
    {
      case TLEntityRef::UNBOUND:
        break;
      case TLEntityRef::TENSOR:
        u._tensor->CollectFreeRef( tslookup );
        break;
      case TLEntityRef::TOP_LEVEL_BLOCK:
        u._tblock->CollectFreeRef( tslookup );
        break;
    }
  }
};

using RVal = RVal32;

struct ParamInfo
{
  enum ParamType
  {
    SLIDER_INT32,
    SLIDER_FLOAT32,
    DIRECTION_FLOAT32,
    CHECKBOX_INT32,
    POINTER_FLOAT32_XY,
    COLOR_PICKER_FLOAT32_V4,
    DROPDOWN_INT32,
  };

  ParamType _ptype;
  std::string _paramName;
  std::string _paramLabel;
  /*
    _valsはParamTypeによって使い方が異なる。
    vals0は結果の値（actual）。

    スライダー: vals1はinit, vals2はmin, vals3はmax
    CHECKBOX: vals1はinit
    DROPDOWN:　ではvals0のみ
    DIRECTION: vals1はinit
    POINTER: vals1がactual2。
  */
  std::vector<RVal> _vals;
  std::vector<std::string> _paramItems;

  ParamInfo( ParamType ptype, const std::string& name, const std::string& label, RVal initVal, RVal minVal, RVal maxVal ) :
    _ptype( ptype ), _paramName( name ), _paramLabel( label ), _vals{ initVal, initVal, minVal, maxVal } {}
  ParamInfo( ParamType ptype, const std::string& name, const std::string& label, RVal initVal ) :
    _ptype( ptype ), _paramName( name ), _paramLabel( label ), _vals{ initVal, initVal } {}

  ParamInfo( const std::string& name, const std::string& label, int initVal, int minVal, int maxVal ) : ParamInfo( SLIDER_INT32, name, label,  { initVal }, { minVal }, { maxVal } ){}
  ParamInfo( const std::string& name, const std::string& label, float initVal, float minVal, float maxVal  ) : ParamInfo( SLIDER_FLOAT32, name, label, { initVal }, { minVal }, { maxVal } ){}
  ParamInfo( ParamType ptype, const std::string& name, const std::string& label ) :  _ptype( ptype ), _paramName( name ), _paramLabel( label )
  {
    // ポインタの時はinitを0.5にしておく
    if (ptype == POINTER_FLOAT32_XY)
    {
      // optsを2つ確保
      _vals.push_back( 0.5F );
      _vals.push_back( 0.5F );
    }
  }
  // DIRECTION用。とりあえず不要なものが無いだけのコンストラクタにしておく。
  // min, maxは適当に入れておくが使わない。
  ParamInfo( ParamType ptype, const std::string& name, float initVal ) : ParamInfo( ptype, name, "(no label)", RVal{ initVal } ) { assert(ptype == DIRECTION_FLOAT32); }

  // CHECKBOX用。上のコンストラクタと同様
  ParamInfo( ParamType ptype, const std::string& name, const std::string& label, int initVal ) : ParamInfo( ptype, name, label, RVal{ initVal } ) { assert(ptype == CHECKBOX_INT32); }

  // DROPDOWN用
  ParamInfo( ParamType ptype, const std::string& name, const std::string& label, std::vector<std::string>&& items ) : _ptype( ptype ), _paramName( name ), _paramLabel( label ), _paramItems( std::move(items) ), _vals{ 0 } { assert(ptype == DROPDOWN_INT32); }

  // COLOR_PICKER用
  ParamInfo( ParamType ptype, const std::string& name, const std::string& label, const std::vector<float>& fvec ) :  _ptype( ptype ), _paramName( name ), _paramLabel( label )
  {
    assert( ptype == COLOR_PICKER_FLOAT32_V4 );
    assert( fvec.size() == 4 );

    // optsを4つ確保
    _vals.push_back( fvec[0] );
    _vals.push_back( fvec[1] );
    _vals.push_back( fvec[2] );
    _vals.push_back( fvec[3] );
  }

  // このパラメータの表示名。UIに使うラベル。
  const std::string& GetLabel() const { return _paramLabel; }

  // このパラメータをUIに表示する時の種類。
  ParamType GetParamType() const { return _ptype; }

  // このパラメータの最小値
  RVal GetMin() const { return _vals[2]; }

  // このパラメータの最大値
  RVal GetMax() const { return _vals[3]; }

  // このパラメータのGUI上の初期値
  RVal GetInit() const { return _vals[1]; }

  const std::vector<std::string>& GetItems() const { return _paramItems; }

  RVal GetActual() const { return _vals[0]; }
  RVal GetY() const { return _vals[1]; }

  std::vector<float> GetBGRA() const
  {
    return { _vals[0].Value<float>(), _vals[1].Value<float>(), _vals[2].Value<float>(), _vals[3].Value<float>() };
  }

  // 最終的にユーザーが設定した値をセット。
  // Tとしては今のところint32_tとfloatがある。
  template<typename T> void Assign( T value ) { _vals[0].Set( value ); }

  // POINTERでユーザーが設定した値をセット。
  // floatは0.0〜1.0。
  void AssignXY( float x, float y )
  {
    assert( _vals.size() >= 2);
    _vals[0].Set( x );
    _vals[1].Set( y );
  }

  void AssignBGRA( float b, float g, float r, float a )
  {
    assert( _vals.size() >= 4);
    _vals[0].Set( b );
    _vals[1].Set( g );
    _vals[2].Set( r );
    _vals[3].Set( a );
  }

  // MFGのランタイム側が使うもの
  void SetupEnv( REnv& env )
  {
    if (_ptype == ParamType::POINTER_FLOAT32_XY)
    {
      env.Assign( Variable::TupleItemName( _paramName, 0 ), _vals[0] );
      env.Assign( Variable::TupleItemName( _paramName, 1 ), _vals[1] );
    }
    else if (_ptype == ParamType::COLOR_PICKER_FLOAT32_V4)
    {
      env.Assign( Variable::TupleItemName( _paramName, 0 ), _vals[0] );
      env.Assign( Variable::TupleItemName( _paramName, 1 ), _vals[1] );
      env.Assign( Variable::TupleItemName( _paramName, 2 ), _vals[2] );
      env.Assign( Variable::TupleItemName( _paramName, 3 ), _vals[3] );
    }
    else
    {
      env.Assign( _paramName, _vals[0] );
    }
  }
};

constexpr auto FORE_COLOR_VNAME = "fore_color_val";
constexpr auto BACK_COLOR_VNAME = "back_color_val";

/*
  ParamInfoはWidgetに対応したパラメータ。
  だがfore_colorなどのようにwidgetがないパラメータもある。
  これらはエンジンのstateに関わるもので、UIがいらないのでアプリごとに対応する必要は無いが、
  言語処理系としては外部からのパラメータとなる。
  こうしたパラメータはUIプログラムの時には意識する必要は無いしswitchでこのケースをいちいち無視するのも煩雑だ。
  そこでこれをSParamInfoと別の構造体として管理する。

  Non UIなParamInfo。
*/
struct SParamInfo
{
  enum SParamType
  {
    FORE_COLOR,
    BACK_COLOR,
  };

  SParamType _stype;
  std::string _paramName;
  /*
    _valsはSParamTypeによって使い方が異なる。
    vals0は結果の値（actual）。

    FORE_COLORはBGRAがvals[4]に入る。
  */
  std::vector<RVal> _vals;

  SParamInfo( SParamType stype, const std::string& name, const std::vector<float>& fvec ) :  _stype( stype ), _paramName( name )
  {
    assert( fvec.size() == 4 );

    _vals.push_back( fvec[0] );
    _vals.push_back( fvec[1] );
    _vals.push_back( fvec[2] );
    _vals.push_back( fvec[3] );
  }

  SParamType GetParamType() const { return _stype; }

  std::vector<float> GetBGRA() const
  {
    return { _vals[0].Value<float>(), _vals[1].Value<float>(), _vals[2].Value<float>(), _vals[3].Value<float>() };
  }

  void AssignBGRA( float b, float g, float r, float a )
  {
    assert( _vals.size() >= 4);
    _vals[0].Set( b );
    _vals[1].Set( g );
    _vals[2].Set( r );
    _vals[3].Set( a );
  }

  // MFGのランタイム側が使うもの
  void SetupEnv( REnv& env )
  {
    assert(_stype == SParamType::FORE_COLOR || _stype == SParamType::BACK_COLOR );
    
    env.Assign( Variable::TupleItemName( _paramName, 0 ), _vals[0] );
    env.Assign( Variable::TupleItemName( _paramName, 1 ), _vals[1] );
    env.Assign( Variable::TupleItemName( _paramName, 2 ), _vals[2] );
    env.Assign( Variable::TupleItemName( _paramName, 3 ), _vals[3] );
  }
};


/*
  乱数のseed関連。基本的には各runtimeでコード生成時に使うものだが、MFGBinaryにぶら下げておくのがpreviewと実行で同じ値にしたりするのに都合がよいので
  MFGBinaryにseedになる元を持たせることにする。

  各カーネル内で使うseedはこのマスターのseedから作り出すが、必ず同じマスターのseedからは同じseedの系列が得られるようにする。
  seedは乱数では無いので規則性があってもいいので単純にインクリメントするだけにする。
*/
struct SeedGenerator
{
  uint32_t _origin;
  uint32_t _cur;

  SeedGenerator( uint32_t origin ) : _origin( origin ), _cur(0)
  {
    Restart();
  }

  uint32_t Next() { return ++_cur; }

  // Nextが同じ系列を返すように最初に戻る
  void Restart() { _cur = _origin + 1; }

  void NewOrigin( uint32_t origin )
  {
    _origin = origin;
    Restart();
  }
};

// IRElemのツリーのholderとなるTensorなどを保持するクラス。
// Parserのハンドラがこれを構築する。
// backendはこのIRBinaryをコンパイルして実行する。
struct IRBinary
{
  InputsInfo _inputsInfo;
  ResultTensorHolder _resultHolder;
  std::vector<ParamInfo> _params;
  std::vector<SParamInfo> _sparams;
  std::vector<std::unique_ptr<TLTensor>> _tensors;
  std::vector<std::unique_ptr<TLBlock>> _tblocks;
  std::vector<ConstTensor> _constTensors;
  std::vector<TLEntityRef> _tles;

  std::vector<ExprV> _globalVariables;
  InlineFuncRegistry _inlfRegistry;

  BoundsInfo _binfo;
  std::string _title;
  std::string _version;
  SeedGenerator _seedGen;

  IRBinary() : _title("(blank)"), _version(""), _seedGen( mfg_pal::Rand32() )
  {
    _inlfRegistry.SetupPredefinedFunctions();
  }

  SeedGenerator& GetSeedGen() { return _seedGen; }
  void NewSeed()
  {
    _seedGen.NewOrigin( mfg_pal::Rand32() );
  }
  void ResetSeedGen() { _seedGen.Restart(); }

  InputsInfo& Input()
  {
    return _inputsInfo;
  }

  template<typename F>
  void ForEachInput( F fun ) { _inputsInfo.ForEachInput( fun ); }

  TLTensor& NewTensor( const std::string& tname, size_t dim )
  {
    TLTensor* t = new TLTensor( tname, dim );
    _tensors.emplace_back( t );
    return *t;
  }

  ConstTensor& NewConstTensor( const std::string& tname, Type tp, std::vector<uint32_t>&& data, std::vector<size_t>&& extents )
  {
    _constTensors.emplace_back( tp, tname, std::move(data), extents );
    return _constTensors.back();
  }

  ConstTensor& NewConstTensor( const std::string& tname, const mfg_pal::Image32& src )
  {
    _constTensors.emplace_back( tname, src );
    return _constTensors.back();
  }

  TLTensor& Result() { return *_resultHolder.GetTensor(); }
  bool ResultDefined() const { return _resultHolder.IsDefined();  }

  void RegisterTopLevelBlock( TLBlock* tblock )
  {
    _tblocks.emplace_back( tblock );
  }

  BoundsInfoRegisterer RegisterBounds() { return BoundsInfoRegisterer( _binfo ); }
  std::vector<size_t> Resolve( const std::string& tensorName, REnv& renv )
  {
    return _binfo.Resolve( tensorName, renv );
  }
  std::vector<size_t> Resolve( const TensorInfo& ts, REnv& renv ) { return Resolve( ts.Name(), renv ); }

  void PushTLEntity( const TLEntityRef& tle )
  {
    _tles.push_back( tle );
  }

  TLTensor* LookupResult( const std::string& tname )
  {
    return _resultHolder.Lookup( tname );
  }



  // TはTStringかstd::string
  template<typename T>
  TLTensor* LookupTensor( const T& tname )
  {
    auto iter = std::find_if( _tensors.begin(), _tensors.end(), [&tname]( std::unique_ptr<TLTensor>& tptr ) { return tname == tptr->Name(); } );
    if (iter == _tensors.end())
      return nullptr;
    return iter->get();
  }

  // テンプレートのLookup用にstd::stringとTStringの２つを用意
  GlobalTensorLike* LookupInput( const std::string& tname )
  {
    return Input().Lookup( tname );
  }

  GlobalTensorLike* LookupInput( const TString& tname )
  {
    return LookupInput( tname.ToString() );
  }

  OneInputInfo* LookupInput( const TString& tname, int layerIndex )
  {
    return Input().Lookup( tname.ToString(), layerIndex );
  }


  // TはTStringかstd::string
  template<typename T>
  ConstTensor* LookupConstTensor( const T& tname )
  {
    auto iter = std::find_if( _constTensors.begin(), _constTensors.end(), [&tname]( ConstTensor& cts ) { return tname == cts.Name(); } );
    if (iter == _constTensors.end())
      return nullptr;
    return &(*iter);
  }

  // TはTStringかstd::string
  template<typename T>
  GlobalTensorLike* LookupGlobalTSLike( const T& tname )
  {
    auto input = LookupInput( tname );
    if (input != nullptr)
      return input;

    auto ts = LookupTensor( tname );
    if (ts != nullptr)
      return ts;
    
    return LookupConstTensor( tname );
  }

  void SetupConstsTSExtents( REnv& env )
  {
    for( const auto& ts : _constTensors )
    {
      env.AddConstTSExtent( ts );
      _binfo.RegisterTensorBoundByImm( ts.Name(), ts.GetExtents() );
    }
  }

  // パース途中に_scopeのグローバルの所に保持している変数。
  // Referenceなどはこれのポインタを保持している。
  // 本来は全て_tblocksの中の変数であるべきだが、
  // ローカル変数の扱いと変えるのも面倒なのでグローバルスコープに保存していた変数は
  // deleteせずにここに持ち越すことにする。
  void OwnGlobalVariables( std::vector<ExprV>&& globvars )
  {
    _globalVariables = std::move( globvars );
  }

  void Lower()
  {
    for( auto& tle : _tles )
    {
      tle.Lower( _inlfRegistry );
    }
  }

  void CollectFreeRef()
  {
    for( auto& tle : _tles )
    {
      tle.CollectFreeRef( [this]( const std::string& name ) { return LookupGlobalTSLike( name ); });
    }
  }


  void AnalyzeTensorLifeTime()
  {
    for( auto refIndex : NRange(_tles.size()) )
    {
      _tles[refIndex].CallTensorReferenced( refIndex );
    }

    // Resultは最後の次に参照される、とする。
    Result().Referenced( _tles.size() );
  }

  void LowerAndAnalyze()
  {
    Lower();
    CollectFreeRef();
    AnalyzeTensorLifeTime();
  }
};


}///< mfg_binary
#endif

