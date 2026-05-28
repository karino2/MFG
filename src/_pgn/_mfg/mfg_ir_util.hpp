/* -*- coding: utf-8 -*- マルチバイト */

// Copyright 2026 Kazuma Arino. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#ifndef MFG_IR_UTIL_HPP_
#define MFG_IR_UTIL_HPP_

#include <mfg_pal/pal.hpp>

#include "mfg_elem.hpp"
#include "mfg_resource.hpp"

/*
  mfg_resource.hppに依存するutil系をひとまずここに置く。
  ir_utilでは無いので本当は他のファイルが良いが良い場所が無いので。
*/
namespace mfg_internal {

/*
  FmtXXXError関連。
  ResourceとExceptionをあわせたものの置き場。
  ir_utilでは無いがとりあえずここに置いておく。
*/
namespace mres = mfg_resource;
using mres::ResId;

template<typename ...ARGS>
std::string FmtRString( ResId rid, ARGS... args )
{
  auto fmt = mres::ResourceString( rid );
  return Format( fmt, args... );
}

template<typename E, typename ...ARGS>
E FmtErrorCommon( ResId rid, ARGS... args )
{
  auto str = FmtRString( rid, args... );
  return E( str );
}

template<typename ...ARGS>
UserError FmtUserError( ResId rid, ARGS... args )
{
  return FmtErrorCommon<UserError>( rid, args... );
}

template<typename ...ARGS>
InternalError FmtInternalError( ResId rid, ARGS... args )
{
  return FmtErrorCommon<InternalError>( rid, args... );
}

template<typename ...ARGS>
SystemError FmtSystemError( ResId rid, ARGS... args )
{
  return FmtErrorCommon<SystemError>( rid, args... );
}

/*
  NYI系のassertなどは、処理系を修正すべきだが、IDEをクラッシュはさせたくない。
  行情報などを失っているのでパースの段階で検出すべきだが、直す前の段階でも落とさずにInternalErrorにしたいものをここに置く。
*/
template<typename ...ARGS>
void ASSERT_INTERNAL( bool cond, ResId rid, ARGS... args )
{
  if (!cond)
    throw FmtInternalError( rid, args... );
}

}


/*
  mfg_ir_util

  IRを組み立てるのに使う言語内DSL。
  基本的にはmfg_elem.hppのみに依存する。
*/
namespace mfg_ir_util
{
using namespace mfg_pal;
using namespace mfg_internal;

// MacでUIntがぶつるかるので明示的にimport。
using mfg_internal::UInt;

inline void MakeCompatible( Expr& a, Expr& b );

/*
  基本的に状態を持たずに、実装継承して使うことを想定したメソッド群を持つ構造体。
  書きやすさやツリーのコードのみやすさを重視し、名前は短くやや暗号的なものもある。
  基本的にはrrefのExpr系列を引数にとりExpr系列を返す
*/
struct IRBuildDSL
{
  Type MayVectorizeType( Expr& arg, Type oneArgType )
  {
    if (arg.GetType().IsTuple())
    {
      // 引数がタプルの場合、returnはoneArgTypeのタプル。
      auto types = MapFn( arg.GetType().ExpandNumericTypes(), [oneArgType]( const NumericType& ) { return oneArgType; } );
      return TupleType( types );
    }
    else
    {
      return oneArgType;
    }
  }

  // ベクトライズも対応したキャスト
  Expr _ToType( Type tp, Expr&& expr )
  {
    auto retType = MayVectorizeType( expr, tp );
    return Expr( new Call( retType, Call::CAST, { std::move(expr) } ) );
  }

  Expr _ToI32( Expr&& expr ){ return _ToType( Int(32), std::move( expr) ); }
  Expr _ToU8V4( Expr&& expr ){ return _ToType( U8V4Type(), std::move( expr) ); }
  Expr _ToF32( Expr&& a ) { return _ToType( Float(32), std::move(a) ); }
  Expr _ToUI32( Expr&& expr ){ return _ToType( UInt(32), std::move( expr) ); }
  Expr _F32( double a ) { return MakeConst( Float(32), a ); }
  Expr _I32( int a ) { return MakeConst( Int(32), a ); }

  Expr _Pow( Expr&& lop, Expr&& rop)
  {
    return Expr( new Call( lop.GetType(), Call::POW, { std::move(lop), std::move(rop) } ) );
  }

  Expr _Call( Type retType, Call::FuncType ftype, std::vector<Expr>&& args )
  {
    return Expr( new Call( retType, ftype, std::move(args) ) );
  }

  /*
    0 <= target && target < extent

    0は含むがextentは含まない事に注意
  */
  Expr _IsInside( Expr&& target, Expr&& extent )
  {
    return _And(
      _Le(
       _I32(0),
       target.Clone()
      ),
      _Lt(
        target.Clone(),
        std::move(extent)
      )
    );
  }

  /*
    全部の引数が0以上extent未満か。つまり以下。
      0 <= arg0 && arg0 < extent(0)
      && 0 <= arg1 && arg1 < extent(1)
      ...
  */
 Expr _IsInside( const TensorInfo& ts, std::vector<Expr>&& args )
 {
  assert( args.size() == ts.Dimensions() );
  Expr cur;
  for (auto i: NRange( args.size() ))
  {
    Expr one = _IsInside( std::move(args[i]), ts.GetExtent(i) );
    if (cur.IsDefined())
    {
      cur = _And( std::move(cur),
                  std::move(one) );
    }
    else
    {
      cur = std::move(one);
    }
  }
  return cur;
 }

  ExprV _Var( Type t, const std::string& name )
  {
    return ExprV( new Variable( t, name ) );  
  }

  // 歴史的事情で順番が逆のペアが多いのでここでも逆にしておく。
  ExprV _Var( const std::pair<std::string, Type>& pair )
  {
    return _Var( pair.second, pair.first );
  }

  ExprV _VarI32( const std::string& name ) { return _Var( Int(32), name ); }
  ExprV _VarF32( const std::string& name ) { return _Var( Float(32), name ); }

  Expr _Ifel( Expr&& cond, Expr&& tcase, Expr&& fcase ) { return Expr( new IfEl( std::move(cond), std::move(tcase), std::move(fcase)) ); }
  Expr _Abs( Expr&& a ) { return Expr( new Call( a.GetType(), Call::ABS, { std::move(a) } ) ); }

  Expr _Swizzle( Expr&& tuple, std::vector<size_t>&& indices ) { return Expr( new SwizzleCall( std::move(tuple), std::move(indices) ) ); }
  Expr _Spread( Expr&& target ) { return Expr( new SpreadElem( std::move(target) ) ); }

  /*
   BinOp共通
  */

  /*
    二項演算のimplicit conversionのルールを実装する
    float と int-uintならfloatに、
    それ以外でどちらかがuintならuintに、
    それ以外ならintに。
  */
  NumericType _BinOpImplicitConvType( NumericType tp1, NumericType tp2 )
  {
    if (tp1.IsFloat())
      return tp1;
    else if (tp2.IsFloat())
      return tp2;
    else if(tp1.IsUInt())
      return tp1;
    else
      return tp2;
  }

  /*
    a*[b, c, d] みたいなケースでの結果の型を返す。

  */
  Type _VectorizedBinOpType( NumericType atomType, Type vecType )
  {
    assert( vecType.IsTuple() );
    std::vector<NumericType> ntypes;

    for( auto np : vecType.ExpandNumericTypes() )
    {
      ntypes.push_back( _BinOpImplicitConvType( atomType, np ) );
    }
    return Type( std::move(ntypes) );
  }

  /*
    BinOpTypeが >=や==などのbooleanの場合、returnの型はint32かそれのベクトルとなる。
    それ以外ならinTypeをそのまま返す。
  */
  Type _BinOpReturnType( BinOp::BinOpType opType, Type inType )
  {
    if (BinOp::IsBoolOp(opType))
    {
      if (inType.IsTuple())
      {
        std::vector<NumericType> ntypes;
        for( auto tp: inType.ExpandNumericTypes() )
        {
          ntypes.emplace_back( NumericType::INT, 32 );
        }
        return Type( std::move(ntypes) );
      }
      else
      {
        return Int(32);
      }
    }
    else
    {
      return inType;  
    }
  }

  Expr _BinOpGen( BinOp::BinOpType opType, Expr&& lop, Expr&& rop )
  {
    if (lop.GetType().IsTuple() || rop.GetType().IsTuple())
    {
      // 両方タプルのケース
      if (lop.GetType().IsTuple() && rop.GetType().IsTuple())
      {
        // チェックは呼び出し側でやる
        // ASSERT_SAME_TUPLE_TYPE( opType, lop, rop );

        // booleanのbinopは結果がintとなる。それ以外はa.getType()。
        Type resType = _BinOpReturnType( opType, lop.GetType() );
        return Expr( new BinOp( resType, opType, std::move(lop), std::move(rop) ) );
      }
      else
      {
        // スカラー*ベクトル みたいなケース、ベクトライズ
        Type rawResType;
        if (lop.GetType().IsTuple())
        {
          assert( !rop.GetType().IsTuple() );
          rawResType = _VectorizedBinOpType( rop.GetType().AsNumeric(), lop.GetType() );
        }
        else
        {
          rawResType = _VectorizedBinOpType( lop.GetType().AsNumeric(), rop.GetType() );
        }
        Type resType = _BinOpReturnType( opType, rawResType );
        return Expr( new BinOp( resType, opType, std::move(lop), std::move(rop) ) );
      }
    }
    else
    {
      // 0xff << 8 とかは型が違っていても問題ないのでスキップ。
      if (!(opType == BinOp::ShiftLeft || opType == BinOp::ShiftRight) )
       MakeCompatible( lop, rop );

      // booleanのbinopは結果がintとなる。それ以外はa.getType()。
      Type resType = _BinOpReturnType( opType, lop.GetType() );
      return Expr( new BinOp( resType, opType, std::move(lop), std::move(rop) ) );
    }
  }

  /*
    個々のBinOp
  */

  Expr _Or( Expr&& a, Expr&& b )
  {
    return _BinOpGen( BinOp::Or, std::move(a), std::move(b) );
  }

  Expr _And( Expr&& a, Expr&& b )
  {
    return _BinOpGen( BinOp::And, std::move(a), std::move(b) ) ;
  }

  Expr _Add( Expr&& x, Expr&& y )
  { 
    return _BinOpGen( BinOp::Add, std::move(x), std::move(y) );
  }
  Expr _Add( Expr&& a, double b ) { return _Add( std::move(a), _F32( b ) ); }
  Expr _Add( Expr&& a, int b ) { return _Add( std::move(a), _I32( b ) ); }
  Expr _Mul( Expr&& x, Expr&& y )
  { 
    return _BinOpGen( BinOp::Mul, std::move(x), std::move(y) );
  }
  Expr _Mul( Expr&& a, int b )
  { 
    return _Mul( std::move(a), _I32(b) );
  }

  Expr _Sub( Expr&& x, Expr&& y )
  { 
    return _BinOpGen( BinOp::Sub, std::move(x), std::move(y) );
  }
  Expr _Sub( Expr&& a, int b )
  { 
    Type t = a.GetBase()->_type;
    return _Sub( std::move(a), MakeConst( t, b ) );
  }
  Expr _Sub( Expr&& a, double b ) { return _Sub( std::move(a), _F32( b ) ); }
  Expr _Sub( double a, Expr &&b ) { return _Sub( _F32(a), std::move( b ) ); }

  Expr _Div( Expr&& x, Expr&& y )
  { 
    return _BinOpGen( BinOp::Div, std::move(x), std::move(y) );
  }

  Expr _Lt( Expr&& a, Expr&& b )
  { 
    return _BinOpGen( BinOp::Lt, std::move(a), std::move(b) );
  }

  Expr _Le( Expr&& a, Expr&& b )
  {
    return _BinOpGen( BinOp::Le, std::move(a), std::move(b) );
  }

  Expr _Gt( Expr&& a, Expr&& b )
  {
    return _BinOpGen( BinOp::Gt, std::move(a), std::move(b) );
  }

  Expr _Ge( Expr&& a, Expr&& b )
  {
    return _BinOpGen( BinOp::Ge, std::move(a), std::move(b) );
  }

  Expr _Neq( Expr&& x, Expr&& y )
  {
    return _BinOpGen( BinOp::Neq, std::move(x), std::move(y) );
  }


  Expr _ClampToEdge( const TensorInfo& ts, Expr&& x, Expr&& y )
  {
    Expr max0 = _Sub( ts.GetExtent(0), 1 );
    Expr max1 = _Sub( ts.GetExtent(1), 1 );
    return _TsVCall( ts, { _Clamp( std::move(x), 0, std::move(max0) ), _Clamp( std::move(y), 0, std::move(max1) ) } );
  }

  Expr _ClampToEdge( const TensorInfo& ts, Expr&& x )
  {
    Expr max0 = _Sub( ts.GetExtent(0), 1 );
    return _TsVCall( ts, { _Clamp( std::move(x), 0, std::move(max0) ) } );
  }

  inline Expr _ClampToBorderValue( const TensorInfo& input, const Expr& defVal,  Expr&& arg1, Expr&& arg2 );
  inline Expr _ClampToBorderValue( const TensorInfo& input, const Expr& defVal,  Expr&& arg1 );

  ExprLET _PrivateLet( const std::string& varName, Expr&& value )
  {
    return ExprLET( Let::MakePrivate( varName, std::move(value) ) );
  }
  ExprLET _Let( const std::string& varName, Expr&& value )
  { 
    return ExprLET( new Let( varName, std::move(value) ) );
  }
  Expr _VLet( const std::string& varName, Expr&& value )
  { 
    return Expr( new VectorLet( varName, std::move(value) ) );
  }

  Expr EnsureUInt32( Expr&& a )
  {
    if (a.GetType().IsUInt())
      return std::move(a);
    return _ToUI32( std::move(a) );
  }

  Expr EnsureInt32( Expr&& a )
  {
    if (a.GetType().IsInt())
      return std::move(a);
    return _ToI32( std::move(a) );
  }

  Expr _Clamp( Expr&& a, Expr&& minVal, Expr&& maxVal )
  {
    assert( a.IsDefined() && minVal.IsDefined() && maxVal.IsDefined() );
    auto tp = a.GetType();
    return Expr( new Call( tp, Call::CLAMP, { std::move(a), std::move(minVal), std::move(maxVal) } ) );
  }

  ExprBODY _BodyElem( std::vector<Expr>&& exprs )
  {
    return ExprBODY( new BodyElem( std::move(exprs) ) );
  }

  ExprT<ExecTensorInitialize> _ExecTsInit( std::vector<size_t>&& bounds, ExprT<TensorElem>&& ts )
  {
    return ExprT<ExecTensorInitialize>( new ExecTensorInitialize( std::move(bounds), std::move(ts) ) );
  }

  Expr _Tuple( std::vector<Expr>&& items )
  {
    return Expr( new TupleElem( std::move(items) ) );
  }

  ExprT<TensorElem> _TsElem( std::vector<std::string>&& args, ExprBODY&& vblock )
  {
    auto argPairs = MapFn( args, [](const std::string& name){ return std::make_pair( name, mfg_internal::Int(32) ); } );
    return ExprT<TensorElem>( new TensorElem( std::move(argPairs), std::move(vblock) ) );
  }

  ExprT<TensorCall> _TsCall( const TensorInfo& t, std::vector<Expr>&& es )
  {
    return ExprT<TensorCall>( new TensorCall( t.GetType(), t, TensorCall::LOAD, false, std::move(es) ) );
  }

  // 引数の範囲チェック済みｎケース
  ExprT<TensorCall> _TsVCall( const TensorInfo& t, std::vector<Expr>&& es )
  {
    return ExprT<TensorCall>( new TensorCall( t.GetType(), t, TensorCall::LOAD, true, std::move(es) ) );
  }
};

// 引数が　Expr&で直接中をいじるもの。Castくらい。
struct MutableIRBuildDSL
{
  void _ToType( Expr& e, Type tp )
  {
    auto base = e.Detach();
    e.Attach( new Call( tp, Call::CAST, { Expr(base) } ) );
  }

  void _ToF32( Expr& e )
  {
    assert( !e.GetType().IsFloat() );
    _ToType( e, Float(32) );
  }
};

// 引数が全てconst referenceでクローンされるDSL
struct ConstIRBuildDSL
{
  struct IRBuildDSL _r;

  Expr _Lt( const Expr& x, const Expr& y )
  {
    return _r._Lt( x.Clone(), y.Clone() );
  }
  Expr _Gt( const Expr& x, const Expr& y )
  {
    return _r._Gt( x.Clone(), y.Clone() );
  }
};


inline void MakeCompatible( Expr& a, Expr& b )
{
  MutableIRBuildDSL md;

  Type ta = a.GetType();
  Type tb = b.GetType();

  if (ta == tb)
    return;

  // これ以外はNYI  
  ASSERT_INTERNAL( ta.IsNumeric() && tb.IsNumeric(), ResId::GENERAL_NYI );

  NumericType na = ta.AsNumeric();
  NumericType nb = tb.AsNumeric();

  if (na.IsFloat() && !nb.IsFloat())
  {
    md._ToF32( b );
    return;
  }
  else if (!na.IsFloat() && nb.IsFloat())
  {
    md._ToF32( a );
    return;
  }
  else if (na.IsInt() && nb.IsUInt())   
  {
    // intとuintがあったら、intが大きければintにする、それ以外はパースエラー。
    ASSERT_INTERNAL( na._bits > nb._bits, ResId::SIGN_UNSIGN_MISMATCH );
    md._ToType( b, Int(na._bits) );
    return;
  }
  else if (na.IsUInt() && nb.IsInt())
  {
    // 同上
    ASSERT_INTERNAL( nb._bits > na._bits, ResId::SIGN_UNSIGN_MISMATCH );
    md._ToType( a, Int(nb._bits) );
    return;    
  }
  else if ((na.IsUInt() && nb.IsUInt()) || (na.IsInt() && nb.IsInt()))
  {
    // bitwidthだけ違うケース
    if (na._bits > nb._bits)
    {
      md._ToType( b, Int(na._bits) );
      return;
    }
    else
    {
      assert( na._bits < nb._bits );
      md._ToType( a, Int(nb._bits) );
      return;
    }
  }

  // NYI.
  assert(false);
}


/*
  ClampToBorderValueの実装
*/
struct BoundedDim
{
  Expr _max0; // max = extent - 1.
  BoundedDim( Expr&& max0 ) :  _max0( std::move(max0) ) {}

  Expr IsOob( const Expr& arg0 )
  {
    IRBuildDSL r;
    ConstIRBuildDSL c;
    return r._Or( c._Lt( arg0, 0 ), c._Gt( arg0, _max0 ) );
  }
};

Expr IRBuildDSL::_ClampToBorderValue( const TensorInfo& tsinfo, const Expr& inputDefVal,  Expr&& arg1 )
{
  Expr defValue( inputDefVal.Clone() );

  BoundedDim bound0( _Sub( tsinfo.GetExtent(0), 1 ) );
 
  Expr outOfBounds = bound0.IsOob( arg1 );

  // selectのショートカットを前提とする。
  return _Ifel( std::move(outOfBounds), std::move(defValue),
    _TsCall( tsinfo, { std::move(arg1) } )
  );
}

Expr IRBuildDSL::_ClampToBorderValue( const TensorInfo& tsinfo, const Expr& inputDefVal,  Expr&& arg1, Expr&& arg2 )
{
  Expr defValue( inputDefVal.Clone() );

  BoundedDim bound0( _Sub( tsinfo.GetExtent(0), 1 ) );
  BoundedDim bound1( _Sub( tsinfo.GetExtent(1), 1 ) );

  Expr outOfBounds = _Or( bound0.IsOob( arg1 ), bound1.IsOob( arg2 ) );

  // selectのショートカットを前提とする。
  return _Ifel( std::move(outOfBounds), std::move(defValue),
    _TsCall( tsinfo, { std::move(arg1), std::move(arg2) } )
  );
}

/*
  TensorにまつわるIRの生成を担当するクラス
*/
struct TensorIRFactory : private IRBuildDSL
{
  const TensorInfo& _tsinfo;

  TensorIRFactory( const TensorInfo& ts ) : _tsinfo( ts ) {}

  Expr Extent( size_t dim ) const
  {
    return _tsinfo.GetExtent( dim );
  }

  Expr FlattenIdx( bool isArgVerified, std::vector<Expr>&& args ) const
  {
    if (isArgVerified)
      return NoClampFlattenIdx( std::move(args) );
    return SafeFlattenIdx( std::move(args) );
  }

  /*
    [x, y, z] を渡して、

    ((((z)*ts.extent.1)+y)*ts.extent.0)+xを返す。

    x, y, zは全てintと思う。
  */
  Expr NoClampFlattenIdx( std::vector<Expr>&& args ) const
  {
    mfg_ir_util::IRBuildDSL d;
    Expr cur;
    for (auto i : RevRange( args.size() ))
    {
      if (i == args.size()-1)
      {
        cur = std::move( args[i] );
      }
      else
      {
        cur = d._Add( d._Mul( std::move(cur), Extent(i) ), std::move(args[i]) );
      }
    }
    return cur;
  }

  /*
    [x, y, z] を渡して、

    clamp(flattenIdx, 0, ts.extent.2*ts.extent.1*ts.extent.0-1)を返す。

    x, y, zは全てintと思う。
    ClampToEdgeと違い、x, y, zといったものをclampするのではなく、
    全indexがはみ出さないようにするだけ。
  */
  Expr SafeFlattenIdx( std::vector<Expr>&& args ) const
  {
    mfg_ir_util::IRBuildDSL d;
    Expr flattenIdx = d.EnsureInt32( NoClampFlattenIdx( std::move(args) ) );

    // 0の場合はclampは不要
    if (flattenIdx.IsZero())
      return flattenIdx;

    Expr maxExpr;
    for (auto i : RevRange( args.size() ))
    {
      if (!maxExpr.IsDefined())
      {
        maxExpr = Extent(i);        
      }
      else
      {
        maxExpr = d._Mul( std::move(maxExpr), Extent(i) );
      }
    }
    maxExpr = d.EnsureInt32( d._Sub( std::move(maxExpr), 1 ) );
    return d._Clamp( std::move(flattenIdx), Expr(0), std::move(maxExpr) );
  }

  std::vector<Expr> GenAllocateStmt( Expr&& size )
  {
    if (_tsinfo.IsVector())
    {
      return { Expr( new AllocateStmt( _tsinfo.ElemType( 0 ), _tsinfo.ElemBufferName( 0 ), _Mul( std::move(size), _tsinfo.VectorExtent() ) ) ) };
    }
    else
    {
     std::vector<Expr> ret;
      for( auto tidx: NRange( _tsinfo.ElemBufferNum() ) )
      {
        ret.emplace_back( new AllocateStmt( _tsinfo.ElemType( tidx ), _tsinfo.ElemBufferName( tidx ), size.Clone() ) );
      }
      return ret;
    }
  }

  std::vector<Expr> ToIndexVariables( std::vector<std::pair<std::string, Type>>::const_iterator beg, std::vector<std::pair<std::string, Type>>::const_iterator end )
  {
    std::vector<Expr> ret;
    auto cur = beg;
    while( cur != end )
    {
      ret.push_back( _Var( *cur ) );
      cur++;
    }
    return ret;
  }

  std::vector<Expr> ToIndexVariables( const std::vector<std::string>& varNames )
  {
    return MapFn( varNames, [this]( const std::string& name ){
      return (Expr)_Var( Int(32), name );
    } );
  }

  // tupleの時は子の配列に、そうでなければ自身のみの1要素配列を返す
  std::vector<Expr> ExpandElem( Expr&& rexpr )
  {
    if ( rexpr.GetElemType() == IRElemType::TupleElem )
    {
      auto tup = rexpr.As<TupleElem>();
      return tup->DetachChildren();
    }
    else
    {
      return { std::move(rexpr) };
    }
  }

  std::vector<Expr> GenStore( std::vector<Expr>&& indexArgs, Expr&& rexpr )
  {
    auto flatten = NoClampFlattenIdx( std::move(indexArgs) );
    if (_tsinfo.ElemNum() == 1 || _tsinfo.IsVector())
    {
      return { Expr( new StoreStmt( _tsinfo.ElemBufferName( 0 ), std::move(flatten), std::move(rexpr) ) ) };
    }
    else
    {
      // タプルのケース。個々のバッファのStoreStmtにする

      auto expanded = ExpandElem( std::move(rexpr) );

      // rexprがタプル型だがTupleElemの時には等しくならないかも。
      // そのケースはLowerで無くなっていると思っているがこのassertがfailしたら調べる。
      assert( _tsinfo.ElemNum() == expanded.size() );

      std::vector<Expr> ret;
      for( auto tidx: NRange( _tsinfo.ElemNum() ) )
      {        
        ret.emplace_back( new StoreStmt( _tsinfo.ElemBufferName( tidx ), flatten.Clone(), std::move(expanded[tidx]) ) );
      }
      return ret;
    }
  }

  Expr GenLoad( std::vector<Expr>&& indexArgs )
  {
    // GenLoadは今のところMayGenLoadでしか使われていない。
    // そして全部のケースで引数はverifyされてる気がするが、少し自信が無いので、Clampしておく（分からない時は安全に倒すという事で）。
    auto flatten = SafeFlattenIdx( std::move(indexArgs) );
    if (_tsinfo.ElemNum() == 1 || _tsinfo.IsVector())
    {
      return Expr( new LoadExpr( _tsinfo.GetType(), _tsinfo.ElemBufferName( 0 ), _tsinfo, std::move(flatten)  ) );
    }
    else
    {
      // タプルはTupleElemとして返す
      // このGenLoadでタプルが来るのはtensorのiterate系でのaccmの初期化のみ。
      std::vector<Expr> items;
      for( auto tidx : NRange( _tsinfo.ElemNum() ) )
      {
        items.emplace_back( new LoadExpr( _tsinfo.ElemType(tidx), _tsinfo.ElemBufferName( tidx ), _tsinfo, flatten.Clone()  ) );
      }
      return Expr( new TupleElem( std::move(items) ) );
    }

  }

  Expr GenTupleLoad( const Type& tp, bool isArgVerified, size_t tupIdx, std::vector<Expr>&& args )
  {
    return Expr( new LoadExpr( tp, _tsinfo.ElemBufferName( tupIdx ), _tsinfo, FlattenIdx( isArgVerified, std::move(args) ) ) );
  }

  bool IsVectorLoad( const Expr& expr )
  {
    if (expr.GetElemType() != IRElemType::LoadExpr)
      return false;
    return expr.As<LoadExpr>()->IsVector();
  }

  // identifier が"_"で無ければ、
  // let identifier = load(ts...)
  // 的なletを生成しdestに入れる。
  // blank idなら何もしない。
  // 
  // tensorの関わるループ関連で使うが、微妙に良い置き場が無いのでここに置く。
  void MayGenLoadAssign( std::vector<Expr>& dest, const std::string& identifier, std::vector<std::pair<std::string, Type>>::const_iterator beg, std::vector<std::pair<std::string, Type>>::const_iterator end )
  {
    if (identifier == "_")
      return;
    
    auto indexArgs = ToIndexVariables( beg, end );
    auto loadExpr = GenLoad( std::move(indexArgs) );

    if(IsVectorLoad( loadExpr ))
    {
      // ベクトルの時はバックエンドによって要素のロードの仕方が異なるので、そのままバックエンドに流す。
      dest.emplace_back( new VectorLet( identifier, std::move(loadExpr) ) );      
    }
    else if (loadExpr.GetType().IsTuple())
    {
      /*
        テンソルがタプルの時のaccmなどへのアサイン。
        let identifer.0 = load.0
        let identifier.1 = load.1
        ...
        的な展開をする。
      */
      assert( loadExpr.GetElemType() == IRElemType::TupleElem );
      auto tup = loadExpr.As<TupleElem>();
      auto items = tup->DetachChildren();
      for( auto tidx : NRange( items.size() ) )
      {
        dest.emplace_back( Let::MakePrivate( Variable::TupleItemName( identifier, tidx ), std::move(items[tidx]) ) );
      }
    }
    else
    {
      dest.emplace_back( Let::MakePrivate( identifier, std::move(loadExpr) ) );
    }
  }
};

/*
インライン関数関連。

インライン関数はIRの組み立てはLowerでいろいろ展開が終わったあとにやる事になるが、引数のチェックなどはパース時にやる事になるので両方にまたがるのでここに置く。
*/

/*
TypeにgenTypeなどを拡張したもの。
genTypeなどはIR上には存在しないのでtypeとは分ける。
*/
struct ExtendType
{
  enum EType
  {
    NORMAL,
    GEN_TYPE
  };

  EType _etype;

  // Normalの時にのみ使われる。
  Type _type;

  ExtendType( EType etype ) : _etype(etype)
  {
    assert( etype == GEN_TYPE );
  }

  ExtendType( EType etype, Type tp ) : _etype(etype), _type(tp)
  {
    assert( etype == NORMAL );
  }

  bool IsGenType() const { return _etype == GEN_TYPE; }
  bool IsNormalType() const { return _etype == NORMAL; }
};

/*
  Inline Functionのうちコピーなどが用意な部分
  名前や型情報など。
*/
struct InlineFuncInfo
{
  std::string _name; // エラー時の情報のために関数名も持っておく
  std::vector<ExtendType> _paramTypes;
  ExtendType _retType;
  size_t _inlfId = 0; // Registerされる時に更新される。vectorのindex。
  InlineFuncInfo( std::string name, std::vector<ExtendType>&& paramTypes, ExtendType&& retType ) :
    _name( std::move(name) ), _paramTypes( std::move(paramTypes) ), _retType( std::move(retType) ) {}
};

struct InlineFunction
{
  InlineFuncInfo _info;
  
  /*
  speadなどが終わった状態で呼ばれる。
  */
  std::function<ExprBODY( std::vector<Expr>&& )> _expand;

  InlineFunction( std::string name, std::vector<ExtendType>&& paramTypes, ExtendType&& retType, std::function<ExprBODY( std::vector<Expr>&& )>&& expandFunc ) :
    _info( std::move(name), std::move(paramTypes), std::move(retType) ),
    _expand( std::move(expandFunc) ) {}
};

struct InlineFuncRegistry : private IRBuildDSL
{
  std::vector<InlineFunction> _inlfs;

  InlineFunction& Push(InlineFunction &&udf)
  {
    size_t index = _inlfs.size();
    _inlfs.push_back( std::move(udf) );
    _inlfs[index]._info._inlfId = index;
    return _inlfs[index];
  }

  /*
    引数や名前などは正しい前提のinlineコール。
  */
  Expr _InlineCall( const std::string& fname, std::vector<Expr>&& args )
  {
    auto itr = std::find_if( _inlfs.begin(), _inlfs.end(), [&fname](InlineFunction& f) {  return f._info._name == fname; } );
    assert( itr != _inlfs.end() );
    auto retType = itr->_info._retType.IsGenType() ? args[0].GetType() : itr->_info._retType._type;
    auto call =  new Call( retType, Call::INLINE, std::move(args) );
    call->_inlfId = itr->_info._inlfId;
    return Expr(
      call
    );
  }

  Expr Dot( Expr&& arg1, Expr&& arg2 )
  {
    return _Call( Float(32), Call::DOT, { std::move(arg1), std::move(arg2) } );
  }


  void SetupPredefinedFunctions()
  {
    Push( InlineFunction( "to_ncolor", { ExtendType( ExtendType::NORMAL, U8V4Type() ) }, ExtendType( ExtendType::NORMAL, F32V4Type() ),
          [this](std::vector<Expr>&& args) {
            return _BodyElem(
              {
                _Div(
                  _ToF32( std::move(args[0]) ),
                  Expr( 255.0F )
                )
              }
            );
          }
        )
    );
    Push( InlineFunction( "to_u8color", { ExtendType( ExtendType::NORMAL, F32V4Type() ) }, ExtendType( ExtendType::NORMAL, U8V4Type() ),
          [this](std::vector<Expr>&& args) {
            // u8(clamp(255.0*args[0], 0.0, 255.0))
            return _BodyElem(
              {
                _ToType(
                  UInt(8),
                  _Clamp(
                    _Mul( Expr(255.0F), std::move(args[0]) ),
                    Expr( 0.0F ),
                    Expr( 255.0F)
                  )
                )
              }
            );
          }
        )
    );
    Push( InlineFunction( "gamma2linear", { ExtendType( ExtendType::GEN_TYPE ) }, ExtendType( ExtendType::GEN_TYPE ),
     [this](std::vector<Expr>&& args) {
      auto tmpVar = _Var( args[0].GetType(), UniqueName('v') );

      //  let tmpVar = args[0]
      //  ifel(tmpVar >= 0.04045,
      //      ((tmpVar+0.055)/1.055)^2.4, tmpVar/12.92)
      return _BodyElem(
        {
          _PrivateLet( tmpVar.GetElem()->_name, std::move( args[0] ) ),
          _Ifel(
            _Ge( tmpVar.Clone(), Expr( 0.04045F ) ),
            _Pow(
              _Div(
                _Add(tmpVar.Clone(), Expr( 0.055F )),
                Expr( 1.055F )
              ),
              Expr( 2.4F )
             ),
            _Div( tmpVar.Clone(), Expr( 12.9231F ) )
          )
        }
      );
     }) );

    Push( InlineFunction( "linear2gamma", { ExtendType( ExtendType::GEN_TYPE ) }, ExtendType( ExtendType::GEN_TYPE ),
     [this](std::vector<Expr>&& args) {
      auto tmpVar = _Var( args[0].GetType(), UniqueName('v') );
      /*
        ifel(tmpVar>=0.0031308,
                        1.055*tmpVar^(1.0/2.4)-[0.055, 0.055, 0.055, 0.055],
                        12.92*tmpVar)
      */
      return _BodyElem(
        {
          _PrivateLet( tmpVar.GetElem()->_name, std::move( args[0] ) ),
          _Ifel(
            _Ge( tmpVar.Clone(), Expr( 0.0031308F ) ),
            // 1.055*tmpVar^(1.0/2.4)-[0.055, 0.055, 0.055, 0.055],
            _Sub(
              _Mul(
                Expr( 1.055F ),
                _Pow(
                  tmpVar.Clone(), 
                  _Div( Expr( 1.0F ), Expr( 2.4F ) )
                  )
              ),
              Expr( 0.055F )
            ),
            // 12.92*tmpVar)
            _Mul( tmpVar.Clone(), Expr( 12.9231F ) )
          )
        }
      );
     }) );

    // BGRAのAだけそのまま素通しするgamma2linear。必ず4次元
    Push( InlineFunction( "gamma2linearA", { ExtendType( ExtendType::NORMAL, F32V4Type() ) }, ExtendType( ExtendType::NORMAL, F32V4Type() ),
          [this](std::vector<Expr>&& args) {
            auto tmpVar = _Var( args[0].GetType(), UniqueName('v') );
            /*
              let tmp = gamma2linear(args[0].xyz)
              [*tmp, args[0].w]
            */
            return _BodyElem(
              {
                _PrivateLet( tmpVar.GetElem()->_name, std::move( args[0] ) ),
                _Tuple(
                  {
                    _Spread( _InlineCall("gamma2linear", {_Swizzle(tmpVar.Clone(), {0, 1, 2})} ) ),
                    _Swizzle(tmpVar.Clone(), {3} )
                  }
                )
              }

            );
          }) );
    // linear2gammaのalphaを素通しするバージョン
    Push( InlineFunction( "linear2gammaA", { ExtendType( ExtendType::NORMAL, F32V4Type() ) }, ExtendType( ExtendType::NORMAL, F32V4Type() ),
          [this](std::vector<Expr>&& args) {
            auto tmpVar = _Var( args[0].GetType(), UniqueName('v') );
            /*
              let tmp = linear2gamma(args[0].xyz)
              [*tmp, args[0].w]
            */
            return _BodyElem(
              {
                _PrivateLet( tmpVar.GetElem()->_name, std::move( args[0] ) ),
                _Tuple(
                  {
                    _Spread( _InlineCall("linear2gamma", {_Swizzle(tmpVar.Clone(), {0, 1, 2})} ) ),
                    _Swizzle(tmpVar.Clone(), {3} )
                  }
                )
              }

            );
          }) );
    /*
      linealized bgr (0.0 to 1.0) to CIE XYZ color.
    */
    Push( InlineFunction( "lbgr_to_xyz", { ExtendType( ExtendType::NORMAL, F32V3Type() ) }, ExtendType( ExtendType::NORMAL, F32V3Type() ),
          [this](std::vector<Expr>&& args) {
            auto bgrVar = _Var( args[0].GetType(), UniqueName('v') );
            /*
              let b_x = dot(b_bgr.xyz, [0.1804, 0.3576, 0.4125])
              let b_y = dot(b_bgr.xyz, [0.0722, 0.7152, 0.2127])
              let b_z = dot(b_bgr.xyz, [0.9502, 0.1192, 0.0193])
              [b_x, b_y, b_z]
            */
            std::vector<Expr> xMat { Expr(0.1804F), Expr(0.3576F), Expr(0.4125F) };
            std::vector<Expr> yMat { Expr(0.0722F), Expr(0.7152F), Expr(0.2127F) };
            std::vector<Expr> zMat { Expr(0.9502F), Expr(0.1192F), Expr(0.0193F) };

            return _BodyElem(
              {
                _PrivateLet( bgrVar.GetElem()->_name, std::move( args[0] ) ),
                _Tuple(
                  {
                    Dot( bgrVar.Clone(), _Tuple(std::move(xMat)) ),
                    Dot( bgrVar.Clone(), _Tuple(std::move(yMat)) ),
                    Dot( bgrVar.Clone(), _Tuple(std::move(zMat)) )
                  }
                )
              }
            );
          }) );
    /*
      u8v4 color (non linealized U8BGRA) to CIE XYZ + alpha (0.0 to 1.0)
    */
    Push( InlineFunction( "to_xyza", { ExtendType( ExtendType::NORMAL, U8V4Type() ) }, ExtendType( ExtendType::NORMAL, F32V4Type() ),
          [this](std::vector<Expr>&& args) {
            auto ncol = _Var( F32V4Type(), UniqueName('v') );
            /*
              let ncol = gamma2linearA(to_ncolor(arg0))
              let xyz = lbgr_to_xyz(ncol.xyz)
              [*xyz, ncol.w]
            */
            return _BodyElem(
              {
                _PrivateLet( ncol.GetElem()->_name,
                  _InlineCall( "gamma2linearA", {
                    _InlineCall("to_ncolor", std::move( args ) )
                    })
                  ),
                _Tuple(
                  {
                    _Spread(
                      _InlineCall("lbgr_to_xyz", 
                        { _Swizzle( ncol.Clone(), {0, 1, 2}) }
                      )
                    ),
                    _Swizzle( ncol.Clone(), {3})
                  }
                )
              }
            );
          }) );
    // CIE XYZ to linealized bgr
    Push( InlineFunction( "xyz_to_lbgr", { ExtendType( ExtendType::NORMAL, F32V3Type() ) }, ExtendType( ExtendType::NORMAL, F32V3Type() ),
          [this](std::vector<Expr>&& args) {
            auto xyzVar = _Var( args[0].GetType(), UniqueName('v') );
            /*
              let b = dot(xyz, [0.05564, -0.204, 1.0572])
              let g = dot(xyz, [-0.9693, 1.8760, 0.04156])
              let r = dot(xyz, [3.2405, -1.5371, -0.4985])

              [b, g, r]
            */
            std::vector<Expr> bMat { Expr(0.05564F), Expr(-0.204F), Expr(1.0572F) };
            std::vector<Expr> gMat { Expr(-0.9693F), Expr(1.8760F), Expr(0.04156F) };
            std::vector<Expr> rMat { Expr(3.2405F), Expr(-1.5371F), Expr(-0.4985F) };

            return _BodyElem(
              {
                _PrivateLet( xyzVar.GetElem()->_name, std::move( args[0] ) ),
                _Tuple(
                  {
                    Dot( xyzVar.Clone(), _Tuple(std::move(bMat)) ),
                    Dot( xyzVar.Clone(), _Tuple(std::move(gMat)) ),
                    Dot( xyzVar.Clone(), _Tuple(std::move(rMat)) )
                  }
                )
              }
            );
          }) );
    Push( InlineFunction( "xyza_to_u8color", { ExtendType( ExtendType::NORMAL, F32V4Type() ) }, ExtendType( ExtendType::NORMAL, U8V4Type() ),
          [this](std::vector<Expr>&& args) {
            auto xyzVar = _Var( args[0].GetType(), UniqueName('v') );
            /*
              let xyza = args[0]
              let bgr = linear2gamma(xyz_to_lbgr(xyza.xyz))
              to_u8color([*bgr, xyza.a])
            */
            return _BodyElem(
              {
                _PrivateLet( xyzVar.GetElem()->_name, std::move( args[0] ) ),
                _InlineCall("to_u8color", {
                  _Tuple(
                    {
                      _Spread(
                        _InlineCall("linear2gamma", 
                          {
                            _InlineCall("xyz_to_lbgr", {
                             _Swizzle( xyzVar.Clone(), {0, 1, 2})

                            })
                          }
                        )
                      ),
                      _Swizzle( xyzVar.Clone(), {3})
                    }
                  )

                })
              }
            );
          }) );
    // linearlized BGRA, 0.0 to 1.0.
    Push( InlineFunction( "to_lbgra", { ExtendType( ExtendType::NORMAL, U8V4Type() ) }, ExtendType( ExtendType::NORMAL, F32V4Type() ),
          [this](std::vector<Expr>&& args) {
            /*
              gamma2linearA(to_ncolor(arg0))
            */
            return _BodyElem(
              {
                _InlineCall( "gamma2linearA", {
                  _InlineCall("to_ncolor", std::move( args ) )
                  }),
              }
            );
          }) );
    Push( InlineFunction( "lbgra_to_u8color", { ExtendType( ExtendType::NORMAL, F32V4Type() ) }, ExtendType( ExtendType::NORMAL, U8V4Type() ),
          [this](std::vector<Expr>&& args) {
            /*
              to_u8color(linear2gammaA(args[0]))
            */
            return _BodyElem(
              {
                _InlineCall("to_u8color", {
                    _InlineCall("linear2gammaA", 
                        { std::move( args[0] ) }
                    ),
                  }
                )
              }
            );
          }) );
  }

  InlineFunction& Lookup( size_t inlfIdx )
  {
    assert( inlfIdx < _inlfs.size() );
    return _inlfs[inlfIdx];
  }

};

} ///< mfg_ir_util

#endif

