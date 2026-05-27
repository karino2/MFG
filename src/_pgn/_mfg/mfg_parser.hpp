/* -*- coding: utf-8 -*- マルチバイト */

// Copyright 2026 Kazuma Arino. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#ifndef MFG_PARSER_HPP_
#define MFG_PARSER_HPP_

#include <mfg_pal/pal.hpp>

#include "mfg_elem.hpp"
#include "mfg_printer.hpp"
#include "mfg_tensor.hpp"

#include "mfg_ir_util.hpp"

#include "mfg_binary.hpp"

#include <algorithm>
#include <map>
#include <regex>
#include <string>
#include <sstream>
#include <utility> // make_pair
#include <vector>


namespace mfg_parser
{
using namespace mfg_pal;
using namespace mfg_internal;
using namespace mfg_ir_util;

// MacでUIntがぶつるかるので明示的にimport。
using mfg_internal::UInt;


enum class TokenType
{
  UNKNOWN, // まだ決まってない状態を表す特別な型
  ATMARK,
  DEF,
  LET,
  BY,
  IDENTIFIER,
  INTEGER,
  UINTEGER,
  HEX,
  UHEX,
  FLOAT,
  BAR, // |
  BAR2, // || 、今の所orとみなすが、引数無しのラムダ式もこれになるかもしれないのでBAR2という名前にしておく。
  PIPE_OP, // |>
  AMPERSAND, // &
  AMPERSAND2, // &&
  DOLLAR, // $
  BLANK_ID, // _ 
  L_BRACE, // {
  R_BRACE, // }
  L_SBRACKET, // [
  R_SBRACKET,  // ]
  L_PAREN, // (
  R_PAREN, // )
  COMMA,
  COLON,
  DOT,
  DOT3, // ...
  HAT, // ^ 
  BANG, // !
  HALF_OPEN_RANGE, // ..<
  EQUAL,
  EQEQ, // ==
  NEQ, // != , BANGとはトークナイズで区別。
  GTGT, // >>
  GT,
  GTEQ,
  LTLT, // <<
  LT,
  LTEQ, // <=
  MINUS,
  PLUS,
  ASTER,
  SLASH,
  PERCENT, // %
  SPACE,
  STRING,
  FN, // fn
  MUT, // mut
  ESCAPED_EOL,
  END_OF_LINE,
  END_OF_SRC
};

extern std::string TokenTypeToStr( TokenType tt );

using mfg_binary::TString;
using mfg_binary::TStringComparator;

struct Token
{
  const char* _src;
  TokenType _ttype;
  size_t _offset;
  TString _str;

  Token( const char* src, TokenType ttype, size_t offset, size_t len ) : _src( src ), _ttype( ttype ), _offset( offset ), _str( src+offset, len )
  {    
  }

  Token( const char * src ): Token( src, TokenType::UNKNOWN, 0, 0 ) {}

  void Set( TokenType ttype, size_t offset, size_t len )
  {
    _ttype = ttype;
    _offset = offset;
    
    _str = TString( _src+offset, len );
  }

  bool IsEqual( const char* pat, size_t len ) const
  {
    return _str.IsEqual( pat, len );
  }

  bool IsEqual( const char* pat ) const
  {
    return IsEqual( pat, std::strlen( pat ) );
  }

  bool OneOf( const std::vector<std::string>& candidates ) const
  {
    for( const auto& candidate : candidates )
    {
      if (IsEqual( candidate.c_str(), candidate.size() ))
        return true;
    }
    return false;
  }

  bool IsCharAt( size_t offset, char expect ) const
  {
    return _str.IsCharAt( offset, expect );
  }

  bool IsStrAt( size_t offset, const char* pat, size_t len ) const
  {
    return _str.IsStrAt( offset, pat, len );
  }

  bool IsDummy() const { return Length() == 0; }

  size_t Length() const { return _str._len; }

  std::string ScanString() const
  {
    assert( _str._len > 2 );
    std::stringstream ss;
    // 最初と最後のダブルクオートは無視。
    size_t pos = 1;
    while (pos < _str._len -1)
    {
      char ch = *(_str._str+pos++);
      if (ch == '\\')
      {
        // 今の所ダブルクオートやバックスラッシュしかエスケープに無いので単にそのまま入れる。
        ss.put( *(_str._str+pos++) );
      }
      else
      {
        ss.put( ch );
      }
    }
    return ss.str();
  }

  std::string ToString() const
  {
    if (_ttype != TokenType::STRING)
      return _str.ToString();
    return ScanString();
  }

  int32_t ToInt() const
  {
    assert( _ttype == TokenType::INTEGER );
    int32_t res = 0;
    for (size_t i = 0; i < Length(); i++)
    {
      res = res*10 + (_str[i] - '0');
    }
    return res;
  }

  uint32_t ToUInt() const
  {
    assert( _ttype == TokenType::UINTEGER );
    uint32_t res = 0;
    for (size_t i = 0; i < Length()-1; i++)
    {
      res = res*10 + (_str[i] - '0');
    }
    return res;
  }

  int32_t ToHexTill( size_t len ) const
  {
    int32_t res = 0;

    // start from 0x, so skip first two char.
    for (size_t i = 2; i < len; i++)
    {
      res *= 16;
      char ch = _str[i];
      if (ch >= 'A' && ch <= 'F')
      {
        res += 10+(ch - 'A');
      }
      else if (ch >= 'a' && ch <= 'f')
      {
        res += 10+(ch - 'a');
      }
      else
      {
        assert( ch >= '0' && ch <= '9' );
        res += ch - '0';
      }
    }
    return res;
  }

  int32_t ToHex() const
  {
    assert( _ttype == TokenType::HEX );
    return ToHexTill( Length() );
  }

  uint32_t ToUHex() const
  {
    assert( _ttype == TokenType::UHEX );
    return (uint32_t)ToHexTill( Length()-1 );
  }


  float ToFloat() const
  {
    assert( _ttype == TokenType::FLOAT );
    return std::stof( ToString() );
  }
};

struct Location
{
  size_t _line;
  size_t _col;
  Location( size_t line, size_t col ) : _line( line ), _col( col ) {}
};

struct ParseError : public UserError
{
  Location _loc;
  ParseError( const std::string& msg, Location loc ) : UserError( msg ), _loc( loc ) {}

  size_t GetLineNum() const { return _loc._line; }
  size_t GetColumn() const { return _loc._col; }
};

template<typename ...ARGS>
ParseError FmtParseError( Location loc, ResId rid, ARGS... args )
{
  return ParseError( FmtRString( rid, args... ), loc );
}

// TokenTypeのキャスト。全引数でTokenTypeだったらintにしたいが、template力が足りずよく分からないのでオーバーロードで対応
inline ParseError FmtParseError( Location loc, ResId rid, TokenType arg1 )
{
  return FmtParseError( loc, rid, static_cast<int>(arg1) );
}

inline ParseError FmtParseError( Location loc, ResId rid, TokenType arg1, TokenType arg2 )
{
  return FmtParseError( loc, rid, static_cast<int>(arg1), static_cast<int>(arg2) );
}

template<typename T>
inline ParseError FmtParseError( Location loc, ResId rid, T arg1, TokenType arg2 )
{
  return FmtParseError( loc, rid, arg1, static_cast<int>(arg2) );
}

inline ParseError FmtParseError( Location loc, ResId rid, TokenType arg1, TokenType arg2, TokenType arg3 )
{
  return FmtParseError( loc, rid, static_cast<int>(arg1), static_cast<int>(arg2), static_cast<int>(arg3) );
}

using mfg_internal::ExprLET;

enum class BinOpType
{
  UNKNOWN,
  ADD,
  SUB,
  MUL,
  DIV,
  MOD,
  EQEQ,
  NEQ,
  OR,
  AND,
  BITWISE_AND,
  BITWISE_OR,
  LE,
  LT,
  GE,
  GT,
  POW,
  PIPE_OP,
  RSHIFT,
  LSHIFT
};

// 名前付き引数のイコールの右辺（値）
// paramでだけ使われる。
// std::stringを使いたいので変にunionにせずにstructにして全部もたせる
struct AttrNamedArgVal
{
  enum Type
  {
    INT,
    FLOAT,
    STRING,
    FLOAT_VEC,
    STRING_VEC
  };

  Type _type;

  std::string _sval;
  float _fval;
  int _ival;
  std::vector<float> _fvec;
  std::vector<std::string> _svec;


  AttrNamedArgVal( Type tp, std::string sval, float fval, int ival ) : _type(tp), _sval( std::move(sval) ), _fval( fval ), _ival( ival ) {}
  AttrNamedArgVal( std::string sval ) : AttrNamedArgVal( STRING, std::move(sval), 0.0F, 0 ) {}
  AttrNamedArgVal( float fval ) : AttrNamedArgVal( FLOAT, "", fval, 0 ) {}
  AttrNamedArgVal( int ival ) : AttrNamedArgVal( INT, "", 0.0F, ival ) {}
  AttrNamedArgVal( std::vector<float>&& fvec ) : _type(FLOAT_VEC), _sval(), _fval( 0.0F ), _ival( 0 ), _fvec(std::move(fvec))  {}
  AttrNamedArgVal( std::vector<std::string>&& svec ) : _type(STRING_VEC), _sval(), _fval( 0.0F ), _ival( 0 ), _svec(std::move(svec))  {}
  AttrNamedArgVal( const AttrNamedArgVal& ) = default;
  AttrNamedArgVal( AttrNamedArgVal&& ) = default;
};

// @param_i32などに使われる名前つき引数。
// 関数呼び出しのNamedArgと一本化しても良いのだが、より文法的に限定されているので分けておく。
struct AttrNamedArg
{
  Token _name;
  AttrNamedArgVal _value; // valueはTokenがいいのか、floatやintの方がいいのか良く分からないのでとりあえずTokenにしてみる。
  AttrNamedArg( const Token& name, const AttrNamedArgVal& value ) : _name( name ), _value( value ) {}
};

enum class WidgetType
{
  SLIDER,
  DIRECTION,
  CHECKBOX,
  POINTER,
  COLOR_PICKER,
  DROPDOWN
};

/*
  reduce<XX>.YY(args) の時、以下のようになる。

  _convType: reduce
  _targetTs: XX
  _method: YY
  _args: args
*/
struct ConvertTensorTree
{
  Token _convType;
  Token _targetTs;
  Token _method;
  std::vector<Expr> _args;

  ConvertTensorTree( const Token& convType, const Token& targetTs, const Token& method, std::vector<Expr>&& args ) :
    _convType(convType), _targetTs(targetTs), _method(method), _args( std::move(args) ) {}

  ConvertTensorTree( ConvertTensorTree&& ) = default;
};

/*
  def ts by [1, 2, 3] や
  def ts by [[1, 2, 3], [4, 5, 6]]

  の、[1, 2, 3]や[[1, 2, 3], [4, 5, 6]]を表すデータ。
*/
struct TensorLiteralData
{
  Type _type;
  std::vector<size_t> _extents;

  // RVal32だが32bitならなんでも良い
  std::vector<uint32_t> _data;

  template<typename T>
  TensorLiteralData( const std::vector<T>& src ) : _type( CType2Type<T>() ), _extents{ src.size() }, _data( src.size() )
  {
    std::memcpy( _data.data(), src.data(), sizeof(mfg_binary::RVal32)*src.size() );
  }

  template<typename T>
  static TensorLiteralData Create2D( const std::vector<T>& src )
  {
    auto ldata = TensorLiteralData( src );
    // add y dim.
    ldata._extents.push_back( 1 );
    return ldata;
  }


  /*
    最初のRowはすでにある前提でRowを足していく。
    型やサイズが違ったらfalseを返す。
  */
  template<typename T>
  bool AddRow( const std::vector<T>& row )
  {
    if (_type != CType2Type<T>())
      return false;

    if (row.size() != _extents[0])
      return false;

    // 2D
    assert( _extents.size() == 2 );

    for( auto elem : row )
    {
      // キャストでは無くreinterpretしたいのでRVal32経由で
      mfg_binary::RVal32 val;
      val.Set( elem ); 

      _data.push_back( val.u._uival );
    }

    _extents[1] += 1;
    return true;
  }
};

//
// TreeBuilder関連
//
struct VerifyArgContext
{
  std::string _fname;
  std::vector<Type> _argTypes;
  std::function<ParseError(const std::string& msg)>& _reportParseError;

  VerifyArgContext( std::string fname, std::vector<Type>&& types, std::function<ParseError(const std::string& msg)>& errorReporter ) : _fname( std::move(fname) ), _argTypes( std::move(types) ), _reportParseError( errorReporter )
  {
  }

  template<typename ...ARGS>
  void ASSERT_MSG( bool cond, ResId rid, ARGS... args )
  {
    if (!cond)
      throw _reportParseError( FmtRString( rid, args... ) );
  }

  VerifyArgContext& ArgNum( size_t num )
  {
    ASSERT_MSG( _argTypes.size() == num, ResId::FNAME_ARG_NUM_DIFFER, _fname, num, _argTypes.size() );
    return *this;
  }

  VerifyArgContext& Vector( size_t idx )
  {
    const auto& argType = _argTypes[idx];
    ASSERT_MSG( argType.IsTuple(), ResId::FNAME_EXPECTS_VECTOR_BUT_ATOM, _fname, idx );
    auto ntypes = argType.ExpandNumericTypes();
    ASSERT_MSG( ntypes.size() > 1, ResId::FNAME_EXPECTS_VECTOR_BUT_1D_TUPLE, _fname, idx );
    ASSERT_MSG( ntypes.size() <= 4, ResId::FNAME_EXPECTS_VECTOR_BUT_MOREDIM,  _fname, idx, ntypes.size() );
    for( auto nidx : NRange(ntypes.size()) )
    {
      ASSERT_MSG( ntypes[nidx] == ntypes[0], ResId::FNAME_EXPECTS_VECTOR_ATOM_TYPE_DIFFER, _fname, idx, nidx );
    }
    return *this;
  }  

  VerifyArgContext& VecDim( size_t idx, size_t dim )
  {
    Vector( idx );
    const auto& argType = _argTypes[idx];
    auto ntypes = argType.ExpandNumericTypes();
    ASSERT_MSG( ntypes.size() == dim, ResId::FNAME_ARG_DIM_DIFFER, _fname, idx, dim, ntypes.size() );
    return *this;
  }

  // floatの2〜4次元タプルである事を保証。
  VerifyArgContext& FVector( size_t idx )
  {
    Vector( idx );
    const auto& argType = _argTypes[idx];
    auto ntypes = argType.ExpandNumericTypes();
    ASSERT_MSG( ntypes[0].IsFloat(), ResId::FNAME_EXPECTS_FVECTOR, _fname, idx );
    return *this;
  }

  // iかuの2〜4次元タプルである事を保証。
  VerifyArgContext& USIntVector( size_t idx )
  {
    Vector( idx );
    const auto& argType = _argTypes[idx];
    auto ntypes = argType.ExpandNumericTypes();
    ASSERT_MSG( ntypes[0].IsUSInt(), ResId::FNAME_EXPECTS_USVECTOR, _fname, idx );
    return *this;
  }

  // f32か全てがf32のタプル。5要素以上あってもOK。
  VerifyArgContext& FloatOrFTuple( size_t idx )
  {
    const auto& argType = _argTypes[idx];
    if (_argTypes[idx].IsNumeric())
    {
      ASSERT_MSG( _argTypes[idx].IsFloat(), ResId::FNAME_EXPECTS_F_OR_FTUP, _fname, idx );
      return *this;
    }

    ASSERT_MSG( argType.IsTuple(), ResId::FNAME_EXPECTS_F_OR_FTUP, _fname, idx );
    auto ntypes = argType.ExpandNumericTypes();
    ASSERT_MSG( ntypes[0].IsFloat(), ResId::FNAME_EXPECTS_F_OR_FTUP, _fname, idx );
    for( auto nidx : NRange(ntypes.size()) )
    {
      ASSERT_MSG( ntypes[nidx] == ntypes[0], ResId::FNAME_EXPECTS_HOMO_TUPLE_ATOM_TYPE_DIFFER, _fname, idx, nidx );
    }
    return *this;
  }


  // u32かi32の2〜4次元タプルである事を確認
  VerifyArgContext& USTuple( size_t idx )
  {
    const auto& argType = _argTypes[idx];
    // 本当はタプルだけど、上でチェック済みなので以下のfailは無いからいいでしょう。
    ASSERT_MSG( argType.IsTuple(), ResId::FNAME_EXPECTS_VECTOR_BUT_ATOM, _fname, idx );

    auto ntypes = argType.ExpandNumericTypes();
    for( auto nidx : NRange(ntypes.size()) )
    {
      ASSERT_MSG( ntypes[nidx].IsUSInt(), ResId::FNAME_EXPECTS_USTUPLE, _fname, idx, nidx );
    }
    return *this;
  }

  VerifyArgContext& U8V4( size_t idx )
  {
    const auto& argType = _argTypes[idx];
    ASSERT_MSG( argType.IsU8V4(), ResId::FNAME_EXPECTS_U8V4, _fname, idx );
    return *this;
  }


  // Numericなベクトル、
  // つまり、f32, i32, u32のどれかのベクトル（2〜4次元の同じ型のタプル）
  VerifyArgContext& NVector( size_t idx )
  {
    Vector( idx );
    const auto& argType = _argTypes[idx];
    auto ntypes = argType.ExpandNumericTypes();
    ASSERT_MSG( ntypes[0].IsNumeric(), ResId::FNAME_EXPECTS_NVECTOR, _fname, idx );
    return *this;
  }

  VerifyArgContext& Scalar( size_t idx )
  {
    ASSERT_MSG( _argTypes[idx].IsNumeric(), ResId::FNAME_EXPECTS_SCALAR, _fname, idx );
    return *this;
  }

  VerifyArgContext& FloatOrFVector( size_t idx )
  {
    if (_argTypes[idx].IsNumeric())
    {
      ASSERT_MSG( _argTypes[idx].IsFloat(), ResId::FNAME_EXPECTS_F_OR_FVEC, _fname, idx );
      return *this;
    }
    return FVector( idx );
  }

  VerifyArgContext& ScalarOrNVector( size_t idx )
  {
    if (_argTypes[idx].IsNumeric())
      return *this;
    return NVector( idx );     
  }

  // 両者が同じ次元か。型は違っても良い
  VerifyArgContext& SameDim( size_t idx0, size_t idx1 )
  {
    size_t arg0Dim = _argTypes[idx0].ExpandNumericTypes().size();
    size_t arg1Dim = _argTypes[idx1].ExpandNumericTypes().size();
    ASSERT_MSG( arg0Dim == arg1Dim, ResId::FNAME_EXPECTS_THE_SAME_DIM, _fname, idx0, idx1  );
    return *this;
  }

  VerifyArgContext& SameFVector( size_t idx0, size_t idx1 )
  {
    FVector( idx0 );
    FVector( idx1 );
    SameDim( idx0, idx1 );
    return *this;
  }  

  VerifyArgContext& SameType( size_t idx0, size_t idx1 )
  {
    if( _argTypes[idx0].IsTuple())
    {
      ASSERT_MSG(_argTypes[idx1].IsTuple(), ResId::FNAME_EXPECTS_THE_SAME_TYPES, _fname, idx0, idx1 );
      auto expanded0 = _argTypes[idx0].ExpandNumericTypes();
      auto expanded1 = _argTypes[idx1].ExpandNumericTypes();
      ASSERT_MSG( expanded0.size() == expanded1.size(), ResId::FNAME_EXPECTS_THE_SAME_DIM, _fname, idx0, idx1 );
      for( auto nidx : NRange(expanded0.size()) )
      {
        ASSERT_MSG(expanded0[nidx] == expanded1[nidx], ResId::FNAME_EXPECTS_THE_SAME_TYPES, _fname, idx0, idx1 );
      }
    }
    else
    {
      ASSERT_MSG(_argTypes[idx0] == _argTypes[idx1], ResId::FNAME_EXPECTS_THE_SAME_TYPES, _fname, idx0, idx1 );
    }
    return *this;
  }

  // fvectorかfloatのスカラー
  VerifyArgContext& SameGenType( size_t idx0, size_t idx1 )
  {
    if (_argTypes[idx0].IsFloat())
    {
      ASSERT_MSG( _argTypes[idx1].IsFloat(), ResId::FNAME_EXPECTS_THE_SAME_TYPES, _fname, idx0, idx1 );
      return *this;
    }
    // このケースはラベルが良くないが、将来の課題という事で。
    SameFVector( idx0, idx1 );
    return *this;
  }  

  // ベクトルはfvec、スカラーはintかfloat。ちょっとスカラーが特殊
  VerifyArgContext& SameOrScalar( size_t vecIdx, size_t vecOrScalarIdx )
  {
    const auto& targetType = _argTypes[vecOrScalarIdx];
    if (targetType.IsFloat() || targetType.IsInt())
      return *this;
    
    FVector( vecOrScalarIdx );
    SameFVector( vecIdx, vecOrScalarIdx );
    return *this;
  }

  VerifyArgContext& SameNVector( size_t idx0, size_t idx1 )
  {
    NVector( idx0 );
    NVector( idx1 );
    auto expanded0 = _argTypes[idx0].ExpandNumericTypes();
    auto expanded1 = _argTypes[idx1].ExpandNumericTypes();
    ASSERT_MSG( expanded0[0] == expanded1[0], ResId::FNAME_EXPECTS_THE_SAME_VECTOR_ATOM_TYPE_DIFFER, _fname, idx0, idx1 );
    ASSERT_MSG( expanded0.size() == expanded1.size(), ResId::FNAME_EXPECTS_THE_SAME_DIM, _fname, idx0, idx1 );
    return *this;
  }  

  // すべての引数が、同じ型であるかをチェック。スカラーならスカラーとして同じ。
  // ベクトルなら次元と構成要素の型が同じ。
  VerifyArgContext& AllArgsSameType()
  {
    if (_argTypes[0].IsTuple())
    {
      for( auto i : NRange(_argTypes.size()))
      {
        SameNVector( 0, i );
      }
    }
    else
    {
      ASSERT_MSG( _argTypes[0].IsNumeric(), ResId::FNAME_EXPECTS_HOMO_NUMERIC_NON_NUMERIC, _fname );

      for( auto i : NRange(_argTypes.size()))
      {
        ASSERT_MSG( _argTypes[0] == _argTypes[i], ResId::FNAME_EXPECTS_HOMO_NUMERIC_ONE_ARG_DIFFER, _fname, i );
      }
    }

    return *this;
  }

  // 引数が全てi32かu32であることをチェック
  VerifyArgContext& AllUSInt()
  {
    for( auto i : NRange(_argTypes.size()))
    {
      ASSERT_MSG( _argTypes[i].IsUSInt(), ResId::FNAME_EXPECTS_ALL_USINT, _fname, i );
    }
    return *this;
  }

  VerifyArgContext& FixedTypes( const std::vector<Type>& types )
  {
    ArgNum( types.size() );
    for( auto i : NRange( types.size() ))
    {
      ASSERT_MSG( _argTypes[i] == types[i], ResId::FNAME_ARG_TYPE_DIFFER, _fname, i );
    }
    return *this;
  }

  // i32かu32のスカラーかタプル
  VerifyArgContext& USIntScalarOrTuple( size_t idx )
  {
    const auto& targetType = _argTypes[idx];
    if (targetType.IsUSInt())
      return *this;
    
    ASSERT_MSG(targetType.IsTuple(), ResId::FNAME_EXPECTS_USSCALAR_OR_TUPLE, _fname, idx );
    USTuple( idx );
    return *this;
  }

  // i32かu32のスカラーかベクトル
  VerifyArgContext& USIntScalarOrUSIntVector( size_t idx )
  {
    const auto& targetType = _argTypes[idx];
    if (targetType.IsUSInt())
      return *this;
    
    ASSERT_MSG(targetType.IsTuple(), ResId::FNAME_EXPECTS_USSCALAR_OR_TUPLE, _fname, idx );
    USTuple( idx );
    return *this;
  }

  // 先頭がスカラーか、またはベクトルで両方が同じ次元
  // ifelがベクトライズされるなら同じ次元を保証
  VerifyArgContext& USIntScalarOrSameDim( size_t vecOrScalarIdx, size_t vecIdx )
  {
    const auto& targetType = _argTypes[vecOrScalarIdx];
    if (targetType.IsUSInt())
      return *this;
    
    ASSERT_MSG(targetType.IsTuple(), ResId::FNAME_EXPECTS_USSCALAR_OR_TUPLE, _fname, vecOrScalarIdx );
    USTuple( vecOrScalarIdx );
    SameDim( vecOrScalarIdx, vecIdx );
    return *this;
  }

  VerifyArgContext& AllFloat()
  {
    for( auto i : NRange(_argTypes.size()))
    {
      ASSERT_MSG( _argTypes[i].IsFloat(), ResId::FNAME_EXPECTS_ALL_FLOAT, _fname, i );
    }
    return *this;
  }

  // Verifyが全部終わったあとに、展開した型が欲しい事が多いので、
  // 最後に呼ぶ事を前提にしたメソッド。
  Type GetExpandedArg( size_t idx )
  {
    return _argTypes[idx];
  }
};

/*
  Scope関連
*/
using namespace mfg_binary;

struct OneScope
{
  using VarMap = std::map<TString, ExprV>;
  using TsMap = std::map<TString, TensorInfo>;
  using SamplerMap = std::map<TString, ExprT<SamplerElem>>;

  VarMap _varMap;
  TsMap _tsMap;
  SamplerMap _samplerMap;

  // Variableがタプルの時、展開した変数を覚えておく。
  // キーは元となる親のVariableのname。
  // TLEのAddReferenceで使う用。
  // 追記: AddReferenceをパースではなくLower以後にしたのでいらなくなっているかも？
  std::map<std::string, std::vector<ExprV>> _expandedVarMap;

  std::vector<Bound> _boundsCand;

  using var_iterator = VarMap::iterator;
  using cvar_iterator = VarMap::const_iterator;
  using var_pair = VarMap::value_type;

  using ts_iterator = TsMap::iterator;
  using cts_iterator = TsMap::const_iterator;
  using ts_pair = TsMap::value_type;

  using sampler_iterator = SamplerMap::iterator;
  using csampler_iterator = SamplerMap::const_iterator;
  using sampler_pair = SamplerMap::value_type;

  OneScope() = default;
  OneScope(OneScope&&) = default;

  var_iterator FindVariable( const TString& name )
  {
    return _varMap.find( name );
  }

  cvar_iterator FindVariable( const TString& name ) const
  {
    return _varMap.find( name );
  }

  var_iterator VarEnd()
  {
    return _varMap.end();
  }

  cvar_iterator VarEnd() const
  {
    return _varMap.end();
  }

  std::pair<var_iterator, bool> InsertVariable( var_pair&& pair )
  {
    return _varMap.insert( std::move(pair) );
  }

  ts_iterator FindTensor( const TString& name )
  {
    return _tsMap.find( name );
  }

  cts_iterator FindTensor( const TString& name ) const
  {
    return _tsMap.find( name );
  }

  ts_iterator TensorEnd()
  {
    return _tsMap.end();
  }

  cts_iterator TensorEnd() const
  {
    return _tsMap.end();
  }

  std::pair<ts_iterator, bool> InsertTensor( ts_pair&& pair )
  {
    return _tsMap.insert( std::move(pair) );
  }

  csampler_iterator FindSamplerDef( const TString& name ) const
  {
    return _samplerMap.find( name );
  }

  sampler_iterator SamplerEnd()
  {
    return _samplerMap.end();
  }

  csampler_iterator SamplerEnd() const
  {
    return _samplerMap.end();
  }

  std::pair<sampler_iterator, bool> InsertSamplerDef( sampler_pair&& pair )
  {
    return _samplerMap.insert( std::move(pair) );
  }

  void InsertExpandedVariable( const std::string& baseName, std::vector<ExprV>&& items )
  {
    _expandedVarMap.emplace( baseName, std::move(items) );
  }

  /*
    変数がタプルの時、その展開した変数のベクトルを返す。
    たとえば変数がvなら[v.0, v.1, ...]を返す。

    戻りは必ず存在する前提。
  */
  std::vector<ExprV>& FindExpandedVariable( const std::string& baseName )
  {
    return _expandedVarMap[ baseName ];
  }

  template<typename ...ARGS>
  void ASSERT_MSG( bool cond, ResId rid, ARGS... args )
  {
    if (!cond)
      throw FmtUserError( rid, args... );
  }
  
  Variable* DefineVariable( const TString& name, ExprV&& vari )
  {
    ASSERT_MSG( FindVariable( name ) == VarEnd(), ResId::DUPLICATE_VARIABLE_DEFINITION, name.ToString() );

    // Tupleだったらexpandした変数をスコープにぶら下げておく。
    // AddReferenceなどで実体を置く場所がないので。
    if (vari.GetType().IsTuple())
    {
      auto items = vari.GetElem()->ExpandTuple<ExprV>();
      InsertExpandedVariable( vari.GetElem()->Name(), std::move(items) );
    }


    const auto& ret = InsertVariable( std::make_pair( TString(name), std::move( vari ) ) );    
    return ret.first->second.GetElem();
  }

};

struct Callable
{
  enum CallableType
  {
    SAMPLER,
    TENSOR,
    NOT_FOUND
  };

  CallableType _ctype;

  union
  {
    const SamplerElem* _sampler;
    const TensorInfo* _tensor;
  };

  Callable( const SamplerElem* sampler ) : _ctype( SAMPLER ), _sampler( sampler ) {}
  Callable( const TensorInfo* ts ) : _ctype( TENSOR ), _tensor( ts ) {}
  Callable() : _ctype( NOT_FOUND ), _sampler( nullptr ) {}
};

class Scope
{
  std::vector<OneScope> _stack;

public:
  Scope()
  {
    _stack.emplace_back(); // rootのスコープ
  }
  Scope(Scope &&that) noexcept = default;
  Scope &operator=(Scope &&that) noexcept = default;

  // コピーは禁止
  Scope(const Scope &) = delete;
  Scope &operator=(const Scope &) = delete;

  void EnterNewScope()
  {
    _stack.emplace_back();
  }

  void LeaveLastScope()
  {
    _stack.pop_back();
  }

  // 現在のスコープに定義されている変数を全て上のスコープに昇格してからpopする。
  void LeaveLastScopeWithUpgrade()
  {
    assert( _stack.size() >= 2 );
    auto& last = _stack.back();
    auto& secLast = _stack[_stack.size()-2];
    for (auto&& pair: last._varMap)
    {
      secLast.InsertVariable( std::move(pair) );
    }
    for (auto&& pair: last._samplerMap)
    {
      secLast.InsertSamplerDef( std::move(pair) );
    }
    for (auto&& pair: last._expandedVarMap)
    {
      secLast.InsertExpandedVariable( pair.first, std::move(pair.second) );
    }
    _stack.pop_back();
  }

  const Variable* Get( const TString& name ) const
  {
    for (auto oneiter = _stack.crbegin(); oneiter != _stack.crend(); oneiter++)
    {
      auto iter = oneiter->FindVariable( name );
      if (iter != oneiter->VarEnd())
        return iter->second.GetElem();
    }
    return nullptr;
  }

  Variable* Get( const TString& name )
  {
    for (auto oneiter = _stack.rbegin(); oneiter != _stack.rend(); oneiter++)
    {
      auto iter = oneiter->FindVariable( name );
      if (iter != oneiter->VarEnd())
        return iter->second.GetElem();
    }
    return nullptr;
  }

  // グローバルのスコープからだけ探す。
  const Variable* GetG( const TString& name ) const
  {
    auto& top = _stack.front();
    auto iter = top.FindVariable( name );
    if (iter != top.VarEnd())
      return iter->second.GetElem();
    return nullptr;
  }

  Variable* GetG( const TString& name )
  {
    auto& top = _stack.front();
    auto iter = top.FindVariable( name );
    if (iter != top.VarEnd())
      return iter->second.GetElem();
    return nullptr;
  }

  /*
    u3_accmなどのプレフィクス付きの内部名を取り出す。
    これは既に解決済みの時しか呼ばれないはずなのでlookupは必ず成功する。
  */
  const std::string& InnerName( const TString& name ) const
  {
    auto v = Get( name );
    assert( v != nullptr );
    return v->_name;
  }

  std::vector<ExprV>& GetExpandedVariableG( const std::string& baseName )
  {
    auto& top = _stack.front();
    return top.FindExpandedVariable( baseName );
  }

  // パーサーが終わりバイナリに分離する時に、グローバル変数のVariableは引き継ぎたい。
  // そのための関数。これを呼んだあとは_scopeはもう無効。
  std::vector<ExprV> DetachGlobalVariables()
  {
    assert( _stack.size() == 1 );
    auto& top = _stack.front();
    std::vector<ExprV> ret;

    for (auto&& pair : top._varMap)
    {
      ret.emplace_back( std::move(pair.second) );
    }
    for (auto&& pair: top._expandedVarMap)
    {
      for (auto &&varexp: pair.second)
      {
        ret.emplace_back( std::move(varexp) );
      }
    }
    return ret;
  }

  // カーネル内のスコープから探す。
  // グローバルな変数は探索外。
  // free variableかどうかを区別するのに使う。
  const Variable* GetK( const TString& name ) const
  {
    if (_stack.size() == 1)
      return nullptr;

    auto end = --_stack.crend();
    for (auto oneiter = _stack.crbegin(); oneiter != end; oneiter++)
    {
      auto iter = oneiter->FindVariable( name );
      if (iter != oneiter->VarEnd())
        return iter->second.GetElem();
    }
    return nullptr;
  }

  bool Contains( const TString& name ) const
  {
    return Get( name ) != nullptr;
  }

  template<typename ...ARGS>
  void ASSERT_MSG( bool cond, ResId rid, ARGS... args )
  {
    if (!cond)
      throw FmtUserError( rid, args... );
  }
  
  Variable* Define( const TString& name, ExprV&& vari )
  {
    auto& topOne = _stack.back();
    return topOne.DefineVariable( name, std::move(vari) );
  }

  Variable* Ensure( const TString& name, ExprV&& vari )
  {
    auto ret = Get( name );
    if (ret != nullptr)
      return ret;
    return Define( name, std::move(vari) );
  }


  // グローバルなスコープに直接定義したい時に使う。
  // tensorのextentなど
  Variable* DefineG( const TString& name, ExprV&& vari )
  {
    auto& bottomOne = _stack.front();
    ASSERT_MSG( bottomOne.FindVariable( name ) == bottomOne.VarEnd(), ResId::DUPLICATE_GLOBAL_VARIABLE_DEFINITION, name.ToString() );
    return bottomOne.DefineVariable( name, std::move(vari) );
  }

  Variable* EnsureG( const TString& name, ExprV&& vari )
  {
    auto ret = GetG( name );
    if (ret != nullptr)
      return ret;
    return DefineG( name, std::move(vari) );
  }

  //
  // SamplerElem関連
  //
  void DefineSampler( const TString& name, ExprT<SamplerElem>&& sampler )
  {
    auto& topOne = _stack.back();
    topOne.InsertSamplerDef( std::make_pair( TString(name), std::move(sampler) ) );
  }

  const SamplerElem* LookupSampler( const TString& name )
  {
    for (auto oneiter = _stack.crbegin(); oneiter != _stack.crend(); oneiter++)
    {
      auto iter = oneiter->FindSamplerDef( name );
      if (iter != oneiter->SamplerEnd())
        return iter->second.GetElem();
    }
    return nullptr;

  }

  //
  // ExecTensorInitialize関連
  //

  void DefineLocalTensor( const TString& name, TensorInfo&& tsinfo )
  {
    auto& topOne = _stack.back();
    ASSERT_MSG( topOne.FindTensor( name ) == topOne.TensorEnd(), ResId::DUPLICATE_LOCAL_TENSOR_DEFINITION, name.ToString() );
    topOne.InsertTensor( std::make_pair( TString(name), tsinfo ) );
  }

  void DefineLocalTensor( const TString& name, const ExecTensorInitialize& ts )
  {
    DefineLocalTensor( name, ts.GetTensorInfo() );
  }

  // TopLevelのTensorはTreeHolderが持つのでここにあるのはすべてLocalTensor。
  const TensorInfo* LookupLocalTensor( const TString& name ) const
  {
    for (auto oneiter = _stack.crbegin(); oneiter != _stack.crend(); oneiter++)
    {
      auto iter = oneiter->FindTensor( name );
      if (iter != oneiter->TensorEnd())
        return &iter->second;
    }
    return nullptr;
  }

  std::vector<Bound>& BoundsCand()
  {
    return _stack.back()._boundsCand;
  }

  // HandleCallでidentifierをresolveするのに使う。
  // Scopeの中にあるCallable（マクロかLocalTensor）を探す。
  // ローカルテンソルやマクロはスコープの順番で探し、
  // 見つからない時はNOT_FOUNDを返す。
  Callable LookupLocalCallabe( const TString& name ) const
  {
    for (auto oneiter = _stack.crbegin(); oneiter != _stack.crend(); oneiter++)
    {
      auto iter = oneiter->FindTensor( name );
      if (iter != oneiter->TensorEnd())
        return Callable( &iter->second );

      auto miter = oneiter->FindSamplerDef( name );
      if (miter != oneiter->SamplerEnd())
        return Callable( miter->second.GetElem() );
    }
    // NOT_FOUND
    return Callable();
  }
};


/*
  ResourceLoader関連
*/
struct ResourceLoader
{
  virtual bool LoadImage( const std::string& name, mfg_pal::Image32& dest ) = 0 ;
  virtual ~ResourceLoader() {}
};

struct NullLoader : ResourceLoader
{
  bool LoadImage( const std::string& /* name */, mfg_pal::Image32& /* dest */ ) override { return false; }
};

class TreeBuilder : private IRBuildDSL
{
  std::unique_ptr<ResourceLoader> _loader{ new NullLoader() };

  template<typename ...ARGS>
  ParseError FmtReportPError( ResId rid, ARGS... args )
  {
    return _reportParseError( FmtRString( rid, args... ) );
  }

  template<typename ...ARGS>
  ParseError FmtReportPErrorAt( const Token& at, ResId rid, ARGS... args )
  {
    return _reportParseErrorAt( FmtRString( rid, args... ), at );
  }


  VerifyArgContext BeginVerify( std::string fname, const std::vector<Expr>& args )
  {
   return VerifyArgContext( std::move(fname), ToSpreadedTypes( args ), _reportParseError );
  }

  void Register0( std::string fname, std::function<Expr()> func )
  {
    _factories.emplace( std::move(fname), [func, this](std::vector<Expr>&& args) {
        ASSERT_MSG( args.size() == 0, ResId::ARG_NUM_DIFFER, 0, args.size() );
        return func();
      } );
  }


  // 1引数の関数で、vectorizeされうる。
  // 1引数の時は引数はfloatで、returnもfloat。
  // 引数がタプルの時は個々の要素に関数を適用したタプルが結果の型となる。
  void RegisterFloatVectorizableCall( std::string fname, Call::FuncType ftype )
  {
    _factories.emplace( fname, [fname, ftype, this]( std::vector<Expr>&& args )
      {
        auto retType = BeginVerify(fname, args)
          .ArgNum( 1 )
          .FloatOrFTuple( 0 )
          .GetExpandedArg( 0 );

        return Expr( new Call( retType, ftype, std::move(args) ) );
      } );
  }
  void RegisterIfEl( std::string fname )
  {
    _factories.emplace( fname, 
      [fname, this](std::vector<Expr>&& args){
        BeginVerify( fname, args )
          .ArgNum( 3 )
          .USIntScalarOrTuple( 0 )
          .USIntScalarOrSameDim( 0, 1 )
          .SameType( 1, 2 );

        return _Ifel( std::move( args[0] ), std::move( args[1] ), std::move( args[2] ) );
    });
  }

  // f32(expr)などの処理。
  // exprがベクトルの場合はベクトル型へのキャストになる
  Expr CastCall( Type oneArgType, Expr&& expr )
  {
    return _ToType( oneArgType, std::move(expr) );
  }

  void RegisterCast( std::string fname, Type oneArgType )
  {
    _factories.emplace( fname, [fname, oneArgType, this]( std::vector<Expr>&& args )
      {
        return CastCall( oneArgType, std::move(args[0]) );
      } );
  }


  // genericだが第一引数がreturn typeになるもの。
  // maxとかclampとか
  void RegisterReturnFirstArgTypeCall( std::string fname, Call::FuncType ftype, size_t argNum )
  {
    _factories.emplace( fname,
      [argNum, ftype, fname, this](std::vector<Expr>&& args){

        auto first = BeginVerify( fname, args )
                      .ArgNum( argNum )
                      .GetExpandedArg( 0 );

        return Expr( new Call( first, ftype, std::move(args) ) );
      });
  }

  void ASSERT_ARG_NUM( std::string funcName, const std::vector<Expr>& args, size_t expectedNum  )
  {
    BeginVerify( funcName, args )
      .ArgNum( expectedNum );
  }

  /*
    Inline関数の引数が定義にあっているかの型チェックをする。
    retTypeを返す（genTypeの場合にそれを解決する、それ以外はinlf._retTypeを返す)
  */
  Type VerifyInlineCall( const InlineFuncInfo& iinfo, const std::vector<Expr>& args )
  {
    /*
      今の所Inline関数のgenTypeは以下のみをサポート。
      
      - genType f(arg:genType)

      これ以外は引数もretTypeもNORMAL_TYPEのみ対応。
      厳密一致で型チェックする。
    */

    if (!iinfo._paramTypes.empty() && iinfo._paramTypes[0].IsGenType() )
    {
      assert( iinfo._retType.IsGenType() );
      return BeginVerify( iinfo._name, args )
                .ArgNum( 1 )
                .FloatOrFTuple( 0 )
                .GetExpandedArg( 0 );

    }
    else
    {
      // これ以外をRegisterする日が来たら直す。
      assert( iinfo._retType.IsNormalType() );

      auto ftypes = MapFn( iinfo._paramTypes, []( const ExtendType& et ){ 
        // パーサーの時点でgenTypeの引数は登録していなく、
        // Bultinでも登録していないのでこの変換は必ず成功する。
        assert( et.IsNormalType() );
        return et._type;
      });

      BeginVerify( iinfo._name, args )
        .FixedTypes( ftypes );

      return iinfo._retType._type;
    }
  }

  /*
    predefinedなinline関数の登録。
  */
  void RegisterInlineFunc( InlineFunction& inlf )
  {
    // inlfの参照はvectorに要素が追加されると動く場合があるので、
    // iinfoをコピーにしてラムダに留める。
    InlineFuncInfo iinfo = inlf._info;
    _factories.emplace( iinfo._name,
      [iinfo, this](std::vector<Expr>&& args){
        auto first = VerifyInlineCall( iinfo, args );

        auto call = new Call( first, Call::INLINE, std::move(args) );
        call->_inlfId = iinfo._inlfId;
        return Expr( call );
      });
  }

  void RegisterAllInlineFunctions()
  {
    for (auto& inlf: _binary->_inlfRegistry._inlfs)
    {
      RegisterInlineFunc( inlf );
    }
  }


  // 引数をExprRangeにする。ExprRangeでなければコンパイルエラーとして扱う。
  std::vector<ExprRANGE> TryConvRanges( std::vector<Expr>::iterator beg, std::vector<Expr>::iterator end, ResId rid )
  {
    std::vector<ExprRANGE> ranges;
    auto cur = beg;
    while( cur != end )
    {
      ASSERT_MSG( (*cur).GetElemType() == IRElemType::RangeElem, rid );
      ranges.emplace_back( (*cur).Detach()->As<RangeElem>() );
      cur++;
    }
    return ranges;
  }

  // tpがf32のタプルの時、同じ次元のi32のタプルの型を返す。
  // tpがスカラーの時はスカラーのi32のTypeを返す
  // boolと言うがMFGではi32
  Type SameWithBool( const Type& tp )
  {
    if (!tp.IsTuple())
      return Int(32);
    std::vector<NumericType> ntypes;
    for( auto& n : tp.ExpandNumericTypes() )
    {
      ntypes.push_back( NumericType::Int(32) );
    }
    return Type( std::move(ntypes) );
  }

  // 引数をBlockElemかどうかチェックし、BlankIdをtmp varに置き換える
  // BlockElem出なければコンパイルエラー
  template<typename ...ARGS>
  ExprT<BlockElem> TryFillBlankId( Expr&& fobjCand, ResId rid, ARGS... args )
  {
    auto fobj = fobjCand.As<BlockElem>();
    ASSERT_MSG( fobj != nullptr, rid, args... );
    fobj->BlankId2TmpVar( 0, fobj->_args.size() );
    return ExprT<BlockElem>( fobjCand.Detach()->As<BlockElem>() );
  }

  Expr CallPrimitive( const std::string& fname, std::vector<Expr>&& args )
  {
      auto fiter = _factories.find( fname );
      // 呼び出し元でチェック済み
      assert( fiter != _factories.end() );
      return fiter->second( std::move(args) );
  }

  void SetupPrimitives()
  {
    _factories.emplace( "all", 
      [this](std::vector<Expr>&& args){
        BeginVerify( "all", args )
          .ArgNum( 1 )
          .USIntScalarOrUSIntVector( 0 );

        return Expr( new Call( Int(32), Call::ALL, std::move(args) ) );
    });
    _factories.emplace( "any", 
      [this](std::vector<Expr>&& args){
        BeginVerify( "any", args )
          .ArgNum( 1 )
          .USIntScalarOrUSIntVector( 0 );

        return Expr( new Call( Int(32), Call::ANY, std::move(args) ) );
    });
    _factories.emplace( "xor", 
      [this](std::vector<Expr>&& args){
        BeginVerify( "xor", args )
          .ArgNum( 2 )
          .USIntScalarOrUSIntVector( 0 )
          .SameType( 0, 1 );

        // Callでは無くBinOpとする。
        // スカラーとベクトライズの組み合わせは
        // 手抜きで両方同じ型の時だけとする。
        // つまり xor([1, 2], 3) みたいなのはサポートしない。
        return _BinOpGen( BinOp::BitwiseXor, std::move(args[0]), std::move(args[1]) );
    });
    _factories.emplace( "rsum", 
      [this](std::vector<Expr>&& args){
        auto ranges = TryConvRanges( args.begin(), args.end() -1, ResId::RSUM_ARG_MUST_BE_RANGE );
        auto fobj = TryFillBlankId( std::move(args.back()), ResId::LAST_ARG_MUST_BE_BLOCK, "rsum" );

        return HandleReduceSum( std::move(ranges), std::move(fobj) );
    });
    _factories.emplace( "reduce", 
      [this](std::vector<Expr>&& args){
        auto ranges = TryConvRanges( args.begin()+1, args.end() -1, ResId::REDUCE_ARG_MUST_BE_RANGE );
        auto fobj = TryFillBlankId( std::move(args.back()), ResId::LAST_ARG_MUST_BE_BLOCK, "reduce" );

        return HandleReduce( std::move(args.front()), std::move(ranges), std::move(fobj) );
    });
    RegisterIfEl( "ifel" );
    // ifelのalias。
    RegisterIfEl( "elif" );

    // elseはidentiyとして引数をそのままかえすだけ。
    // 本来はifelのあとでないといけない訳だが、そういう事は気にせずidentity関数として実装する。
    _factories.emplace( "else", 
      [this](std::vector<Expr>&& args){
        // 他の引数と一致するかどうかはチェクしない。
        // ...が解決された後にifelの引数としてチェックされる。
        BeginVerify( "else", args )
          .ArgNum( 1 );

        return std::move(args[0]);
    });

    RegisterCast( "u8", UInt(8) );
    RegisterCast( "u16", UInt(16) );
    RegisterCast( "i32", Int(32) );
    RegisterCast( "u32", UInt(32) );
    RegisterCast( "f32", Float(32) );
    // 端数関連
    RegisterFloatVectorizableCall( "ceil", Call::CEIL );
    RegisterFloatVectorizableCall( "floor", Call::FLOOR );
    RegisterFloatVectorizableCall( "fract", Call::FRACT );
    RegisterFloatVectorizableCall( "round", Call::ROUND );
    RegisterFloatVectorizableCall( "trunc", Call::TRUNC );
    // 三角関数等
    RegisterFloatVectorizableCall( "exp", Call::EXP_F32 );
    RegisterFloatVectorizableCall( "exp2", Call::EXP2 );
    RegisterFloatVectorizableCall( "sin", Call::SIN_F32 );
    RegisterFloatVectorizableCall( "cos", Call::COS_F32 );
    RegisterFloatVectorizableCall( "tan", Call::TAN_F32 );
    RegisterFloatVectorizableCall( "log", Call::LOG );
    RegisterFloatVectorizableCall( "log2", Call::LOG2 );
    RegisterFloatVectorizableCall( "saturate", Call::SATURATE );
    RegisterFloatVectorizableCall( "sign", Call::SIGN );
    RegisterFloatVectorizableCall( "sqrt", Call::SQRT_F32 );


    // ベクトル関係
    _factories.emplace( "atan2",
      [this](std::vector<Expr>&& args){
        auto first =
          BeginVerify( "atan2", args )
            .ArgNum( 2 )
            .SameType( 0, 1 )
            .FloatOrFVector( 0 )
            .GetExpandedArg( 0 );
          
        return Expr( new Call( first, Call::ATAN2_F32, std::move(args) ) );
    });
    _factories.emplace( "clamp", 
      [this](std::vector<Expr>&& args){
        auto first = 
          BeginVerify( "clamp", args )
            .ArgNum( 3 )
            .ScalarOrNVector( 0 )
            .GetExpandedArg( 0 );

        return Expr( new Call( first, Call::CLAMP, std::move(args) ) );
    });
    _factories.emplace( "length", 
      [this](std::vector<Expr>&& args){
        BeginVerify( "length", args )
          .ArgNum( 1 )
          .FVector( 0 );
        return Expr( new Call( Float(32), Call::LENGTH, std::move(args) ) );
    });
    _factories.emplace( "cross", 
      [this](std::vector<Expr>&& args){
        auto first = BeginVerify( "cross", args )
          .ArgNum( 2 )
          .NVector( 0 )
          .SameNVector( 0, 1 )
          .GetExpandedArg( 0 );
        
        return Expr( new Call( first, Call::CROSS, std::move(args) ) );
    });
    _factories.emplace( "dot", 
      [this](std::vector<Expr>&& args){
        BeginVerify( "dot", args )
          .ArgNum( 2 )
          .SameFVector( 0, 1 );
        return Expr( new Call( Float(32), Call::DOT, std::move(args) ) );
    });
    _factories.emplace( "distance", 
      [this](std::vector<Expr>&& args){
        BeginVerify( "distance", args )
          .ArgNum( 2 )
          .SameFVector( 0, 1 );
        return Expr( new Call( Float(32), Call::DISTANCE, std::move(args) ) );
    });
    _factories.emplace( "isinf", 
      [this](std::vector<Expr>&& args){
        auto first = BeginVerify( "isinf", args)
                      .ArgNum( 1 )
                      .FloatOrFTuple( 0 )
                      .GetExpandedArg( 0 );

        auto retType = SameWithBool( first );
        return Expr( new Call( retType, Call::ISINF, std::move(args) ) );
    });
    _factories.emplace( "isnan", 
      [this](std::vector<Expr>&& args){
        auto first = BeginVerify( "isnan", args)
                      .ArgNum( 1 )
                      .FloatOrFTuple( 0 )
                      .GetExpandedArg( 0 );

        auto retType = SameWithBool( first );
        return Expr( new Call( retType, Call::ISNAN, std::move(args) ) );
    });
    _factories.emplace( "normalize", 
      [this](std::vector<Expr>&& args){
        auto first = BeginVerify( "normalize", args )
                      .ArgNum( 1 )
                      .FVector( 0 )
                      .GetExpandedArg( 0 );
        return Expr( new Call( first, Call::NORMALIZE, std::move(args) ) );
    });
    _factories.emplace( "mix", 
      [this](std::vector<Expr>&& args){
        auto first = BeginVerify( "mix", args)
                      .ArgNum( 3 )
                      .SameGenType( 0, 1 )
                      .SameOrScalar( 0, 2 )
                      .GetExpandedArg( 0 );
        return Expr( new Call( first, Call::MIX, std::move(args) ) );
    });
    _factories.emplace( "smoothstep", 
      [this](std::vector<Expr>&& args){
        auto retType = BeginVerify( "smoothstep", args )
                      .ArgNum( 3 )
                      .FloatOrFVector( 2 )
                      .SameOrScalar( 2, 0 )
                      .SameGenType( 0, 1 )
                      .GetExpandedArg( 2 );
        return Expr( new Call( retType, Call::SMOOTHSTEP, std::move(args) ) );
    });
    _factories.emplace( "step", 
      [this](std::vector<Expr>&& args){
        auto retType = BeginVerify( "step", args )
                      .ArgNum( 2 )
                      .FloatOrFVector( 1 )
                      .SameOrScalar( 1, 0 )
                      .GetExpandedArg( 1 );                      
        return Expr( new Call( retType, Call::STEP, std::move(args) ) );
    });

    _factories.emplace( "max", 
      [this](std::vector<Expr>&& args){
        auto retType = BeginVerify( "max", args )
                        .ScalarOrNVector( 0 )
                        .AllArgsSameType()
                        .GetExpandedArg( 0 );

        return Expr( new Call( retType, Call::MAX, std::move(args) ) );
    });
    _factories.emplace( "min", 
      [this](std::vector<Expr>&& args){
        auto retType = BeginVerify( "min", args )
                        .ScalarOrNVector( 0 )
                        .AllArgsSameType()
                        .GetExpandedArg( 0 );

        return Expr( new Call( retType, Call::MIN, std::move(args) ) );
    });
    _factories.emplace( "vec2",
      [this](std::vector<Expr>&& args){
        auto first =
          BeginVerify( "vec2", args )
            .ArgNum( 1 )
            .Scalar( 0 )
            .GetExpandedArg( 0 );
        
        std::vector<Type> types( 2, first );

        return Expr( new Call( TupleType( types ), Call::VEC2, std::move(args) ) );
    });
    _factories.emplace( "vec3", 
      [this](std::vector<Expr>&& args){
        auto first = 
          BeginVerify( "vec3", args )
            .ArgNum( 1 )
            .Scalar( 0 )
            .GetExpandedArg( 0 );
        
        std::vector<Type> types( 3, first );

        return Expr( new Call( TupleType( types ), Call::VEC3, std::move(args) ) );
    });
    _factories.emplace( "vec4", 
      [this](std::vector<Expr>&& args){
        auto first = 
          BeginVerify( "vec4", args )
            .ArgNum( 1 )
            .Scalar( 0 )
            .GetExpandedArg( 0 );
        
        std::vector<Type> types( 4, first );

        return Expr( new Call( TupleType( types ), Call::VEC4, std::move(args) ) );
    });
    Register0( "rand", []() {
      return ExprT<Call>( new Call( Float(32), Call::RAND_F32, std::vector<Expr>{}  ) );
    });
    Register0( "fore_color", [this]() {
        _binary->_sparams.emplace_back( SParamInfo::FORE_COLOR, FORE_COLOR_VNAME, std::vector<float>{0.0F, 0.0F, 1.0F, 1.0F} );
        return ExprV( new Variable( F32V4Type(), FORE_COLOR_VNAME, true ) );  // Free variable.
    });
    Register0( "back_color", [this]() {
        _binary->_sparams.emplace_back( SParamInfo::BACK_COLOR, BACK_COLOR_VNAME, std::vector<float>{0.0F, 0.0F, 1.0F, 1.0F} );
        return ExprV( new Variable( F32V4Type(), BACK_COLOR_VNAME, true ) );  // Free variable.
    });
    RegisterReturnFirstArgTypeCall( "abs", Call::ABS, 1 );
    _factories.emplace( "to_ncoord",
      [this](std::vector<Expr>&& args){
        ASSERT_MSG( _curEntity._etype == TLEntityRef::TENSOR, ResId::TONCOORD_MUST_BE_INSIDE_TSDEF );
        ASSERT_MSG( IsIntTuple2D( args[0].GetType() ), ResId::TONCOORD_ARG_MUST_BE_2D_INT );
        
        return Expr( new TensorCall( TupleType( { Float(32), Float(32) } ), _curEntity.u._tensor->GetTensorInfo(), TensorCall::TO_NCOORD, true, std::move(args) ) );
      });
    RegisterAllInlineFunctions();
  }


  /*
    intかuintのタプルで2Dである事をチェック
  */
  bool IsIntTuple2D( const Type& tp )
  {
    if (!tp.IsTuple())
      return false;

    auto ntps = tp.ExpandNumericTypes();
    if (ntps.size() != 2)
      return false;

    for( auto& ntp : ntps )
    {
      if( (!ntp.IsInt()) && (!ntp.IsUInt()) )
      {
        return false;
      }
    }
    return true;
  }

  std::map<std::string, std::function<Expr(std::vector<Expr>&&)>, TStringComparator> _factories;
  Scope _scope;
  TLEntityRef _curEntity; // デフォルトはUNBOUND
  std::unique_ptr<TLBlock> _tblockCand;

public:
  std::unique_ptr<IRBinary> _binary;
  std::function<ParseError(const std::string& msg, const Token& at)> _reportParseErrorAt;
  std::function<ParseError(const std::string& msg)> _reportParseError;

  TreeBuilder() : _binary( new IRBinary() )
  {
    SetupPrimitives();
  }

  void SetLoader( std::unique_ptr<ResourceLoader>&& loader ) {
    _loader = std::move(loader);
  }

  IRBinary* DetachEntityHolder()
  { 
    _binary->OwnGlobalVariables( _scope.DetachGlobalVariables() );
    return _binary.release();
  }

  template<typename ...ARGS>
  void ASSERT_MSG_AT( bool cond, const Token& tk, ResId rid, ARGS... args )
  {
    if (!cond)
    {
      throw FmtReportPErrorAt( tk, rid, args... );
    }
  }

  // スコープ内に定義されている変数名かどうか
  bool IsDefinedVariable( const Token& varToken )
  {
    return _scope.Get( varToken._str) != nullptr;
  }

  ExprV HandleVariable( const Token& varToken )
  {
    auto vari = _scope.GetK( varToken._str );
    if (vari != nullptr)
    {
      return ExprV( (Variable*)vari->Clone() );
    }

    // カーネル内のスコープに無い、つまりfree variable。
    // グローバルスコープからとるとともに、変数の参照はオープンな参照であるフラグを設定
    auto gvari = _scope.GetG( varToken._str );
    ASSERT_MSG_AT( gvari != nullptr, varToken, ResId::UNKNOWN_VAR_REF, varToken._str.ToString() );

    auto variRef = ExprV( (Variable*)gvari->Clone() );
    variRef.GetElem()->_freeV = true;
    return variRef;
  }

  ExprT<ImmElem> HandleIntImm( int32_t ival )
  {
    return ExprT<ImmElem>( new ImmElem( Int(32), ival ) );
  }

  ExprT<ImmElem> HandleUIntImm( uint32_t uival )
  {
    return ExprT<ImmElem>( new ImmElem( mfg_internal::UInt(32), uival ) );
  }

  ExprT<ImmElem> HandleFloatImm( float fval )
  {
    return ExprT<ImmElem>( new ImmElem( Float(32), fval ) );
  }

  void ASSERT_SAME_TUPLE_TYPE( BinOp::BinOpType opType, const Type& ltp, const Type& rtp )
  {
    auto binopName = BinOp::ToDisplayName( opType );
    auto header = "binop(" + BinOp::ToDisplayName( opType ) + ")";
    auto lntypes = ltp.ExpandNumericTypes();
    auto rntypes = rtp.ExpandNumericTypes();

    ASSERT_MSG( lntypes.size() == rntypes.size(), ResId::BINOP_ARGS_DIM_DIFFER, binopName );
    for( auto dim : NRange(lntypes.size()) )
    {
      ASSERT_MSG( lntypes[dim] == rntypes[dim], ResId::BINOP_ARG_TYPE_DIFFER, binopName, dim );
    }
  }

  // 0xff << 8 とかは型が違っていても問題ないのでスキップ。
  // その他判定が難しいものはスキップ。
  bool NeedsCompatArgBinOp( BinOp::BinOpType opType )
  {
    if (opType == BinOp::ShiftLeft || opType == BinOp::ShiftRight)
      return false;
    return true;
  }

  void ASSERT_COMPAT_NTYPE( NumericType na, NumericType nb )
  {
    if (na.IsFloat() || nb.IsFloat())
    {
      // いつもFloatには出来るので、compatible
      return;
    }
    else if (na.IsInt() && nb.IsUInt())   
    {
      // intとuintがあったら、intが大きければintにする、それ以外はパースエラー。
      ASSERT_MSG( na._bits > nb._bits, ResId::SIGN_UNSIGN_MISMATCH );
      return;
    }
    else if (na.IsUInt() && nb.IsInt())
    {
      // 同上
      ASSERT_MSG( nb._bits > na._bits, ResId::SIGN_UNSIGN_MISMATCH );
      return;    
    }
  }

  void ASSERT_VECTORIZE_TYPE( BinOp::BinOpType opType, const Type& scalarType, const Type& vecType )
  {
    if (!NeedsCompatArgBinOp(opType) )
      return;
    
    auto ntypes = vecType.ExpandNumericTypes();
    auto sntype = scalarType.AsNumeric();

    for( auto dim : NRange(ntypes.size()) )
    {
      ASSERT_COMPAT_NTYPE( sntype, ntypes[dim] );
    }
  }

  /*
    Lowerまで行ってしまうと字句情報を失うので、なるべくこの時点で型チェックをする。
    ただし難しいものは諦める。
  */
  void ASSERT_BINOP_ARG_TYPE( BinOp::BinOpType opType, const Type& ltp, const Type& rtp )
  {
    if (ltp.IsTuple() && rtp.IsTuple())
    {
      ASSERT_SAME_TUPLE_TYPE( opType, ltp, rtp );
    }
    else if (ltp.IsTuple() || rtp.IsTuple())
    {
      if (ltp.IsTuple())
      {
        ASSERT_VECTORIZE_TYPE( opType, rtp, ltp );
      }
      else
      {
        ASSERT_VECTORIZE_TYPE( opType, ltp, rtp );
      }
    }
  }

  Expr HandleBin( BinOp::BinOpType opType, Expr&& lop, Expr&& rop )
  {
    // チェックはir_utilを呼び出す前に行う。
    ASSERT_BINOP_ARG_TYPE( opType, lop.GetType(), rop.GetType() );
    return _BinOpGen( opType, std::move(lop), std::move(rop) );
  }

  BinOp::BinOpType ParserBinOpTypeToIRBinOpType( BinOpType btype )
  {
    switch (btype)
    {
      case BinOpType::ADD:
        return BinOp::Add;
      case BinOpType::SUB:
        return BinOp::Sub;
      case BinOpType::MUL:
        return BinOp::Mul;
      case BinOpType::DIV:
        return BinOp::Div;
      case BinOpType::MOD:
        return BinOp::Mod;
      case BinOpType::EQEQ:
        return BinOp::Eq;
      case BinOpType::NEQ:
        return BinOp::Neq;
      case BinOpType::OR:
        return BinOp::Or;
      case BinOpType::AND:
        return BinOp::And;
      case BinOpType::LT:
        return BinOp::Lt;
      case BinOpType::LE:
        return BinOp::Le;
      case BinOpType::GT:
        return BinOp::Gt;
      case BinOpType::GE:
        return BinOp::Ge;
      case BinOpType::RSHIFT:
        return BinOp::ShiftRight;
      case BinOpType::LSHIFT:
        return BinOp::ShiftLeft;
      case BinOpType::BITWISE_AND:
        return BinOp::BitwiseAnd;
      case BinOpType::BITWISE_OR:
        return BinOp::BitwiseOr;
      case BinOpType::POW:
      case BinOpType::UNKNOWN:
      default:
        throw InternalError( "Illegal binop type." );
    }
  }

  Expr HandleBinOp( BinOpType btype, Expr&& lop, Expr&& rop )
  {
    if (btype == BinOpType::POW)
    {
      // POWはcall。
      return Expr( new Call( lop.GetType(), Call::POW, { std::move(lop), std::move(rop) } ) );
    }
    auto irbtype = ParserBinOpTypeToIRBinOpType( btype );
    return HandleBin( irbtype, std::move(lop), std::move(rop) );
  }

  Expr HandleUnaryMinus( Expr&& op )
  {
    auto t = op.GetType();

    // Immの時は-の値にする。それ以外は 0 - op とする。
    if (op.IsConst())
    {
      auto imm = op.As<ImmElem>();
      assert( imm != nullptr );
      
      if (t.IsInt())
      {
        return MakeConst( t, -imm->Value<int32_t>() );
      }
      else if(t.IsUInt())
      {
        // UIntの-の単項、微妙だが変数と挙動を揃えるべくUIntのままにしておく。
        return MakeConst( t, -static_cast<int32_t>( imm->Value<uint32_t>() ) );
      }
      else
      {
        assert(t.IsFloat());
        return MakeConst( t, -imm->Value<float>() );
      }
    }
    else
    {
      return _Sub( MakeZero(t), std::move(op) );
    }
  }

  Expr HandleUnaryLogicalNot( Expr&& op )
  {
    return Expr( new Call( op.GetType(), Call::LOGICAL_NOT, { std::move( op ) } ));
  }

  ExprT<CompoundAssignment> HandleCompoundAssignment( ExprT<TensorCall>&& load, Expr&& rexpr )
  {
    return ExprT<CompoundAssignment>( new CompoundAssignment( std::move(load), std::move(rexpr) ) );
  }

  void VerifySamplerCallArgs( const SamplerElem* sampDef, std::vector<Expr>& args )
  {    
    auto tsDim = sampDef->_target.Dimensions();
    if (sampDef->_coordType == SamplerElem::PIXEL_COORD)
    {
      BeginVerify( "sampler", args )
        .ArgNum( tsDim )
        .AllUSInt();
    }
    else
    {
      BeginVerify( "float sampler", args )
        .ArgNum( tsDim )
        .AllFloat();
    }
  }

  Expr CreateGlobalExtentVariable( const std::string& extentName, bool /* insideTarget */)
  {
    auto tstr = TString::Create( extentName );
    auto vari = _scope.EnsureG( tstr, _Var( Int(32), extentName ) );

    Expr varRef( vari->CloneBaseExpr() );
    varRef.As<Variable>()->_freeV = true;
    return varRef;
  }

  Expr CreateLocalExtentVariable( const std::string& extentName )
  {
    auto tstr = TString::Create( extentName );
    auto vari = _scope.Ensure( tstr, _Var( Int(32), extentName ) );
    return Expr( vari->CloneBaseExpr() );
  }

  Expr CreateExtentCall( const TensorInfo& ts, const Token& methodId, std::vector<Expr>&& args, std::function<Expr(const std::string&)> createVarFunc )
  {
    if (args.size() == 0)
    {
      // ts.extent() は全extentのタプルを返す。
      std::vector<Expr> varis;

      for( auto dim : NRange( ts.Dimensions() ) )
      {
        auto extentName = ts.GetExtentName( dim );      

        varis.push_back( createVarFunc( extentName ) );
      }
      return Expr( new TupleElem( std::move(varis) ) );
    }
    else
    {
      ASSERT_MSG_AT( args.size() == 1 && args[0].IsConst(), methodId, ResId::EXTENT_FOR_NON_IMM_NYI ); // 今の所immだけ対応。必要になったらvariableも対応する
      auto dim = (size_t)args[0].AsInt();

      auto extentName = ts.GetExtentName( dim );      

      return createVarFunc( extentName );
    }
  }

  // intバージョンのextent
  Expr HandleTensorExtentICall( const Token& targetId, const Token& methodId, std::vector<Expr>&& args )
  {
    auto gtptr = LookupTensor( targetId );
    if (gtptr != nullptr)
    {
      bool insideTarget =  ( _curEntity._etype == TLEntityRef::TENSOR && targetId._str == _curEntity.u._tensor->Name() );
      return CreateExtentCall( gtptr->GetTensorInfo(), methodId, std::move(args), [ insideTarget, this ]( const std::string& extentName ) {
        return CreateGlobalExtentVariable( extentName, insideTarget );
      });
    }
    else
    {
      auto ltptr = LookupLocalTensor( targetId );
      ASSERT_MSG_AT( ltptr != nullptr, targetId, ResId::TENSOR_NOT_FOUND, targetId.ToString() );
      return CreateExtentCall( *ltptr, methodId, std::move(args), [ this ]( const std::string& extentName ) {
        return CreateLocalExtentVariable( extentName );
      });
    }
  }


  Expr HandleTensorExtentCall( const Token& targetId, const Token& methodId, std::vector<Expr>&& args )
  {
    auto iextent = HandleTensorExtentICall( targetId, methodId, std::move(args) );
    if (methodId.IsEqual( "extentf" ))
    {
      return _ToF32( std::move(iextent) );
    }
    return iextent;
  }

  Expr HandleIsInsideCall( const TensorInfo& ts, const Token& /* targetId */, const Token& methodId, std::vector<Expr>&& args )
  {
    // 現状methodのVerifyはパーサー側に仕組みが無い。
    // もう少しメソッドが増えたら作る。
    // 今はここで手でverifyする。

    ASSERT_MSG_AT( ts.Dimensions() == args.size(), methodId, ResId::ARG_NUM_DIFFER, ts.Dimensions(), args.size() );
    for( auto i : NRange( args.size() ) )
    {
      ASSERT_MSG_AT( args[i].GetType().IsUSInt(), methodId, ResId::FNAME_EXPECTS_ALL_USINT, methodId.ToString(), i );
    }
    
    return Expr( new TensorCall( Int(32), ts, TensorCall::IS_INSIDE, true, std::move( args ) ) );
  }

  // 特殊なのばかりなので、実質これは今のところis_insideのみ
  Expr HandleTensorSimpleCall( const Token& targetId, const Token& methodId, std::vector<Expr>&& args )
  {
    assert( methodId.IsEqual( "is_inside" ) );

    auto gtptr = LookupTensor( targetId );
    if (gtptr != nullptr)
    {
      return HandleIsInsideCall( gtptr->GetTensorInfo(), targetId, methodId, std::move(args) );
    }

    auto ltptr = LookupLocalTensor( targetId );
    ASSERT_MSG_AT( ltptr != nullptr, targetId, ResId::TENSOR_NOT_FOUND, targetId.ToString() );
    return HandleIsInsideCall( *ltptr, targetId, methodId, std::move(args) );
  }

  Expr HandleMethodCall( const Token& targetId, const Token& methodId, std::vector<Expr>&& args )
  {
    if (methodId.IsEqual( "extent" ) || methodId.IsEqual( "extentf" ))
    {
      return HandleTensorExtentCall( targetId, methodId, std::move(args) );
    }
    else if(methodId.IsEqual("is_inside"))
    {
      return HandleTensorSimpleCall( targetId, methodId, std::move(args) );
    }
    else if (targetId._ttype != TokenType::UNKNOWN && ( methodId.IsEqual( "for_each" ) || methodId.IsEqual( "sum" ) ))
    {
      // tensor iterator関連
      
      TensorInfo tsinfo("", false, 0, Int(32));

      auto found = LookupTensorInfo( targetId, tsinfo );
      ASSERT_MSG_I( found, ResId::UNKNOWN_REFERENCE_OF_TS_NEVER_HAPPENS );

      ASSERT_MSG_AT( args.size() == 1, methodId, ResId::TS_ITER_WRONG_ARG_NUM );
      auto fobjPtr = args[0].Detach()->As<BlockElem>();
      ASSERT_MSG_AT( fobjPtr != nullptr, methodId, ResId::TS_ITER_ARG_MUST_BE_BLOCK );

      // ループインデックスのBlankIdはここでtemp変数に置き換える。
      fobjPtr->BlankId2TmpVar( 0, tsinfo.Dimensions() );

      ExprT<BlockElem> fobj( fobjPtr );

      auto itype = methodId.IsEqual( "for_each" ) ? TensorIterator::EXEC_FOREACH : TensorIterator::REDUCE_SUM;
      return Expr( new TensorIterator( itype, tsinfo, std::move(fobj) ) );
    }
    else
    {
      throw FmtReportPErrorAt( methodId, ResId::UNKNOWN_METHOD, methodId.ToString() );
    }
  }

  ExprT<SwizzleCall> HandleSwizzleCall( Expr&& tupTarget, std::vector<size_t>&& indices )
  {
    return ExprT<SwizzleCall>( new SwizzleCall( std::move(tupTarget), std::move( indices ) ) );
  }

  ExprT<TransformTensor> HandleTensorTransform( const Token& targetId, const Token& methodId, std::vector<Expr>&& args )
  {
    auto ts = LookupLocalTensor( targetId );
    ASSERT_MSG_I( ts != nullptr, ResId::UNKNOWN_REFERENCE_OF_LOCAL_TS_NEVER_HAPPENS );

    if (methodId.IsEqual("accumulate"))
    {
      ASSERT_MSG_AT( args.size() == 2, methodId, ResId::ACCM_WRONG_ARG_NUM );
      auto fobjPtr = args[1].Detach()->As<BlockElem>();
      ASSERT_MSG_AT( fobjPtr != nullptr, methodId, ResId::LAST_ARG_MUST_BE_BLOCK, "accumulate" );

      fobjPtr->BlankId2TmpVar( 0, ts->Dimensions() );

      ASSERT_MSG_AT( fobjPtr->_args.back().first != "_", methodId, ResId::ACCM_MUST_NOT_BE_UNDERSCORE );

      return ExprT<TransformTensor>( new TransformTensor( TensorInfo( *ts ), TransformTensor::ACCM, { std::move(args[0]), Expr(fobjPtr) } ) );
    }
    else if (methodId.IsEqual("sort"))
    {
      ASSERT_MSG_AT( args.size() == 1, methodId, ResId::ARG_NUM_DIFFER, 1, args.size() );
      return ExprT<TransformTensor>( new TransformTensor( TensorInfo( *ts ), TransformTensor::SORT, { std::move(args[0]) } ) );
    }
    else if (methodId.IsEqual("cumsum"))
    {
      ASSERT_MSG_AT( args.size() == 1, methodId, ResId::ARG_NUM_DIFFER, 1, args.size() );
      ASSERT_MSG_AT( ts->ElemNum() == 1, targetId, ResId::TUPLE_CUMSUM_NYI );

      // accumulateのcumsum相当のBlockを組み立てる。
      // argは
      // |x, y, val, accm| ...
      // で、最初のx, yなどはtensorの次元から、valとaccmはtensorの要素の型から作る。
      std::vector<std::pair<std::string, Type>> argTypes;

      // tensorのループインデックス
      for (auto i : NRange(ts->Dimensions()))
      {
        argTypes.emplace_back( UniqueName( 't' ), Int(32) );
      }

      // val
      auto valType = ts->ElemType( 0 );
      auto valName = UniqueName( 't' );
      argTypes.emplace_back( valName, valType );

      // accmの変数
      auto accmName = UniqueName( 't' );
      argTypes.emplace_back( accmName, valType );

      // Blockの中身。 val+accm
      auto vblock = _BodyElem( {
          _Add(
            _Var( valType, valName ),
            _Var( valType, accmName )
          )
        }
      );

      return ExprT<TransformTensor>( new TransformTensor( TensorInfo( *ts ), TransformTensor::ACCM, { std::move(args[0]), Expr(new BlockElem( std::move(argTypes), std::move(vblock) )) } ) );
    }
    else
    {
      //　パーサー側ですでにvalidなどれかなのは確認済み
      throw FmtInternalError( ResId::UNKNOWN_TRANS, methodId.ToString() );
    }
  }

  struct ReduceTensorAccmData
  {
    Token _tsTk; // パースエラーの場所用
    const TensorInfo& _ts;
    Expr _dim;
    Expr _init;
    ExprT<BlockElem> _fobj;

    ReduceTensorAccmData( const Token& tsTk, const TensorInfo& ts, Expr&& dim, Expr&& init, ExprT<BlockElem>&& fobj ) :
      _tsTk( tsTk ), _ts( ts ), _dim( std::move(dim) ), _init( std::move(init) ), _fobj( std::move(fobj) ) {}

    ReduceTensorAccmData( ReduceTensorAccmData&& ) = default;
  };

  ExprVOID DefByReduceAccumulate( const Token& destTs, const Token& srcTs, const TensorInfo& ts, Expr&& dim, Expr&& init, ExprT<BlockElem>&& fexpr )
  {
    ASSERT_MSG_AT( fexpr.GetElem()->_args.back().first != "_", srcTs, ResId::ACCM_MUST_NOT_BE_UNDERSCORE );

    if (ts.Dimensions() == 1)
    {
      // 0次元へのreduce。結果は変数になる。変数を生成するのはLowerの責任。
      // ReduceToScalarのExprとそれを右辺とするLetを生成する。
      ASSERT_MSG_AT( ts.ElemNum() == 1, srcTs, ResId::REDUCE_TO_ZERO_TUPLE_NYI );

      auto reduceToZero = new ReduceToScalar( TensorInfo( ts ), std::move( dim ), std::move( init ), std::move( fexpr ) ) ;
      // def tmp2 by ... とあったら、
      // let tmp2 = _rname
      // となるようにする。
      // 直接reduceToZeroのrnameをtmp2にしてしまっても良いのだけれど、
      // 将来的にexprの中の方でも同じ扱いに出来るように一旦_rnameにする事にする。
      return HandleLet( destTs, Expr(reduceToZero) ).AsVOID();
    }
    else
    {
      auto defByReduce = new DefByReduce( TensorInfo( ts ), std::move( dim ), std::move( init ), std::move( fexpr ) ) ;
      _scope.DefineLocalTensor( destTs._str, defByReduce->GetTensorInfo() );

      return ExprVOID( defByReduce );
    }

  }

  /*
    fobjの組み立て直し。以下を
    |i1, i2, val| { ...stmts... vexpr }

    accmを追加して、以下のように変形
    |i1, i2, val, accm|
    {
      ...stmts...
      ifel( accm != -1, accm,
            elif( vexpr, ix, -1))
    }

    ixはdimで指定されたindex。
  */
  ExprT<BlockElem> FindFirstIndexFObj2AccmFObj( ExprT<BlockElem>&& fobj, int dim, const Token& errorPos )
  {
    auto argTypes = std::move( fobj.GetElem()->_args );
    ASSERT_MSG_AT( dim < (int)argTypes.size()-1, errorPos, ResId::DIM_OUT_OF_BOUNDS, dim, argTypes.size()-1 );
    auto targetArgPair = argTypes[ dim ];

    auto accmName = UniqueName( 't' );
    argTypes.emplace_back( accmName, Int(32) );

    auto bodyVec = fobj.GetElem()->GetBody()->DetachExprs();
    auto rexpr = _Ifel(
      _Neq( _Var( Int(32), accmName ), Expr( -1 ) ),
      _Var( Int(32), accmName ),
      _Ifel(
        Expr( bodyVec.back().Detach() ),
        _Var( targetArgPair ),
        Expr( -1 )
      )
    );
    bodyVec.back() = std::move(rexpr);
    return ExprT<BlockElem>( new BlockElem( std::move(argTypes), _BodyElem( std::move(bodyVec) ) ) );
  }


  ReduceTensorAccmData ExpandToAccumulate( ConvertTensorTree&& convTree )
  {
    auto& args = convTree._args;
    auto convType = convTree._convType;
    auto convMethod = convTree._method;
    auto targetTs = convTree._targetTs;

    ASSERT_MSG_AT( convType.IsEqual("reduce"), convType, ResId::UNKNOWN_CONVERTER, convType.ToString() );
    auto ts = LookupLocalTensor( targetTs );
    ASSERT_MSG_AT( ts != nullptr, targetTs, ResId::TENSOR_NOT_FOUND, targetTs.ToString() );

    if (convMethod.IsEqual( "accumulate" ))
    {
      ASSERT_MSG_AT( args.size() == 3, convMethod, ResId::ARG_NUM_DIFFER, 3, args.size() );
      ASSERT_MSG_AT( args[0].GetElemType() == IRElemType::ImmElem, convType, ResId::DIM_MUST_BE_IMM );
      ASSERT_MSG_AT( args[2].GetElemType() == IRElemType::BlockElem, convType, ResId::LAST_ARG_MUST_BE_BLOCK, "reduce" );

      ExprT<BlockElem> fexpr( args[2].Detach()->As<BlockElem>() );
      fexpr.GetElem()->BlankId2TmpVar( 0, ts->Dimensions() );
      return ReduceTensorAccmData( targetTs, *ts, std::move( args[0] ), std::move( args[1] ), std::move( fexpr ) );
    }
    else if (convMethod.IsEqual( "find_first_index" ))
    {
      /*
        reduce<ts>.find_first_index(dim=0) |ix, iy, val| { vexpr }
        まずは型をチェック
      */
      ASSERT_MSG_AT( args.size() == 2, convMethod, ResId::ARG_NUM_DIFFER, 2, args.size() );
      ASSERT_MSG_AT( args[0].GetElemType() == IRElemType::ImmElem, convType, ResId::DIM_MUST_BE_IMM );
      ASSERT_MSG_AT( args[1].GetElemType() == IRElemType::BlockElem, convType, ResId::LAST_ARG_MUST_BE_BLOCK, "reduce<>.find_first_Index" );

      ExprT<BlockElem> fexpr( args[1].Detach()->As<BlockElem>() );
      fexpr.GetElem()->BlankId2TmpVar( 0, ts->Dimensions() );

      auto fexpr2 = FindFirstIndexFObj2AccmFObj( std::move(fexpr), args[0].As<ImmElem>()->Value<int32_t>(), convMethod );
      return ReduceTensorAccmData( targetTs, *ts, std::move( args[0] ), Expr(-1), std::move( fexpr2 ) );
    }
    else
    {
      // パーサー側でチェックしているはずなのでここには来ないはず。
      throw FmtInternalError( ResId::UNKNOWN_REDUCE, convMethod.ToString() );
    }
  }

  ExprVOID HandleDefByConvert( const Token& destTs, ConvertTensorTree&& convTree )
  {
    auto accmData = ExpandToAccumulate( std::move(convTree) );
    return DefByReduceAccumulate( destTs, accmData._tsTk, accmData._ts, std::move(accmData._dim), std::move(accmData._init), std::move(accmData._fobj) );
  }

  Expr HandleConvertToScalar( ConvertTensorTree&& convTree )
  {
    auto accmData = ExpandToAccumulate( std::move(convTree) );
    return Expr(new ReduceToScalar( TensorInfo( accmData._ts ), std::move(accmData._dim), std::move(accmData._init), std::move(accmData._fobj) ) );
  }

  bool IsPrimitive( const Token& identifier )
  {
    return _factories.find( identifier._str ) != _factories.end();
  }

  ExprT<TensorCall> HandleGlobalTensorCall( GlobalTensorLike& tslike, std::vector<Expr>&& args )
  {
    BeginVerify( "Tensor " + tslike.TsName(), args )
      .AllUSInt();
    return _TsCall( tslike.GetTensorInfo(), std::move( args ) );
  }

  // SamplerやTsCallの事もあるのでBaseExprElemを返す
  Expr HandleCall( const Token& identifier, std::vector<Expr>&& args )
  {
    auto callable = _scope.LookupLocalCallabe( identifier._str );
    if (callable._ctype == Callable::SAMPLER)
    {
      const SamplerElem* sampDef = callable._sampler;
      
      VerifySamplerCallArgs( sampDef, args );
      
      return Expr( new SamplerCall( sampDef, std::move(args) ) );
    }
    else if (callable._ctype == Callable::TENSOR)
    {
      BeginVerify( "Tensor " + callable._tensor->Name(), args )
        .AllUSInt();
      return _TsCall( *callable._tensor, std::move( args ) );
    }
    else
    {
      // こちらはグローバルのTensor
      auto tptr = LookupTensor( identifier );
      if (tptr != nullptr)
      {
        return HandleGlobalTensorCall( *tptr, std::move( args ) );
      }

      // プリミティブの関数
      // CallPrimitiveとほぼ同じだが無いケースが微妙に共通化出来なかった。
      auto fiter = _factories.find( identifier._str );
      if (fiter != _factories.end())
      {
        return fiter->second( std::move(args) );
      }

      throw FmtReportPErrorAt( identifier, ResId::UNKNOWN_FUNCTION, identifier.ToString() );
    }
  }

  // inputで他のレイヤーを参照する場合。特別扱い。
  ExprT<TensorCall> HandleInputCall( const Token& identifier, IOTensorType /* itype */, int layerIndex, std::vector<Expr>&& args )
  {
    auto input = LookupInput( identifier, layerIndex );
    // パース側でチェック出来ているのでこのケースは起こらないはず。
    assert( input != nullptr );

    return HandleGlobalTensorCall( *input, std::move( args) );
  }

  // 今の所SamplerTargetでしか使われない。
  // globalなテンソルかinput_uXXかinput_uXX[IMM]。
  GlobalTensorLike& HandleTensorReference( const Token& ident, IOTensorType itype, int layerIndex )
  {
    if (itype == IOTensorType::UNSPECIFIED)
    {
      // input_uXXじゃない。layerIndexなどは無視
      auto gtptr = LookupTensor( ident );    
      ASSERT_MSG_AT( gtptr != nullptr, ident, ResId::UNKNOWN_SAMPLER_TARGET, ident._str.ToString() );
      return *gtptr;
    }
    else
    {
      return *LookupInput( ident, layerIndex );
    }
  }

  void VerifySamplerArgs( SamplerElem::CoordType /* ctype */, SamplerElem::AddressType addressMode, const std::map<std::string, Expr>& argMap, Type targetType, size_t /* targetDim */ )
  {
    if (addressMode == SamplerElem::CLAMP_TO_BORDER_VALUE)
    {
      auto iter = argMap.find( "border_value" );
      ASSERT_MSG( iter != argMap.end(), ResId::MISSING_BORDER_VALUE_FOR_CLAMP_TO_BORDER_VALUE );
      ASSERT_MSG( iter->second.GetType() == targetType, ResId::CLAMP_TO_BORDER_VALUE_ARG_TYPE_DIFFER );
    }
  }

  // 一時的にsamplerの処理はこちらで行う。将来はこちら一本になる。
  //   virtual ExprT<SamplerElem> HandleCreateSamplerElem( const Token& samplerId, const Token& tsId, std::map<std::string, Expr>&& argMap ) = 0;
  ExprT<SamplerElem> HandleCreateSamplerElem( const Token& /* samplerId */, GlobalTensorLike& samplerTarget, std::map<std::string, Expr>&& argMap )
  {
    auto addressType = SamplerElem::NORMAL_EDGE;
    auto coordType = SamplerElem::PIXEL_COORD;

    auto coordArg = argMap.find( "coord" );
    if (coordArg != argMap.end())
    {
      // この時点ではi32のIMMになっているはず。
      assert( coordArg->second.GetType().IsInt() );
      switch( coordArg->second.AsInt() )
      {
        case 1:
          coordType = SamplerElem::NORMALIZED_COORD_NEARESTNEIGHBOR;
          break;
        case 2:
          coordType = SamplerElem::NORMALIZED_COORD_BILINEAR;
          break;
      }
    }

    auto addressArg = argMap.find( "address" );
    if (addressArg != argMap.end())
    {
      // この時点ではi32のIMMになっているはず。
      assert( addressArg->second.GetType().IsInt() );
      switch( addressArg->second.AsInt() )
      {
        case 1:
          addressType = SamplerElem::CLAMP_TO_EDGE;
          break;
        case 2:
          addressType = SamplerElem::CLAMP_TO_BORDER_VALUE;
          break;
      }
    }

    auto tsinfo = samplerTarget.GetTensorInfo();
    VerifySamplerArgs( coordType, addressType, argMap, tsinfo.GetType(), tsinfo.Dimensions() );
    std::vector<Expr> args;
    if (addressType == SamplerElem::CLAMP_TO_BORDER_VALUE)
    {
      // border_valueが存在して型があっているのは既にチェック済み。
      args.push_back( std::move( argMap.find("border_value")->second ) );
    }

    return ExprT<SamplerElem>( new SamplerElem( addressType, coordType, tsinfo, std::move(args) ) );
  }

  template<typename ...ARGS>
  void ASSERT_MSG( bool cond, ResId rid, ARGS... args )
  {
    if (!cond)
      throw _reportParseError( FmtRString( rid, args... ) );
  }

  // InternalError
  template<typename ...ARGS>
  void ASSERT_MSG_I( bool cond, ResId rid, ARGS... args )
  {
    if (!cond)
      throw FmtInternalError( rid, args... );
  }

  /*
    let文などにぶつかった時に、TLEが無ければTopLevelBlockを作って_curEntityとする。
    現在はrevertが面倒なのでlazyにTopLevelBlockを必要になった時に作っている。    
  */
  void EnterTopLevelBlock()
  {
    // internal errorかも
    ASSERT_MSG( _curEntity._etype == TLEntityRef::UNBOUND, ResId::ENTER_TOPLEVEL_BLOCK_INSIDE_SOME_BLOCK );
    _tblockCand.reset( new TLBlock() );
    _curEntity.Assign( _tblockCand.get() );

    // TopLevelBlockが複数ある時に、前のLetはツリーに含まれないのでカーネルの引数で渡す必要がある。
    // その為にAddReferenceするかどうかを現在のカーネルスコープにあるかどうかでHandleVariableで判断している。
    // だからトップレベルブロックでもこのEnterNewScopeと終わる時にLeaveしつつUpgradeという事が必要になる。
    _scope.EnterNewScope();
  }

  /*
    UserVarName、内部的にはu3_などのu+数字で始める。(#965, #1140)
    スコープ上はu3_がつかない名前。
  */
  static std::string ToUVName( const std::string& vname )
  {
    if (vname == "_")
      return vname;
    return UniqueName('u') + "_" + vname;
  }

  static std::string ToUVName( const Token& vident )
  {
    return ToUVName( vident.ToString() );
  }

  ExprLET HandleLet( const Token& identifier, Expr&& rexpr )
  {
    auto uvname = ToUVName( identifier );
    _scope.Define( identifier._str, _Var( rexpr.GetType(), uvname ) );
    return ExprLET( new Let( uvname, std::move(rexpr) ) );
  }

  ExprBODY HandleBody( std::vector<Expr>&& bodys )
  {
    return ExprBODY( new BodyElem( std::move( bodys ) ) );
  }


  // let [a, b, c] = rexpr
  // をTupleLetとして返す。
  // a, b, cなどは変数としてScopeに定義される。
  ExprT<TupleLet> HandleTupleLet( std::vector<Token> lefts, Expr&& rexpr )
  {
    // スコープに変数を定義しつつ、シンタックスエラーもチェック。
    ASSERT_MSG( rexpr.GetType().IsTuple(), ResId::NO_TUPLE_IN_TUPLE_LET );

    auto types = rexpr.GetBase()->_type.ExpandNumericTypes();

    ASSERT_MSG_AT( types.size() == lefts.size(), lefts[0], ResId::TUPLE_LET_SIZE_DIFFER, lefts.size(), types.size());

    std::map<TString, std::string> vmap;
    for (auto i : NRange(types.size()))
    {
      auto iname = ToUVName( lefts[i] );
      vmap[lefts[i]._str] = iname;

      if (lefts[i]._ttype == TokenType::BLANK_ID)
        continue;
      
      auto ntp = types[i];
      _scope.Define( lefts[i]._str, _Var( Type(ntp), std::move(iname) ) );
    }

    return ExprT<TupleLet>( new TupleLet( MapFn( lefts, [&vmap, this](const Token& tk) { return vmap[tk._str]; } ), std::move(rexpr) ) );
  }

  std::vector<Bound> _boundsCand;

  std::vector<ExprLET> RegisterBoundsCand( std::vector<Bound>& boundsCand, std::vector<Expr>&& args )
  {
    ASSERT_MSG( boundsCand.size() == 0, ResId::DUPLICATE_BOUNDS );

    std::vector<ExprLET> ret;
    for (auto&& arg : args)
    {
      if (arg.IsConst())
      {
        boundsCand.emplace_back( (size_t)arg.AsInt() );
      }
      else if (arg.IsVariable())
      {
        auto vari = arg.As<Variable>();
        boundsCand.emplace_back( vari->_name );
      }
      else
      {
        auto tmpName = UniqueName( 't' );
        auto tmpVar = _Var( mfg_internal::UInt(32), tmpName );
        ret.push_back( _Let( tmpName,  std::move(arg) ) );

        boundsCand.emplace_back( tmpName );
      }
    }

    return ret;
  }

  // 次のtensor defで使うboundを作る。
  // でもテンポラリのletなどが必要になるかもしれないのでtop level blockの一部としてパースする。
  std::vector<ExprLET> HandleBoundsAttribute( std::vector<Expr>&& args )
  {
    if (_curEntity._etype == TLEntityRef::TENSOR )
    {
      // local tensor
      return RegisterBoundsCand( _scope.BoundsCand(), std::move(args) );
    }
    else 
    {
      return RegisterBoundsCand( _boundsCand, std::move(args) );
    }
  }

  /*
    @print_expr(sin(3.14/4)) を、

    let tmp_var = sin(3.14/4)
    @print_expr(tmp_var)

    と解釈して、letを返す。
  */
  ExprLET HandlePrintExpr( Expr&& arg )
  {
    ASSERT_MSG( _curEntity._etype == TLEntityRef::TOP_LEVEL_BLOCK, ResId::PRINT_EXPR_OUTSIDE_OF_TOP );
    ASSERT_MSG( arg.GetType().IsNumeric(), ResId::PRINT_EXPR_NONE_NUMERIC );

    auto tmpName = UniqueName( 't' );
    _curEntity.u._tblock->AddLogEntry( arg.GetType(), tmpName );

    auto retLet = _Let( tmpName, std::move(arg) );

    return retLet;
  }

  void DefineVariable( const TString& varName, ExprV&& vari )
  {
    _scope.Define( varName, std::move( vari ) );
  }

  void DefineVariable( const Token& varNameTk, ExprV&& vari )
  {
    DefineVariable( varNameTk._str, std::move(vari) );
  }

  std::vector<std::string> ToInnerNames( const std::vector<std::pair<bool, std::string>>& args ) const
  {
    return MapFn( args, [this](const std::pair<bool, std::string>& p){ return p.first? p.second : InnerName(p.second); } );
  }

  void EnterRangeBlock( const std::vector<std::pair<bool, std::string>>& args )
  {
    _scope.EnterNewScope();
    for( const auto& arg : args)
    {
      bool isTmp = arg.first;
      const std::string& vname = arg.second;
      std::string innerName = isTmp ? vname: ToUVName( vname );
      DefineVariable( TString::Create( vname ), _Var( Int(32), innerName ) );
    }
  }

  void EnterTensorDef( const Token& tname, const std::vector<std::pair<bool, std::string>>& args )
  {
    // 今の所tensorのdefはトップレベルだけ。
    ASSERT_MSG( _curEntity._etype == TLEntityRef::UNBOUND, ResId::TOP_LEVEL_TS_DEF_INSIDE_SOME_ENTITY );

    auto tnamestr = tname.ToString();

    TLTensor* next;

    // まずresult_xxかどうかチェック。
    next = _binary->LookupResult( tnamestr );

    if (next != nullptr)
    {
      // result_xxxだった。
      // 
      // resultにboundsのattributeがあるのは不正。
      ASSERT_MSG( _boundsCand.size() == 0, ResId::BOUNDS_FOR_RESULT_TS );
    }
    else
    {
      ASSERT_MSG_AT( _boundsCand.size() != 0, tname, ResId::MISSING_BOUNDS, tname.ToString() );
      ASSERT_MSG_AT( _boundsCand.size() == args.size(), tname, ResId::BOUNDS_AND_TS_ARGNUM_DIFFER, _boundsCand.size(), args.size() );

      next = &_binary->NewTensor( tnamestr, args.size() );

      _binary->RegisterBounds()
        .OfTensor( *next, std::move(_boundsCand) );
      _boundsCand.clear();
    }

    _curEntity.Assign( next );
    _binary->PushTLEntity( _curEntity );

    EnterRangeBlock( args );
  }

  void EnterLocalTensorDef( const Token& tname, const std::vector<std::pair<bool, std::string>>& args )
  {
    ASSERT_MSG_AT( _scope.BoundsCand().size() != 0, tname, ResId::MISSING_BOUNDS_FOR_LOCAL_TS, tname.ToString() );

    EnterRangeBlock( args );
  }

  ExprT<ExecTensorInitialize> HandleLocalTensorDefAndLeave( const Token& identifier, std::vector<std::pair<bool, std::string>>&& argNames, ExprT<BodyElem>&& vblock )
  {
    auto nargNames = ToInnerNames( argNames );
    _scope.LeaveLastScope();

    auto& boundsCand = _scope.BoundsCand();

    assert( boundsCand.size() != 0 );
    std::vector<size_t> resolvedBounds;
    for (auto& b : boundsCand)
    {
      ASSERT_MSG_AT( b.IsImm(), identifier, ResId::LOCAL_TS_BOUNDS_MUST_BE_IMM );
      resolvedBounds.push_back( b._imm );
    }
    boundsCand.clear();

    auto ret = _ExecTsInit(
      std::move(resolvedBounds),
      _TsElem(
        std::move(nargNames),
        std::move(vblock)
      )
    );

    // ローカル変数のスコープ内ではこのExecTensorInitializeが探せるようにする。
    // 探した結果はts(x, y)や、ts[1](x, y)として使うだけなので名前と型情報だけが必要。
    _scope.DefineLocalTensor( identifier._str, *ret.GetElem() );
    return ret;
  }

  void AppendIndexArgs( std::vector<std::pair<Token, Type>>& dest, const std::vector<Token>& args, size_t indexNum )
  {
    for (auto i : NRange(indexNum))
    {
      dest.push_back( std::make_pair( args[i], Int(32) ) );
    }
  }

  /*
    | x, y, val, ...| のvalまでを登録。valはindexNumの次と想定。
  */
  void AppendTensorIterateArgs( std::vector<std::pair<Token, Type>>& dest, const TensorInfo& ts, const std::vector<Token>& args, size_t indexNum )
  {
    AppendIndexArgs( dest, args, indexNum );
    const auto& valToken = args[indexNum];
    dest.push_back( std::make_pair( valToken, ts.GetType() ) );
  }

  /*
    ts.sum |x, y, val| { ... }

    などのケース。  
    argsのうち、最後の一つはvalue、それ以外はindex。
  */
  std::vector<std::pair<Token, Type>> ResolveAsTensorIteratorArgs( const Token& tname, const std::vector<Token>& args )
  {
    TensorInfo ts("", false, 0, Int(32));

    auto found = LookupTensorInfo( tname, ts );
    ASSERT_MSG( found, ResId::TENSOR_NOT_FOUND, tname.ToString() );

    std::vector<std::pair<Token, Type>> ret;

    // まずはindexを生成
    for (auto i : NRange(args.size()-1))
    {
      ret.push_back( std::make_pair( args[i], Int(32) ) );
    }
    const auto& valToken = args.back();
    ret.push_back( std::make_pair( valToken, ts.GetType() ) );
    return ret;
  }

  /*
    trans<hist>.accumlate!(0) |i, h, accm| {...}
    など。とりあえずaccmはhと同じとする。
    つまりtensoriteratorの型に最後elemの型を追加する感じ。
  */
  std::vector<std::pair<Token, Type>> ResolveAsTransformTensorArgs( const Token& tname, const std::vector<Token>& args )
  {
    auto ts = LookupLocalTensor( tname );
    ASSERT_MSG( ts != nullptr, ResId::LOCAL_TS_NOT_FOUND, tname.ToString() );
    // このケースはreduceの対応で動くようになったかもしれないが、確認してみないと分からないのでassertは残しておく。引っかかるケースが作れたら確認して削除してOK。
    ASSERT_MSG( ts->ElemNum() == 1, ResId::TUPLE_TRNSFORM_NYI ); // tupleはNYI

    std::vector<std::pair<Token, Type>> ret;

    AppendTensorIterateArgs( ret, *ts, args, args.size() - 2 );

    const auto& accmToken = args.back();
    ret.push_back( std::make_pair( accmToken, ts->GetType() ) );
    return ret;
  }

  /*
    def XXX by reduce<hist>.accumulate(dim=0, init=1) |i, h, accm| {...}
    など。
    <trans>.accumulateと似ているが、accmの型はinitと同じとする。
  */
  std::vector<std::pair<Token, Type>> ResolveAsReduceAccmArgs( const Token& tname, const std::vector<Expr>& plainArgs, const std::vector<Token>& args )
  {
    auto ts = LookupLocalTensor( tname );
    ASSERT_MSG( ts != nullptr, ResId::LOCAL_TS_NOT_FOUND, tname.ToString() );
    // このケースはreduceの対応で動くようになったかもしれないが、確認してみないと分からないのでassertは残しておく。引っかかるケースが作れたら確認して削除してOK。
    ASSERT_MSG( ts->ElemNum() == 1, ResId::TUPLE_REDUCE_NYI ); // tupleはNYI
    ASSERT_MSG( plainArgs.size() == 2, ResId::ARG_NUM_DIFFER, 2, plainArgs.size() );

    auto initType = plainArgs[1].GetType();

    std::vector<std::pair<Token, Type>> ret;

    AppendTensorIterateArgs( ret, *ts, args, args.size() - 2 );

    const auto& accmToken = args.back();
    ret.push_back( std::make_pair( accmToken, initType ) );
    return ret;
  }

  /*
    def XXX by reduce<hist>.find_first_index(dim=0) |i1, i2, h| {...}
    reduce.accumulateとの違いはaccmが無い事。
  */
  std::vector<std::pair<Token, Type>> ResolveAsReduceFindFirstIndexArgs( const Token& tname, const std::vector<Expr>& plainArgs, const std::vector<Token>& args )
  {
    auto ts = LookupLocalTensor( tname );
    ASSERT_MSG( ts != nullptr, ResId::LOCAL_TS_NOT_FOUND, tname.ToString() );
    // このケースはreduceの対応で動くようになったかもしれないが、確認してみないと分からないのでassertは残しておく。引っかかるケースが作れたら確認して削除してOK。
    ASSERT_MSG( ts->ElemNum() == 1, ResId::TUPLE_REDUCE_NYI ); // tupleはNYI
    ASSERT_MSG( plainArgs.size() == 1, ResId::ARG_NUM_DIFFER, 1, plainArgs.size() );

    std::vector<std::pair<Token, Type>> ret;

    AppendTensorIterateArgs( ret, *ts, args, args.size() - 1 );

    return ret;
  }

  // trans<ts>.accumulate!(a, b) |...| の時に、...の所を解決する。
  // 
  // この時、
  // convType = trans
  // parentTarget = ts
  // parentMethodId = accumulate
  // plainArgs = a, b
  // となる。
  // convTypeはコンバータじゃない時はDummyToken（使わない）
  std::vector<std::pair<Token, Type>> ResolveBlockArgTypes( const Token& convType, const Token& parentTarget, const Token& parentMethodId, const std::vector<Expr>& plainArgs, const std::vector<Token>& args )
  {
    if (convType.IsEqual("trans"))
    {
      if (parentMethodId.IsEqual("accumulate"))
      {
        return ResolveAsTransformTensorArgs( parentTarget, args );
      }
      else
      {
        // parseでチェック済みなので来ないかもしれないが自信が無いので同じ処理をしておく。
        throw FmtReportPErrorAt( parentMethodId, ResId::UNKNOWN_TRANS, parentMethodId.ToString() );
      }
    }
    else if(convType.IsEqual("reduce"))
    {
      if (parentMethodId.IsEqual("accumulate"))
      {
        return ResolveAsReduceAccmArgs( parentTarget, plainArgs, args );
      }
      else if (parentMethodId.IsEqual("find_first_index"))
      {
        return ResolveAsReduceFindFirstIndexArgs( parentTarget, plainArgs, args );
      }
      else
      {
        // パースでチェック済みで来ないかもしれないが自信が無いので同じ処理をしておく。
        throw FmtReportPErrorAt( parentMethodId, ResId::UNKNOWN_REDUCE, parentMethodId.ToString() );
      }      
    }
    else if (parentMethodId.IsEqual("sum") || parentMethodId.IsEqual("for_each"))
    {
      return ResolveAsTensorIteratorArgs( parentTarget, args );
    }
    else if (parentMethodId.IsEqual("reduce"))
    {
      // |rx, ry, accm| {...}
      // rx, ryはint、accmはinitと同じ型
      if (args.size() > 3)
      {
        throw FmtReportPErrorAt( parentMethodId, ResId::UNSUPPORTED_DIM_REDUCE, args.size()-1 );
      }
      std::vector<std::pair<Token, Type>> ret;
      for( auto i : NRange(args.size()) )
      {
        // 最後だけaccm
        if (i == args.size()-1)
        {
          ret.push_back( std::make_pair( args[i], plainArgs[0].GetType() ) );
        }
        else
        {
          // それ以外は添字なのでi32
          ret.push_back( std::make_pair( args[i], Int(32) ) );
        }
      }
      return ret;
    }
    else if(parentMethodId.IsEqual( "rsum" ))
    {
      return MapFn( args, [](const Token& arg) { return std::make_pair( arg, Int(32) ); } );
    }
    else
    {
      throw FmtReportPErrorAt( parentMethodId, ResId::UNKNOWN_METHOD, parentMethodId.ToString() );
    }
  }

  /*
    u3_accmなどのプレフィクス付きの内部名を取り出す。
    これは既に解決済みの時しか呼ばれないはずなのでlookupは必ず成功する。
  */
  const std::string& InnerName( const TString& name )
  {
    return _scope.InnerName( name );
  }

  std::string InnerName( std::string name ) const
  {
    return _scope.InnerName( TString::Create(std::move(name)) );
  }

  void EnterBlockScope( const std::vector<std::pair<Token, Type>>& args )
  {
    _scope.EnterNewScope();
    for (auto& arg : args)
    {
      // blank_id以外をスコープに入れる
      if (arg.first._ttype != TokenType::BLANK_ID)
        DefineVariable( arg.first, _Var( arg.second, ToUVName(arg.first) ) );
    }
  }

  void LeaveBlockScope()
  {
    _scope.LeaveLastScope();
  }

  void EnterFunctionScope( const std::vector<std::pair<Token, Type>>& args )
  {
    /*
      インライン関数のformal parametersは_は無い。
      変数名はinline functionのcallの所でインデックスをつけたものにreplaceされるので、
      この時点ではプレフィクス無しの名前で良い。
    */
    _scope.EnterNewScope();
    for (auto& arg : args)
    {
      DefineVariable( arg.first, _Var( arg.second, arg.first.ToString() ) );
    }
  }


  void HandleTensorDefAndLeave( const Token& identifier, std::vector<std::pair<bool, std::string>>&& argNames, ExprT<BodyElem>&& vblock )
  {
    ASSERT_MSG_I( _curEntity._etype == TLEntityRef::TENSOR, ResId::TENSOR_LEAVE_OUTSIDE_TENSOR_NEVER_HAPPEN );

    auto args = MapFn( ToInnerNames(argNames), [](const std::string& name){ return std::make_pair( name, mfg_internal::Int(32) ); } );
    _scope.LeaveLastScope();

    TLTensor* tensor = _curEntity.u._tensor;
    _curEntity._etype = TLEntityRef::UNBOUND;

    tensor->Define( ExprT<mfg_internal::TensorElem>( new mfg_internal::TensorElem( std::move(args), std::move( vblock ) ) ) );

    AssertResultType( identifier, tensor );
  }

  void HandleFunctionDefAndLeave( const Token& identifier, const std::vector<std::pair<Token, Type>>& formal , ExprBODY&& vblock )
  {
    LeaveBlockScope();

    std::vector<ExtendType> paramTypes = MapFn( formal, []( const std::pair<Token, Type>& one ) { return ExtendType( ExtendType::NORMAL, one.second ); } );

    auto& fref = _binary->_inlfRegistry.Push(
      InlineFunction( identifier.ToString(), std::move(paramTypes), ExtendType( ExtendType::NORMAL, vblock.GetElem()->GetReturnExpr()->_type ),
        [this, formal, vblock]( std::vector<Expr>&& args ) {
          std::vector<Expr> newBody;

          for( auto i : NRange( args.size() ))
          {
            newBody.push_back(
              // inline callの所でプレフィクスをつけたものにreplaceされるので、
              // この時点ではローカル変数の名前はプレフィクス無しで良い。
              _PrivateLet( formal[i].first.ToString(), std::move(args[i]) )
            );
          }
          auto oldBody = vblock.CloneT();
          auto exprs = oldBody.GetElem()->DetachExprs();
          AppendTail( newBody, std::move(exprs) );
          return _BodyElem( std::move(newBody) );
        }
      )
    );
    RegisterInlineFunc( fref );
  }

  // Tensor定義の最後でresult_XXXだったら型があってるかチェック
  void AssertResultType(const Token& identifier, TLTensor* tensor )
  {
    if (identifier.IsEqual("result_u8"))
    {
      ASSERT_MSG( tensor->GetType().IsU8V4(), ResId::RESULT_U8_MUST_BE_U8VEC );
    }
    else if (identifier.IsEqual("result_u16"))
    {
      ASSERT_MSG( tensor->GetType().IsU16V4(), ResId::RESULT_U16_MUST_BE_U16VEC );
    }
  }

  void HandleTopLevelBlockDefAndLeave( std::vector<Expr>&& lets )
  {
    ASSERT_MSG_I( _curEntity._etype == TLEntityRef::TOP_LEVEL_BLOCK, ResId::TOPLEVEL_LEAVE_OUTSIDE_TOPLEVEL_NEVER_HAPPEN );

    if (lets.size() == 0)
    {
      _tblockCand.reset( nullptr );
      _curEntity._etype = TLEntityRef::UNBOUND;
      _scope.LeaveLastScopeWithUpgrade(); // サンプラーのletはscopeに入るのでこのケースでもUpgradeが要る
      return;
    }
    _binary->RegisterTopLevelBlock( _tblockCand.release() );
    _binary->PushTLEntity( _curEntity );

    _curEntity.u._tblock->Define( ExprT<BodyElem>( new BodyElem( std::move(lets) )) );
    _scope.LeaveLastScopeWithUpgrade();

    _curEntity._etype = TLEntityRef::UNBOUND;
  }

  void HandleTensorLiteralDef( const Token& identifier, TensorLiteralData&& tdata )
  {
    _binary->NewConstTensor( identifier.ToString(), tdata._type, std::move(tdata._data), std::move(tdata._extents) );
  }

  void HandleResourceTensorDef( const Token& identifier, const Token& resName )
  {
    mfg_pal::Image32 tmp;
    std::string fname = resName.ToString();
    if (!_loader->LoadImage( fname, tmp ))
    {
      throw FmtReportPError( ResId::IMAGE_RESOURCE_NOT_FOUND, fname );
    }
    _binary->NewConstTensor( identifier.ToString(), tmp );
  }

  Expr HandleReduceSum( std::vector<ExprRANGE>&& ranges, ExprT<BlockElem>&& fobj )
  {
    return Expr( new ReduceSum( std::move(ranges), std::move(fobj) ) );
  }

  Expr HandleReduce( Expr&& initValue, std::vector<ExprRANGE>&& ranges, ExprT<BlockElem>&& fobj )
  {
    ASSERT_MSG( ranges.size() == 1 || ranges.size() == 2, ResId::UNSUPPORTED_DIM_REDUCE, ranges.size() );

    return Expr( new Reduce( std::move(initValue), std::move(ranges), std::move(fobj) ) );
  }

  void HandleTitleDef( const std::string& title )
  {
    _binary->_title = title;
  }

  void HandleVersionDef( const std::string& version )
  {
    _binary->_version = version;
  }

  void HandleParamSliderInt32( const Token& identifier, std::vector<AttrNamedArg> rest )
  {
    std::string label = "(empty)";
    int minVal = 1;
    int maxVal = 300;
    int initVal = 10;

    for( auto& narg: rest )
    {
      if (narg._name.IsEqual("label"))
      {
        ASSERT_MSG( narg._value._type == AttrNamedArgVal::STRING, ResId::LABEL_MUST_BE_STRING );
        label = narg._value._sval;
      }
      else if (narg._name.IsEqual("min"))
      {
        ASSERT_MSG( narg._value._type == AttrNamedArgVal::INT, ResId::X_OF_INTSLIDER_MUST_BE_INTEGER, "min" );
        minVal = narg._value._ival;
      }
      else if (narg._name.IsEqual("max"))
      {
        ASSERT_MSG( narg._value._type == AttrNamedArgVal::INT, ResId::X_OF_INTSLIDER_MUST_BE_INTEGER, "max" );
        maxVal = narg._value._ival;

      }
      else if (narg._name.IsEqual("init"))
      {
        ASSERT_MSG( narg._value._type == AttrNamedArgVal::INT, ResId::X_OF_INTSLIDER_MUST_BE_INTEGER, "init" );
        initVal = narg._value._ival;
      }
      else
      {
        throw FmtReportPErrorAt( narg._name, ResId::UNKNOWN_PARAM_ARG_NAME, narg._name.ToString() );
      }
    }

    auto variName = ToUVName( identifier );

    auto vari = _Var( Int(32), variName );

    _binary->_params.emplace_back( variName, label, initVal, minVal, maxVal );
    _scope.DefineG( identifier._str, std::move(vari) );
  }

  void HandleParamDropdownInt32( const Token& identifier, std::vector<AttrNamedArg> rest )
  {
    std::string label = "(empty)";
    std::vector<std::string> items;

    for( auto& narg: rest )
    {
      if (narg._name.IsEqual("label"))
      {
        ASSERT_MSG( narg._value._type == AttrNamedArgVal::STRING, ResId::LABEL_MUST_BE_STRING );
        label = narg._value._sval;
      }
      else if (narg._name.IsEqual("items"))
      {
        ASSERT_MSG( narg._value._type == AttrNamedArgVal::STRING_VEC, ResId::ITEMS_MUST_BE_STRING_VECTOR );
        items = narg._value._svec;
      }
      else
      {
        throw FmtReportPErrorAt( narg._name, ResId::UNKNOWN_PARAM_ARG_NAME, narg._name.ToString() );
      }
    }

    auto variName = ToUVName( identifier );

    auto vari = _Var( Int(32), variName );

    _binary->_params.emplace_back( ParamInfo::DROPDOWN_INT32, variName, label, std::move(items) );
    _scope.DefineG( identifier._str, std::move(vari) );
  }


  void HandleParamCheckboxInt32( const Token& identifier, std::vector<AttrNamedArg> rest )
  {
    std::string label = "(empty)";
    int initVal = 0;

    for( auto& narg: rest )
    {
      if (narg._name.IsEqual("label"))
      {
        ASSERT_MSG( narg._value._type == AttrNamedArgVal::STRING, ResId::LABEL_MUST_BE_STRING );
        label = narg._value._sval;
      }
      else if (narg._name.IsEqual("init"))
      {
        ASSERT_MSG( narg._value._type == AttrNamedArgVal::INT, ResId::INIT_MUST_BE_X, "i32" );
        initVal = narg._value._ival;
      }
      else
      {
        throw FmtReportPErrorAt( narg._name, ResId::UNKNOWN_PARAM_ARG_NAME, narg._name.ToString() );
      }
    }

    auto variName = ToUVName( identifier );

    auto vari = _Var( Int(32), variName );

    _binary->_params.emplace_back( ParamInfo::CHECKBOX_INT32, variName, label, initVal );
    _scope.DefineG( identifier._str, std::move(vari) );
  }

  void HandleParamInt32( const Token& identifier, WidgetType wtype, std::vector<AttrNamedArg> rest )
  {
    if (wtype == WidgetType::SLIDER)
    {
      HandleParamSliderInt32( identifier, std::move(rest) );
    }
    else if (wtype == WidgetType::DROPDOWN)
    {
      HandleParamDropdownInt32( identifier, std::move(rest) );
    }
    else
    {
      assert( wtype == WidgetType::CHECKBOX );
      HandleParamCheckboxInt32( identifier, std::move(rest) );
    }
  }

  void HandleParamSliderFloat32( const Token& identifier, std::vector<AttrNamedArg> rest )
  {
    std::string label = "(empty)";
    float minVal = 0.5f;
    float maxVal = 300.0f;
    float initVal = 3.0f;

    for( auto& narg: rest )
    {
      if (narg._name.IsEqual("label"))
      {
        ASSERT_MSG( narg._value._type == AttrNamedArgVal::STRING, ResId::LABEL_MUST_BE_STRING );
        label = narg._value._sval;
      }
      else if (narg._name.IsEqual("min"))
      {
        ASSERT_MSG( narg._value._type == AttrNamedArgVal::FLOAT, ResId::X_OF_FLOATSLIDER_MUST_BE_FLOAT, "min" );
        minVal = narg._value._fval;
      }
      else if (narg._name.IsEqual("max"))
      {
        ASSERT_MSG( narg._value._type == AttrNamedArgVal::FLOAT, ResId::X_OF_FLOATSLIDER_MUST_BE_FLOAT, "max" );
        maxVal = narg._value._fval;

      }
      else if (narg._name.IsEqual("init"))
      {
        ASSERT_MSG( narg._value._type == AttrNamedArgVal::FLOAT, ResId::X_OF_FLOATSLIDER_MUST_BE_FLOAT, "init" );
        initVal = narg._value._fval;
      }
      else
      {
        throw FmtReportPErrorAt( narg._name, ResId::UNKNOWN_PARAM_ARG_NAME, narg._name.ToString() );
      }
    }

    auto variName = ToUVName( identifier );
    auto vari = _Var( Float(32), variName );
    _binary->_params.emplace_back( variName, label, initVal, minVal, maxVal );
    _scope.DefineG( identifier._str, std::move(vari) );
  }

  void HandleParamDirectionFloat32( const Token& identifier, std::vector<AttrNamedArg> rest )
  {
    float initVal = 0.0f;

    for( auto& narg: rest )
    {
      if (narg._name.IsEqual("init"))
      {
        ASSERT_MSG( narg._value._type == AttrNamedArgVal::FLOAT, ResId::INIT_MUST_BE_X, "f32" );
        initVal = narg._value._fval;
      }
      else
      {
        throw FmtReportPErrorAt( narg._name, ResId::UNKNOWN_PARAM_ARG_NAME, narg._name.ToString() );
      }
    }

    auto variName = ToUVName( identifier );
    auto vari = _Var( Float(32), variName );
    _binary->_params.emplace_back( ParamInfo::DIRECTION_FLOAT32, variName, initVal );
    _scope.DefineG( identifier._str, std::move(vari) );
  }

  void HandleParamFloat32( const Token& identifier, WidgetType wtype, std::vector<AttrNamedArg> rest )
  {
    if (wtype == WidgetType::SLIDER)
    {
      HandleParamSliderFloat32( identifier, std::move(rest) );
    }
    else
    {
      assert( wtype == WidgetType::DIRECTION );
      HandleParamDirectionFloat32( identifier, std::move(rest) );
    }
  }

  void HandleParamF32V2( const Token& identifier, WidgetType wtype, const std::string& title )
  {
    assert( wtype == WidgetType::POINTER );
    auto variName = ToUVName( identifier );
    auto vari = _Var( TupleType( { Float(32), Float(32) } ), variName );
    _binary->_params.emplace_back( ParamInfo::POINTER_FLOAT32_XY, variName, title );
    _scope.DefineG( identifier._str, std::move(vari) );
  }

  void HandleParamF32V4( const Token& identifier, WidgetType wtype, std::vector<AttrNamedArg> rest )
  {
    std::string label = "(empty)";
    // デフォルトは赤
    std::vector<float> init {0.0F, 0.0F, 1.0F, 1.0F};

    // 型はパースの方でチェック済み
    for( auto& narg: rest )
    {
      if (narg._name.IsEqual("label"))
      {
        label = narg._value._sval;
      }
      else if (narg._name.IsEqual("init"))
      {
        init = narg._value._fvec;
      }
    }

    assert( wtype == WidgetType::COLOR_PICKER );
    auto variName = ToUVName( identifier );
    auto vari = _Var( F32V4Type(), variName );
    _binary->_params.emplace_back( ParamInfo::COLOR_PICKER_FLOAT32_V4, variName, label, init );
    _scope.DefineG( identifier._str, std::move(vari) );
  }

  GlobalTensorLike*
  LookupTensor( const Token& tsName )
  {
    return _binary->LookupGlobalTSLike( tsName._str );
  }

  OneInputInfo*
  LookupInput( const Token& tk, int layerIndex )
  {
    return _binary->LookupInput( tk._str, layerIndex );
  }

  GlobalTensorLike*
  LookupTensorByName( const std::string& tsName )
  {
    return _binary->LookupGlobalTSLike( tsName );
  }

  const TensorInfo* LookupLocalTensor( const Token& tsName ) const
  {
    return _scope.LookupLocalTensor( tsName._str );
  }

  // globalもlocalも探す。見つからなければfalse
  bool LookupTensorInfo( const Token& tsName, TensorInfo& out ) const
  {
    auto local = _scope.LookupLocalTensor( tsName._str );
    if (local != nullptr)
    {
      out = *local;
      return true;
    }
    auto global = _binary->LookupGlobalTSLike( tsName._str );
    if (global != nullptr)
    {
      out = global->GetTensorInfo();
      return true;
    }
    return false;
  }

  void HandleSamplerDef( const Token& assigned, ExprT<SamplerElem>&& sampler )
  {
    _scope.DefineSampler( assigned._str, std::move(sampler) );
  }

};

/*
  Parser関連
*/

/*
  スクリプトの先頭からのオフセットを、行数、列数に変えるクラス
*/
struct LocationMap
{
  // 各行の最初のオフセット。
  // 0番目は使われない。1番目は必ず0。
  std::vector<size_t> _lineStart; 
  size_t _size;
  LocationMap( std::vector<size_t>&& lineStart, size_t size ) : _lineStart( std::move(lineStart) ), _size( size ) {}
  LocationMap( LocationMap&& ) = default; // コピーは禁止してmoveだけ許しておく。

  static LocationMap Of( const std::string& ssrc )
  {
    const char* src = ssrc.c_str();
    size_t len = ssrc.size();

    std::vector<size_t> lineStart;
    lineStart.push_back( 0 );
    lineStart.push_back( 0 );
    for (size_t pos = 0; pos < len; pos++)
    {
      if (src[pos] == '\n')
      {
        lineStart.push_back( pos+1 );
      }
    }
    return LocationMap( std::move(lineStart), len );
  }

  size_t GetLineAt( size_t offset )
  {
    auto lower = std::lower_bound( _lineStart.begin(), _lineStart.end(), offset );
    if (lower == _lineStart.end())
    {
      // 最後の行
      return _lineStart.size() - 1;
    }
    auto dist = std::distance( _lineStart.begin(), lower );

    // 0番目は使わない、1番目が1行目
    if (dist <= 1)
      return 1;

    // イコールの時はdist行の開始の位置に一致している事なのでdist
    if (_lineStart[dist] == offset)
      return dist;

    // 次の行の最初がdistなのでその一行前
    return dist - 1;
  }

  Location ToLocation( size_t offset )
  {
    if (offset > _size)
    {
      throw FmtInternalError( ResId::OFFSET_IS_OUTSIDE_OF_SOURCE_SIZE, offset, _size );
    }
    size_t line = GetLineAt( offset );
    size_t col = offset - _lineStart[ line ] + 1;
    return Location( line, col );
  }
};

struct Tokenizer;
struct PosSaver
{
  Tokenizer& _tokenizer;  
  Token _pos;
  inline PosSaver( Tokenizer& tokenizer );
  inline ~PosSaver();
  inline void Detach();
};

struct Tokenizer
{
  std::string _src;
  size_t _pos;

  Token _current;
  LocationMap _locmap;
  std::map<TString, TokenType> _keywords;

  Tokenizer( std::string src ) : _src( std::move(src) ), _pos( 0 ), _current( _src.c_str() ), _locmap( LocationMap::Of( _src ) )
  {
    _keywords["def"] = TokenType::DEF;
    _keywords["let"] = TokenType::LET;
    _keywords["by"] = TokenType::BY;
    _keywords["mut"] = TokenType::MUT;
    _keywords["_"] = TokenType::BLANK_ID;
    _keywords["fn"] = TokenType::FN;
  }

  // 使わない引数のダミーなど用
  Token DummyToken() const { return Token( _src.c_str() ); }

  // 現在の位置からoffsetだけ先の文字を読む。EOFなら-1
  int Peek( int offset )
  {
    if (_pos+offset >= _src.size())
      return -1;
    return _src[_pos+offset];
  }

  bool IsEqual( const char* pat, size_t len )
  {
    if (_pos+len >= _src.size())
      return false;
    return 0 == std::strncmp( &_src[_pos], pat, len );
  }

  void ResetCurrent( const Token& newcur )
  {
    _pos = newcur._offset;
    _current = newcur;
  }

  // 現在の位置のトークンとして値をセット
  void SetToken( TokenType ttype, size_t len )
  {
    _current.Set( ttype, _pos, len );
  }

  void SetTokenType( TokenType ttype )
  {
    _current._ttype = ttype;
  }
  bool CurrentIs( TokenType ttype ) const
  { 
    return _current._ttype == ttype; 
  }


  template<typename C>
  bool IsDigit( C ch )
  {
    return (ch >= '0' && ch <='9');
  }

  template<typename C>
  bool IsXDigit( C ch )
  {
    return (ch >= '0' && ch <='9') || (ch >= 'a' && ch <='f') || (ch >= 'A' && ch <='F');
  }

  // 現在の位置がidentifierの始まりであるのを前提に、identifierとしてscanする。
  void ScanIdentifier()
  {
    size_t epos = _pos+1;
    while (epos < _src.size())
    {
      char c = _src[epos];
      if ((c >= 'a' && c <= 'z') ||
          (c >='A' && c <= 'Z') ||
          IsDigit( c ) ||
          (c == '_'))
      {
        epos++;
        continue;
      }
      break;
    }
    SetToken( TokenType::IDENTIFIER, epos - _pos );
  }

  void SkipComment()
  {
    int ch = Peek( 0 );
    assert( ch == '#' );
    while (ch != '\n' && ch != -1)
    {
      _pos++;
      ch = Peek( 0 );
    }
  }

  // 現在地をスキップの次まで移動する
  void SkipSpace()
  {
    int ch = Peek( 0 );
    while( ch == ' ' || ch == '\t')
    {
      _pos++;
      ch = Peek( 0 );
    }
    if (ch == '#')
    {
      // コメントは空白とくっつけてtokenizerのレベルで読み飛ばす。
      SkipComment();
    }
  }

  // 現在のTokenを消費して次に_posを進める
  void NextNoScan()
  {
    if (_pos >= _src.size())
      return;
    _pos += _current.Length();
    SkipSpace();
    SetToken( TokenType::UNKNOWN, 0 );
  }

  // 現在のTokenを消費して次に_posを進め、その地点のtokenを確定する
  // Next without EOL, EOLは空白では無くトークンとして扱う。
  // escaped eolはスキップする。
  void NextWOEInternal()
  {
    do
    {
      NextNoScan();
      Scan();  
    }while(CurrentIs( TokenType::ESCAPED_EOL ));
  }

  // NextWOEInternalに、先読みしてパイプかどうかの処理もする
  // 次のトークンがパイプ演算子の時だけ改行は空白とみなす。
  // 詳細はMEP 27参照: [MEP 27: パイプライン演算子だけは次の行に書ける例外とするルール](https://github.com/karino2/MFG/blob/main/docs/ja/MEP/27.md)
  void NextWOE()
  {
    NextWOEInternal();

    {
      PosSaver saver( *this );
      SkipEOL();
      if (CurrentIs( TokenType::PIPE_OP ))
        saver.Detach();
    }
  }

  void SkipEOL()
  {
    while(CurrentIs( TokenType::END_OF_LINE ))
    {
      NextWOEInternal();
    }
  }

  Location LocationAt( size_t offset )
  {
    return _locmap.ToLocation( offset );
  }

  Location CurrentLoc() { return LocationAt( _pos ); }

  template<typename ...ARGS>
  ParseError FmtPErrorAt( size_t offset, ResId rid, ARGS... args )
  {
    return FmtParseError( LocationAt( offset ), rid, args... );
  }

  void ScanString()
  {
    int pos = 1; // 次の文字の位置
    while (true)
    {
      int ch = Peek( pos++ );
      if (ch == '\\')
      {
        pos++; // 次の文字はなんでもvalidなのでその次に進む
        continue;
      }
      else if ( ch  == '"')
      {
        SetToken( TokenType::STRING, pos );
        return;
      }
      else if ( ch == '\n')
      {
        throw FmtPErrorAt( _pos+pos -1, ResId::NEWLINE_INSIDE_STRING );
      }
      // それ以外は次の文字へ
    }
  }

  // intかfloatをスキャンする
  // 最初は0-9
  void ScanNumber()
  {
    int first = Peek( 0 );
    int pos = 1; // 次の文字の位置

    if (first == '0')
    {
      // 0か0xのhexか0.1などのfloatの三択
      int ch = Peek( pos++ );
      if (ch == 'x')
      {
        do
        {
          ch = Peek( pos++ );
        } while ( IsXDigit( ch ) );

        if (ch == 'u')
        {
          SetToken( TokenType::UHEX, pos );
          return;
        }
        SetToken( TokenType::HEX, pos -1 );
        return;        
      }
      else if (ch == '.')
      {
        // 0..< など、小数の「.」じゃない「.」のケース
        if (!IsDigit( Peek( pos ) ) )
        {
          SetToken( TokenType::INTEGER, 1 );
          return;
        }

        do
        {
          ch = Peek( pos++ );
        } while ( IsDigit(ch) );
        SetToken( TokenType::FLOAT, pos -1 );
        return;        
      }
      else if (IsDigit( ch ))
      {
        throw FmtPErrorAt( _pos + pos - 1,  ResId::LEADING_ZERO_IN_NUMBER );
      }
      else
      {
        // 0
        if (ch == 'u')
        {
          // 0u
          SetToken( TokenType::UINTEGER, 2 );
          return;
        }
        SetToken( TokenType::INTEGER, 1 );
        return;
      }
    }
    else
    {
      while(true)
      {
        int ch = Peek( pos++ );
        if (ch == '.')
        {
          // 123..< など、小数の「.」じゃない「.」のケース
          if (!IsDigit( Peek( pos ) ) )
          {
            SetToken( TokenType::INTEGER, pos -1 );
            return;
          }

          do
          {
            ch = Peek( pos++ );
          } while ( IsDigit(ch) );
          SetToken( TokenType::FLOAT, pos-1 );
          return;
        }
        else if( !IsDigit(ch) )
        {
          if (ch == 'u')
          {
            SetToken( TokenType::UINTEGER, pos );
            return;
          }
          SetToken( TokenType::INTEGER, pos-1 );
          return;
        }
      }
    }
  }

  // 現在の位置のトークンが何なのかを調べて確定する
  void Scan()
  {

    int ch = Peek( 0 );
    switch(ch)
    {
      case -1:
        SetToken( TokenType::END_OF_SRC, 0 );
        return;
      case '@':
        SetToken( TokenType::ATMARK, 1 );
        return;
      case '|':
        if (Peek(1) == '|')
          SetToken( TokenType::BAR2, 2 );
        else if (Peek(1) == '>')
          SetToken( TokenType::PIPE_OP, 2 );
        else
          SetToken( TokenType::BAR, 1 );
        return;
      case '&':
        if (Peek(1) == '&')
          SetToken( TokenType::AMPERSAND2, 2 );
        else
          SetToken( TokenType::AMPERSAND, 1 );
        return;
      case '%':
        SetToken( TokenType::PERCENT, 1 );
        return;
      case '$':
        SetToken( TokenType::DOLLAR, 1 );
        return;
      case '{':
        SetToken( TokenType::L_BRACE, 1 );
        return;
      case '}':
        SetToken( TokenType::R_BRACE, 1 );
        return;
      case '[':
        SetToken( TokenType::L_SBRACKET, 1 );
        return;
      case ']':
        SetToken( TokenType::R_SBRACKET, 1 );
        return;
      case '(':
        SetToken( TokenType::L_PAREN, 1 );
        return;
      case ')':
        SetToken( TokenType::R_PAREN, 1 );
        return;
      case ',':
        SetToken( TokenType::COMMA, 1 );
        return;
      case ':':
        SetToken( TokenType::COLON, 1 );
        return;
      case '.':
        {
          if (Peek(1) == '.')
          {
            if (Peek(2) == '.')
            {
              SetToken( TokenType::DOT3, 3 );
              return;
            }
            else if(Peek(2) == '<')
            {
              SetToken( TokenType::HALF_OPEN_RANGE, 3 );
              return;
            }
            else
            {
              throw FmtPErrorAt( _pos, ResId::ILLEGAL_DOUBLE_DOT_TOKEN );
            }
          }
          SetToken( TokenType::DOT, 1 );
          return;
        }
      case '^':
        SetToken( TokenType::HAT, 1 );
        return;
      case '!':
        if (Peek(1) == '=')
          SetToken( TokenType::NEQ, 2 );
        else        
          SetToken( TokenType::BANG, 1 );
        return;
      case '=':
        if (Peek(1) == '=')
          SetToken( TokenType::EQEQ, 2 );
        else
          SetToken( TokenType::EQUAL, 1 );
        return;
      case '-':
        SetToken( TokenType::MINUS, 1 );
        return;
      case '+':
        SetToken( TokenType::PLUS, 1 );
        return;
      case '*':
        SetToken( TokenType::ASTER, 1 );
        return;
      case '/':
        SetToken( TokenType::SLASH, 1 );
        return;
      case '\n':
        SetToken( TokenType::END_OF_LINE, 1 );
        return;
      case '\\':
        if (Peek(1) == '\n')
        {
          SetToken( TokenType::ESCAPED_EOL, 2 );
          return;
        }
        throw FmtPErrorAt( _pos, ResId::UNKNOWN_CHARACTER, ch );
      case '>':
        if (Peek(1) == '>')
        {
          SetToken( TokenType::GTGT, 2 );
          return;
        }
        else if(Peek(1) == '=')
        {
          SetToken( TokenType::GTEQ, 2 );
          return;
        }
        SetToken( TokenType::GT, 1 );
        return;
      case '<':
        if (Peek(1) == '=')
          SetToken( TokenType::LTEQ, 2 );
        else if (Peek(1) == '<')
          SetToken( TokenType::LTLT, 2 );
        else
          SetToken( TokenType::LT, 1 );
        return;
      case '"':
        ScanString();
        return;

      default:
        if ((ch >= 'a' && ch <='z') || ( ch >='A' && ch <= 'Z') || (ch == '_'))
        {
          ScanIdentifier();
          auto kiter = _keywords.find( _current._str );
          if (kiter != _keywords.end())
          {
            SetTokenType( kiter->second );
          }
          return;
        }
        if (IsDigit( ch ))
        {
          ScanNumber();
          return;
        }
        throw FmtPErrorAt( _pos, ResId::UNKNOWN_CHARACTER, ch );
    }
  }

};

/*
  Enum関連
  - Symbolizer
  - EnumResolver
  - EnumSystem
*/

struct Symbolizer
{
  std::map<std::string, size_t, TStringComparator> _symMap;
  size_t _lastSymId = 1;

  size_t Intern( const std::string& name )
  {
    auto iter = _symMap.find( name );
    if (iter != _symMap.end())
      return iter->second;
      
    auto id = _lastSymId++;
    _symMap.emplace( name, id );
    return id;
  }

  size_t Intern( const char* str ) { return Intern( std::string(str) ); }

  size_t Intern( const TString& name )
  {
    auto iter = _symMap.find( name );
    if (iter != _symMap.end())
      return iter->second;

    auto id = _lastSymId++;
    _symMap.emplace( name.ToString(), _lastSymId++ );
    return id;
  }

  Type EnumType( const char* str ) { return Type::Enum( Intern( std::string(str) ) ); }

  // TSTRはTStringかstd::string
  template<typename TSTR>
  bool Lookup( const TSTR& name, size_t& outId )
  {
    auto iter = _symMap.find( name );
    if (iter == _symMap.end())
      return false;

    outId = iter->second;
    return true;
  }

  const std::string& ToName( size_t symId )
  {
    for( auto& pair: _symMap )
    {
      if (pair.second == symId)
        return pair.first;
    }
    throw InternalError( "Never reached here." );
  }
};

/*
  各enum型ごとに存在。
  シンボルのIDから自身のIDへのマップを返す。
*/
struct EnumResolver
{
  size_t _enumTypeId;
  // {シンボルID: enumの値} のマップ
  std::map<size_t, size_t> _symMap;

  EnumResolver( Symbolizer& symbolizer, size_t enumTypeId, const std::vector<std::string>& names ) : _enumTypeId( enumTypeId )
  {
    for (auto i : NRange( names.size() ))
    {
      _symMap.emplace( symbolizer.Intern( names[i] ), i );
    }
  }

  bool Lookup( size_t symId, size_t& outVal )
  {
    auto iter = _symMap.find( symId );
    if (iter == _symMap.end())
      return false;
    
    outVal = iter->second;
    return true;
  }
};

struct EnumSystem
{
  Symbolizer _symbolizer;
  std::map<size_t, EnumResolver> _enumResolvers;

  void Register( const std::string& enumTypeName, const std::vector<std::string>& valNames )
  {
    size_t etypeId = _symbolizer.Intern( enumTypeName );
    _enumResolvers.emplace(
      etypeId,
      EnumResolver( _symbolizer, etypeId, valNames )
    );
  }

  // TSTRはTStringかstd::string
  template<typename TSTR>
  bool LookupSymbol( const TSTR& name, size_t& outValue)
  {
    return _symbolizer.Lookup( name, outValue );
  }

  bool Resolve( size_t etypeId, size_t valSymId, size_t& outValue )
  {
    auto iter = _enumResolvers.find( etypeId );
    if (iter == _enumResolvers.end())
      return false;
    
    return iter->second.Lookup( valSymId, outValue );
  }

  // Unit Test用
  bool Resolve( size_t etypeId, const std::string& valName, size_t& outValue )
  {
    size_t valSym;
    if (!LookupSymbol( valName, valSym ))
      return false;
    return Resolve( etypeId, valSym, outValue );
  }
};

// 名前付き引数の型情報
// 型が不定な場合（他の引数に依る場合やgenerics的な関数など）はUnit型を入れておく。
// Enumのresolveでしか使ってないのでEnumの時だけちゃんと入ってれば良い
struct ArgTypeInfo
{
  std::string _name;
  Type _type; 

  ArgTypeInfo( const std::string& name, const Type& tp ) : _name( name ), _type( tp ) {}
};

/*
  名前付き引数などの情報を持つ
*/
struct FuncTypeInfo
{
  std::map<std::string, std::vector<std::string>, TStringComparator> _transMethods;
  std::map<std::string, std::vector<std::string>, TStringComparator> _reduceMethods;
  std::map<std::string, std::vector<ArgTypeInfo>, TStringComparator> _freeStanding;

  FuncTypeInfo( Symbolizer& sym ) : _transMethods {
                    {"accumulate", { "dim" } },
                    {"cumsum", { "dim" } },
                    {"sort", { "dim" } }
                  },
                   _reduceMethods{
                      { "accumulate", { "dim", "init" } },
                      { "find_first_index", { "dim"} }
                    },
                   _freeStanding{
                      { "reduce", { ArgTypeInfo( "init", Type() ) } },
                      { "sampler", { ArgTypeInfo( "coord", sym.EnumType("SamplerCoord") ), ArgTypeInfo( "address", sym.EnumType("SamplerAddressMode") ), ArgTypeInfo( "border_value", Type() ) }}
                  }
  {
  }

  bool IsKnownTransMethod( const Token& methodId )
  {
    return _transMethods.find( methodId._str ) != _transMethods.end();
  }

  const std::vector<std::string>& TransMethodNamedArgs( const Token& methodId )
  {
    return _transMethods.find( methodId._str )->second;
  }

  bool IsKnownReduceMethod( const Token& methodId )
  {
    return _reduceMethods.find( methodId._str ) != _reduceMethods.end();
  }

  const std::vector<std::string>& ReduceMethodNamedArgs( const Token& methodId )
  {
    return _reduceMethods.find( methodId._str )->second;
  }

  bool IsKnownFreeStanding( const Token& funcId )
  {
    return _freeStanding.find( funcId._str ) != _freeStanding.end();
  }

  std::vector<std::string> FreeStandingNamedArgNames( const Token& fundId )
  {
    return MapFn( _freeStanding.find( fundId._str )->second, []( const ArgTypeInfo& ai ) { return ai._name; } );
  }

  const std::vector<ArgTypeInfo>& FreeStandingNamedArgInfos( const Token& fundId )
  {
    return _freeStanding.find( fundId._str )->second;
  }
};

PosSaver::PosSaver( Tokenizer& tokenizer ) : _tokenizer(tokenizer), _pos( tokenizer._current ) {}
PosSaver::~PosSaver()
{
  if (!_pos.IsDummy())
    _tokenizer.ResetCurrent( _pos );
}

/*
  これを呼ぶとデストラクタで巻き戻さなくなる。
*/
void PosSaver::Detach()
{
  _pos = _tokenizer.DummyToken();
}

/*
  ...の処理に関わるコンテキスト。
  普段はカッコの外の次のexprに置き換えるが、パイプライン演算子のrightOpの時にはleftOpに置き換える。
  leftOpは以前にパース済みのもののはずで、それを保持する。
*/
struct EllipsisContext
{
  bool _byPrev = false;
  Expr _candidate;

  bool ByPrev() const { return _byPrev; }

  void EnterByPrev( Expr&& candidate )
  {
    // ネストは禁止、byPrevで無いかは外でチェックするのでここまでは来ないはず。
    assert( !_byPrev );

    _byPrev = true;
    _candidate = std::move(candidate);
  }

  Expr Resolve()
  {
    assert( _byPrev );
    _byPrev = false;
    return std::move(_candidate);
  }
};

using ResStringMap = std::map<mfg_pal::NLanguage, std::map<std::string, std::string>>;


/*
  marパッケージ内のjsonを使ったリソースのlookupに使うクラス。
  ただしやっているのはResStringMapを使って一般的な解決をしているだけでjson特有の事情は特に無し。
*/
struct LocalResMap
{
  const ResStringMap& _rmap;
  LocalResMap( const ResStringMap& rmap ) : _rmap( rmap ) {}

  // langを探して、なければNL_ENGLISHを探す
  bool Lookup( const std::string& key, std::string& out )
  {
    auto lang = mfg_resource::GetCurrentLanguage();

    if(LookupAtLang( lang, key, out ))
      return true;
    if (lang != NL_ENGLISH)
    {
      return LookupAtLang( mfg_pal::NL_ENGLISH, key, out );
    }
    return false;
  }


  bool LookupAtLang( mfg_pal::NLanguage lang, const std::string& key, std::string& out )
  {
    auto iliter = _rmap.find( lang );
    if (iliter != _rmap.end())
    {
      auto &smap = iliter->second;
      auto iter2 = smap.find( key );
      if (iter2 != smap.end())
      {
        out = iter2->second;
        return true;
      }
    }
    return false;
  }
};

struct Parser
{
  TreeBuilder& _handler;
  Tokenizer _tokenizer;
  EllipsisContext _ectx;
  ResStringMap _localStrMap;

  struct Precedance
  {
    int _prec;
    bool _isLeftAssoc;
    Precedance( int prec, bool leftAssoc = true ) : _prec( prec ), _isLeftAssoc( leftAssoc ) {}
    Precedance( Precedance&& ) = default;

    int NextPrec() const { return _isLeftAssoc ? _prec+1 : _prec; }
  };

  // 今の所右結合の演算子は無いので単にprecedanceだけを持つ。
  std::map<BinOpType, Precedance> _binOpPrecedance;
  EnumSystem _enumSystem;
  FuncTypeInfo _funcTypeInfo;

  Parser( TreeBuilder& handler, std::string src ) : _handler( handler ), _tokenizer( std::move( src ) ), _funcTypeInfo( _enumSystem._symbolizer )
  {
    _handler._reportParseErrorAt = [this]( const std::string& msg, const Token& tk ) {
      return PErrorAt( msg, tk._offset );
    };
    _handler._reportParseError = [this]( const std::string& msg ) {
      return PErrorAt( msg, Current()._offset );
    };

    // Pythonを参考にしている。 https://docs.python.org/3/reference/expressions.html#operator-precedence
    _binOpPrecedance.insert( { BinOpType::OR, Precedance( 1 ) } );
    _binOpPrecedance.insert( { BinOpType::AND, Precedance( 2 ) } );
    _binOpPrecedance.insert( { BinOpType::EQEQ, Precedance( 3 ) } );
    _binOpPrecedance.insert( { BinOpType::NEQ, Precedance( 3 ) } );
    _binOpPrecedance.insert( { BinOpType::LE, Precedance( 3 ) } );
    _binOpPrecedance.insert( { BinOpType::LT, Precedance( 3 ) } );
    _binOpPrecedance.insert( { BinOpType::GT, Precedance( 3 ) } );
    _binOpPrecedance.insert( { BinOpType::GE, Precedance( 3 ) } );
    _binOpPrecedance.insert( { BinOpType::PIPE_OP, Precedance( 3 ) } );
    _binOpPrecedance.insert( { BinOpType::BITWISE_OR, Precedance( 4 ) } );
    _binOpPrecedance.insert( { BinOpType::BITWISE_AND, Precedance( 5 ) } );
    _binOpPrecedance.insert( { BinOpType::RSHIFT, Precedance( 6 ) } );
    _binOpPrecedance.insert( { BinOpType::LSHIFT, Precedance( 6 ) } );
    _binOpPrecedance.insert( { BinOpType::ADD, Precedance( 7 ) } );
    _binOpPrecedance.insert( { BinOpType::SUB, Precedance( 7 ) } );
    _binOpPrecedance.insert( { BinOpType::MUL, Precedance( 8 ) } );
    _binOpPrecedance.insert( { BinOpType::DIV, Precedance( 8 ) } );
    _binOpPrecedance.insert( { BinOpType::MOD, Precedance( 8 ) } );
    _binOpPrecedance.insert( { BinOpType::POW, Precedance( 9, false ) } );

    _enumSystem.Register( "SamplerCoord", { "Pixel", "NormalizedNearest", "NormalizedLinear" } );
    _enumSystem.Register( "SamplerAddressMode", { "None", "ClampToEdge", "ClampToBorderValue" } );
  }

  const Token& Current() const { return _tokenizer._current; }

  /*
    WOEはWithout EOLの略で、EOLは飛ばさない事を意味する。
  */
  void NextWOE() { _tokenizer.NextWOE(); }
  bool CurrentIs( TokenType ttype ) const { return _tokenizer.CurrentIs( ttype ); }
  void SkipEOL() { _tokenizer.SkipEOL(); }


  // EOLの時はその次がttypeかどうか。
  // それ以外はCurrentIsと同様。
  // CurrentIs with EOLの略
  bool CurrentIsWEOL( TokenType ttype )
  { 
    PosSaver saver( _tokenizer );

    if (CurrentIs( TokenType::END_OF_LINE))
      SkipEOL();

    return CurrentIs( ttype );
  }


  bool CurrentIsIdentifier( const char* name ) const
  {
    if (!CurrentIs(TokenType::IDENTIFIER))
      return false;
    return Current().IsEqual( name );
  }
  Token DummyToken() const { return _tokenizer.DummyToken(); }

  Location CurrentLoc() { return _tokenizer.CurrentLoc(); }


  template<typename ...ARGS>
  ParseError FmtPError( ResId rid, ARGS... args ) { return FmtParseError( CurrentLoc(), rid, args... ); }

  template<typename ...ARGS>
  ParseError FmtPErrorAt( size_t offset, ResId rid, ARGS... args ) { return FmtParseError( _tokenizer.LocationAt( offset ), rid, args... ); }

  template<typename ...ARGS>
  ParseError FmtPErrorAt( const Token& tk, ResId rid, ARGS... args ) { return FmtPErrorAt( tk._offset, rid, args... ); }

  ParseError PErrorAt( const std::string& msg, size_t offset ) { return ParseError( msg, _tokenizer.LocationAt( offset ) ); }

  // NextWOE()してEOLだったらそれをスキップ
  void Next()
  {
    NextWOE();
    SkipEOL();
  }

  void MustBeValidVarName()
  {
    MustBeIdLike();
    ASSERT_VAR_NAME( Current() );
  }

  /*
    one_var = identifier | blank_id

    varlist = one_var
            | one_var ',' varlist
  */
  std::vector<Token> ParseVarList()
  {
    MustBeValidVarName();

    std::vector<Token> ret;
    ret.push_back( Current() );
    Next();
    while( CurrentIs( TokenType::COMMA ) )
    {
      Next();
      MustBeValidVarName();
      ret.push_back( Current() );
      Next();
    }
    return ret;
  }

  void MustBe( TokenType ttype )
  {
    if (!CurrentIs( ttype ))
    {
      throw FmtPError( ResId::TOKEN_TYPE_DIFFER, TokenTypeToStr( ttype ), TokenTypeToStr( Current()._ttype ) );
    }
  }

  void MustBeEither( TokenType ttype1, TokenType ttype2 )
  {
    if (!(CurrentIs( ttype1 ) || CurrentIs( ttype2 )))
    {
      // token typeの文字列への変換はもう少し仕様が固まってから行う。
      throw FmtPError( ResId::EITHER_TOKEN_TYPE_DIFFER, TokenTypeToStr( ttype1 ), TokenTypeToStr( ttype2 ), TokenTypeToStr( Current()._ttype ) );
    }
  }

  void MustBeIdLike()
  {
    MustBeEither( TokenType::IDENTIFIER, TokenType::BLANK_ID );
  }

  void MustBeIdentifier( const char* name )
  {
    MustBe( TokenType::IDENTIFIER );
    if (!CurrentIsIdentifier( name ))
    {    
      throw FmtPError( ResId::ID_DIFFER, name, Current().ToString() );
    }
  }

  template<typename ...ARGS>
  void ASSERT_MSG( bool cond, ResId rid, ARGS... args )
  {
    if (!cond)
      throw FmtPError( rid, args... );
  }

  // 変数名として使っていいかのチェック。
  // とりあえずprimitiveの関数名とかぶっている時は弾く（rsumを間違って使うケースがちょくちょくあるのでとりあえずそれは弾きたい）
  void ASSERT_VAR_NAME( const Token& idtk )
  {
    ASSERT_MSG( !_handler.IsPrimitive( idtk ), ResId::USE_PRIMITIVE_AS_VAR, idtk.ToString() );
  }


  // 
  // let_stmt = 'let' identifier = expr
  //          | 'let' '[' varlist ']' = expr
  //
  // LetかTupleLetか空のExprVOIDを返す。sampler定義の時は空。
  ExprVOID ParseLetStmt()
  {
    MustBe( TokenType::LET );
    Next();

    if (CurrentIs( TokenType::IDENTIFIER))
    {
      auto ident = Current();
      ASSERT_VAR_NAME( ident );

      Next();
      MustBe( TokenType::EQUAL );
      Next();
      auto expr = ParseExpr();
      if (expr.GetElemType() == IRElemType::SamplerElem)
      {
        _handler.HandleSamplerDef( ident, std::move(expr).AsExprT<SamplerElem>() );
        return ExprVOID();
      }
      else
      {
        return _handler.HandleLet( ident, std::move( expr ) ).AsVOID();
      }
    }
    else
    {
      // let [a, b, c] = expr 的なlet
      MustBe( TokenType::L_SBRACKET); // これだとエラーメッセージはよくなさそう。エラーメッセージの仕組みを作ったら改善。
      Next();
      auto lefts = ParseVarList();
      MustBe( TokenType::R_SBRACKET);
      Next();
      MustBe( TokenType::EQUAL );
      Next();
      auto right = ParseExpr();
      return _handler.HandleTupleLet( std::move( lefts ), std::move( right ) ).AsVOID();
    }
  }

  struct NamedArg
  {
    Token _name;
    Expr _arg;

    NamedArg( const Token& name, Expr&& arg ) : _name( name ), _arg( std::move(arg) ) {}
    NamedArg( NamedArg&& ) = default;
  };

  bool CurrentIsNamedArg()
  {
    if (!CurrentIs( TokenType::IDENTIFIER) )
      return false;
    {
      PosSaver saver( _tokenizer );

      Next();
      return CurrentIs( TokenType::EQUAL );
    }    
  }

  /*
    named_arg = ident '=' expr
  */
  NamedArg ParseNamedArg()
  {
    MustBe( TokenType::IDENTIFIER );
    auto name = Current();

    Next();
    MustBe( TokenType::EQUAL );

    Next();
    auto expr = ParseExpr();
    return NamedArg( name, std::move(expr) );
  }

  struct ArgList
  {
    bool _hasEllipsis;
    std::vector<Expr> _positionalArgs;
    std::vector<NamedArg> _namedArgs;

    ArgList( bool hasEllipsis, std::vector<Expr>&& positional, std::vector<NamedArg>&& named ) : _hasEllipsis( hasEllipsis ), _positionalArgs( std::move(positional) ), _namedArgs( std::move(named) ) {}
    ArgList( ArgList&& ) = default;

    bool HasNamedArgs() const { return !_namedArgs.empty(); }
  };

  /*
    one_arg = range
            | spreadable_elem
            | named_arg

    range = expr '..<' expr

    結果はout引数に入れる
  */
  void ParseOneArg( std::vector<Expr>& outPosList, std::vector<NamedArg>& outNamedList )
  {
    if (CurrentIsNamedArg())
    {
      auto namedArg = ParseNamedArg();
      outNamedList.push_back( std::move(namedArg) );
    }
    else if (CurrentIs( TokenType::ASTER ))
    {
      outPosList.push_back( ParseSpreadableElem() );
    }
    else
    {
      auto expr = ParseExpr();
      if (CurrentIs( TokenType::HALF_OPEN_RANGE) )
      {
        Next();
        auto expr2 = ParseExpr();
        outPosList.push_back( Expr( new RangeElem( std::move(expr), std::move(expr2) ) ) );
      }
      else
      {
        outPosList.push_back( std::move(expr) );
      }
    }
  }

  /*
    外の関数オブジェクトの処理も...の処理も行わない、カッコ内だけの引数のパース
    カッコは含まない。

    raw_arg_list = one_arg (','  one_arg)*
             | one_arg (','  one_arg)* ',' '...' 
             | '...' 
  */
  ArgList ParseRawArgList()
  {
    if (CurrentIs( TokenType::DOT3 ))
    {
      Next();
      return ArgList( true, std::vector<Expr>(), std::vector<NamedArg>() );
    }

    std::vector<Expr> posList;
    std::vector<NamedArg> namedList;

    // 最初の引数
    ParseOneArg( posList, namedList );

    while( CurrentIs( TokenType::COMMA ) )
    {
      Next();
      if (CurrentIs( TokenType::DOT3 ))
      {
        Next();
        return ArgList( true, std::move( posList ), std::move( namedList ) );
      }
      else
      {
        ParseOneArg( posList, namedList );
      }
    }

    return ArgList( false, std::move( posList ), std::move( namedList ) );
  }

  /*
    normal_paren_arg_list = '(' ')'
                          | '(' expr_list ')'

    名前付き引数、...、カッコ外の関数オブジェクトなどの無い
    カッコでくくられたexpr_listのパース
  */
  std::vector<Expr> ParseNormalParenArgList()
  {
    MustBe( TokenType::L_PAREN );
    Next();
    if (CurrentIs( TokenType::R_PAREN ))
    {
      Next();
      return {};
    }
    auto argList = ParseRawArgList();
    MustBe( TokenType::R_PAREN );
    Next();

    ASSERT_MSG( !argList._hasEllipsis, ResId::DOT3_NOT_ALLOWED_IN_NORMAL_PAREN_ARG_LIST );
    ASSERT_MSG( !argList.HasNamedArgs(), ResId::NAMED_ARG_NOT_ALLOWED_IN_NORMAL_PAREN_ARG_LIST );
    return std::move(argList._positionalArgs);
  }


  /*
    func_paren_arg_list = '(' ')'
                        | '(' raw_arg_list ')' 
    
    カッコでくくられた関数コールの引数のパース。
    最後の引数の関数オブジェクトを外に出す特殊扱いをしないarg list。
    最後の引数が...の時は次のExprをパースして追加する処理はここで行う。

    WOEは最後のEOLを飛ばさずに残す事を意味する。
    WOEについてはParseExprWOEのコメントや(#975)参照。
  */
  ArgList ParseFuncParenArgListWOE()
  {
    MustBe( TokenType::L_PAREN );
    Next();
    if (CurrentIs( TokenType::R_PAREN ))
    {
      NextWOE();
      return ArgList( false, {}, {} );
    }
    auto argList = ParseRawArgList();
    MustBe( TokenType::R_PAREN );
    NextWOE();

    // 最後が '...'のケース。
    if (argList._hasEllipsis)
    {
      if (_ectx.ByPrev())
      {
        // パイプ演算子のrightOp。 ectxのExprに置き換え
        argList._positionalArgs.emplace_back( _ectx.Resolve() );
        return argList;
      }
      else
      {
        // 次のexprを最後の引数にする。
        SkipEOL();
        auto last = ParseExprWOE();
        argList._positionalArgs.emplace_back( std::move(last) );
        return argList;
      }
    }
    else
    {
      return argList;
    }
  }

ArgList ParseFuncParenArgList()
{
  auto ret = ParseFuncParenArgListWOE();
  SkipEOL();
  return ret;
}


  // 名前付き引数とpositonal argを両方まとめて全てpositonal argにする。
  // 名前付き引数の情報に入ってないargは触らず、Positonal Argの現在の進めた位置と解決した結果のペアを返す。
  std::pair<size_t, std::vector<Expr>> ResolveNamedArgsCommon( ArgList&& argList,  const std::vector<std::string>& names  )
  {
    std::map<std::string, size_t> nameMap;
    for( auto i : NRange( argList._namedArgs.size() ) )
    {
      nameMap.emplace( argList._namedArgs[i]._name.ToString(), i );
    }

    std::vector<Expr> ret;
    size_t curPos = 0;
    for( auto i : NRange( names.size() ))
    {
      auto iter = nameMap.find( names[i] );
      if (iter == nameMap.end())
      {
        ret.push_back( std::move( argList._positionalArgs[curPos++] ) );
      }
      else
      {
        ret.push_back( std::move( argList._namedArgs[ iter->second ]._arg ) );
      }
    }
    return std::make_pair( curPos, ret );
  }

  // 名前付き引数とpositonal argを両方まとめて全てpositonal argにする。
  // 最後の引数になるBlockElemは名前に入ってない
  std::vector<Expr> ResolveNamedArgs( ArgList&& argList,  const std::vector<std::string>& names )
  {
    auto respair = ResolveNamedArgsCommon( std::move(argList), names );
    size_t curPos = respair.first;

    // 最後のBlockElemは名前無し。この時点である場合（カッコ内になる時）と無い時（カッコ外で定義してある場合）がある。
    // 以下はある場合の処理
    if (curPos < argList._positionalArgs.size())
    {
      respair.second.push_back( std::move( argList._positionalArgs[curPos++] ) );
    }
    ASSERT_MSG( curPos == argList._positionalArgs.size(), ResId::TOO_MANY_POSITIONAL_ARGS );
    return std::move(respair.second);
  }

  // positional argsが可変長のケース。
  std::vector<Expr> ResolveVariableLengthNamedArgs( ArgList&& argList,  const std::vector<std::string>& names )
  {
    auto respair = ResolveNamedArgsCommon( std::move(argList), names );
    size_t curPos = respair.first;

    while (curPos < argList._positionalArgs.size())
    {
      respair.second.push_back( std::move( argList._positionalArgs[curPos++] ) );
    }
    return std::move(respair.second);
  }


  std::vector<Expr> ResolveNamedArgsByFuncType( const Token& convType, const Token& /* target */, const Token& methodId, ArgList&& argList )
  {
    auto methodIdStr = methodId.ToString();
    if (convType.IsEqual("reduce") && _funcTypeInfo.IsKnownReduceMethod( methodId )) 
    {
      return ResolveNamedArgs( std::move(argList), _funcTypeInfo.ReduceMethodNamedArgs( methodId ) );
    }
    else if(convType.IsEqual("trans") && _funcTypeInfo.IsKnownTransMethod( methodId ))
    {
      return ResolveNamedArgs( std::move(argList), _funcTypeInfo.TransMethodNamedArgs( methodId ) );
    }
    else if (convType.IsDummy() && methodId.IsEqual("reduce")) 
    {
      return ResolveVariableLengthNamedArgs( std::move(argList), _funcTypeInfo.FreeStandingNamedArgNames( methodId ) );
    }
    else
    {
      ASSERT_MSG( !argList.HasNamedArgs(), ResId::ILLEGAL_NAMED_ARG_CALL, methodId.ToString() );
      return std::move( argList._positionalArgs );
    }
  }

  std::string InnerName( const Token& vident ) const 
  {
    if (vident._ttype == TokenType::BLANK_ID)
      return "_";
    return _handler.InnerName( vident._str );
  }

  /*
    block = block_args '{' body '}'
     
    Blockのパース。
    仮引数の型を知る必要があるが、これは引数に使うであろう関数の型から推論するものなので、
    関数の引数である事を前提に関数の型から推論する事になる。
    まだそうした情報をパーサー側で持つのは時期尚早と見て、とりあえずどの関数の引数なのかをハンドラー側にわたす事にする。
    結果としてBlockを引数以外で使うケースに対応できないが、
    その場合はどちらにせよ仮引数の型指定が必須になりそれは未対応なので現時点で対応出来なくて当然。
    また引数の何番目かを渡していないので最後の引数以外のケースも対応出来ない。
    これも今の所そんな関数は無いので問題無い。

    という事で、このParseArgOfBlockの引数のparentTargetとparentMethodIdは、
    対象となるBlockを引数にする、親のメソッドのselfとmethodId。

    plainArgsは最後のBlockより前の引数のパース結果。
    reduceなどでinitをreturn typeに使いたいので型だけ必要。
  */
  ExprT<BlockElem>
  ParseBlockAsLastArgWOE( const Token& convType, const Token& parentTarget, const Token& parentMethodId, const std::vector<Expr>& plainArgs )
  {
    MustBe( TokenType::BAR );
    auto argNames = ParseBlockArgs();
    auto typedArgs = _handler.ResolveBlockArgTypes( convType, parentTarget, parentMethodId, plainArgs, argNames );

    _handler.EnterBlockScope( typedArgs );

    MustBe( TokenType::L_BRACE );
    auto body = ParseBody();
    MustBe( TokenType::R_BRACE );
    NextWOE();

    auto strArgs = MapFn( typedArgs, [this](const std::pair<Token, Type>& arg ) { return std::make_pair( InnerName( arg.first ), arg.second ); } );
    _handler.LeaveBlockScope();

    // 本来はhandler側で作るものだが別段向こうの情報を必要としないのでサボってここで作ってしまう。
    return ExprT<BlockElem>( new BlockElem( std::move(strArgs), std::move(body) ) );
  }


  /*
    args = func_paren_arg_list
          | func_paren_arg_list block
          | block

    func_objのパース時に仮引数の型情報が必要になる。
    これは本来はmethod定義から引っ張ってくる必要のあるものなので、
    とりあえずパーサーとしては何のメソッドの引数なのかの情報だけ渡す。
    これでは最後の引数以外では不十分になりそうだが、とりあえずはいいでしょう。

    ...とfunc_objの両方があるとかなり変な挙動になるが、
    普通は使わないと思うのでいいでしょう。仕様としては...の解決を先に行ったあとに
    Blockのパースを行うという順序にしておく。

    trans<hist>.accumulate!(dim=0)の時、
    - convTypeはtrans
    - targetはhist
    - methodIdはaccumulate

    ts.extent(0) の時は、
    - convTypeはDummyToken
    - targetはts
    - methodIdはextent

    など。
  */
  std::vector<Expr> ParseArgsWOE( const Token& convType, const Token& target, const Token& methodId )
  {
    if (CurrentIs(TokenType::L_PAREN))
    {
      auto plain = ParseFuncParenArgListWOE();
      auto expanded = ResolveNamedArgsByFuncType( convType, target, methodId, std::move(plain) );
      if (CurrentIsWEOL( TokenType::BAR ))
      {
        SkipEOL();
        auto lastBlock = ParseBlockAsLastArgWOE( convType, target, methodId, expanded );
        expanded.push_back( std::move(lastBlock) );
        return expanded;
      }
      else
      {
        return expanded;
      }
    }
    else
    {
      MustBe( TokenType::BAR );
      auto lastBlock = ParseBlockAsLastArgWOE( convType, target, methodId, {} );
      return ToExprVec( std::move( lastBlock ) );
    }
  }

  /*
    resMapにpositional argのうち、空きを埋める。空きはnamesのstartからはじめて探す。
    埋めたらそこの次のインデックスを返す（次回のstart）
    TODO: enumのresolve
  */
  size_t FillOneNamedArgsByPositionalArg( std::map<std::string, Expr>& resMap, size_t start, const std::vector<ArgTypeInfo>&argInfos, Expr&& positionalArg )
  {
    for( size_t pos = start; pos < argInfos.size(); pos++ )
    {
      const ArgTypeInfo& ainfo = argInfos[pos];
      if( resMap.find( ainfo._name ) != resMap.end() )
      {
        Expr expr = MayResolveEnum( ainfo, std::move(positionalArg) );
        resMap.emplace( ainfo._name, std::move(expr) );
        return pos+1;
      }
    }
    throw FmtPError( ResId::TOO_MANY_POSITIONAL_ARGS );
  }

  Expr MayResolveEnum( const ArgTypeInfo& info, Expr&& expr )
  {
    if (!info._type.IsEnum())
    {
      ASSERT_MSG( expr.GetElemType() != IRElemType::EnumElem, ResId::UNEXPECTED_ENUM, info._name );
      return std::move(expr);
    }
    ASSERT_MSG( expr.GetElemType() == IRElemType::EnumElem, ResId::UNEXPECTED_NON_ENUM, info._name );

    size_t enumVal;
    if (!_enumSystem.Resolve( info._type.EnumTypeId(), expr.As<EnumElem>()->_symId, enumVal ))
    {
        throw FmtPError( ResId::UNKNOWN_ENUM_NAME, info._name );
    }
    return Expr( (int32_t)enumVal );
  }

  /*
    現状、名前付き引数があったり無かったりする組み合わせが許されるのはsamplerだけなので、それ専用のパースを行う。
    結果はすべてnamed argsとして、存在しないエントリはmapに含まれない。

    今の所samplerは...やブロックがないのでその辺は手抜きしている。

    原理的にはParseArgsもこうすべきと思うが、今の時点ではやめておく。

    現状、methodIdはいつもsampler。
    sampler<...>(XXX) のXXXをパースする。
  */
  std::map<std::string, Expr> ParseVarLenNamedArgsWOE( const Token& methodId )
  {
    MustBe(TokenType::L_PAREN);

    auto argList = ParseFuncParenArgListWOE();

    // 以下はResolveNamedArgsCommonのmap版。他でも使う事になるかもしれないが、今はこの関数の中で処理してしまう。
    // ルールとしては
    // 1. named argsを埋める
    // 2. あいているnamed argsを順番にpositional argsで埋める
    const std::vector<ArgTypeInfo>& argInfos = _funcTypeInfo.FreeStandingNamedArgInfos( methodId );
    std::map<std::string, ArgTypeInfo, TStringComparator> argMap;
    for( const ArgTypeInfo& info : argInfos )
    {
      argMap.emplace( info._name, info );
    }

    std::map<std::string, Expr> resMap;
    for( auto i : NRange( argList._namedArgs.size() ) )
    {
      auto& oneArg = argList._namedArgs[i];
      auto iter = argMap.find( oneArg._name._str );
      ASSERT_MSG( iter != argMap.end(), ResId::UNKNOWN_NAMED_ARG, oneArg._name.ToString() );
      auto expr = MayResolveEnum( iter->second, std::move(oneArg._arg) );
      resMap.emplace( oneArg._name.ToString(), std::move(expr) );
    }

    size_t filled = 0;
    for( auto i : NRange( argList._positionalArgs.size() ) )
    {
      filled = FillOneNamedArgsByPositionalArg( resMap, filled, argInfos, std::move(argList._positionalArgs[i]) );
    }

    return resMap;
  }

  /*
    atom = primary
        | '-' primary
        | '!' primary
  */
  Expr ParseAtomWOE()
  {
    auto ctp = Current()._ttype;
    if (ctp == TokenType::MINUS)
    {
      Next();
      auto prim = ParsePrimaryWOE();
      return _handler.HandleUnaryMinus( std::move(prim) );
    }
    if (ctp == TokenType::BANG)
    {
      Next();
      auto prim = ParsePrimaryWOE();
      return _handler.HandleUnaryLogicalNot( std::move(prim) );
    }
    return ParsePrimaryWOE();
  }

  /*
    spreadable_elem = expr
                    | '*' expr
  */
  Expr ParseSpreadableElem()
  {
    // spread
    if (CurrentIs( TokenType::ASTER ))
    {
      Next();
      auto expr = ParseExpr();
      ASSERT_MSG( expr.GetType().IsTuple(), ResId::NON_TUPLE_SPLAT );
      return Expr( new SpreadElem( std::move(expr) ) );
    }
    else
    {
      auto ret = ParseExpr();
      return ret;
    }
  }

  void ASSERT_ALL_NUMERIC( const std::vector<Expr>& items )
  {
    auto types = ToSpreadedTypes( items );
    for( auto i : NRange(types.size()) )
    {
      ASSERT_MSG( types[i].IsNumeric(), ResId::NON_NUMERIC_TUPLE_ELEM, i );
    }
  }

  /*
    tuple_expr = '[' tuple_list ']'

    tuple_list = tuple_elem (',' tuple_elem)*

    tuple_elem = spreadable_elem
  */
  Expr ParseTupleExprWOE()
  {
    IRBuildDSL d;
    std::vector<Expr> tuplist;
    MustBe( TokenType::L_SBRACKET );
    Next();

    tuplist.push_back( ParseSpreadableElem() );
    while (CurrentIs( TokenType::COMMA ))
    {
      Next();
      tuplist.push_back( ParseSpreadableElem() );
    }
    MustBe( TokenType::R_SBRACKET );
    NextWOE();
    ASSERT_ALL_NUMERIC( tuplist );
    return d._Tuple( std::move(tuplist) );
  }

  // '<' ident '>'  だったらTrue.
  bool CurrentIsBraketTS()
  {
    if (!CurrentIs( TokenType::LT ))
      return false;
    PosSaver saver( _tokenizer );
    Next();
    if (!CurrentIs( TokenType::IDENTIFIER ))
      return false;
    Next();
    return CurrentIs( TokenType::GT );
  }

  std::map<char, size_t> SwizzleXYZWMap()
  {
    return std::map<char, size_t> { { 'x', 0 }, { 'y', 1 }, { 'z', 2 }, { 'w', 3 } };
  }

  /*
    xwyx などを、 {0, 3, 1, 0}にする。
    swizzleでなければパースエラー

    swizzle_char = 'x' | 'y' | 'z' | 'w'

    swizzle = swizzle_char+
  */
  std::vector<size_t> ToSwizzleIndices( const Token& identifier )
  {
    std::vector<size_t> indices;

    auto imap = SwizzleXYZWMap();

    for( auto i : NRange(identifier.Length()) )
    {
      auto iter = imap.find(identifier._str[i]);
      if(iter == imap.end())
      {
        throw FmtPErrorAt( identifier, ResId::UNKNOWN_CHAR_INSIDE_SWIZZLE, identifier._str[i] );
      }
      indices.push_back( iter->second );
    }
    return indices;
  }

  ExprT<ImmElem> HandleImm( const Token& cur )
  {
    switch( cur._ttype )
    {
      case TokenType::INTEGER:
      {
        return _handler.HandleIntImm( cur.ToInt() );
      }
      case TokenType::UINTEGER:
      {
        return _handler.HandleUIntImm( cur.ToUInt() );
      }
      case TokenType::HEX:
      {
        return _handler.HandleIntImm( cur.ToHex() );
      }
      case TokenType::UHEX:
      {
        return _handler.HandleUIntImm( cur.ToUHex() );
      }
      case TokenType::FLOAT:
      {
        return _handler.HandleFloatImm( cur.ToFloat() );
      }
      default:
        throw FmtPError( ResId::EXPECTED_IMM, cur._ttype );
    }
  }

  /*
    imm = INTEGER
          | UINTEGER
          | FLOAT
          | HEX
  */
  ExprT<ImmElem> ParseImmWOE()
  {
    auto cur = Current();
    auto ret = HandleImm( cur );
    NextWOE();
    return ret;
  }

  ExprT<ImmElem> ParseImm()
  {
    auto ret = ParseImmWOE();
    SkipEOL();
    return ret;
  }

  /*
    targetable_expr = variable
          | imm
          | tuple_expr
          | '(' expr ')'

  */
  Expr ParseTargetableExprWOE()
  {
    auto cur = Current();
    switch( cur._ttype )
    {
      case TokenType::INTEGER:
      case TokenType::UINTEGER:
      case TokenType::HEX:
      case TokenType::UHEX:
      case TokenType::FLOAT:
      {
        return ParseImmWOE();
      }
      case TokenType::L_SBRACKET:
      {
        return ParseTupleExprWOE();
      }
      case TokenType::L_PAREN:
      {
        Next();
        auto ret = ParseExpr();
        MustBe( TokenType::R_PAREN );
        NextWOE();
        return ret;
      }
      case TokenType::IDENTIFIER:
      {
        auto ident = Current();
        NextWOE();

        // Variable
        return _handler.HandleVariable( ident );        
      }
      default:
        throw FmtPError( ResId::UNEXPECTED_TOKEN_IN, "ParseTargetableExprWOE", TokenTypeToStr( cur._ttype ) );
    }
  }

  /*
    input_u8, input_u16などにマッチ
  */
  IOTensorType MatchInputId( const Token& ident )
  {
    IOTypeMatcher inputMatcher( "input" );
    return inputMatcher.MatchType( ident.ToString() );
  }

  /*
    layer_subscript = '[' INT_IMM ']'

    input_u8[-1]などのsubscript。
    とりあえず即値じゃないといけないとする。
  */
  int ParseLayerSubscript()
  {
    MustBe( TokenType::L_SBRACKET );
    Next();

    // インデックスは数値の即値じゃないといけないとする。
    auto indexExpr = ParseAtomWOE();
    ASSERT_MSG( indexExpr.IsConst() && indexExpr.GetType().IsUSInt(), ResId::INPUT_SUBSCRIPT_MUST_BE_IMM ); 
    int index = indexExpr.AsInt();

    MustBe( TokenType::R_SBRACKET);
    Next();
    return index;
  }

  /*
    sampler_target = input_id layer_subscript
                    | input_id
                    | ident
    
    identはinput以外のテンソル関連のidentifier。
    layer_subscriptかどうかは'['で判定
  */
  GlobalTensorLike& ParseSamplerTarget()
  {
    MustBe( TokenType::IDENTIFIER );

    auto ident = Current();
    Next();

    auto itype = MatchInputId( ident );
    if (itype == IOTensorType::UNSPECIFIED)
    {
      return _handler.HandleTensorReference( ident, itype, 0 );
    }

    if (CurrentIs( TokenType::L_SBRACKET))
    {
      // input_u8[-1] など。
      int index = ParseLayerSubscript();
      return _handler.HandleTensorReference( ident, itype, index );
    }
    else
    {
      return _handler.HandleTensorReference( ident, itype, 0 );
    }
  }

  /*
    tensor_or_func_call = 
          'sampler' '<' sampler_target '>' '(' arg_list ')'
          | ident '(' arg_list ')'
          | ident tuple_expr
          | ident '.' ident '(' arg_list ')'
          | ident '<' ident '>' '.' ident '(' arg_list ')'
          | input_id layer_subscript '(' arg_list ')'

    関数やテンソルは今のところExprでは無いので、identから直接パースしている。
    それ関連の文法ルール。
    identがスコープにある変数でない事は呼び出し側で確認済み

    ident tuple_exprは u8[1, 2, 3]などで u8([1, 2, 3])のシンタックスシュガー

    これは最後のEOLを区切りとして扱う（ParsePrimaryWOEのコメント参照）
  */
  Expr TensorOrFunctionCallWOE()
  {
    MustBe( TokenType::IDENTIFIER );

    auto ident = Current();
    Next();

    if (ident.IsEqual("sampler"))
    {
      // 'sampler' '<' sampler_target '>' '(' arg_list ')'
      ASSERT_MSG( CurrentIs( TokenType::LT ), ResId::INVALID_SAMPLER_SYNTAX );
      Next();

      auto& target = ParseSamplerTarget();

      MustBe( TokenType::GT );
      Next();

      auto argMap = ParseVarLenNamedArgsWOE( ident );
      return _handler.HandleCreateSamplerElem( ident, target, std::move(argMap) );
    }
    else if (CurrentIs( TokenType::DOT ))
    {
      Next();
      MustBe( TokenType::IDENTIFIER );
      auto methodId = Current();
      Next();

      if (CurrentIs(TokenType::L_PAREN) || CurrentIs(TokenType::BAR))
      {
        // ident '.' ident '(' exprlist ')'
        // ts.sum |i, val| {...} のケースも
        auto args = ParseArgsWOE( DummyToken(), ident, methodId );
        return _handler.HandleMethodCall( ident, methodId, std::move(args) );
      }
      else
      {
        throw FmtPError( ResId::UNEXPECTED_TOKEN_IN, "TensorOrFunctionCallWOE", TokenTypeToStr(Current()._ttype) );
      }

    }
    else if (CurrentIsBraketTS()) // a < b と sampler<ts>() を区別する為に先読みが必要
    {
      /*
      | ident '<' ident '>' '(' arg_list ')'
      | ident '<' ident '>' '.' ident '(' arg_list ')'
      */
      Next();
      MustBe( TokenType::IDENTIFIER );
      auto tsId = Current();
      Next();
      MustBe( TokenType::GT );
      Next();

      if (CurrentIs(TokenType::DOT))
      {
        // ReduceToScalar
        // 本質的にはParseConvertTensorなんだが、途中までTokenを進めている都合で、
        // ここで手作業でパースしてしまう方が手早いので手作業でパースする。
        Next();
        MustBe( TokenType::IDENTIFIER );
        auto methodName = Current();
        ASSERT_MSG( _funcTypeInfo.IsKnownReduceMethod( methodName ), ResId::UNKNOWN_REDUCE, methodName.ToString() );

        Next();
        MustBe( TokenType::L_PAREN );

        auto args = ParseArgsWOE( ident, tsId, methodName );
        return _handler.HandleConvertToScalar( ConvertTensorTree( ident, tsId, methodName, std::move(args) ) );
      }
      else
      {
        throw FmtPError( ResId::UNKNOWN_ID_IN_CALLLIKE, ident.ToString() );
      }
    }
    else if (CurrentIs( TokenType::L_PAREN))
    {
      auto args = ParseArgsWOE( DummyToken(), DummyToken(), ident );
      return _handler.HandleCall( ident, std::move(args) );
    }
    else if (CurrentIs( TokenType::L_SBRACKET))
    {
      auto itype = MatchInputId( ident );
      if (itype == IOTensorType::UNSPECIFIED)
      {
        // u8 [1, 2, 3] などのケース。
        // u8([1, 2, 3])のシンタックスシュガー
        auto arg = ParseTupleExprWOE();
        return _handler.HandleCall( ident, { std::move(arg) } );
      }
      else
      {        
        // input_u8[-1](3, 4) などのケース
        // input_u8の状態でIRにするのでは無く、input_u8[-1]をパースの段階で処理してしまう事に。
        // input_u8とinput_u16を変数に入れて〜とか、-1を変数にして〜とかはできなくなるが、まずは単純にそういうのは禁止という事で実装。
        int index = ParseLayerSubscript();

        // あとは(3, 4)など。
        auto args = ParseArgsWOE( DummyToken(), DummyToken(), ident );
        return _handler.HandleInputCall( ident, itype, index, std::move(args) );
      }
    }

    throw FmtPErrorAt( ident, ResId::UNKNOWN_ID_IN_CALLLIKE, ident.ToString() );
  }

  /*
    primary_target = tensor_or_func_call
                   | targetable_expr

   tensor_or_func_callは先頭がidentだがスコープの変数に無いもの（IRElemでは無いメソッドっぽいものや関数呼び出しなど）

   これは最後のEOLを区切りとして扱う（ParsePrimaryWOEのコメント参照）
  */
 Expr ParsePrimaryTargetWOE()
 {
    if (CurrentIs(TokenType::IDENTIFIER))
    {
      auto ident = Current();

      // テンソル関連か関数呼び出し、
      if (!_handler.IsDefinedVariable( ident ))
      {
        return TensorOrFunctionCallWOE();
      }
    }
    return ParseTargetableExprWOE();
  }

  /*
   primary = primary_target
           | primary_target '.' swizzle
           | '.' identifier

    . identifier はenum。

    WOEについてはParseExprWOEのコメントや(#975)参照。
  */
  Expr ParsePrimaryWOE()
  {
    if (CurrentIs(TokenType::DOT))
    {
      // .CLAMP_TO_EDGE などのenum。
      Next();
      MustBe(TokenType::IDENTIFIER);

      auto enumName = Current();
      NextWOE();
      // HandlerにHandleEnumElemを定義するのが正しい分担だが、
      // enumはパースの時にsymbolとの間を行ったり来たりするので、Parser側に閉じている方が楽なのでここで返してしまう。

      size_t enumSym;
      if (!_enumSystem.LookupSymbol(enumName._str, enumSym))
      {
        throw FmtPErrorAt( enumName, ResId::UNKNOWN_ENUM_NAME, enumName.ToString() );
      }

      return Expr( new EnumElem( enumSym ) );

    }
    auto target = ParsePrimaryTargetWOE();

    if (CurrentIs(TokenType::DOT))
    {    
      Next();
      if( CurrentIs(TokenType::IDENTIFIER) )
      {
        // tup.hoge 型。
        auto methodId = Current();
        NextWOE();

        // 今のところ他のプロパティは無い（将来ここでサポートするかも）  
        auto indices = ToSwizzleIndices( methodId );
        auto maxIndex = *std::max_element( indices.begin(), indices.end() );
        
        ASSERT_MSG( target.GetType().IsTuple(), ResId::SWIZLE_ON_NONE_TUPLE );
        ASSERT_MSG( maxIndex < target.GetType().ItemNum(), ResId::SWIZLE_OOB, maxIndex, target.GetType().ItemNum() );

        return _handler.HandleSwizzleCall( std::move(target), std::move(indices) );
      }
      else if(CurrentIs(TokenType::INTEGER))
      {
        // tup.3 型
        auto indexToken = Current();
        NextWOE();
        auto index = indexToken.ToInt();
        
        ASSERT_MSG( target.GetType().IsTuple(), ResId::TUPLE_INDEX_OP_ON_NONE_TUPLE );
        ASSERT_MSG( index < target.GetType().ItemNum(), ResId::TUPLE_INDEX_OOB, index, target.GetType().ItemNum() );

        std::vector<size_t> indices{ (size_t)index };
        return _handler.HandleSwizzleCall( std::move(target), std::move(indices) );
      }
      else
      {
        throw FmtPError( ResId::UNEXPECTED_TOKEN_AFTER_DOT, Current()._ttype );
      }
    }

    return target;
  }

  // infixのbinopをパースしてenumを返す。
  // 知らないbinopの場合はUNKNOWNを返す。
  // カーソルの位置は進めない。
  BinOpType LookupBinOpOperator()
  {
    auto opttype = Current()._ttype;
    switch( opttype )
    {
      case TokenType::MINUS:
        return BinOpType::SUB;
      case TokenType::PLUS:
        return BinOpType::ADD;
      case TokenType::EQEQ:
        return BinOpType::EQEQ;
      case TokenType::NEQ:
        return BinOpType::NEQ;
      case TokenType::BAR2:
        return BinOpType::OR;
      case TokenType::BAR:
        return BinOpType::BITWISE_OR;
      case TokenType::AMPERSAND2:
        return BinOpType::AND;
      case TokenType::AMPERSAND:
        return BinOpType::BITWISE_AND;
      case TokenType::LT:
        return BinOpType::LT;
      case TokenType::LTEQ:
        return BinOpType::LE;
      case TokenType::LTLT:
        return BinOpType::LSHIFT;
      case TokenType::GT:
        return BinOpType::GT;
      case TokenType::GTEQ:
        return BinOpType::GE;
      case TokenType::GTGT:
        return BinOpType::RSHIFT;
      case TokenType::ASTER:
        return BinOpType::MUL;
      // //はまだ対応しない（本当にそのシンタックスにするかも決めてない）
      case TokenType::SLASH:
        return BinOpType::DIV;
      case TokenType::PERCENT:
        return BinOpType::MOD;
      case TokenType::HAT:
        return BinOpType::POW;
      case TokenType::PIPE_OP:
        return BinOpType::PIPE_OP;
      default:
        return BinOpType::UNKNOWN;
    }
  }

  // precedence climbingによる実装
  // https://eli.thegreenplace.net/2012/08/02/parsing-expressions-by-precedence-climbing
  //
  Expr ParseBinOpWOE( int prec )
  {
    auto left = ParseAtomWOE();

    while (true)
    {
      auto optype = LookupBinOpOperator();
      switch( optype )
      {
        case BinOpType::UNKNOWN:
          return left;
        case BinOpType::ADD:
        case BinOpType::SUB:
        case BinOpType::MUL:
        case BinOpType::DIV:
        case BinOpType::MOD:
        case BinOpType::EQEQ:
        case BinOpType::NEQ:
        case BinOpType::OR:
        case BinOpType::AND:
        case BinOpType::BITWISE_AND:
        case BinOpType::BITWISE_OR:
        case BinOpType::RSHIFT:
        case BinOpType::LSHIFT:
        case BinOpType::POW:
        case BinOpType::LT:
        case BinOpType::LE:
        case BinOpType::GT:
        case BinOpType::GE:
        case BinOpType::PIPE_OP:
        {
          auto& opprec = LookupPrecedance( optype );
          if (opprec._prec < prec)
            return left;

          /*
            binopのスキップ。
            binopのあとの改行は認める。
            let a = 3 +
            5
            みたいなケース。
          */
          Next();
          if (optype == BinOpType::PIPE_OP)
          {
            // パイプラインのシンタックスシュガー処理。
            ASSERT_MSG( !_ectx.ByPrev(), ResId::PIPE_NEST_NYI );
            _ectx.EnterByPrev( std::move(left) );
            left = ParseBinOpWOE( opprec.NextPrec() );
            ASSERT_MSG( !_ectx.ByPrev(), ResId::PIPE_UNRESOLVE );
          }
          else
          {
            // 通常のbinop。
            auto right = ParseBinOpWOE( opprec.NextPrec() );
            left = _handler.HandleBinOp( optype, std::move(left), std::move(right) );
          }
          continue;
        }
      }
    }
  }

  Precedance& LookupPrecedance( BinOpType btype )
  {
    auto iter = _binOpPrecedance.find( btype );
    if (iter == _binOpPrecedance.end())
      throw InternalError("No precedance found. Never reached here.");
    return iter->second;
  }

  /*
    block_args = '|' idlike_list '|'
  */
  std::vector<Token> ParseBlockArgs()
  {
    MustBe( TokenType::BAR );
    Next();
    auto argNames = ParseIdLikeList();
    MustBe( TokenType::BAR );
    Next();
    return argNames;
  }

  /*
    expr = binop

    ParseExprは最後の改行を区切りとして扱う。
    最後の改行を特別扱いするのはParseExprとそこから呼ばれるParseXXXだけ。
    これは
    let a = 5
    -3
    みたいなケースで-3がreturnになるため。
    詳細は(#975)参照。
  */
  Expr ParseExprWOE()
  {
    return ParseBinOpWOE( 0 );
  }

  Expr ParseExpr()
  {
    auto ret = ParseExprWOE();
    SkipEOL();
    return ret;
  }

  /*
    Tensor定義のブロック引数の処理。
    "_"の時はtemp変数名に置き換えする。

    TensorDef以外はBlockのBlankId2TmpVarを使うので微妙に処理の場所に一貫性が無いが、
    TensorDefはパースの時点で特別扱いされているのでこうなっている。

    pariのfirstにはtmp引数ならtrue、それ以外ならfalseが入る
  */
  std::vector<std::pair<bool, std::string>> ToTSArgs( std::vector<Token>& argTokens )
  {
    return MapFn( argTokens, [](const Token& tk){ 
      return tk._ttype == TokenType::BLANK_ID ? std::make_pair(true, UniqueName('t')) :  std::make_pair(false, tk.ToString()); 
    } );
  }

  // local_tensor_def = bounds_attr normal_tensor_def
  //
  // 現状はlocal_tensor_defはboundsが必須。将来的にguessするようになるとここを変更する。
  std::vector<ExprVOID>
  ParseLocalTensorDef()
  {
    auto boundsLet = ParseBoundsAttribute();

    MustBe( TokenType::DEF );
    Next();
    MustBe( TokenType::IDENTIFIER );
    auto tensorName = Current();
    Next();
    MustBe( TokenType::BAR );
    auto argTks = ParseBlockArgs();
    auto argNames = ToTSArgs( argTks );

    MustBe( TokenType::L_BRACE );
    // ここから新たなスコープ。
    // 仮引数を変数として登録する。
    _handler.EnterLocalTensorDef( tensorName, argNames );

    auto vblock = ParseBody();

    MustBe( TokenType::R_BRACE );

    auto initStmt = _handler.HandleLocalTensorDefAndLeave( tensorName, std::move(argNames), std::move( vblock ) );
    Next();

    if (boundsLet.size() == 0)
    {
      return ToVOIDVec( std::move(initStmt) );
    }

    auto ret = ToVOIDVec( std::move(boundsLet) );
    ret.push_back( std::move(initStmt).AsVOID() );
    return ret;
  }

  /*
    conv_tensor = 'reduce<' identifier '>.' identifier args

    ただし現状は
    reduce<YY>.accumulate(IMM, initExpr) |...| {...}
    の形しかサポートしない。
    そしてargのresolveでは、YY.reduceでresolveしてしまう
    （def以外のreduceも作る事になったら困るが、その時が来たら考える）
  */
  ConvertTensorTree
  ParseConvertTensor()
  {
    MustBe( TokenType::IDENTIFIER );
    auto convType = Current();
    Next();

    MustBe( TokenType::LT );
    Next();
    MustBe( TokenType::IDENTIFIER );
    auto srcTsName = Current();
    Next();
    MustBe( TokenType::GT );
    Next();

    MustBe( TokenType::DOT );
    Next();
    MustBe( TokenType::IDENTIFIER );
    auto methodName = Current();
    ASSERT_MSG( _funcTypeInfo.IsKnownReduceMethod( methodName ), ResId::UNKNOWN_REDUCE, methodName.ToString() );

    Next();
    MustBe( TokenType::L_PAREN );

    auto args = ParseArgsWOE( convType, srcTsName, methodName );
    SkipEOL();
    return ConvertTensorTree( convType, srcTsName, methodName, std::move(args) );
  }

  /*
    local_def_by_conv = 'def' identifier 'by' conv_tensor
  */
  ExprVOID
  ParseLocalDefByConvert()
  {
    MustBe( TokenType::DEF );
    Next();
    MustBe( TokenType::IDENTIFIER );
    auto destTsName = Current();
    Next();

    MustBe( TokenType::BY );
    Next();

    ConvertTensorTree convTree = ParseConvertTensor();

    return _handler.HandleDefByConvert( destTsName, std::move(convTree) );
  }

  /*
    mut_stmt = compound_assign
              | transform_stmt

    transform_stmt = 'mut' '!' 'trans<' ident '>.' ident '(' exprlist ')'

    compund_assign = 'mut' '!' ts_call '+=' expr

    ts_call = ident '(' args ')'
  */
  ExprVOID
  ParseMutationStmt()
  {
    MustBe( TokenType::MUT );
    Next();
    MustBe( TokenType::BANG );
    Next();

    MustBe( TokenType::IDENTIFIER );
    auto ident = Current();
    Next();

    if (CurrentIs(TokenType::L_PAREN))
    {
      // compound assignment
      auto args = ParseNormalParenArgList();

      auto callExpr = _handler.HandleCall( ident, std::move(args) );

      // HandleCallはts_call以外のケースもあるが、
      // このケースはts_callなので必ずTensorCallのLOAD。
      ASSERT_MSG( callExpr.GetElemType() == IRElemType::TensorCall, ResId::NONE_TENSOR_MUT_ASSIGN );
      ExprT<TensorCall> load( callExpr.Detach()->As<TensorCall>() );

      MustBe( TokenType::PLUS );
      Next();
      MustBe( TokenType::EQUAL );
      Next();

      auto rexpr = ParseExpr();
      return _handler.HandleCompoundAssignment( std::move(load), std::move(rexpr) ).AsVOID();
    }
    else if(CurrentIs(TokenType::LT))
    {
      // 例えば以下のようなケース
      // mut! trans<hist>.accumulate!(0) |i, h, accm| { h+accm }

      ASSERT_MSG( ident.IsEqual("trans"), ResId::UNKNOWN_MUT_ANGLEBRACKET );

      Next();
      auto tsname = Current();
      MustBe( TokenType::IDENTIFIER );

      Next();
      MustBe( TokenType::GT );
      Next();
      MustBe( TokenType::DOT );
      Next();

      MustBe( TokenType::IDENTIFIER );
      auto methodId = Current();
      ASSERT_MSG( _funcTypeInfo.IsKnownTransMethod( methodId ), ResId::UNKNOWN_TRANS, methodId.ToString() );

      Next();
      MustBe( TokenType::BANG );
      Next();
      MustBe( TokenType::L_PAREN );

      auto args = ParseArgsWOE( ident, tsname, methodId );

      return _handler.HandleTensorTransform( tsname, methodId, std::move(args) ).AsVOID();
    }
    else
    {
      throw FmtPError( ResId::UNKNOWN_MUT_STMT, Current()._str.ToString() );
    }

  }

  /*
    ExprかStmtかの判別で、Stmtである事を返す。
    これは例えばATMARKはlocal_tensor_defとみなす、
    などの決め打ちがあるため、StmtでもExprでも無いものは判別出来ないので使う場所に注意が必要。

    ts.for_eachなどはこの段階では区別出来ないのでExprとみなす。
  */
  bool IsStmtLike()
  {
    return CurrentIs( TokenType::LET )
    || CurrentIs( TokenType::ATMARK )
    || CurrentIs( TokenType::MUT )
    || CurrentIs( TokenType::DEF );
  }

  /*
    stmt = let_stmt
          | local_tensor_def
          | compound_assign

    let_stmtはdestructuringされると複数のletになる事があるのでvectorが返る

    local_tensor_defは今の所@bounds必須

    概念としてはExprVOID以外にもts.for_eachなどは本来はStmtであるはずだが、
    この文法要素としてのstmtはExprVOIDに限定している。
    完全な定義では無いが、少なくともこのパースが成功するものはStmt的なものなのは保証されるので、意味はある。
  */
  std::vector<ExprVOID>
  ParseStmt()
  {
    assert( IsStmtLike() );

    if (CurrentIs( TokenType::LET ))
    {
      auto letExpr = ParseLetStmt();
      if (!letExpr.IsDefined())
      {
        return {};
      }
      return ToVOIDVec( std::move(letExpr) );
    }
    else if (CurrentIs( TokenType::ATMARK ))
    {
      return ParseLocalTensorDef();
    }
    else if (CurrentIs( TokenType::MUT ))
    {
      auto comp = ParseMutationStmt();
      return ToVOIDVec( std::move(comp) );
    }
    else if (CurrentIs( TokenType::DEF ))
    {
      // @boundsの無いdef、これは現時点ではby reduceのみ
      auto defbyconv = ParseLocalDefByConvert();
      return ToVOIDVec( std::move(defbyconv) );
    }
    else
    {
      assert( false );
      throw InternalError("Never reached here");
    }
  }

  /*
    ts.for_eachなどのStmtとして振る舞う特殊なexprか？
  */
  bool IsStmtLikeExpr( const Expr& expr )
  {
    auto ti = expr.As<TensorIterator>();
    if (ti == nullptr)
      return false;

    return ti->IsStmtLike();
  }

  /*
   body =  '{' stmt_list '}'

   stmt_list = stmt+
  */
  ExprBODY ParseBody()
  {
    std::vector<Expr> stmts;

    MustBe( TokenType::L_BRACE );
    Next();
    while( !CurrentIs( TokenType::R_BRACE) )
    {
      if (IsStmtLike())
      {
        auto stmt = ParseStmt();

        AppendTail( stmts, std::move(stmt) );
        SkipEOL();  
      }
      else
      {
        //　expr。
        auto one = ParseExpr();
        auto stmtLike = IsStmtLikeExpr( one );

        stmts.push_back( std::move(one) );
        SkipEOL();

        // ts.for_eachなど以外だったらこれが最後のexpr
        if (!stmtLike)
        {
          // 最後のexpr。閉じ括弧じゃなければパースエラー。
          ASSERT_MSG( CurrentIs( TokenType::R_BRACE),  ResId::AFTER_LAST_EXPR_MUST_BE_CLOSE_BRACE );
          // 正常。抜ける。
          break;
        }          
      }
    }

    return _handler.HandleBody( std::move( stmts ) );
  }

  /*
    idlike = identifier | blank_id

    idlike_list = idlike
            | idlike ',' idlist
  */
  std::vector<Token> ParseIdLikeList()
  {
    std::vector<Token> ret;
    MustBeIdLike();

    ret.push_back( Current() );
    Next();
    while( CurrentIs( TokenType::COMMA ) )
    {    
      Next();

      MustBeIdLike();
      ret.push_back( Current() );
      Next();
    }
    return ret;
  }

  template<typename T, typename Fi, typename Fui, typename Ff>
  T ImmVecToTypedVec( std::vector<ExprT<ImmElem>>&& src, Fi fi, Fui fui, Ff ff )
  {
    Type tp = src[0].GetType();
    if (tp.IsInt())
    {
      auto ivec = MapFn( std::move(src), [](ExprT<ImmElem>&& elem) { return elem.GetElem()->Value<int32_t>(); } );
      return fi( std::move(ivec) );
    }
    else if(tp.IsUInt())
    {
      auto uvec = MapFn( std::move(src), [](ExprT<ImmElem>&& elem) { return elem.GetElem()->Value<uint32_t>(); } );
      return fui( std::move(uvec) );
    }
    else
    {
      assert(tp.IsFloat());
      auto fvec = MapFn( std::move(src), [](ExprT<ImmElem>&& elem) { return elem.GetElem()->Value<float>(); } );
      return ff( std::move(fvec) );
    }

  }

  TensorLiteralData
  CreateTensorLiteralData1D( std::vector<ExprT<ImmElem>>&& src )
  {
    return ImmVecToTypedVec<TensorLiteralData>( std::move(src),
      []( std::vector<int32_t>&& ivec ) { return TensorLiteralData( ivec ); },
      []( std::vector<uint32_t>&& uvec ) { return TensorLiteralData( uvec ); },
      []( std::vector<float>&& fvec ) { return TensorLiteralData( fvec ); }
      );
  }

  TensorLiteralData
  CreateTensorLiteralData2D( std::vector<ExprT<ImmElem>>&& src )
  {
    return ImmVecToTypedVec<TensorLiteralData>( std::move(src),
      []( std::vector<int32_t>&& ivec ) { return TensorLiteralData::Create2D( ivec ); },
      []( std::vector<uint32_t>&& uvec ) { return TensorLiteralData::Create2D( uvec ); },
      []( std::vector<float>&& fvec ) { return TensorLiteralData::Create2D( fvec ); }
      );
  }

  bool
  AddImmRowToTensorLiteralData( TensorLiteralData& ldata, std::vector<ExprT<ImmElem>>&& src )
  {
    return ImmVecToTypedVec<bool>( std::move(src),
      [&ldata]( std::vector<int32_t>&& ivec ) { return ldata.AddRow( ivec ); },
      [&ldata]( std::vector<uint32_t>&& uvec ) { return ldata.AddRow( uvec ); },
      [&ldata]( std::vector<float>&& fvec ) { return ldata.AddRow( fvec ); }
      );
  }

  /*
    literal_vector = imm
                | imm ',' literal_vector

    immは同じ型で無いといけない事がパースの時点でチェックされる。
  */
  std::vector<ExprT<ImmElem>>
  ParseLiteralVector()
  {
    auto imm = ParseImm();

    Type vecType( imm.GetType() );
    std::vector<ExprT<ImmElem>> vec;
    vec.push_back( std::move(imm) );

    while( CurrentIs(TokenType::COMMA) )
    {
      Next();
      auto imm2 = ParseImm();
      ASSERT_MSG( vecType == imm2.GetType(), ResId::NONE_HOMO_VEC_TS );
      vec.push_back( std::move(imm2) );
    }
    MustBe( TokenType::R_SBRACKET );

    return vec;
  }

  /*
    tensor_literal_data = tensor_literal_data_1d
                        | '[' tensor_literal_data_2d_content  ']'

    tensor_literal_data_1d = '[' literal_vector ']'
    tensor_literal_data_2d_content = tensor_literal_data_1d
                                    | tensor_literal_data_1d ',' tensor_literal_data_2d_content 

  */
  TensorLiteralData
  ParseTensorLiteralData()
  {
    MustBe( TokenType::L_SBRACKET );
    Next();
    if (CurrentIs(TokenType::L_SBRACKET))
    {
      // 2D.
      Next();
      ASSERT_MSG( !CurrentIs(TokenType::L_SBRACKET), ResId::TENSOR_LITERAL_OVER_2D );

      auto firstVec = ParseLiteralVector();

      MustBe( TokenType::R_SBRACKET );
      Next();

      auto ldata = CreateTensorLiteralData2D( std::move(firstVec) );
      while( CurrentIs( TokenType::COMMA ) )
      {
        Next();
        MustBe( TokenType::L_SBRACKET );
        Next();

        auto vec = ParseLiteralVector();
        bool isOk = AddImmRowToTensorLiteralData( ldata, std::move(vec) );
        ASSERT_MSG( isOk, ResId::NONE_HOMO_ROW_TS );

        MustBe( TokenType::R_SBRACKET );
        Next();
      }

      MustBe( TokenType::R_SBRACKET );
      Next();
      return ldata;
    }
    else
    {
      auto vec = ParseLiteralVector();

      MustBe( TokenType::R_SBRACKET );
      Next();
      
      return CreateTensorLiteralData1D( std::move(vec) );
    }
  }

  /*
    tensor_def = normal_tensor_def
               | def_by_tensor

    normal_tensor_def = 'def' identifier block_args '{' EOL? vblock EOL? '}'

    def_by_tensor = 'def' identifier 'by' literal_or_load

    literal_or_load = tensor_literal_data
                    | 'load' '(' STRING ')'
  */
  void ParseTensorDef()
  {
    MustBe( TokenType::DEF );
    Next();
    MustBe( TokenType::IDENTIFIER );
    auto tensorName = Current();
    Next();

    if (CurrentIs( TokenType::BAR ))
    {
      // normal_tensor_def

      auto argTks = ParseBlockArgs();
      auto argNames = ToTSArgs( argTks );
      
      MustBe( TokenType::L_BRACE );
      // ここから新たなスコープ。
      // 仮引数を変数として登録する。
      _handler.EnterTensorDef( tensorName, argNames );

      auto vblock = ParseBody();
      MustBe( TokenType::R_BRACE );

      _handler.HandleTensorDefAndLeave( tensorName, std::move(argNames), std::move( vblock ) );
      Next();

    }
    else
    {
      // def_by_tensor
      MustBe( TokenType::BY );
      Next();

      if (CurrentIs(TokenType::L_SBRACKET))
      {
        auto tdata = ParseTensorLiteralData();

        _handler.HandleTensorLiteralDef( tensorName, std::move(tdata) );
      }
      else
      {
        // def ts by load("hoge.png")
        CurrentIsIdentifier( "load" );
        Next();
        MustBe(TokenType::L_PAREN);
        Next();
        MustBe(TokenType::STRING);

        Token resName = Current();
        _handler.HandleResourceTensorDef( tensorName, resName );

        Next();
        MustBe(TokenType::R_PAREN);
        Next();        
      }

    }
  }

  Type MayVec( Type btp, size_t vdim )
  {
    if (vdim == 0)
      return btp;
    
    std::vector<Type> tps;
    for(auto i : NRange(vdim))
    {
      tps.push_back( btp );
    }
    return TupleType( tps );
  }

  /*
    type_prefixとvector_suffixの間は空白無し（これは２つのトークンでは無い）

    type_id = type_prefix vector_suffix?

    type_prefix = 'f32'
              | 'i32'
              | 'u32'
              | 'u8'

    type_suffix = 'v2'
                | 'v3'
                | 'v4'
  */
  Type TypeTkToType( const Token& typeId )
  {
    size_t bits = 8;
    if (typeId.IsStrAt( 1, "32", 2))
      bits = 32;
    else if(typeId.IsStrAt( 1, "16", 2))
      bits = 16;
    else if(typeId.IsStrAt( 1, "8", 1))
      bits = 8;
    else
      throw FmtPError( ResId::UNKNOWN_TYPE_ID, typeId.ToString() );

    // i32vXXX -> 3
    // i8vXXX -> 2
    size_t voff = bits < 10 ? 2 : 3;
    size_t vdim = 0;
    if (typeId.IsCharAt( voff, 'v'))
    {
      // i32v10 is not supported.
      if (typeId.Length() != voff+2)
        throw FmtPError( ResId::UNKNOWN_TYPE_ID, typeId.ToString() );

      if (typeId.IsCharAt( voff+1, '2'))
        vdim = 2;
      else if(typeId.IsCharAt( voff+1, '3'))
        vdim = 3;
      else if(typeId.IsCharAt( voff+1, '4'))
        vdim = 4;
      else
        throw FmtPError( ResId::UNKNOWN_TYPE_ID, typeId.ToString() );
    }

    if (typeId.IsCharAt( 0, 'f'))
    {
      return MayVec( Float( bits ), vdim );
    }
    else if(typeId.IsCharAt( 0, 'i'))
    {
      return MayVec( Int( bits ), vdim );
    }
    else if(typeId.IsCharAt( 0, 'u'))
    {
      return MayVec( UInt( bits ), vdim );
    }
    else
    {
      throw FmtPError( ResId::UNKNOWN_TYPE_ID, typeId.ToString() );
    }
  }

  Type ParseTypeId()
  {
    MustBe( TokenType::IDENTIFIER );
    auto typeId = Current();
    Next();
    return TypeTkToType( typeId );
  }

  /*
    one_formal_arg = identifier ':' type_id
  */
  std::pair<Token, Type> ParseOneFormalArg()
  {
      MustBe( TokenType::IDENTIFIER );
      Token vid = Current();
      Next();
      MustBe( TokenType::COLON );
      Next();
      Type tp = ParseTypeId();
      return std::make_pair( vid, tp );
  }

  /*
   formal_args = '||'
              | '|' formal_arg_list '|'
    
 
    formal_arg_list = one_formal_arg (',' one_formal_arg)*
  */
  std::vector<std::pair<Token, Type>> ParseFormalArgs()
  {
    std::vector<std::pair<Token, Type>> ret;

    if (CurrentIs(TokenType::BAR2))
    {
      Next();
      return ret;
    }
    
    MustBe( TokenType::BAR );
    Next();

    if (CurrentIs( TokenType::BAR ))
    {
      Next();
      return ret;
    }

    auto first = ParseOneFormalArg();
    ret.push_back( first );

    while(CurrentIs( TokenType::COMMA ))
    {
      Next();
      auto p = ParseOneFormalArg();
      ret.push_back( p );
    }
    MustBe( TokenType::BAR );
    Next();
    return ret;
  }

  /*
    function_def = 'fn' identifier formal_args '{' EOL? vblock EOL? '}'
  */
  void ParseFunctionDef()
  {
    MustBe( TokenType::FN );
    Next();
    MustBe( TokenType::IDENTIFIER );
    auto fname = Current();
    Next();
    auto fargs = ParseFormalArgs();

    _handler.EnterFunctionScope( fargs );

    MustBe( TokenType::L_BRACE );
    auto body = ParseBody();
    MustBe( TokenType::R_BRACE );
    NextWOE();

    _handler.HandleFunctionDefAndLeave( fname, fargs, std::move(body) );
  }

  std::string LookupUIResString( const std::string& key )
  {
    LocalResMap lres( _localStrMap );
    std::string ret;
    if (lres.Lookup( key, ret ))
      return ret;

    mfg_resource::UIResId resId = mfg_resource::LookupUIResId( key.c_str() );
    return std::string( mfg_resource::UIResourceString( resId ) );
  }

  /*
    文字列リテラルか、リソース指定のどちらか。

    string_value = STRING
                  | $ identifier

    後者はlabel=$LABEL_SIZEのような国際化のケース。
    LABEL_SIZEがリソースID。
    identifierは大文字のアンダースコア区切り推奨。

    リソースIDが存在しない場合などはUserError。
  */
  std::string ParseStringValue()
  {
    Token value = Current();
    if (value._ttype == TokenType::STRING)
    {
      Next();
      return value.ToString();
    }

    /*
      $ identifier
    */
    MustBe( TokenType::DOLLAR );
    Next();

    MustBe( TokenType::IDENTIFIER );
    auto resIdStr = Current().ToString();
    Next();
    return LookupUIResString( resIdStr );
  }

  // '[' (-)FLOAT (, (-)FLOAT)* ']'のうち、最初の'-'かFLOATのトークンの上で呼ばれる。
  std::vector<float> ParseFloatVector()
  {
    std::vector<float> fvec;

    while( !CurrentIs( TokenType::R_SBRACKET ))
    {
      bool is_minus = false;
      if (CurrentIs( TokenType::MINUS ))
      {
        is_minus = true;
        Next();
      }

      MustBe( TokenType::FLOAT );
      Token value = Current();
      float fval = is_minus ? -1.0f * value.ToFloat() : value.ToFloat();
      fvec.push_back( fval );

      Next();
      if (CurrentIs(TokenType::COMMA))
        Next();
    }

    MustBe( TokenType::R_SBRACKET );
    Next();    
    return fvec;
  }

  void MustBeString()
  {
    MustBeEither( TokenType::STRING, TokenType::DOLLAR );
  }


  // '[' STRING (, STRING)* ']'のうち、最初のSTRINGのトークンの上で呼ばれる。
  std::vector<std::string> ParseStringVector()
  {
    std::vector<std::string> svec;
    MustBeString();

    while( !CurrentIs( TokenType::R_SBRACKET ))
    {
      MustBeString();
      svec.push_back( ParseStringValue() );

      if (CurrentIs(TokenType::COMMA))
        Next();
    }

    MustBe( TokenType::R_SBRACKET );
    Next();    
    return svec;
  }

  /*
    FloatかStringのベクトル。

    attr_named_arg_vector_value = float_vector_value | string_vector_value

    float_vector_value = '[' (-)FLOAT (, (-)FLOAT)* ']'
    string_vector_value = '[' STRING (, STRING)* ']'

  */
  AttrNamedArgVal ParseAttrNamedArgVectorValue()
  {
    MustBe( TokenType::L_SBRACKET );
    Next();

    if (CurrentIs( TokenType::MINUS ) || CurrentIs( TokenType::FLOAT ))
    {
      std::vector<float> fvec = ParseFloatVector();
      return AttrNamedArgVal( std::move(fvec) );
    }
    MustBeString();
    std::vector<std::string> svec = ParseStringVector();
    return AttrNamedArgVal( std::move(svec) );
  }


  /*
    attr_named_arg_value = (-) INTEGER
                    | (-) FLOAT
                    | string_value
                    | attr_named_arg_vector_value
  */
  AttrNamedArgVal ParseAttrNamedArgValue()
  {
    Token value = Current();

    if (value._ttype == TokenType::L_SBRACKET)
      return ParseAttrNamedArgVectorValue();

    bool is_minus = false;
    if (value._ttype == TokenType::MINUS)
    {
      is_minus = true;
      Next();
      value = Current();
    }

    switch(value._ttype)
    {
      case TokenType::INTEGER:
      {
        Next();
        int sign = is_minus ? -1 : 1;
        return AttrNamedArgVal( sign*value.ToInt() );
      }
      case TokenType::FLOAT:
      {
        Next();
        float fval = is_minus ? -1.0f * value.ToFloat() : value.ToFloat();
        return AttrNamedArgVal( fval );
      }
      case TokenType::STRING:
      case TokenType::DOLLAR:
        try {
          auto res = ParseStringValue();
          return AttrNamedArgVal( res );
        }
        catch (const UserError&)
        {
          // 別のエラーメッセージの方がいいかもしれないが、とりあえず。
          throw FmtPError( ResId::UNKNOWN_PARAM_NAMED_ARG_VTYPE );
        }
      default:
        throw FmtPError( ResId::UNKNOWN_PARAM_NAMED_ARG_VTYPE );
    }
  }

  /*
    attr_named_arg = identifier '=' attr_named_arg_value

  */
  AttrNamedArg ParseAttrNamedArg()
  {
    MustBe( TokenType::IDENTIFIER );
    Token name = Current();
    Next();

    MustBe( TokenType::EQUAL );
    Next();

    auto v = ParseAttrNamedArgValue();
    return AttrNamedArg( name, std::move(v) );
  }

  /*
    param_rest_args = (',' attr_named_arg)*

    Currentが','じゃない場合は空のベクトルを返す。
  */
  std::vector<AttrNamedArg> ParseParamRestArgs()
  {
    std::vector<AttrNamedArg> ret;

    while(CurrentIs( TokenType::COMMA ))
    {
      Next();
      ret.emplace_back( ParseAttrNamedArg() );
    }
    return ret;
  }

  /*
    pure_attribute = param_def
                   | title_def

    title_def = '@' 'title' string_value

    param_def = '@' paramt identifier '(' param_def_args ')'

    paramt = 'param_i32'
           | 'param_f32'
           | 'param_pos' (deprecated)
           | `param_f32v2`
           | `param_f32v4`

    param_def_args = widget_type
                    | widget_type param_rest_args    

    widget_type = 'SLIDER' | 'POINTER' | 'DIRECTION' | 'CHECKBOX' | 'COLOR_PICKER' | 'DROPDOWN'
  */
  void ParsePureAttribute()
  {
    MustBe( TokenType::ATMARK );
    Next();

    MustBe( TokenType::IDENTIFIER );

    auto paramt = Current();

    if (paramt.IsEqual( "title" ))
    {
      Next();
      auto title = ParseStringValue();
      _handler.HandleTitleDef( title );
      return;
    }
    else if (paramt.IsEqual( "version" ))
    {
      Next();
      auto version = ParseStringValue();
      _handler.HandleVersionDef( version );
      return;
    }
    else
    {
      ASSERT_MSG( paramt.OneOf({ "param_i32", "param_f32", "param_pos", "param_f32v2", "param_f32v4" }), ResId::UNKNOWN_ATTR, paramt.ToString() );

      Next();

      MustBe( TokenType:: IDENTIFIER );
      auto paramId = Current();
      Next();

      MustBe( TokenType::L_PAREN );
      Next();

      MustBe( TokenType::IDENTIFIER );
      auto widgetTypeId = Current();
      ASSERT_MSG( widgetTypeId.OneOf({ "SLIDER", "POINTER", "DIRECTION", "CHECKBOX", "COLOR_PICKER", "DROPDOWN" }), ResId::UNKNOWN_WIDGET, widgetTypeId.ToString() );
      Next();

      auto rest = ParseParamRestArgs();
      
      MustBe( TokenType::R_PAREN );
      Next();


      if (paramt.IsEqual( "param_i32" ))
      {
        ASSERT_MSG( widgetTypeId.OneOf({ "SLIDER", "CHECKBOX", "DROPDOWN" }), ResId::UNSUPPORTED_PARAM_ON_WIDGET, paramt.ToString(), widgetTypeId.ToString() );
        WidgetType wtype = widgetTypeId.IsEqual("SLIDER") ? WidgetType::SLIDER : widgetTypeId.IsEqual("DROPDOWN") ? WidgetType::DROPDOWN : WidgetType::CHECKBOX;
        _handler.HandleParamInt32( paramId, wtype, std::move(rest) );
      }
      else if (paramt.IsEqual( "param_f32" ))
      {
        ASSERT_MSG( widgetTypeId.OneOf({ "SLIDER", "DIRECTION" }), ResId::UNSUPPORTED_PARAM_ON_WIDGET, paramt.ToString(), widgetTypeId.ToString() );
        WidgetType wtype = widgetTypeId.IsEqual("SLIDER") ? WidgetType::SLIDER : WidgetType::DIRECTION;
        _handler.HandleParamFloat32( paramId, wtype, std::move(rest) );
      }
      else if (paramt.IsEqual( "param_pos" ) || paramt.IsEqual( "param_f32v2" ))
      {
        ASSERT_MSG( widgetTypeId.OneOf({ "POINTER" }), ResId::UNSUPPORTED_PARAM_ON_WIDGET,paramt.ToString(), widgetTypeId.ToString() );
        ASSERT_MSG( rest.size() == 1 && rest[0]._name.IsEqual( "label" ), ResId::UNKNOWN_ARG_FOR_PARAM, "@param_f32v2" );
        ASSERT_MSG( rest[0]._value._type == AttrNamedArgVal::STRING, ResId::LABEL_MUST_BE_STRING );
        _handler.HandleParamF32V2( paramId, WidgetType::POINTER, rest[0]._value._sval );
      }
      else if (paramt.IsEqual( "param_f32v4" ))
      {
        ASSERT_MSG( widgetTypeId.OneOf({ "COLOR_PICKER" }), ResId::UNSUPPORTED_PARAM_ON_WIDGET, paramt.ToString(), widgetTypeId.ToString() );
        for( auto& arg : rest )
        {
          if (arg._name.IsEqual("label"))
          {
            ASSERT_MSG( arg._value._type == AttrNamedArgVal::STRING, ResId::LABEL_MUST_BE_STRING );
          }
          else if(arg._name.IsEqual("init"))
          {
            ASSERT_MSG( arg._value._type == AttrNamedArgVal::FLOAT_VEC && arg._value._fvec.size() == 4, ResId::INIT_MUST_BE_X, "f32v4" );
          }
          else
          {
            throw FmtPError( ResId::UNKNOWN_ARG_FOR_PARAM, "@param_f32v4" );
          }
        }
        _handler.HandleParamF32V4( paramId, WidgetType::COLOR_PICKER, std::move(rest) );
      }
      else
      {
        throw FmtPErrorAt( paramt, ResId::UNKNOWN_ATTR, paramt.ToString() );
      }
    }
  }

  /*
    bounds_attr = '@' 'bounds' '(' arg_list ')'

    expr_listがImmやVariableならreturnは空。
    それ以外の場合はtemp variableへのLetStmtが返り、set_boundとしてはこのtemp variableへのVariableとみなす。
    これらは次のtensor defの時の特殊処理として扱われる。
  */
  std::vector<ExprLET> ParseBoundsAttribute()
  {
    std::vector<ExprLET> ret;
    MustBe( TokenType::ATMARK );
    Next();

    auto attrName = Current();
    ASSERT_MSG( attrName.IsEqual("bounds"), ResId::EXPECT_BOUND, attrName.ToString() );
    Next();
    MustBe( TokenType::L_PAREN );
    auto args = ParseNormalParenArgList();

    // @boundsのattributeはdefの直前しか許さない。
    MustBe( TokenType::DEF );
    return _handler.HandleBoundsAttribute( std::move(args) );
  }

  /*
    print_expr_attr = '@' 'print_expr' '(' expr ')'
  */
  ExprLET ParsePrintExprAttr()
  {
    MustBe( TokenType::ATMARK );
    Next();
    
    MustBe( TokenType::IDENTIFIER );

    auto ident = Current();    
    ASSERT_MSG( ident.IsEqual( "print_expr" ), ResId::UNKNOWN_TOPLEVEL_ATTR, ident.ToString() );

    Next();
    MustBe( TokenType::L_PAREN );

    Next();
    auto argExpr = ParseExpr();
    
    MustBe( TokenType::R_PAREN );
    Next();

    return _handler.HandlePrintExpr( std::move(argExpr) );
  }

  /*
    toplevel_nonpure_attr = print_expr_attr
                          | bounds_attr
  */
  std::vector<Expr> ParseTopLevelNonPureAttr()
  {
    MustBe( TokenType::ATMARK );
    if (CurrentIsBoundsAttribute())
    {
      return ToExprVec( ParseBoundsAttribute() );
    }
    else
    {
      return ToExprVec( ParsePrintExprAttr() );
    }
  }

  /*
      toplevel_stmt = letlike_stmt
                    | toplevel_nonpure_attr
      
      bounds_attrなどで複数のletが出る事があるのでvector。
  */
  std::vector<Expr> ParseTopLevelStmt()
  {
    MustBeTopLevelStmt();

    if (CurrentIs(TokenType::LET))
    {
      auto letlike = ParseLetStmt();
      if (!letlike.IsDefined())
        return {};
      return { std::move(letlike) };
    }
    else
    {
      return ParseTopLevelNonPureAttr();
    }
  }

  // @bounds かどうか。
  bool CurrentIsBoundsAttribute()
  {
    if (!CurrentIs( TokenType::ATMARK ))
      return false;

    {
      PosSaver saver( _tokenizer );

      Next();
      return Current().IsEqual("bounds");
    }
  }

  /*
    param_def, tensor_def以外。

    letか @boundsか@print_exrのどれか。
    @の方はPureAttributeでは無い、という条件にしている。
  */
  bool CurrentIsTopLevelStmt()
  {
    return CurrentIs( TokenType::LET ) || 
      (CurrentIs( TokenType::ATMARK ) && !CurrentIsPureAttribute() );
  }

  void MustBeTopLevelStmt()
  {
    ASSERT_MSG( CurrentIsTopLevelStmt(), ResId::EXPECT_TOPLEVEL_STMT );
  }

  /*
    トップレッベルで実行されるletなど。まとめてTopLevelBlockになる。

    toplevel_block = toplevel_stmt+
  */
  void ParseTopLevelBlock()
  {
    std::vector<Expr> lets;
    MustBeTopLevelStmt();
    _handler.EnterTopLevelBlock();

    while( CurrentIsTopLevelStmt() )
    {
      auto letlikes = ParseTopLevelStmt();
      if (letlikes.size() != 0)
        AppendTail( lets, std::move(letlikes) );
      SkipEOL();
    }
    _handler.HandleTopLevelBlockDefAndLeave( std::move(lets) );
  }

  /*
    letの生成を伴わないattribute。
    @param_f32, @param_i32, @title など。
  */
  bool CurrentIsPureAttribute()
  {
    if (!CurrentIs( TokenType::ATMARK ))
      return false;

    {
      PosSaver saver( _tokenizer );

      Next();
      return Current().IsEqual("param_f32") || Current().IsEqual("param_i32") || Current().IsEqual("title") || Current().IsEqual("version") ||Current().IsEqual("param_pos") || Current().IsEqual("param_f32v2") || Current().IsEqual("param_f32v4");
    }
  }

  /*
    toplevelone = pure_attr
                | toplevel_block
                | tensor_def
                | function_def
  */
  void ParseTopLevelOne()
  {
    if (CurrentIsPureAttribute())
    {
      ParsePureAttribute();
      return;
    }
    else if (CurrentIs( TokenType::DEF ))
    {
      ParseTensorDef();
      return;
    }
    else if (CurrentIs( TokenType::FN))
    {
      ParseFunctionDef();
      return;
    }
    else if (CurrentIsTopLevelStmt())
    {
      ParseTopLevelBlock();
      return;
    }
    else
    {
      throw FmtPError( ResId::UNKNOWN_TOPLEVEL );
    }
  }

  bool IsEOS() { return CurrentIs( TokenType::END_OF_SRC ); }

  void ParseAll()
  {
    _tokenizer.SkipSpace();
    _tokenizer.Scan();
    SkipEOL();
    while (!IsEOS())
    {
      ParseTopLevelOne();
      SkipEOL();
    }
  }
};

}///< mfg_parser

#endif

