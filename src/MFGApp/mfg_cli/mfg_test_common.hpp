// Copyright 2026 Kazuma Arino. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#ifndef __MFG_TEST_COMMON_HPP__
#define __MFG_TEST_COMMON_HPP__

#include "forest.hpp"
#include "mfg.hpp"
#include <string>
#include <iostream>
#include <sstream>
#include "app_util.hpp"

#define _NFIFTEST_SUBTEST_
#include "nfiftest.hpp"
using namespace nfiftest;

// インテリセンスの効きがいまいちなので同じファイルに再定義
#undef REQUIRE
#define REQUIRE(expr) if(!(expr)) throw nfiftest::assert_fail_error(__FILE__, __LINE__, #expr)

namespace mfg_test_common {
using namespace std;
using namespace mfg_internal;
using namespace mfg;
using namespace mfg_ir_util;
using namespace mfg_pal;

using mfg_cli_app_util::PrintExprS;
using mfg_cli_app_util::ReadContent;
using mfg_cli_app_util::GenShaderSources;

/*
  UnitTest用にC++のAPIとして使う、Variableを保持するクラス。
  必要に応じてExprにラップしたVariableを生成する。

  VarというのがすでにあったのでVariableのExprを表すクラスという事でVarEと名付けた。
*/
struct VarE
{
  Type _type;
  std::string _name;

  const std::string& Name() const
  {
    return _name;
  }

  VarE( Type t, const std::string& name ) : _type(t), _name(name) {}

  operator Expr() const
  { 
    IRBuildDSL d;
    return d._Var( _type, _name );
  }

  operator ExprV() const
  {
    IRBuildDSL d;
    return d._Var( _type, _name );
  }
};


//
// かつてはExpr_XXXだったAPIを、テストを書き直すのが面倒なのでここにアダプタを作る。
//

inline ExprT<Call> Expr_FCallOne( Call::FuncType ftype, Expr&& arg0 )
{
  return ExprT<Call>( new Call( mfg_internal::Float(32), ftype, { std::move( arg0 ) } ) );
}

inline Expr Expr_Add( Expr&& x, Expr&& y )
{
  IRBuildDSL d;
  return d._Add( std::move(x), std::move(y) );
}

inline Expr Expr_Add( const Expr& x, const Expr& y )
{
  return Expr_Add( x.Clone(), y.Clone() );
}

inline Expr Expr_Sub( Expr&& x, Expr&& y )
{
  IRBuildDSL d;
  return d._Sub( std::move(x), std::move(y) );
}

inline Expr Expr_Mul( Expr&& x, Expr&& y )
{
  IRBuildDSL d;
  return d._Mul( std::move(x), std::move(y) );
}

inline Expr Expr_CastFloat32( Expr&& expr )
{ 
  IRBuildDSL d;
  return d._ToF32( std::move(expr) );
}

inline ExprLET Expr_Let( const VarE& variable, Expr&& value )
{
  IRBuildDSL d;
  return d._Let( variable._name, std::move(value) );
}

// 本当は可変長だが、テストでは一つのケースしかもう残ってないので一つ決め打ちで。
inline ExprBODY Expr_BodyElem( Expr&& arg )
{  
  return ExprBODY( new BodyElem( { std::move(arg) } ) );
}

inline ExprT<BlockElem> Expr_Block( std::vector<std::string>&& args, ExprBODY&& vblock )
{
  auto argPairs = MapFn( args, [](const std::string& name){ return std::make_pair( name, mfg_internal::Int(32) ); } );
  return ExprT<BlockElem>( new BlockElem( std::move(argPairs), std::move(vblock) ) );
}

inline ExprT<ReduceSum> Expr_ReduceSum( std::vector<int> intRanges, ExprT<BlockElem>&& fobj )
{
  assert( intRanges.size() % 2 == 0 );  
  std::vector<ExprRANGE> ranges;
  for( auto i : NRange(intRanges.size()/2) )
  {
    ranges.emplace_back( new RangeElem( MakeConst( mfg_internal::Int(32), intRanges[2*i] ), MakeConst( mfg_internal::Int(32), intRanges[2*i+1] ) ) );
  }
  return ExprT<ReduceSum>( new ReduceSum( std::move(ranges), std::move(fobj) ) );
}

inline void CollectArgsToExprVector( std::vector<Expr>& accm) {}

template<typename... Args>
void CollectArgsToExprVector( std::vector<Expr>& accm, const Expr &x, Args &&... args )
{
  accm.push_back( x.Clone() );
  CollectArgsToExprVector( accm, std::forward<Args>(args)...);    
}

template<typename... ARGS>
Expr Expr_TensorCall( const TensorInfo& t, const Expr &x, ARGS &&...args )
{
    IRBuildDSL d;
    std::vector<Expr> accm;
    CollectArgsToExprVector( accm, x, std::forward<ARGS>(args)... );
    return d._TsCall( t, std::move(accm) );
}

// inoutPosから先にpatが存在するか。
// 存在した場合、そのendの場所をinouotPosに入れる
inline bool Contains( string target, string pat, size_t& inoutPos)
{
  if (target.size() <= inoutPos)
    return false;

  auto pos = target.find( pat, inoutPos );
  if (string::npos == pos)
  {
    return false;
  }
  inoutPos = pos+pat.size();
  return true;    
}

inline size_t CountContains( string target, string pat )
{
  if (pat.length() == 0)
    throw std::runtime_error("Invalid empty pattern");

  size_t num = 0;
  std::string::size_type pos = 0;
  while ((pos = target.find( pat, pos )) != string::npos)
  {
    num++;
    pos += pat.length();
  }
  return num;
}

inline bool Contains( string target, string expect )
{
  return CountContains( target, expect ) != 0;
}

inline void CheckContains( string target, string expect )
{
  if (!Contains( target, expect ))
  {
    cout << "CheckContains: fail." << endl;
    cout << "expect: " << expect << endl;
    cout << "target: " << target << endl;
  }
  REQUIRE( Contains( target, expect ) );
}

inline void CheckContains( string target, string expect, size_t& inoutPos )
{
  size_t tmppos = inoutPos;
  if (!Contains( target, expect, tmppos ))
  {
    cout << "CheckContains pos: fail." << endl;
    cout << "expect: " << expect << endl;
    cout << "target: " << target << endl;
  }
  REQUIRE( Contains( target, expect, inoutPos ) );
}

inline void CheckNotContains( string target, string expect )
{
  REQUIRE( !Contains( target, expect ) );
}

inline void CheckNotContains( string target, string expect, size_t& inoutPos )
{
  REQUIRE( !Contains( target, expect, inoutPos ) );
}

inline void CheckStringEquals( string str1, string str2 )
{
  if (str1.size() != str2.size())
    cout << "size differ: len1(" << str1.size() << "), len2(" << str2.size() << ")" << endl;
  int range = std::min( str1.size(), str2.size() );
  for( auto i : NRange( range ))
  {
    if (str1[i] != str2[i])
    {
      cout << "differ at " << i << endl;
      cout << "src1: " << str1.substr( i, 100 ) << endl;
      cout << "src2: " << str2.substr( i, 100 ) << endl;
      return;
    }
  }
}

inline void VerifyEquals( const string& expect, const string& actual )
{
  if (expect != actual)
  {
    cout << actual << endl << endl;
    CheckStringEquals( expect, actual );
  }
  REQUIRE( expect == actual );
}



inline string PrintExprS( const Expr& elem )
{
  return PrintExprS( elem.GetBase() );
}

#define REQUIRE_FEQUALS( expr1, expr2 ) if( std::abs((expr1) - (expr2)) > 0.001 )  throw nfiftest::assert_fail_error(__FILE__, __LINE__, std::string(": expr1(") + #expr1 + std::string("= ") + std::to_string((expr1)) + std::string(", expr2(") + (#expr2) + ")=" + std::to_string((expr2)))

inline std::pair<std::vector<Expr>, std::vector<BaseExprElem*>> ToExprPair( const std::vector<int>& ivec )
{
  auto evec = MapFn(ivec, [](int i) { return MakeConst( Int(32), i ); });
  auto bvec = MapFn(evec, [](const Expr& e){ return (BaseExprElem*)e.GetBase(); });
  return { std::move(evec), std::move(bvec) };
}

/*
 ir_utilっぽいもの
 パーサーでは使っていないしTLTensorに依存するのでこちらに置く。
*/


/*
  ts.sum | a, b, c | {...}

  の時のa, b, cの型をtsから類推してペアにして返す。
  最初はdimensionに応じたindex、それよりあとはtsの_outputTypesに応じた型。
*/
inline std::vector<std::pair<std::string, Type>> MakeTSArgPair( TLTensor& ts, const std::vector<std::string>& argNames )
{
  auto tsinfo = ts.GetTensorInfo();
  assert( tsinfo.Dimensions()+tsinfo.ElemNum() == argNames.size() );

  std::vector<std::pair<std::string, Type>> ret;
  for (auto i : NRange( tsinfo.Dimensions() ))
  {
    ret.emplace_back( argNames[i], mfg_internal::Int(32) );
  }

  size_t offset = tsinfo.Dimensions();
  for (auto i : NRange( tsinfo.ElemNum() ))
  {
    ret.emplace_back( argNames[offset+i], tsinfo.ElemType( i ) );
  }
  return ret;
}

inline ExprT<BlockElem> Expr_TSDomainBlock( TLTensor&ts, std::vector<std::string>&& args, ExprBODY&& vblock )
{
  auto argPairs = MakeTSArgPair( ts, args );
  return ExprT<BlockElem>( new BlockElem( std::move(argPairs), std::move(vblock) ) );
}


inline ExprT<TensorIterator> Expr_ReduceTensorSum( TLTensor& ts, ExprT<BlockElem>&& fobj )
{
  return ExprT<TensorIterator>( new TensorIterator( TensorIterator::REDUCE_SUM, ts.GetTensorInfo(), std::move(fobj) ) );
}

} /// < mfg_test_common
#endif