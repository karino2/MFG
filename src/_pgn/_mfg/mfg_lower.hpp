/* -*- coding: utf-8 -*- マルチバイト */

// Copyright 2026 Kazuma Arino. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#ifndef MFG_LOWER_HPP_
#define MFG_LOWER_HPP_


#include <mfg_pal/pal.hpp>
#include "mfg_elem.hpp"
#include "mfg_ir_util.hpp"
#include <map>

namespace mfg_internal
{
using namespace mfg_pal;
using mfg_ir_util::InlineFuncRegistry;

inline ExprBODY LowerAll( ExprBODY&& blk, InlineFuncRegistry& inlfr );
inline std::vector<Expr> LowerOneLevel( std::vector<Expr>&& stmts, InlineFuncRegistry& inlfr );

using mfg_ir_util::IRBuildDSL;
using mfg_ir_util::TensorIRFactory;

/*
  FはTからstd::vector<T2>を返す。
  結果は1レベルだけflattenされたstd::vector<T2>
*/
template<typename T, typename F>
auto FlatMapFn( std::vector<T>&& src, F fun ) -> std::vector<typename std::remove_reference<decltype( fun(std::move(src[0]))[0] )>::type >
{
  std::vector<typename std::remove_reference<decltype( fun(std::move(src[0]))[0] )>::type> ret;
  for( auto& one : src )
  {
    auto oneRet = fun( std::move(one) );
    AppendTail( ret, std::move(oneRet) );
  }
  return ret;
}



struct CommonLower
{
  Expr _ForStmt( const std::string& name, Expr&& begExpr, Expr&& endExpr, Expr&& body )
  {
    return Expr( new ForStmt( name, std::move(begExpr), std::move(endExpr), std::move(body) ) );
  }

  /*
    srcは0からend-1までstd::moveされる。最後の一つがmoveされないのでsigniatureでうまく表現出来なかった。
  */
  void Append( std::vector<Expr>& dest, std::vector<Expr>& src, size_t end )
  {
    for( auto i : NRange(end) )
    {
      dest.push_back(std::move( src[i] ) );
    }
  }

  /*
    srcをLowerしていき、ReturnExprだけreturnしてそれ以外はdestにAppendTailする。
  */
  Expr LowerAppendReturnExpr( std::vector<Expr>& dest, std::vector<Expr>&& src, InlineFuncRegistry& ifr )
  {
    auto tmp = LowerOneLevel( std::move(src), ifr );
    Append( dest, tmp, tmp.size() - 1 );
    return std::move(tmp.back());
  }

  std::string HoistResultVarName( const std::string &name, size_t tupidx )
  {
    return "rres." + name + "." + std::to_string(tupidx);
  }


  /*
    rsum, ts.sum, reduce関連。
    どれもReduceの一種として統一的に扱う。
  */

  std::string ReduceResultVarName( BlockConsumer &rd, size_t tupidx )
  {
    return HoistResultVarName( rd.Name(), tupidx );
  }

  std::string ReduceBeginVarName( BlockConsumer &rd, size_t dim )
  {
    return "rbegin." + rd.Name() + "." + std::to_string(dim);
  }

  std::string ReduceEndVarName( BlockConsumer &rd, size_t dim )
  {
    return "rend." + rd.Name() + "." + std::to_string(dim);
  }

  ExprT<Variable> GenReduceResultVar( BlockConsumer &rd, size_t tupidx )
  {
    IRBuildDSL d;
    return d._Var( rd.GetReturnType().ElemType( tupidx ),  ReduceResultVarName( rd, tupidx ) );
  }
};

/*
  ExecTensorInitialize (block(BodyElem))
  を
  let bounds
  AllocateStmt
  ForStmt
    body
    Store returnExpr

  に変換する。

*/
struct ExecTensorInitializeLower
{
  ExprT<ExecTensorInitialize> _execTs;
  TensorInfo _tsinfo;
  TensorIRFactory _factory;
  CommonLower _common;
  std::vector<Expr> _results;

  ExecTensorInitializeLower( ExprT<ExecTensorInitialize>&& execTs ) : _execTs( std::move(execTs) ), _tsinfo( _execTs.GetElem()->GetTensorInfo() ), _factory( _tsinfo ) {}

  ExecTensorInitialize* GetExecTS() { return _execTs.GetElem(); }

  std::vector<size_t>& Bounds() { return _execTs.GetElem()->_bounds; }

  /*
    let _extent.r0.0:int32_t = 256;
    let _extent.r0.1:int32_t = 48;
    などを生成。
  */
  void GenBoundLets()
  {
    for (auto i : NRange( Bounds().size() ))
    {
      _results.emplace_back( Let::MakePrivate( _tsinfo.GetExtentName( i ),  (int32_t)Bounds()[i] ) );
    }
  }

  /*
    int32_t _r0[256];

    的なIRを生成。中のサイズはboundsを全てかけ合わせたもの。

    タプルの時は
    int32_t _r0_0[256];
    float _r0_1[256];
    等が生成される。
  */
  void GenAllocateStmt()
  {
    size_t accm = 1;
    for( auto dim : Bounds() )
    {
      accm *= dim;
    }

    AppendTail( _results, _factory.GenAllocateStmt( (int32_t)accm ) );
  }

  std::vector<Expr> GenStore( Expr&& returnExpr )
  {
    auto tselem = GetExecTS()->GetTensor();
    return _factory.GenStore(
              _factory.ToIndexVariables( tselem->_args.cbegin(), tselem->_args.cend() ),
              std::move(returnExpr) );
  }

  /*
    detachされているFunObjのbodyから、

    bodyWOR
    store returnExpr

    を含んだvectorを生成して返す。
  */
  std::vector<Expr> GenInnerBody( ExprBODY&& body, InlineFuncRegistry& ifr )
  {
    auto newBody = LowerAll( std::move(body), ifr );

    auto bodyWOR = newBody.GetElem()->GetExprsWithoutReturn();
    auto returnExpr = newBody.GetElem()->GetReturnExpr();

    std::vector<Expr> innerBodies;
    for( auto one : bodyWOR )
    {
      one->Unchain();
      innerBodies.emplace_back( one );
    }

    returnExpr->Unchain();

    auto store = GenStore( Expr(returnExpr) );

    AppendTail( innerBodies, std::move(store) );
    return innerBodies;
  }

  /*
    以下のようなIRを生成。

    For( _x: min=0, extent=3)
    {
      For(_y: min=0, extent=256)
      {
        body
        store returnExpr
      }
    }
  */
  void GenForStmt( InlineFuncRegistry& ifr )
  {
    auto tselem = GetExecTS()->GetTensor();
    auto& args = tselem->_args;

    auto body = tselem->GetBody();
    body->Unchain();

    // 中から外へ作る。
    auto innerBodiesVec = GenInnerBody( ExprBODY( body ), ifr );
    Expr cur( new BodyElem( std::move(innerBodiesVec) ) );

    for (auto i: RevRange( args.size()))
    {
      cur = _common._ForStmt( args[i].first, (int32_t)0, _tsinfo.GetExtent(i), std::move(cur) );
    }
    _results.push_back( std::move(cur) );
  }

  void Lower( InlineFuncRegistry& ifr )
  {
    GenBoundLets();
    GenAllocateStmt();
    GenForStmt( ifr );
  }

};

/*
  accumulate系列で共通に使えるLower。

  let val = load:ts[(y*input_u8.extent.0)+x]
  ... stmt ...
  // _accm = _accm+_val; 的なもの
  store accm = ...expr...

  を生成する。
*/
struct AccmLower
{
  TensorIRFactory& _factory;
  BlockElem &_block;
  CommonLower& _common;
  std::vector<std::pair<std::string, Type>>& _args;

  AccmLower( TensorIRFactory& factory,  BlockElem &block, CommonLower& common, 
             std::vector<std::pair<std::string, Type>>& args ) : _factory( factory ), _block( block ), _common( common ), _args( args )
  {
  }

  /*
    let val = load:ts[(y*input_u8.extent.0)+x]
    ... stmt ...
    // _accm = _accm+_val; 的なもの
    store accm = ...expr...
  */
  std::vector<Expr> GenBody( InlineFuncRegistry& ifr )
  {
    std::vector<Expr> ret;
    auto &accmPair = _args.back();
    auto &valPair = _args[ _args.size() - 2 ];

    _factory.MayGenLoadAssign( ret, valPair.first,
      _args.cbegin(), _args.cend() - 2 );
      
    auto bodyExprs = _block.GetBody()->DetachExprs();
    auto last = _common.LowerAppendReturnExpr( ret, std::move(bodyExprs), ifr );

    ret.emplace_back( new VarStore( accmPair.first, std::move(last) ) );
    return ret;
  }

};

/*
  dim = 0の時
  ForStmt(y: begin=0, end=12)
    var accm = load:ts[(y*input_u8.extent.0)+0]
    ForStmt(x: begin=1, end=16)
      let val = load:ts[(y*input_u8.extent.0)+x]

      // _accm = _accm+_val; 的なもの
      store accm = ...expr...

      store ts[y*input_u8.extent.0+x] = accm;

  dim = 1の時
  ForStmt(x: begin=0, end=16)
    var accm = load:ts[(0*input_u8.extent.0)+x]
    ForStmt(y: begin=1, end=12)
      let val = load:ts[(y*input_u8.extent.0)+x]

      // _accm = _accm+_val; 的なもの
      store accm = ...expr...

      store ts[y*input_u8.extent.0+x] = accm
*/
struct TransAccmTensorLower : private IRBuildDSL
{
  ExprT<TransformTensor> _trans;
  TransAccmAccessor _accm;
  TensorInfo _tsinfo;
  TensorIRFactory _factory;
  CommonLower _common;
  BlockElem* _block;
  std::vector<std::pair<std::string, Type>>& _args;
  size_t _targetDim;

  TransAccmTensorLower( ExprT<TransformTensor>&& trans ) : _trans( std::move(trans) ), _accm( *_trans.GetElem() ), _tsinfo( _trans.GetElem()->GetTensorInfo() ), _factory( _tsinfo ), _block( _accm.GetBlock() ), _args( _block->_args ), _targetDim( (size_t)_accm.GetTargetDimAsImm() ) {}

  using const_arg_iterator = std::vector<std::pair<std::string, Type>>::const_iterator;

  // tsの添字。StoreStmtで使う
  // valとaccmを除いた引数から作る
  std::pair<const_arg_iterator, const_arg_iterator> IndexRange()
  {
    return std::make_pair( _args.cbegin(), _args.cend() - 2 );
  }

  /*
    let val = load:ts[(y*input_u8.extent.0)+x]

    // _accm = _accm+_val; 的なもの
    store accm = ...expr...

    store ts[y*input_u8.extent.0+x] = accm
  */
  std::vector<Expr> GenInnerBody( InlineFuncRegistry& ifr )
  {
    AccmLower accmLower( _factory, *_block, _common, _args );
    auto ret = accmLower.GenBody( ifr );
    auto &accmPair = _args.back();

    auto range = IndexRange();
    auto indices = _factory.ToIndexVariables( range.first, range.second );

    // tsへのstore
    AppendTail( ret, _factory.GenStore( std::move(indices), _Var( accmPair ) ) );

    return ret;
  }

  /*
    var accm = load:ts[(y*input_u8.extent.0)+0]

    どこに0が来るかはtargetDimで決まる。
  */

  Expr GenAccmVarDef()
  {
    auto &accmPair = _args.back();
    std::vector<Expr> indexArgs;

    for (auto i: NRange( _tsinfo.Dimensions() ))
    {
      if (_targetDim == i)
      {
        indexArgs.push_back( Expr(0) );
      }
      else
      {
        indexArgs.push_back( _Var( _args[i] ) );
      }
    }

    return Expr( new VarLet( accmPair.first, _factory.GenLoad( std::move(indexArgs)) ) );
  }

  /*
    accmの更新と一番内側のtargetDimのForを生成
    Forのbinは1から。(accmの0番目は要素自身なのでスキップ出来る)

    var accm = load:ts[(y*input_u8.extent.0)+0]
    ForStmt(x: begin=1, end=16)
      innerBody
  */
  Expr GenBody( InlineFuncRegistry& ifr )
  {
    std::vector<Expr> ret;

    ret.push_back( GenAccmVarDef() );

    // targetDimのForStmtの生成
    auto innerBody = GenInnerBody( ifr );

    ret.push_back( _common._ForStmt( _args[_targetDim].first, (int32_t)1, _tsinfo.GetExtent( _targetDim ), _BodyElem( std::move(innerBody) ) ) );

    return _BodyElem( std::move(ret) );
  }

  /*
    targetDim以外のFor+Body
  */
  Expr Lower( InlineFuncRegistry& ifr )
  {
    auto cur = GenBody( ifr );

    for (auto i: RevRange( _tsinfo.Dimensions() ))
    {
      if (_targetDim != i)
      {
        cur = _common._ForStmt( _args[i].first, (int32_t)0, _tsinfo.GetExtent(i), std::move(cur) );
      }
    }

    return cur;
  }
};

/*
  reduce<src>.accumulate(dim=0, init=-1) |x, val, accm| {...}  
  のreduceの結果0次元となるケースのlower。

  DefByReduceのLowerと似ているが、
  この場合は生成されるのはローカルテンソルではなく単なる変数。dimは必ず0。
  rnameが_rとすると、hoistでは_r.result.0などの変数に置き換わっているので、
  ここのLowerでは最後に_r.result.0へのletを生成すれば良い。
  だから以下のようになる。

  var accm = initValue;
  ForStmt(x: begin=0, end=src.extent.0)
    let val = load:src[x]
    ...stmt...
    store accm = ...expr...;
  let _r.resut.0 = accm;

*/
struct ReduceToScalarLower : private IRBuildDSL
{
  ExprT<ReduceToScalar> _reduce;
  TensorInfo _srcInfo;
  TensorIRFactory _srcFactory;
  CommonLower _common;
  BlockElem* _block;
  std::vector<std::pair<std::string, Type>>& _args;

  ReduceToScalarLower( ExprT<ReduceToScalar>&& reduce ) : _reduce( std::move(reduce) ),  _srcInfo( _reduce.GetElem()->GetSrcTensorInfo() ), _srcFactory( _srcInfo ), _block( _reduce.GetElem()->GetBlock() ), _args( _block->_args )
  {
    assert( _srcInfo.Dimensions() == 1 );
    if ( 0 != _reduce.GetElem()->GetTargetDimAsImm() )
      throw FmtUserError( ResId::ONE_D_REDUCE_WITH_NONE_ZERO_DIM, _reduce.GetElem()->GetTargetDimAsImm() );
  }

  // ほとんどDefByReduceLowerのGenAccmLoopと同じ。
  std::vector<Expr> Lower( InlineFuncRegistry& ifr )
  {
    std::vector<Expr> ret;
    auto &accmPair = _args.back();

    // var accm = iniValue
    ret.emplace_back( new VarLet( accmPair.first, Expr( _reduce.GetElem()->GetInitValue()->CloneBaseExpr() ) ) );

    AccmLower accmLower( _srcFactory, *_block, _common, _args );

    // For    
    auto body = _BodyElem( accmLower.GenBody( ifr ) );
    ret.push_back(
      _common._ForStmt(
        _args[0].first, 0, _srcInfo.GetExtent( 0 ),
        std::move(body)
      )
    );

    ASSERT_INTERNAL( !accmPair.second.IsTuple(), ResId::GENERAL_NYI );

    ret.emplace_back( new Let( _common.ReduceResultVarName( *_reduce.GetElem(), 0 ), _Var( accmPair ) ) );
    return ret;
  }

};

struct SamplerCallLower : private IRBuildDSL
{
  ExprT<SamplerCall> _scallExpr;
  SamplerElem* _samplerDef;
  const TensorInfo& _tensor;
  CommonLower _common;

  SamplerCallLower( ExprT<SamplerCall>&& reduce ) : _scallExpr( std::move(reduce) ),  _samplerDef( _scallExpr.GetElem()->GetDef() ), _tensor( _samplerDef->_target )
  {
  }

  Expr Extent( size_t dim )
  {
    return _tensor.GetExtent( dim );
  }

  std::string ResVName( size_t tupidx )
  {
    return _common.HoistResultVarName( _scallExpr.GetElem()->Name(), tupidx );
  }

  Expr ExpandAddressModeCall( const SamplerElem* sampDef, Expr&& ix, Expr&& iy )
  {
      switch( sampDef->_addressType )
      {
        case SamplerElem::CLAMP_TO_EDGE:
        {
          return _ClampToEdge( sampDef->_target, std::move(ix), std::move(iy) );
        }
        case SamplerElem::CLAMP_TO_BORDER_VALUE:
        {
          // 多分sampDefのArg0もdetachして良いはずだが念の為Cloneにしておく。
          return _ClampToBorderValue( sampDef->_target, Expr( sampDef->GetEdgeArg( 0 )->CloneBaseExpr() ), std::move(ix), std::move(iy) );
        }
        case SamplerElem::NORMAL_EDGE:
        {
          throw InternalError( "Never reached here." );
        }
      }
  }

  // 1引数と一つに出来そうだが、オーバーロードにしておく。
  Expr ExpandAddressModeCall( const SamplerElem* sampDef, Expr&& ix )
  {
      switch( sampDef->_addressType )
      {
        case SamplerElem::CLAMP_TO_EDGE:
        {
          return _ClampToEdge( sampDef->_target, std::move(ix) );
        }
        case SamplerElem::CLAMP_TO_BORDER_VALUE:
        {
          // 多分sampDefのArg0もdetachして良いはずだが念の為Cloneにしておく。
          return _ClampToBorderValue( sampDef->_target, Expr( sampDef->GetEdgeArg( 0 )->CloneBaseExpr() ), std::move(ix) );
        }
        case SamplerElem::NORMAL_EDGE:
        {
          throw InternalError( "Never reached here." );
        }
      }

  }

  Expr TargetCall( Expr&& ix, Expr&& iy )
  {
    if (_samplerDef->_addressType == SamplerElem::NORMAL_EDGE)
    {
      // load
      return _TsCall( _tensor, { std::move(ix), std::move(iy) } );
    }
    else
    {
      // sampler call
      //
      // coordの処理を解決した残り、つまりaddress modeの処理であるClampToEdgeやClampToBorderValueの処理。
      // SamplerElemはdetachして再利用しても良いのだけれど、
      // 大したことないコストをケチって変なバグ入れたくないのでconst refで。
      return ExpandAddressModeCall( _samplerDef, std::move(ix), std::move(iy) );
    }
  }

  Expr TargetCall( Expr&& ix )
  {
    if (_samplerDef->_addressType == SamplerElem::NORMAL_EDGE)
    {
      // load
      return _TsCall( _tensor, { std::move(ix) } );
    }
    else
    {
      // sampler call
      //
      // coordの処理を解決した残り、つまりaddress modeの処理であるClampToEdgeやClampToBorderValueの処理。
      // SamplerElemはdetachして再利用しても良いのだけれど、
      // 大したことないコストをケチって変なバグ入れたくないのでconst refで。
      return ExpandAddressModeCall( _samplerDef, std::move(ix) );
    }
  }

  /*
    タプルの時:
      let [rres.r0.0, rres.r0.1, rres.r0.2] = rvalue

    タプルでない時:
      let rres.r0.0 = rvalue
  */
  Expr FinalLet( Expr&& rvalue )
  {
    if (rvalue.GetType().IsTuple())
    {
      auto otypes = rvalue.GetType().ExpandNumericTypes();
      std::vector<std::string> resVNames;

      for( auto i : NRange(otypes.size()))
      {
        resVNames.push_back( ResVName(i) );
      }
      return Expr( new TupleLet( std::move(resVNames), std::move(rvalue) ) );
    }
    else
    {
      return _Let( ResVName( 0 ), std::move(rvalue) );
    }

  }

  // 1.0の時にはみ出してしまわないように1.0の時は少し小さい値にする。
  // ifel( abs(expr - 1.0) < 0.00001, 0.99999, expr  )
  Expr CutOff( const Expr& expr )
  {
    return _Ifel( _Lt( _Abs( _Sub( expr.Clone(), 1.0 ) ), _F32(0.000001) ), _F32( 0.999999 ), expr.Clone() );
  }

  std::vector<Expr> _ret;

  // _retにemplaceするものをMのプレフィクスで区別する。
  void MLet( const std::string& name, Expr&& val ) { _ret.emplace_back( new Let( name, std::move(val) ) ); }
  void MTupleLet( const std::vector<std::string>& names, Expr&& val )
  { 
    if (names.size() == 1)
    {
      return MLet( names[0], std::move(val) );
    }
    
    auto tnames = names;
    _ret.emplace_back( new TupleLet( std::move(tnames), std::move(val) ) );
  }

  /*
    fcall( arg0, arg1 )
    ->

    # 1.0の時にはみ出してしまうので特別扱い
    let fx = cutoff(arg0)
    let fy = cutoff(arg1)

    let ix = i32( fx*input_u8.extent(0) )
    let iy = i32( fy*input_u8.extent(1) )
    let [rres.r0.0, rres.r0.1, rres.r0.2] = SamplerCall( ix, iy )

    最後はタプルじゃなければ
    let rres.r0.0 = SamplerCall( ix, iy )
  */
  std::vector<Expr> NCoordNearestLower()
  {
    assert( _samplerDef->_coordType == SamplerElem::NORMALIZED_COORD_NEARESTNEIGHBOR );

    auto args = _scallExpr.GetElem()->GetArgs()->DetachChildren();
    assert( args.size() == 2 );
    std::string s_fx = UniqueName( 't' );
    std::string s_fy = UniqueName( 't' );
    std::string s_ix = UniqueName( 't' );
    std::string s_iy = UniqueName( 't' );

    MLet( s_fx, CutOff( args[0] ) );
    MLet( s_fy, CutOff( args[1] ) );

    MLet( s_ix, _ToI32( _Mul( _VarF32( s_fx ), Extent(0) ) ) );
    MLet( s_iy, _ToI32( _Mul( _VarF32( s_fy ), Extent(1) ) ) );

    auto targetCall = TargetCall( _VarI32( s_ix ), _VarI32( s_iy ) );

    _ret.push_back( FinalLet( std::move(targetCall) ) );
    return _ret;
  }

  /*
    vi0*(1.0-vf)+vi1*vf

    キャストは外でする（mixはネストするので）
  */
  Expr Mix( Expr&& vi0, Expr&& vi1, const std::string& vfname )
  {
    return _Add( _Mul( std::move(vi0), _Sub(1.0, _VarF32(vfname))), _Mul( std::move(vi1), _VarF32(vfname) ) );
  }

  Expr Mix( Type tp, const std::string& viname0, const std::string& viname1, const std::string& vfname )
  {
    return Mix( _Var( tp, viname0 ), _Var( tp, viname1 ), vfname );
  }

  /*
    interpolation用のTargetCall。
    NormalEdgeのときもClampToEdgeとして扱うTargetCall。
  */
  Expr TargetInterpCall( Expr&& ix, Expr&& iy )
  {
    if (_samplerDef->_addressType == SamplerElem::NORMAL_EDGE)
    {
      return _ClampToEdge( _tensor, std::move(ix), std::move(iy) );
    }
    else
    {
      return TargetCall( std::move(ix), std::move(iy) );
    }
  }

  Expr TargetInterpCall( Expr&& ix )
  {
    if (_samplerDef->_addressType == SamplerElem::NORMAL_EDGE)
    {
      return _ClampToEdge( _tensor, std::move(ix) );
    }
    else
    {
      return TargetCall( std::move(ix) );
    }
  }

  std::vector<std::string> MakeVarNames( size_t dim )
  {
    std::vector<std::string> names;
    for (auto i : NRange(dim))
    {
      names.push_back( UniqueName('t') );
    }
    return names;
  }

  /*
    2次元のNormalizedLinear。
    タプルの要素ごとに平均をとる。
    とりあえず4次元のケースで書いておくがタプルの要素数は任意。

    let fx = arg0*(input_u8.extent(0)-1)
    let fy = arg1*(input_u8.extent(1)-1)

    let ix = i32( fx )
    let iy = i32( fy )
    let ix1 = ix+1
    let iy1 = iy+1

    let ratio_x = fx - f32( ix )
    let ratio_y = fy - f32( iy )

    let [v0_00, v1_00, v2_00, v3_00] = TargetInterpCall( ix, iy )
    let [v0_10, v1_10, v2_10, v3_10] = TargetInterpCall( ix1, iy )
    let [v0_01, v1_01, v2_01, v3_01] = TargetInterpCall( ix, iy1 )
    let [v0_11, v1_11, v2_11, v3_11] = TargetInterpCall( ix1, iy1 )

    let rres.r0.0 = mix( mix(v0_00, v0_10, ratio_x), mix(v0_01, v0_11, ratio_x), ratio_y )
    let rres.r0.1 = mix( mix(v1_00, v1_10, ratio_x), mix(v1_01, v1_11, ratio_x), ratio_y )
    let rres.r0.2 = mix( mix(v2_00, v2_10, ratio_x), mix(v2_01, v2_11, ratio_x), ratio_y )
    let rres.r0.3 = mix( mix(v3_00, v3_10, ratio_x), mix(v3_01, v3_11, ratio_x), ratio_y )
  */
  std::vector<Expr> NCoordBilinearLower2D()
  {
    auto args = _scallExpr.GetElem()->GetArgs()->DetachChildren();
    auto& ts = _samplerDef->_target;
    assert( args.size() == 2 );
    auto types = ts._type.ExpandNumericTypes();
    std::string s_fx = UniqueName( 't' );
    std::string s_fy = UniqueName( 't' );
    std::string s_ix = UniqueName( 't' );
    std::string s_iy = UniqueName( 't' );
    std::string s_ix1 = UniqueName( 't' );
    std::string s_iy1 = UniqueName( 't' );
    std::string s_xratio = UniqueName( 't' );
    std::string s_yratio = UniqueName( 't' );

    MLet( s_fx, _Mul( args[0].Clone(), _Sub( Extent(0), 1 ) ) );
    MLet( s_fy, _Mul( args[1].Clone(), _Sub( Extent(1), 1 ) ) );
    MLet( s_ix, _ToI32( _VarF32(s_fx) ) );
    MLet( s_iy, _ToI32( _VarF32(s_fy) ) );

    MLet( s_ix1, _Add( _VarI32(s_ix), 1 ) );
    MLet( s_iy1, _Add( _VarI32(s_iy), 1 ) );

    MLet( s_xratio, _Sub( _VarF32(s_fx), _VarI32(s_ix) ) );
    MLet( s_yratio, _Sub( _VarF32(s_fy), _VarI32(s_iy) ) );

    auto s_var00 = MakeVarNames( types.size() );
    auto s_var10 = MakeVarNames( types.size() );
    auto s_var01 = MakeVarNames( types.size() );
    auto s_var11 = MakeVarNames( types.size() );

    MTupleLet( s_var00, TargetInterpCall( _VarI32(s_ix), _VarI32(s_iy) ) );
    MTupleLet( s_var10, TargetInterpCall( _VarI32(s_ix1), _VarI32(s_iy) ) );
    MTupleLet( s_var01, TargetInterpCall( _VarI32(s_ix), _VarI32(s_iy1) ) );
    MTupleLet( s_var11, TargetInterpCall( _VarI32(s_ix1), _VarI32(s_iy1) ) );

    /*
    tpは以下のどれか。f32, i32, u8, etc.

    let rres.r0.0 = tp( mix( mix(b00, b10, ratio_x), mix(b01, b11, ratio_x), ratio_y ) )
    let rres.r0.1 = tp( mix( mix(g00, g10, ratio_x), mix(g01, g11, ratio_x), ratio_y ) )
    ...
    */
    for( auto i : NRange(types.size()) )
    {
      Type tp = types[i];
      MLet( ResVName( i ), _ToType( tp, Mix(  Mix( tp, s_var00[i], s_var10[i], s_xratio ), Mix( tp, s_var01[i], s_var11[i], s_xratio ), s_yratio ) ) );
    }
    return _ret;
  }

  /*
    1次元のNormalizedLinear。
    タプルの要素ごとに平均をとる。
    とりあえず3次元のケースで書いておくがタプルの要素数は任意。

    let fx = arg0*(input_u8.extent(0)-1)

    let ix = i32( fx )
    let ix1 = ix+1

    let ratio_x = fx - f32( ix )

    let [v0_0, v1_0, v2_0] = TargetInterpCall( ix )
    let [v0_1, v1_1, v2_1] = TargetInterpCall( ix1 )

    let rres.r0.0 = tp(mix(v0_0, v0_1, ratio_x) )
    let rres.r0.1 = tp(mix(v1_0, v1_1, ratio_x) )
    let rres.r0.2 = tp(mix(v2_0, v2_1, ratio_x) )
  */
  std::vector<Expr> NCoordBilinearLower1D()
  {
    auto args = _scallExpr.GetElem()->GetArgs()->DetachChildren();
    auto& ts = _samplerDef->_target;
    assert( args.size() == 1 );
    auto types = ts._type.ExpandNumericTypes();
    std::string s_fx = UniqueName( 't' );
    std::string s_ix = UniqueName( 't' );
    std::string s_ix1 = UniqueName( 't' );
    std::string s_xratio = UniqueName( 't' );

    MLet( s_fx, _Mul( args[0].Clone(), _Sub( Extent(0), 1 ) ) );
    MLet( s_ix, _ToI32( _VarF32(s_fx) ) );

    MLet( s_ix1, _Add( _VarI32(s_ix), 1 ) );

    MLet( s_xratio, _Sub( _VarF32(s_fx), _VarI32(s_ix) ) );

    auto s_var0 = MakeVarNames( types.size() );
    auto s_var1 = MakeVarNames( types.size() );

    MTupleLet( s_var0, TargetInterpCall( _VarI32(s_ix) ) );
    MTupleLet( s_var1, TargetInterpCall( _VarI32(s_ix1) ) );

    /*
    tpは以下のどれか。f32, i32, u8, etc.

    let rres.r0.0 = tp(mix(v0_0, v0_1, ratio_x) )
    let rres.r0.1 = tp(mix(v1_0, v1_1, ratio_x) )
    let rres.r0.2 = tp(mix(v2_0, v2_1, ratio_x) )
    ...
    */
    for( auto i : NRange(types.size()) )
    {
      Type tp = types[i];
      MLet( ResVName( i ), _ToType( tp, Mix( tp, s_var0[i], s_var1[i], s_xratio ) ) );
    }
    return _ret;
  }

  std::vector<Expr> NCoordBilinearLower()
  {
    assert( _samplerDef->_coordType == SamplerElem::NORMALIZED_COORD_BILINEAR );
    auto& tsinfo = _samplerDef->_target;

    if (tsinfo.Dimensions()== 1)
    {
      return NCoordBilinearLower1D();
    }
    else
    {
      assert( tsinfo.Dimensions() == 2 );
      return NCoordBilinearLower2D();
    }
  }

  std::vector<Expr> PixelCoordLower()
  {
    auto args = _scallExpr.GetElem()->GetArgs()->DetachChildren();
    assert( args.size() == 2 );
    auto rval = ExpandAddressModeCall( _samplerDef, std::move(args[0]), std::move(args[1]) );

    return { FinalLet( std::move(rval) ) };
  }

  std::vector<Expr> LowerSampler()
  {
    switch( _samplerDef->_coordType )
    {
      case SamplerElem::NORMALIZED_COORD_NEARESTNEIGHBOR:
        return NCoordNearestLower();
      case SamplerElem::NORMALIZED_COORD_BILINEAR:
        return NCoordBilinearLower();
      case SamplerElem::PIXEL_COORD:
        return PixelCoordLower();
    }
  }

  std::vector<Expr> Lower( InlineFuncRegistry& ifr )
  {
    auto stmts = LowerSampler();
    return LowerOneLevel( std::move(stmts), ifr );
  }
};

struct InlineFuncLower : private IRBuildDSL
{
  std::string _prefix; // ローカル変数に使うprefix。
  ExprT<InlineFuncLet> _inflExpr;
  Call* _infc;
  mfg_ir_util::InlineFunction& _inlf;
  CommonLower _common;

  InlineFuncLower( std::string prefix, ExprT<InlineFuncLet>&& inflExpr, mfg_ir_util::InlineFunction& inlf ) : _prefix( std::move(prefix)+"_" ), _inflExpr( std::move(inflExpr) ),  _infc( _inflExpr.GetElem()->GetInlineFunc() ), _inlf( inlf )
  {
  }

  std::string NewName( const std::string& orgName )
  {
    return _prefix + orgName;
  }

  void ReplaceLet( Let* let, std::map<std::string, std::string>& nnmap )
  {
    auto org = let->_name;
    auto newName = NewName( org );
    nnmap.emplace( org, newName );
    let->_name = newName;
  }

  void ReplaceTupleLet( TupleLet* tlet, std::map<std::string, std::string>& nnmap )
  {
    auto orgs = tlet->_names;
    auto newNames = MapFn( orgs, [this]( const std::string& org ) { return NewName( org ); } );
    for( auto i : NRange(orgs.size()) )
    {
      nnmap.emplace( orgs[i], newNames[i] );
    }
    tlet->_names = newNames;
  }

  void ReplaceVar( Variable* v, std::map<std::string, std::string>& nnmap )
  {
    auto iter = nnmap.find( v->_name );
    if (iter == nnmap.end())
      return;
    
    v->_name = iter->second;
  }

  /*
    _nameのあるelem。ReduceSum, Reduceなど。
  */
  template<typename T>
  void ReplaceName( T* elem )
  {
    auto org = elem->_name;
    auto newName = NewName( org );
    elem->_name = newName;
  }

  void MayReplaceName( IRElem* elem )
  {
    if (auto rsum = elem->As<ReduceSum>())
    {
      ReplaceName( rsum );
    }
    else if (auto red = elem->As<Reduce>())
    {
      ReplaceName( red );
    }
    else if (auto ti = elem->As<TensorIterator>())
    {
      ReplaceName( ti );
    }
    else if (auto sc = elem->As<SamplerCall>())
    {
      // SamlerCallの_nameはlazyに初期化するので
      // replaceする前に初期化する。
      sc->Name();
      ReplaceName( sc );
    }
    // fnの中ではローカルテンソルは禁止なのでTransformTensorなどは無い
  }


  /*
    letにsuffixをつけて、その名前のvarもsuffixについた名前で置き換える
  */
  void AddInlfSuffix( BodyElem* body )
  {
    std::map<std::string, std::string> nameMap;
    body->GetExprs()->ForEach<BaseExprElem>([&nameMap, this]( BaseExprElem* telem ){
      for( auto iter = telem->_node->begin(); iter != telem->_node->end(); iter++ )
      {
        // leadingでletを置き換え、trailingでVarを置き換える
        if ( iter.IsLeading() )
        {
          auto& irelem = iter.GetContent();
          auto let = irelem->As<Let>();
          if (let != nullptr)
          {
            ReplaceLet( let, nameMap );
          }
          auto tlet = irelem->As<TupleLet>();
          if (tlet != nullptr)
          {
            ReplaceTupleLet( tlet, nameMap );
          }
          // VectorLet, VarLetはLowerで作られるものでこの時点ではまだ無い。

          MayReplaceName( irelem.get() );
        }

        if( iter.IsTrailing() )
        {
          auto v = iter.GetContent()->As<Variable>();
          if (v != nullptr)
          {
            ReplaceVar( v, nameMap );
          }
        }
      }
    });
  }

  std::vector<Expr> Lower( InlineFuncRegistry& ifr )
  {
    auto args = _infc->DetachArgs();
    auto body = _inlf._expand( std::move(args) );
    AddInlfSuffix( body.GetElem() );

    auto bexprs = body.GetElem()->DetachExprs();
    // 最後の式をInlineFuncLetの変数名へのletに差し替え
    bexprs.back() = _PrivateLet( _inflExpr.GetElem()->_name, std::move(bexprs.back()) );

    // lowerする。これでletがベクトルだったりしても善きに計らう
    return LowerOneLevel( std::move(bexprs), ifr );
  }
};

/*
  def _r by reduce<src>.accumulate(dim=1, init=-1) |x, y, z, val, accm| {...}
  のLower。

  int32_t src[256*16*3]; の時、以下のようなコードを生成。

  let r.extent.0 = src.extent.0
  let r.extent.1 = src.extent.2
  allocate r:int32_t[r.extent.1*r.extent.0]

  ForStmt(x: begin=0, end=src.extent.0)
    ForStmt(z: begin=0, end=src.extent.2)
      var accm = initValue;
      ForStmt(y: begin=0, end=src.extent.1)
        let val = load:src[((z*src.extent.1)+y)*src.extent.0 + x]
        ...stmt...
        store accm = ...expr...;
      store r[(z*r.extent.0)+x] = accm
*/
struct DefByReduceLower : private IRBuildDSL
{
  ExprT<DefByReduce> _reduce;
  TensorInfo _destInfo;
  TensorInfo _srcInfo;
  TensorIRFactory _srcFactory;
  TensorIRFactory _destFactory;
  CommonLower _common;
  BlockElem* _block;
  std::vector<std::pair<std::string, Type>>& _args;
  size_t _targetDim;
  std::vector<Expr> _results;
  IRBuildDSL _dsl;

  DefByReduceLower( ExprT<DefByReduce>&& reduce ) : _reduce( std::move(reduce) ), _destInfo( _reduce.GetElem()->GetTensorInfo() ), _srcInfo( _reduce.GetElem()->GetSrcTensorInfo() ), _srcFactory( _srcInfo ), _destFactory( _destInfo ), _block( _reduce.GetElem()->GetBlock() ), _args( _block->_args ), _targetDim( (size_t)_reduce.GetElem()->GetTargetDimAsImm() ) {}

  size_t GetSrcExtentDim( size_t destDim ) const
  {
    return _reduce.GetElem()->GetSrcExtentDim( destDim );
  }


  /*
    let r.extent.0 = src.extent.0
    let r.extent.1 = src.extent.2
  */
  void GenBounds()
  {
    for( auto i : NRange( _destInfo.Dimensions() ))
    {
      _results.emplace_back( Let::MakePrivate( _destInfo.GetExtentName(i), _srcInfo.GetExtent( GetSrcExtentDim(i) ) ) );
    }
  }


  /*
    allocate r:int32_t[r.extent.1*r.extent.0]
  */ 
  void GenAllocate()
  {
    Expr cur;
    for( auto i : NRange( _destInfo.Dimensions() ))
    {
      if( i == 0 )
      {
        cur = _destInfo.GetExtent( i );
      }
      else
      {
        cur = _dsl._Mul( std::move(cur),  _destInfo.GetExtent( i ) );
      }
    }
    AppendTail( _results, _destFactory.GenAllocateStmt( std::move(cur) ) );
  }

  /*
    let val = load:src[((z*src.extent.1)+y)*src.extent.0 + x]
    ...stmt...
    store accm = ...expr...;
  */
  std::vector<Expr> GenInnerBody( InlineFuncRegistry& ifr )
  {
    AccmLower accmLower( _srcFactory, *_block, _common, _args );
    return accmLower.GenBody( ifr );
  }

  /*
    var accm = initValue
    ForStmt(y: begin=0, end=src.extent.1)
      InnerBody
    store r[(z*r.extent.0)+x] = accm
  */
  std::vector<Expr> GenAccmLoop( InlineFuncRegistry& ifr )
  {
    std::vector<Expr> ret;

    auto &accmPair = _args.back();

    // var accm = iniValue
    // detachしてもいいはずだが、child取得の順番が変わりそうで怖いのでclone。
    ret.emplace_back( new VarLet( accmPair.first, Expr( _reduce.GetElem()->GetInitValue()->CloneBaseExpr() ) ) );

    // For    
    auto body = _BodyElem( GenInnerBody( ifr ) );
    ret.push_back(
      _common._ForStmt(
        _args[_targetDim].first,
        0, _srcInfo.GetExtent( _targetDim ),
        std::move(body)
      )
    );

    // Store
    std::vector<std::string> varNames;
    for( auto i : NRange( _destInfo.Dimensions() ))
    {
      varNames.push_back( _args[ GetSrcExtentDim(i) ].first );
    }

    AppendTail( ret, 
      _destFactory.GenStore(
        _destFactory.ToIndexVariables( varNames ),
        _Var( accmPair )
      )
    );

    return ret;
  }

  /*
  ForStmt(x: begin=0, end=src.extent.0)
    ForStmt(z: begin=0, end=src.extent.2)
      AccmLoop
  */
  void GenForBlock( InlineFuncRegistry& ifr )
  {
    Expr cur = _BodyElem( GenAccmLoop( ifr ) );
    for( auto i : RevRange( _srcInfo.Dimensions()))
    {
      if (_targetDim != i)
      {
        cur = _common._ForStmt( _args[i].first, 
                                          0, _srcInfo.GetExtent(i),
                                          std::move(cur) );
      }
    }
    _results.push_back( std::move(cur) );
  }

  void Lower( InlineFuncRegistry& ifr )
  {
    GenBounds();
    GenAllocate();
    GenForBlock( ifr );
  }

};

/*
  最終的にはTensorIteratorで統一したいが、まずは一番単純なForEachだけ作ってみる。

  ForStmt(x: begin=0, end=16)
    ForStmt(y: begin=0, end=12)
      let val = load:ts[y*ts.extent.0+x]
      ...body...

*/
struct TensorForEachLower : private IRBuildDSL
{
  ExprT<TensorIterator> _tsForEach;
  TensorInfo _tsinfo;
  TensorIRFactory _factory;
  CommonLower _common;
  BlockElem* _block;
  std::vector<std::pair<std::string, Type>>& _args;

  TensorForEachLower( ExprT<TensorIterator>&& trans ) : _tsForEach( std::move(trans) ), _tsinfo( _tsForEach.GetElem()->GetTensorInfo() ), _factory( _tsinfo ), _block( _tsForEach.GetElem()->GetBlock() ), _args( _block->_args ) {}

  /*
    let val = load:ts[y*ts.extent.0+x]
    ...body...
  */
  std::vector<Expr> GenInnerBody( InlineFuncRegistry& ifr )
  {
    std::vector<Expr> ret;
    auto& valPair = _args.back();

    _factory.MayGenLoadAssign( ret, valPair.first, _args.cbegin(), _args.cend() - 1 );

    auto bodyExprs = _block->GetBody()->DetachExprs();
    bodyExprs = LowerOneLevel( std::move(bodyExprs), ifr );
    AppendTail( ret, std::move(bodyExprs) );

    return ret;
  }

  /*
  ForStmt(x: begin=0, end=16)
    ForStmt(y: begin=0, end=12)
      innerBody
  */
  Expr Lower( InlineFuncRegistry& ifr )
  {
    Expr cur = _BodyElem( GenInnerBody( ifr ) );
    for (auto i : RevRange( _args.size()-1 ))
    {
      cur = _common._ForStmt( _args[i].first, 0, _tsinfo.GetExtent(i), std::move(cur) );
    }
    return cur;
  }
};

/*
  rsumとts.sum、reduceの共通処理。
  rsumの基本的な処理が入っていて、ts.sumやreduceでは共通なものだけ使う感じにしている。
*/
struct ReduceDomainLower : private IRBuildDSL
{
  BlockConsumer& _rd;
  BlockElem* _block;
  std::vector<std::pair<std::string, Type>>& _args;
  std::vector<Expr>& _results;
  CommonLower _common;

  ReduceDomainLower( BlockConsumer& rd, std::vector<Expr>& results ) : _rd( rd ), _block( _rd.GetBlock() ), _args( _block->_args ), _results( results ) {}

  // let rd.begin.0 = 0
  void DefineBeginLet( size_t dim, Expr&& value )
  {
    _results.emplace_back( Let::MakePrivate( _common.ReduceBeginVarName( _rd, dim ), std::move(value) ) );
  }

  // let rd.extent.0 = 20
  void DefineEndLet( size_t dim, Expr&& value )
  {
    _results.emplace_back( Let::MakePrivate( _common.ReduceEndVarName( _rd, dim ), std::move(value) ) );
  }

  /*
    var rd.result.0:float = 0.0
    var rd.result.1:int = 0
    ...
  */
  void GenAccmDef()
  {
    auto otypes = _rd.GetReturnType().ExpandNumericTypes();
    for( auto i : NRange( otypes.size() ) )
    {
      _results.emplace_back( new VarLet( _common.ReduceResultVarName( _rd, i ), MakeZero( otypes[i] ) ) );
    }
  }

  /*
    ...body...
    store rd.result.0 = rd.result.0 + rexpr
  */
  std::vector<Expr> GenInnerBody( InlineFuncRegistry& ifr )
  {
    std::vector<Expr> ret;

    auto exprs = _block->GetBody()->DetachExprs();
    auto rexpr = _common.LowerAppendReturnExpr( ret, std::move(exprs), ifr );

    // store、タプルの場合かそうでないか
    if (rexpr.GetElemType() == IRElemType::TupleElem)
    {
      auto tup = rexpr.As<TupleElem>();
      auto expanded = tup->DetachChildren();
      for( auto tupidx: NRange( expanded.size() ))
      {
        auto rvarName = _common.ReduceResultVarName( _rd, tupidx );
        auto rvar = _Var( expanded[tupidx].GetType(), rvarName );

        // rres.r0.i = rres.r0.i + rexpr[i]
        ret.emplace_back( new VarStore( rvarName, _Add( std::move(rvar), std::move(expanded[tupidx]) )));
      }
    }
    else
    {
      auto rvarName = _common.ReduceResultVarName( _rd, 0 );
      auto rvar = _Var( rexpr.GetType(), rvarName );
      ret.emplace_back( new VarStore( rvarName, _Add( std::move(rvar), std::move(rexpr) )));
    }
    return ret;
  }

  Expr GenForStmt( const std::string& varName, size_t dim, Expr&& body )
  {
    return _common._ForStmt( varName, 
              _Var( Int(32), _common.ReduceBeginVarName( _rd, dim ) ),
              _Var( Int(32), _common.ReduceEndVarName( _rd, dim ) ),
              std::move(body)
            );
  }

  /*
    以下を生成。

    ForStmt(y: begin=rd.begin.1, end=rd.end.1)
      ForStmt(x: begin=rd.begin.0, end=rd.end.0)
        innerBody

    argEndはargs.back()がvalの場合とindexの変数の場合があるのでindexの終わりを指定する。
  */
  void GenForBlock( std::vector<Expr>&& innerBody, size_t argEnd )
  {
    Expr cur = _BodyElem( std::move(innerBody) );
    for( auto i : NRange( argEnd ))
    {
      cur = GenForStmt( _args[i].first, i, std::move(cur) );
    }
    _results.push_back( std::move(cur) );
  }

};

/*
  let rd.begin.0 = 0
  let rd.end.0 = 20
  ...

  var rd.result.0:float = 0.0
  ForStmt(y: begin=rd.begin.1, end=rd.end.1)
    ForStmt(x: begin=rd.begin.0, end=rd.end.0)
      ...body...
      store rd.result.0 = rd.result.0 + rexpr
*/
struct ReduceSumLower
{
  ExprT<ReduceSum> _rsumExpr;
  ReduceSum* _rsum;
  BlockElem* _block;
  std::vector<std::pair<std::string, Type>>& _args;
  CommonLower _common;
  std::vector<Expr> _results;
  ReduceDomainLower _rdlower;

  ReduceSumLower( ExprT<ReduceSum>&& reduce ) : _rsumExpr( std::move(reduce) ), _rsum( _rsumExpr.GetElem() ), _block( _rsumExpr.GetElem()->GetBlock() ), _args( _block->_args ), _rdlower( *_rsum, _results ) {}

  /*
    let rd.begin.0 = 0
    let rd.extent.0 = 20
    ...
  */
  void GenExtents()
  {
    auto ranges = _rsum->GetRanges()->ShallowCopy<RangeElem>();
    for( auto i : NRange( ranges.size() ) )
    {
      _rdlower.DefineBeginLet( i, Expr( ranges[i]->GetBegin()->CloneBaseExpr() ) );
      _rdlower.DefineEndLet( i, Expr( ranges[i]->GetEnd()->CloneBaseExpr() ) );
    }
  }

  /*
    ...body...
    store rd.result.0 = rd.result.0 + rexpr
  */
  std::vector<Expr> GenInnerBody( InlineFuncRegistry& ifr )
  {
    return _rdlower.GenInnerBody( ifr );
  }

  /*
  ForStmt(y: begin=rd.begin.1, end=rd.end.1)
    ForStmt(x: begin=rd.begin.0, end=rd.end.0)
      innerBody
  */
  void GenForBlock( InlineFuncRegistry& ifr )
  {
    _rdlower.GenForBlock( GenInnerBody( ifr ), _args.size() );
  }

  void Lower( InlineFuncRegistry& ifr )
  {
    GenExtents();
    _rdlower.GenAccmDef();
    GenForBlock( ifr );
  }
};

/*
  let rd.begin.0 = 0
  let rd.end.0 = 20
  ...

  var accm = initValue
  ForStmt(x: begin=rd.begin.0, end=rd.end.0)
    ...body...
    store accm = ...expr...
  let rd.result.0 = accm
*/
struct ReduceLower : private IRBuildDSL
{
  ExprT<Reduce> _reduceExpr;
  Reduce* _reduce;
  BlockElem* _block;
  std::vector<std::pair<std::string, Type>>& _args;
  CommonLower _common;
  std::vector<Expr> _results;

  // for文生成周辺以外は使わない。
  ReduceDomainLower _rdlower;

  ReduceLower( ExprT<Reduce>&& reduce ) : _reduceExpr( std::move(reduce) ), _reduce( _reduceExpr.GetElem() ), _block( _reduceExpr.GetElem()->GetBlock() ), _args( _block->_args ), _rdlower( *_reduce, _results ) {}

  /*
    let rd.begin.0 = 0
    let rd.extent.0 = 20
    ...
  */
  void GenExtents()
  {
    auto ranges = _reduce->GetRanges()->ShallowCopy<RangeElem>();
    for( auto i : NRange( ranges.size() ) )
    {
      _rdlower.DefineBeginLet( i, Expr( ranges[i]->GetBegin()->CloneBaseExpr() ) );
      _rdlower.DefineEndLet( i, Expr( ranges[i]->GetEnd()->CloneBaseExpr() ) );
    }
  }

  std::string AccmTupName( const std::string& accmBaseName, size_t tupIndex )
  {
    return Variable::TupleItemName( accmBaseName, tupIndex );
  }

  /*
    var accm.0:float = ...initValue.0...
    var accm.1:int = ...initValue.1...
    ...
  */
  void GenAccmDef()
  {
    auto &accmPair = _args.back();
    auto pInitValue = _reduce->GetInitValue();

    if (pInitValue->_type.IsTuple())
    {
      auto expandedInit = (pInitValue->As<TupleElem>())->ExpandTuple();
      for( auto i : NRange( expandedInit.size() ) )
      {
        _results.emplace_back( new VarLet( AccmTupName( accmPair.first, i ), std::move( expandedInit[i] ) ) );
      }
    }
    else
    {
      _results.emplace_back( new VarLet( accmPair.first, Expr( pInitValue->CloneBaseExpr() ) ) );
    }

  }


  /*
    ...body...
    store accm = rexpr
  */
  std::vector<Expr> GenInnerBody( InlineFuncRegistry& ifr )
  {
    std::vector<Expr> ret;

    auto exprs = _block->GetBody()->DetachExprs();
    auto rexpr = _common.LowerAppendReturnExpr( ret, std::move(exprs), ifr );

    auto &accmPair = _args.back();

    // store、タプルの場合かそうでないか
    if (rexpr.GetElemType() == IRElemType::TupleElem)
    {
      auto tup = rexpr.As<TupleElem>();
      auto expanded = tup->DetachChildren();
      for( auto tupidx: NRange( expanded.size() ))
      {
        // store accm.0 = ...expr.0...
        // store accm.1 = ...expr.1...
        ret.emplace_back( new VarStore( AccmTupName( accmPair.first, tupidx ), std::move(expanded[tupidx]) ));
      }
    }
    else
    {
      ret.emplace_back( new VarStore( accmPair.first, std::move(rexpr) ));
    }
    return ret;
  }

  /*
    ForStmt(x: begin=rd.begin.0, end=rd.end.0)
      innerBody
  */
  void GenForBlock( InlineFuncRegistry& ifr )
  {
    _rdlower.GenForBlock( GenInnerBody( ifr ), _args.size()-1 );
  }

  /*
  let rd.result.0 = accm.0
  let rd.result.1 = accm.1
  */
  void GenLastAssign()
  {
    auto initValue = _reduce->GetInitValue();
    auto &accmPair = _args.back();

    // store、タプルの場合かそうでないか
    if (initValue->_elemType == IRElemType::TupleElem)
    {
      auto tup = initValue->As<TupleElem>();
      auto expanded = tup->DetachChildren();
      for( auto tupidx: NRange( expanded.size() ))
      {
        // let rd.result.0 = accm.0
        // let rd.result.1 = accm.1
        _results.emplace_back( new Let( _common.ReduceResultVarName( *_reduce, tupidx ), _Var( expanded[tupidx].GetType(), AccmTupName( accmPair.first, tupidx )  ) ) );
      }
    }
    else
    {
      // let rd.result.0 = accm
      // 型はこの場合はaccmPairからわかる。
      _results.emplace_back( new Let( _common.ReduceResultVarName( *_reduce, 0 ), _Var( accmPair.second, accmPair.first ) ) );
    }

  }

  void Lower( InlineFuncRegistry& ifr )
  {
    GenExtents();
    GenAccmDef();
    GenForBlock( ifr );
    GenLastAssign();
  }
};


/*
  ReduceSumLowerとの一番の違いはForStmtの直下のvalのload。

  let rd.begin.0 = 0
  let rd.end.0 = ts.extent.0
  ...

  var rd.result.0:float = 0.0
  ForStmt(y: begin=rd.begin.1, end=rd.end.1)
    ForStmt(x: begin=rd.begin.0, end=rd.end.0)
      let val = load:ts(y*..+x)
      ...body...
      store rd.result.0 = rd.result.0 + rexpr
*/
struct TensorSumLower
{
  ExprT<TensorIterator> _tsSumExpr;
  TensorIterator* _tsSum;
  TensorInfo _ts;
  BlockElem* _block;
  std::vector<std::pair<std::string, Type>>& _args;
  TensorIRFactory _tsFactory;
  CommonLower _common;
  std::vector<Expr> _results;
  ReduceDomainLower _rdlower;

  TensorSumLower( ExprT<TensorIterator>&& reduce ) : _tsSumExpr( std::move(reduce) ), _tsSum( _tsSumExpr.GetElem() ), _ts( _tsSum->GetTensorInfo() ), _block( _tsSumExpr.GetElem()->GetBlock() ), _args( _block->_args ), _tsFactory( _ts ), _rdlower( *_tsSum, _results ) {}

  /*
    let rd.begin.0 = 0
    let rd.extent.0 = ts.extent.0
    ...
  */
  void GenExtents()
  {
    for (auto i: NRange( _ts.Dimensions() ))
    {
      _rdlower.DefineBeginLet( i, 0 );
      _rdlower.DefineEndLet( i, _ts.GetExtent(i ) );
    }
  }

  /*
    reduce_sumとの主な違いは先頭のval。


    let val = load:ts...    
    ...body...
    store rd.result.0 = rd.result.0 + rexpr
  */
  std::vector<Expr> GenInnerBody( InlineFuncRegistry& ifr )
  {
    std::vector<Expr> ret;

    _tsFactory.MayGenLoadAssign( ret, _args.back().first, _args.cbegin(), _args.cend() -1 );
    
    auto commonBody = _rdlower.GenInnerBody( ifr );
    AppendTail( ret, std::move(commonBody) );
    return ret;
  }

  /*
  reduce_sumとの違いはargsの何番目まで生成するか、だけ（あとGenInnnderBodyも違う）

  ForStmt(y: begin=rd.begin.1, end=rd.end.1)
    ForStmt(x: begin=rd.begin.0, end=rd.end.0)
      innerBody
  */
  void GenForBlock( InlineFuncRegistry& ifr )
  {
    _rdlower.GenForBlock( GenInnerBody( ifr ), _args.size() - 1 );
  }

  void Lower( InlineFuncRegistry& ifr )
  {
    GenExtents();
    _rdlower.GenAccmDef();
    GenForBlock( ifr );
  }

};

struct Replacer
{
  /*
    iterの指す要素をnewElemに差し替えて、古い要素を返す。
    iterはTrailingになる。
    このメソッド単体でも使える。
  */
  Expr DoReplace( FNode::iterator& iter, Expr&& newElem )
  {
    // unchainはleading必須。どうせ終わったあとtrailingにするので一時的にLeadingにする。
    auto oldNode = iter.ToLeading().Unchain();
    Expr old( (BaseExprElem*)oldNode->_data.get() );
    iter = iter.Chain( newElem.Detach()->EnsureNode() );
    iter.ToTrailing();
    return old;
  }

  /*
    TrailingでreplaceOneを呼んでいき、replaceされていたら置き換える。
    Fはstd::pair<bool, Expr>( IRElem* one )
  */
  template<typename F>
  Expr ReplaceAll( Expr&& root, F replaceOne )
  {
    auto rootNode = root.GetBase()->EnsureNode();
    for (auto iter = rootNode->begin(); iter != rootNode->end(); iter++)
    {
      auto& elem = iter.GetContent();
      auto elemType = elem->_elemType;

      // ブロックの中は別の所で扱うべき
      if (elemType == IRElemType::BlockElem)
      {
        iter.ToTrailing();
        continue;
      }

      // 子供から順番に置き換えていく。
      if (iter.IsTrailing())
      {
        bool replaced;
        Expr newElem;
        std::tie( replaced, newElem ) = replaceOne( elem.get() );
        if (replaced)
        {
          // rootのreplaceは出来ないのでreturnするものが変わる
          if (elem.get() == root.GetBase() )
          {
            return newElem;
          }
          else
          {
            DoReplace( iter, std::move(newElem) );
          }
        }
      }
    }

    return std::move(root);
  }

  /*
    TrailingでreplaceOneを呼んでいき、replaceされていたら置き換える。
    Fはstd::pair<bool, Expr>( IRElem* one ) のシグニチャに相当する関数
  */
  template<typename F>
  std::vector<Expr> ReplaceAll( std::vector<Expr>&& stmts, F replaceOne )
  {
    return MapFn( std::move(stmts), [this, &replaceOne]( Expr&& one ) { return ReplaceAll( std::move(one), replaceOne ); } );
  }
};

/*
  rsumやts.sumなどのVExprのループを、rd.result.0などの変数に置き換えるとともに
  もとのrsumなどを_hoistedに集めていくクラス

  ベクトルを返す関数、例えばnormalizeとかも、

  length(normalize(a))

  となっていたら、

  let tmp = normalize(a)
  length(tmp)

  のようにHoistする。
*/
struct StmtLikeVExprHoister : private IRBuildDSL
{
  std::vector<Expr> _hoisted;
  CommonLower _common;
  Replacer _replacer;
  Expr _root;

  /*
    iterの指す要素をnewElemに差し替えて、古い要素を返す。
    iterはTrailingになる。
  */
  Expr DoReplace( FNode::iterator& iter, Expr&& newElem )
  {
    if (iter.GetContent().get() == _root.GetBase())
    {
      // rootはreplaceせずに_rootを差し替える
      Expr ret( std::move(_root) );
      _root = std::move(newElem);
      return ret;
    }
    else
    {
      return _replacer.DoReplace( iter, std::move(newElem) );
    }
  }

  // [rd.result.0, rd.result.1, ...] を生成
  // タプルじゃない時は rd.result.0などのVariableを返す。
  Expr ExpandToVars( BlockConsumer* rd )
  {
    auto otypes = rd->GetReturnType().ExpandNumericTypes();

    // これでは要素数1のtupleがうまく扱えないが必要になるまで直さない。
    if (otypes.size() == 1)
    {
      return _common.GenReduceResultVar( *rd, 0 );
    }
    else
    {
      std::vector<Expr> items;
      for (auto i : NRange(otypes.size()))
      {
        items.push_back( _common.GenReduceResultVar( *rd, i ) );
      }
      return Expr( new TupleElem( std::move(items) ) );
    }
  }

  void TryReplaceOne( FNode::iterator& iter, IRElem* one )
  {
    auto oneType = one->_elemType;
    if( oneType == IRElemType::ReduceSum )
    {
      auto newElem = ExpandToVars( one->As<ReduceSum>() );
      _hoisted.push_back( DoReplace( iter, std::move(newElem) ) );
    }
    else if(oneType == IRElemType::Reduce)
    {
      auto newElem = ExpandToVars( one->As<Reduce>() );
      _hoisted.push_back( DoReplace( iter, std::move(newElem) ) );
    }
    else if(oneType == IRElemType::ReduceToScalar)
    {
      auto newElem = ExpandToVars( one->As<ReduceToScalar>() );
      _hoisted.push_back( DoReplace( iter, std::move(newElem) ) );
    }
    else if (oneType == IRElemType::TensorIterator && !one->_type.IsVoid() )
    {
      auto newElem = ExpandToVars( one->As<TensorIterator>() );
      _hoisted.push_back( DoReplace( iter, std::move(newElem) ) );
    }
    else if (oneType == IRElemType::IfEl)
    {
      /*
        ifel(rand()>0.5, [0, 0], [1, 2])
        みたいなケースでは、そのままだと以下のように展開されてしまう。
        [rand()>0.5?0:1, rand()>0.5?0:2]
        そこで、condをhoistする必要がある。

        この場合はiterが指すのはselectなのでちょっと特別扱いが必要。
      */
      auto op = one->As<IfEl>();
      if (op->_type.IsTuple())
      {
        auto cond0 = op->GetCondition();

        // 簡単なケースは無視
        if (cond0->IsConst() || cond0->_elemType == IRElemType::Variable)
          return;

        BaseExprElem* cond;
        BaseExprElem* tval;
        BaseExprElem* fval;

        std::tie( cond, tval, fval ) = op->DetachChildren();
        Expr condExpr( cond );
        Expr tvalExpr( tval );
        Expr fvalExpr( fval );

        // condをtmpVarに差し替えて、_hoistedには let tmpVar = condExprを入れる。
        auto tmpVar = _Var( cond->_type, UniqueName('t') );
        _hoisted.push_back( _PrivateLet( tmpVar.GetElem()->_name, std::move(condExpr) ) );

        op->AttachChildren( std::move(tmpVar), std::move(tvalExpr), std::move(fvalExpr) );
      }
    }
    else if (oneType == IRElemType::Call)
    {
      auto op = one->As<Call>();
      if (op->NeedVectorHoist())
      {
        // tmpのVariableのexpandしたものに差し替えて、
        // hoistとしてはそのtmpの名前のletで右辺がCallとなるものにする。
        auto tmpVar = _Var( op->_type, UniqueName('t') );
        auto oldCall = DoReplace( iter, Expr( new TupleElem( tmpVar.GetElem()->ExpandTuple<Expr>() ) ));
        _hoisted.emplace_back( new VectorLet( tmpVar.GetElem()->_name, std::move(oldCall) ) );
      }
      else if(op->IsInlineFunc())
      {
        // inline関数はbodyにstmtがあるかもしれないのでタプルでなくてもいつもhoistしておく。
        auto tmpVarName = UniqueName('t');
        auto tmpVar = _Var( op->_type, tmpVarName );
        auto repElem = op->_type.IsTuple() ? Expr( new TupleElem( tmpVar.GetElem()->ExpandTuple<Expr>() ) ) : std::move(tmpVar); 
        auto oldCall = DoReplace( iter, std::move(repElem) );
        _hoisted.emplace_back( new InlineFuncLet( tmpVarName, std::move(oldCall) ) );
      }
    }
    else if (oneType == IRElemType::TensorCall)
    {
      auto op = one->As<TensorCall>();
      if (op->IsVectorLoad())
      {
        // とりあえずCallと同じように処理しておく。        
        auto tmpVar = _Var( op->_type, UniqueName('t') );
        auto oldLoad = DoReplace( iter, Expr( new TupleElem( tmpVar.GetElem()->ExpandTuple<Expr>() ) ));
        _hoisted.emplace_back( new VectorLet( tmpVar.GetElem()->_name, std::move(oldLoad) ) );
      }
    }
    else if (oneType == IRElemType::SamplerCall)
    {
      auto op = one->As<SamplerCall>();
      auto sampler = op->GetDef();
      // ExpandToVarsとほとんど同じだがBlockConsumerじゃない。
      auto& ts = sampler->_target;

      if (ts.GetType().IsTuple())
      {
        std::vector<Expr> items;
        auto otypes = ts.GetType().ExpandNumericTypes();
        for (auto i : NRange(otypes.size()))
        {
          items.push_back( _Var( Type( otypes[i] ), _common.HoistResultVarName( op->Name(), i ) ) );
        }
        _hoisted.push_back( DoReplace( iter, Expr( new TupleElem( std::move(items) ) ) ) );
      }
      else
      {
        _hoisted.push_back( DoReplace( iter, _Var( ts.GetType(), _common.HoistResultVarName( op->Name(), 0 ) ) ) );
      }
    }
  }


  Expr ReplaceAll( Expr&& one )
  {
    _root = std::move(one);

    // ReplacerのReplaceAllと似ている。
    auto root = _root.GetBase();
    auto rootNode = root->EnsureNode();
    for (auto iter = rootNode->begin(); iter != rootNode->end(); iter++)
    {
      auto& elem = iter.GetContent();
      auto elemType = elem->_elemType;
      if (elemType == IRElemType::BlockElem)
      {
        // Blockの中は「この式」の範囲の外なのでスキップ。
        iter.ToTrailing();
        continue;
      }

      // 子供から順番にHoistしていく。
      if (iter.IsTrailing())
      {
        TryReplaceOne( iter, elem.get() );
      }      
    }

    return std::move(_root);
  }

};

/*
  rsumやts.sumなどの他のExpr内部で使われるVExprをhoistしてループとして先に生成しつつ、
  参照先を rd.result.0などの変数に置き換える
  戻りのvectorの最後の要素がいつも元の式と同じ値の式となる（展開されたTupleElemなど）
*/
inline std::vector<Expr> HoistOneStmtLikeVExpr( Expr&& one )
{
  StmtLikeVExprHoister hoister;
  Expr newOne = hoister.ReplaceAll( std::move(one) );

  if (hoister._hoisted.size() == 0)
    return { std::move(newOne) };
  
  auto& hoisted = hoister._hoisted;
  hoisted.push_back( std::move(newOne) );
  return std::move(hoisted);
}

/*
  タプル型のSelectやVariableをTupleElemに展開し、Spreadも展開する。
  hoistの必要なVExprは既にhoistされてTupleElemになっている前提。
*/
struct TupleSpreadExpander : private IRBuildDSL
{
  Replacer _replacer;

  /*
    factoryは BaseExprElem*(Type, Epxr&& lop, Expr&& rop) の関数
  */
  template<typename F>
  Expr VectorizeBinOpLike( Expr&& lop, Expr&& rop, F factory )
  {
    bool leftTuple = lop.GetType().IsTuple();
    bool rightTuple = rop.GetType().IsTuple();
    bool bothTuple = leftTuple && rightTuple;
    if (bothTuple)
    {
      // [a, b, c]*[d, e, f]のケース

      // [a*d, b*e, c*f] にする。
      assert( lop.GetElemType() == IRElemType::TupleElem && rop.GetElemType() == IRElemType::TupleElem );

      auto litems = lop.As<TupleElem>()->DetachChildren();
      auto ritems = rop.As<TupleElem>()->DetachChildren();
      std::vector<Expr> resItems;

      for( auto i : NRange( litems.size() ) )
      {
        resItems.emplace_back( factory( litems[i].GetType(), std::move(litems[i]), std::move(ritems[i]) ) );
      }
      return Expr( new TupleElem( std::move(resItems) ) );
    }
    else if (leftTuple)
    {
      // [a, b, c]*dのケース。

      assert( lop.GetElemType() == IRElemType::TupleElem );
      auto tup = lop.As<TupleElem>();
      tup->Transform([&rop, &factory]( Expr&& child ) {
        Expr clonedRop = rop.Clone();
        mfg_ir_util::MakeCompatible( child, clonedRop );
        return Expr( factory( child.GetType(), std::move(child), std::move(clonedRop) ) );
      });

      return std::move(lop);
    }
    else
    {
      // a*[b, c, d]のケース
      assert( rightTuple );

      assert( rop.GetElemType() == IRElemType::TupleElem );
      auto tup = rop.As<TupleElem>();
      tup->Transform([&lop, &factory]( Expr&& child ) {
        auto clonedLop = lop.Clone();
        mfg_ir_util::MakeCompatible( clonedLop, child );
        return Expr( factory( clonedLop.GetType(), std::move(clonedLop), std::move(child) ) );
      });

      return std::move(rop);
    }

  }

  Expr VectorizeBinOp( BinOp* op )
  {
    auto opType = op->_opType;
    auto terms = op->DetachChildren();
    Expr lop( terms.first );
    Expr rop( terms.second );

    return VectorizeBinOpLike( std::move(lop), std::move(rop), [opType](Type tp, Expr&& l, Expr&&r ){
      return new BinOp( tp, opType, std::move(l), std::move(r) );
    } );
  }

  void ExpandSpreadArgs( ExprVector* vec )
  {
    auto argNum = vec->CountSize();
    if (argNum == 0)
      return;

    // spreadがあったら展開、それ以外は元のまま。
    std::vector<Expr> newChildren;

    auto oldChildren = vec->DetachChildren();
    for( auto& old: oldChildren )
    {
      if (old.GetElemType() == IRElemType::SpreadElem)
      {
        auto spread = old.As<SpreadElem>();
        // Expandの後なのでTupleElemになっているはず。
        assert( spread->GetTarget()->_elemType == IRElemType::TupleElem );
        auto expanded = spread->GetTarget()->As<TupleElem>()->ExpandTuple();
        AppendTail( newChildren, std::move(expanded) );
      }
      else
      {
        newChildren.push_back( std::move(old) );
      }
    }

    vec->AttachChildren( std::move(newChildren) );
  }


  void ExpandSpread( IRElem* elem )
  {
    auto elemType = elem->_elemType;
    if (elemType == IRElemType::Call)
    {
      ExpandSpreadArgs( elem->As<Call>()->GetArgs() );
    }
    else if (elemType == IRElemType::SamplerCall)
    {
      ExpandSpreadArgs( elem->As<SamplerCall>()->GetArgs() );
    }
    else if (elemType == IRElemType::TensorCall)
    {
      ExpandSpreadArgs( elem->As<TensorCall>()->GetArgs() );
    }
    else if(elemType == IRElemType::TupleElem)
    {
      ExpandSpreadArgs( elem->As<TupleElem>()->GetItems() );
    }
  }

  // Trailingで呼ばれる前提。つまり子供はすべて置き換えが終わった状態で呼ばれる。
  std::pair<bool, Expr> ReplaceOne( IRElem* one )
  {
    ExpandSpread( one );
    if (one->_elemType == IRElemType::IfEl)
    {
      auto op = one->As<IfEl>();
      if (op->_type.IsTuple())
      {
        /*
          IfEl(cond, tval, fval)
          を
          [IfEl(cond, tval[0], fval[0]),
            IfEl(cond, tval[1], fval[1]),
            ...
            ]
            にして返す。
        */
        BaseExprElem* cond;
        BaseExprElem* tval;
        BaseExprElem* fval;

        std::tie( cond, tval, fval ) = op->DetachChildren();
        Expr condExpr( cond );
        Expr tvalExpr( tval );
        Expr fvalExpr( fval );

        // Tralingで呼ばれるので子供はすべてTupleElemになっている。
        assert (tval->_elemType == IRElemType::TupleElem );
        assert (fval->_elemType == IRElemType::TupleElem );
        auto tvalItems = tval->As<TupleElem>()->DetachChildren();
        auto fvalItems = fval->As<TupleElem>()->DetachChildren();

        if (condExpr.GetType().IsTuple())
        {
          /*
          ifel([1, 0, 1], [t1, t2, t3], [f1, f2, f3])型
          */

          assert (cond->_elemType == IRElemType::TupleElem );
          auto cvalItems = cond->As<TupleElem>()->DetachChildren();
          assert( cvalItems.size() == tvalItems.size() );
          std::vector<Expr> tupItems;
          for( auto dim: NRange(tvalItems.size()) )
          {
            tupItems.emplace_back( new IfEl( std::move(cvalItems[dim]), std::move(tvalItems[dim]), std::move(fvalItems[dim])) );
          }
          return std::make_pair( true, Expr( new TupleElem( std::move(tupItems) ) ) );
        }
        else
        {
          /*
          ifel(c1, [t1, t2, t3], [f1, f2, f3])型
          */
          std::vector<Expr> tupItems;
          for( auto dim: NRange(tvalItems.size()) )
          {
            tupItems.emplace_back( new IfEl( condExpr.Clone(), std::move(tvalItems[dim]), std::move(fvalItems[dim])) );
          }
          return std::make_pair( true, Expr( new TupleElem( std::move(tupItems) ) ) );
        }

      } 
    }
    else if(one->_elemType == IRElemType::Call)
    {
      auto op = one->As<Call>();
      if (op->IsExpandAsTuple())
      {
        auto args = op->GetArgs()->DetachChildren();
        assert( args.size() == 1 );
        auto ftype = op->_funcType;
        assert( ftype == Call::VEC2 || ftype == Call::VEC3 || ftype == Call::VEC4 );
        std::vector<Expr> elems;

        size_t num = ftype == Call::VEC2 ? 2 : (ftype == Call::VEC3 ? 3: 4);

        for (auto i : NRange(num))
        {
          elems.push_back( args[0].Clone() );
        }

        return std::make_pair( true, Expr( new TupleElem( std::move(elems) ) ) );
      }
      else if (op->IsVectorized())
      {
        auto ftype = op->_funcType;
        if (ftype == Call::POW)
        {
          // [1, 2, 3]^[4, 5, 6]のようなケース。
          // POWはシンタックス的にはBinOpだがバックエンド的にはCallなので扱いが中途半端。
          // ここではBinOpのように振る舞うべき。
          auto children = op->DetachArgs();

          assert( children.size() == 2 );
          Expr lop( std::move(children[0]) );
          Expr rop( std::move(children[1]) );

          auto ret = VectorizeBinOpLike( std::move(lop), std::move(rop), [ftype](Type tp, Expr&& l, Expr&&r ){
            return new Call( tp, ftype, { std::move(l), std::move(r) } );
          } );

          return std::make_pair( true, std::move(ret) );
        }
        else
        {
          // 通常のCallのvectorizeのケース
          // sin([a, b, c]) を
          // [sin(a), sin(b), sin(c)]にする
          std::vector<Expr> elems;

          ASSERT_INTERNAL( op->GetArgs()->CountSize() == 1, ResId::GENERAL_NYI ); // 引数1以外はNYI。
          auto tuple = op->GetArg( 0 ); 
          assert( tuple->_elemType == IRElemType::TupleElem );

          auto items = tuple->As<TupleElem>()->DetachChildren();
          assert( op->_type.ItemNum() == items.size() );

          for( auto idx : NRange(items.size()) )
          {
            elems.emplace_back( new Call( op->_type.ElemType(idx), ftype, { std::move(items[ idx ]) } ) );
          }

          return std::make_pair( true, Expr( new TupleElem( std::move(elems) ) ) );

        }
      }
      else if (op->ShouldExpandVarArgToBin())
      {
        // max( a, b, c ) などを max( max(a, b), c ) に
        auto ftype = op->_funcType;
        auto retType = op->_type;
        auto args = op->DetachArgs();
        assert( args.size() >= 3 );

        Expr ret( new Call( retType, ftype, { std::move(args[0]), std::move(args[1]) } ) );
        for (auto i : NRange(args.size()-2))
        {
          ret = Expr( new Call( retType, ftype, { std::move(ret), std::move(args[i+2]) } ) );
        }
        return std::make_pair( true, std::move(ret) );
      }
    }
    else if(one->_elemType == IRElemType::BinOp)
    {
      auto op = one->As<BinOp>();
      if (op->IsVectorize())
      {
        return std::make_pair( true, VectorizeBinOp( op ) );
      }
    }
    else if(one->_elemType == IRElemType::TensorCall)
    {
      auto op = one->As<TensorCall>();
      // VectorLoadはすでにhoistされている右辺なので、何もしなくてOK。それ以外を処理。
      if (op->_mtype == TensorCall::LOAD && op->_type.IsTuple() && !op->IsVectorLoad())
      {
        // Tensorがタプルのケース。
        // この場合、 ts(x, y) は、
        // [ts[0](x, y), ts[1](x, y), ts[2](x, y)...]
        // とTensrCallのTUPLE_LOADのTupleElemに展開される。
        //
        // この時点ではspreadが終わってないのでLoadExprには出来ない
        auto args = op->GetArgs()->DetachChildren();
        const auto& tsinfo = op->GetTensorInfo();

        std::vector<Expr> children;
        for( auto tupdim: NRange( tsinfo.ElemNum() ))
        {
          children.emplace_back( new TensorCall( tsinfo.ElemType( tupdim ),  tsinfo, TensorCall::TUPLE_LOAD, op->_isArgVerified, tupdim, CloneExprs( args ) ) );
        }
        return std::make_pair( true, Expr( new TupleElem( std::move(children) ) ) );

      }
      else if(op->_mtype == TensorCall::TO_NCOORD)
      {
        // ncoordの展開はここでやる。 *to_ncoord() のスプレッドが動くように
        auto args = op->GetArgs()->DetachChildren();
        assert( args.size() == 1 && args[0].GetElemType() == IRElemType::TupleElem );

        auto vec2d = args[0].As<TupleElem>()->ExpandTuple();
        assert( vec2d.size() == 2 );
        assert( vec2d[0].GetType().IsInt() && vec2d[1].GetType().IsInt() );

        auto ext0 = op->GetTensorInfo().GetSelfExtent( 0 );
        auto ext1 = op->GetTensorInfo().GetSelfExtent( 1 );

        auto expandedTuple = Expr( new TupleElem( { NormalizeByExtent( std::move(vec2d[0]), std::move(ext0) ), NormalizeByExtent( std::move(vec2d[1]), std::move(ext1) ) } ) );
        return std::make_pair( true, std::move(expandedTuple) );
      }
      else if(op->_mtype == TensorCall::IS_INSIDE)
      {
        // ncoordの展開と同じ場所でやっておく。こちらはベクトルじゃないけれど。
        auto args = op->GetArgs()->DetachChildren();
        auto tsinfo = op->GetTensorInfo();
        assert( args.size() == tsinfo.Dimensions() );

        /*
          0 <= arg0 && arg0 < extent(0)
          && 0 <= arg1 && arg1 < extent(1)
          ...
        */
       Expr bexpr = _IsInside( tsinfo, std::move(args) );
       return std::make_pair( true, std::move(bexpr) );
      }
      return std::make_pair( false, Expr() );
    }
    else if(one->_elemType == IRElemType::SwizzleCall)
    {
      auto op = one->As<SwizzleCall>();
      auto tupItems = op->GetTargetTuple()->As<TupleElem>()->DetachChildren();

      // スカラー
      if( op->_indices.size() == 1 )
      {
        return std::make_pair( true, Expr( std::move(tupItems[ op->_indices[0] ]) ) );
      }
      else
      {
        std::vector<Expr> resItems;

        for( auto idx : op->_indices )
        {
          resItems.emplace_back( tupItems[idx].Clone() );
        }
        return std::make_pair( true, Expr( new TupleElem( std::move(resItems) ) ) );
      }
    }
    else if(one->_elemType == IRElemType::Variable)
    {
      auto op = one->As<Variable>();
      if (op->_type.IsTuple())
      {
        auto expanded = op->ExpandTuple<Expr>();
        return std::make_pair( true, Expr( new TupleElem( std::move(expanded) ) ) );
      }
    }
    return std::make_pair( false, Expr() );
  }

  // extentから1を引いてFloatにキャストして割る
  Expr NormalizeByExtent( Expr&& intA, Expr&& intExtent )
  {
    return _Div( _ToF32( std::move(intA) ), _ToF32( _Sub( std::move(intExtent), 1 ) ) );
  }


  Expr DoAll( Expr&& root )
  {
    return _replacer.ReplaceAll( std::move(root), [this]( IRElem* one ){
      return ReplaceOne( one );
    });
  }
};

inline Expr LowerTransSortOne( TensorInfo& tsinfo, size_t bufIndex )
{
  IRBuildDSL d;
  auto bufName = tsinfo.ElemBufferName( bufIndex );
  ASSERT_INTERNAL( tsinfo.Dimensions() == 1, ResId::GENERAL_NYI ); // 1d以外はNYI
  return Expr( new CallStmt("sort_int32_1d", { d._Var( ObjectType(), bufName ), d._I32(tsinfo.VectorExtent()), tsinfo.GetExtent(0) } ) );
}


inline std::vector<Expr> LowerTransSort( TransformTensor& trans )
{
  auto& tsinfo = trans._tsinfo;
  auto args = trans.GetArgs()->ShallowCopy<BaseExprElem>();

  // argsはdimのみで、
  assert (args.size() == 1 );
  // 今の所1dなのでdim=0のみ。
  ASSERT_INTERNAL( args[0]->_elemType == IRElemType::ImmElem, ResId::SORT_ONLY_SUPPORT_1D_INT );
  ASSERT_INTERNAL( args[0]->As<ImmElem>()->Value<int32_t>() == 0 , ResId::SORT_ONLY_SUPPORT_1D_INT );

  std::vector<Expr> ret;
  for( auto i : NRange( tsinfo.ElemBufferNum() ))
  {
    // 今の所sortはint以外NYI
    ASSERT_INTERNAL( tsinfo.ElemType(i).IsInt(), ResId::SORT_ONLY_SUPPORT_1D_INT );
    ret.push_back( LowerTransSortOne( tsinfo, i ) );
  }
  return ret;
}

/*
 let [a, b, c] = d
 を
 let a = d.0
 let b = d.1
 let c = d.2

 的なものに展開する。dはTupleElemでd.0等は各要素
*/
struct TupleLetLower : private IRBuildDSL
{
  ExprT<TupleLet> _tupleLet;
  CommonLower _common;

  TupleLetLower( ExprT<TupleLet>&& tupleLet ) : _tupleLet( std::move(tupleLet) ) {}

  std::vector<ExprLET> ExpandTupleLet( const std::vector<std::string>& lefts, Expr&& rexpr )
  {
    assert( rexpr.GetElemType() == IRElemType::TupleElem );
    auto tuples = rexpr.As<TupleElem>()->DetachChildren();

    std::vector<ExprLET> ret;
    for (auto i : NRange(tuples.size()))
    {
      if (lefts[i] == "_")
        continue;
      
      ret.push_back(
          _Let(
            lefts[i],
            std::move( tuples[i] )
          )
      );
    }
    return ret;
  }

  std::vector<Expr> Lower( InlineFuncRegistry& ifr )
  {
    std::vector<Expr> ret;
    auto rexprs = LowerOneLevel( { _tupleLet.GetElem()->DetachValue() }, ifr );
    _common.Append( ret, rexprs, rexprs.size() - 1 );

    auto expandedLets = ExpandTupleLet( _tupleLet.GetElem()->_names, std::move( rexprs.back() ) );
    AppendTail( ret, std::move(expandedLets) );
    return ret;
  }
};

struct Lower : private IRBuildDSL
{
  std::vector<Expr> HoistStmtLikeVExpr( std::vector<Expr>&& stmts )
  {
    return FlatMapFn( std::move(stmts), []( Expr&& one ) { return HoistOneStmtLikeVExpr( std::move(one) ); } );
  }

  std::vector<Expr> ExpandTupleAndSpread( std::vector<Expr>&& stmts )
  {
    TupleSpreadExpander tupleExpander;
    return MapFn( std::move(stmts), [&tupleExpander]( Expr&& one ) { return tupleExpander.DoAll( std::move(one) ); } );
  }

  std::vector<Expr> InlineSimpleCall( std::vector<Expr>&& stmts )
  {
    Replacer replacer;

    return replacer.ReplaceAll( std::move(stmts), [this]( IRElem* elem ) {
      auto elemType = elem->_elemType;
      if(elemType == IRElemType::TensorCall)
      {
        auto op = elem->As<TensorCall>();
        auto args = op->GetArgs()->DetachChildren();

        switch( op->_mtype )
        {
          case TensorCall::LOAD:
          case TensorCall::TUPLE_LOAD:
          {
            // ここまで来るとすでにタプルは展開されているのでLOADの時はタプルでは無い。どちらにせよtupleIdxのElemBufferNameで良い。
            const auto& tsinfo = op->GetTensorInfo();
            TensorIRFactory factory( tsinfo );
            return std::make_pair( true, factory.GenTupleLoad( op->_type, op->_isArgVerified, op->_tupleIdx, std::move(args) ) );
          }
          case TensorCall::IS_INSIDE:
          case TensorCall::TO_NCOORD:
          {
            // 既にTupleSpreadExpanderで展開されているはずなのでここには来ない
            assert(false);
            throw InternalError("Never reached here");
          }
        }
      }
      return std::make_pair( false, Expr() );
    });

  }


  /*
    Stmtまたは最後のReturnExprのvectorを渡して、内部のVExprの必要なLowerを行う。
    具体的には以下を行う。

    1. Hoistが必要なものはHoist
    2. タプル型のVariableやSelectなどをTupleElemに展開、spreadも展開
    3. サンプラーなどのマクロ的なCallを展開

    2を行ったあとは、このレベルのStmtのタプルはすべてTupleElemな事が期待出来る。
    なお、BlockElemの中には入っていかない。あくまでこのレベル内だけのVExprを置き換える。
  */
  std::vector<Expr> LowerVExpr( std::vector<Expr>&& stmts )
  {
    auto tmpRet = HoistStmtLikeVExpr( std::move(stmts) );
    tmpRet = ExpandTupleAndSpread( std::move(tmpRet) );
    tmpRet = InlineSimpleCall( std::move(tmpRet) );
    return tmpRet;
  }

  bool IsVectorValue( const BaseExprElem* valElem)
  {
    if (valElem->_elemType == IRElemType::Call && valElem->As<Call>()->NeedVectorHoist())
      return true;
    if (valElem->_elemType == IRElemType::LoadExpr && valElem->As<LoadExpr>()->IsVector())
      return true;
    return false;
  }  

  /*
    Stmt的な式を一つLowerする。
    内部のVExprのLowerは既に終わっているという前提で、自身のStmtだけを処理する。
  */
  std::vector<Expr> LowerOneStmt( Expr&& one, InlineFuncRegistry& ifr )
  {
    auto elemType = one.GetElemType();
    if (elemType == IRElemType::ExecTensorInitialize)
    {
      ExecTensorInitializeLower lower( ExprT<ExecTensorInitialize>( one.Detach()->As<ExecTensorInitialize>() ) );
      lower.Lower( ifr );
      return lower._results;
    }
    else if (elemType == IRElemType::TransformTensor)
    {
      auto trans = one.As<TransformTensor>();
      if (trans->_method == TransformTensor::SORT)
      {
        return LowerTransSort( *trans );
      }
      else
      {
        assert( trans->_method == TransformTensor::ACCM );
        TransAccmTensorLower lower( ExprT<TransformTensor>( one.Detach()->As<TransformTensor>() ) );
        return { lower.Lower( ifr ) };
      }
    }
    else if (elemType == IRElemType::DefByReduce)
    {
      ExprT<DefByReduce> reduceExpr( one.Detach()->As<DefByReduce>() );
      DefByReduceLower lower( std::move(reduceExpr) );
      lower.Lower( ifr );
      return lower._results;
    }
    else if (elemType == IRElemType::TensorIterator)
    {
      if (one.As<TensorIterator>()->_itype == TensorIterator::EXEC_FOREACH)
      {
        TensorForEachLower lower( ExprT<TensorIterator>( one.Detach()->As<TensorIterator>() ) );
        return { lower.Lower( ifr ) };
      }
      else
      {
        assert( one.As<TensorIterator>()->_itype == TensorIterator::REDUCE_SUM );

        TensorSumLower lower( ExprT<TensorIterator>( one.Detach()->As<TensorIterator>() ) );
        lower.Lower( ifr );
        return std::move(lower._results);
      }
    }
    else if (elemType == IRElemType::TupleLet)
    {
      ExprT<TupleLet> tletExpr( one.Detach()->As<TupleLet>() );
      TupleLetLower lower( std::move(tletExpr) );
      return lower.Lower( ifr );
    }
    else if (elemType == IRElemType::Let && one.As<Let>()->GetValueType().IsTuple() )
    {
      // let a = [1, 2]
      // を
      // let a.0 = 1
      // let a.1 = 2
      // に展開。
      // 専用のLowerクラスを作るまでも無いのでここで行ってしまう
      Let* op = one.As<Let>();
      std::vector<Expr> ret;

      auto valElem = op->GetValue();

      assert( valElem->_elemType == IRElemType::TupleElem );
      auto expanded = valElem->As<TupleElem>()->DetachChildren();
      for( auto tupIdx: NRange(expanded.size()) )
      {
        ret.emplace_back( new Let( Variable::TupleItemName( op->_name, tupIdx ),  std::move(expanded[tupIdx]) ) );
      }
      return ret;
    }
    // VExprのHoistしたもの。HoistしたあとはStmtとして扱う
    else if (elemType == IRElemType::ReduceSum)
    {
      ReduceSumLower lower( ExprT<ReduceSum>( one.Detach()->As<ReduceSum>() ) );
      lower.Lower( ifr );
      return std::move(lower._results);
    }
    else if (elemType == IRElemType::Reduce)
    {
      ReduceLower lower( ExprT<Reduce>( one.Detach()->As<Reduce>() ) );
      lower.Lower( ifr );
      return std::move(lower._results);
    }
    else if (elemType == IRElemType::ReduceToScalar)
    {
      ReduceToScalarLower lower( ExprT<ReduceToScalar>( one.Detach()->As<ReduceToScalar>() ) );
      return lower.Lower( ifr );
    }
    else if (elemType == IRElemType::SamplerCall)
    {
      SamplerCallLower lower( ExprT<SamplerCall>( one.Detach()->As<SamplerCall>() ) );
      return lower.Lower( ifr );
    }
    else if (elemType == IRElemType::InlineFuncLet)
    {
      auto ifl = one.Detach()->As<InlineFuncLet>();
      InlineFuncLower lower( UniqueName('i'), ExprT<InlineFuncLet>( ifl ), ifr.Lookup( ifl->GetInlfIdx() ) );
      return lower.Lower( ifr );
    }
    // VExprのHoistしたもの終わり。
    else
    {
      return { std::move(one) };
    }
  }

  std::vector<Expr> LowerStmts( std::vector<Expr>&& stmts, InlineFuncRegistry& ifr )
  {
    return FlatMapFn( std::move(stmts), [&ifr, this]( Expr&& one ) { return LowerOneStmt( std::move(one), ifr ); } );
  }

  std::vector<Expr> DoAll( std::vector<Expr>&& stmts, InlineFuncRegistry& ifr )
  {
    auto ret = LowerVExpr( std::move(stmts) );
    ret = LowerStmts( std::move(ret), ifr );
    return ret;
  }
};

/*
  BlockElemには入っていかない範囲でLowerを行う。
  ただし、BlockElemを保持しているStmtのLowerの中でさらにこのLowerOneLevelが呼ばれる事で結局すべての子供がLowerされる。
*/
inline std::vector<Expr> LowerOneLevel( std::vector<Expr>&& stmts, InlineFuncRegistry& ifr )
{
  Lower lower;
  return lower.DoAll( std::move(stmts), ifr );
}

/*
  reduce等をForなどに置き換えたり、
  TupleLetを展開したりする。
*/
inline ExprBODY LowerAll( ExprBODY&& blk, InlineFuncRegistry& ifr )
{
  auto bodies = blk.GetElem()->DetachExprs();
  bodies = LowerOneLevel( std::move(bodies), ifr );

  return ExprBODY( new BodyElem( std::move(bodies) ) );
}

}///< mfg_internal

#endif

