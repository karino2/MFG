/* -*- coding: utf-8 -*- マルチバイト */

// Copyright 2026 Kazuma Arino. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#ifndef MFG_RUNTIME_D3D_HPP_
#define MFG_RUNTIME_D3D_HPP_

// Direct Computeのランタイム。Direct3Dという事でd3dと略す。

#include <mfg_pal/pal.hpp>

#include "mfg_elem.hpp"
#include "mfg_printer.hpp"
#include "mfg_tensor.hpp"
#include "mfg_binary.hpp"
#include "mfg_runtime_common.hpp"

#include <string>
#include <sstream>
#include <vector>
#include <numeric>

/*
  HLSLGenerator関連

  TensorなどをHLSLのカーネルのソースコードに変換する。
  単にテキストを生成するだけなので、これ自体はランタイムには依存しない。
*/
namespace mfg_hlsl_converter {
using namespace mfg_internal;
using namespace mfg_binary;
using namespace mfg_runtime_common;
using namespace mfg_clike_generator;

struct ExprToHLSL : public ExprToCLike
{
  std::string ToAsOp( NumericType tp )
  {
    if (tp.IsFloat())
    {
      return "asfloat";
    }
    else if (tp.IsUInt())
    {
      return "asuint";
    }
    else
    {
      assert( tp.IsInt() );
      return "asint";
    }
  }

  ExprToHLSL( std::stringstream& stream ) : ExprToCLike( stream )
  {
    _overwriter = [this]( ExprToCLike&, BaseExprElem* elem )
    {
      auto etype = elem->_elemType;
      switch(etype)
      {
        case IRElemType::CallStmt:
        {
          // callstmt: sort_int32_1d(r0, extent.r0.0)
          auto op = elem->As<CallStmt>();

          // まずはこのケース決め打ち
          assert( op->_name == "sort_int32_1d" );
          auto args = op->GetArgs()->ShallowCopy<BaseExprElem>();

          assert( args.size() == 3 );
          assert( args[0]->_elemType == IRElemType::Variable && args[1]->_elemType == IRElemType::ImmElem && args[2]->_elemType == IRElemType::Variable );
          auto bufName = _nconv.Ident( args[0]->As<Variable>()->_name );
          int vecExtent = args[1]->AsInt();
          auto extentName = _nconv.Ident( args[2]->As<Variable>()->_name );

          // mfg_runtime_metal.hppのsort_int32_1dと同じようなコードをインラインに生成する。
          _stream << "// sort_int32_1d: " << bufName << ", " << vecExtent << ", " << extentName << std::endl;
          _stream << "{" << std::endl;

          // 一回しか実行されないループは怒られるので特別扱い
          if (vecExtent == 1)
          {
          _stream << "  for( int j = 1; j < " << extentName << R"(; j++ )
  {
    int one = )" << bufName << R"([j];
    int i = j - 1;
    while( i >= 0 && )" << bufName << R"([i] > one)
    {
      )" << bufName << "[i+1] = " << bufName <<"[i];" << R"(
      i--;
    }
    )" << bufName << R"([i+1] = one;
  }
})";
          }
          else
          {
          _stream << "  for( int j = 1; j < " << extentName << "; j++ ) {" << std::endl
  << "for( int v = 0; v < " << vecExtent << "; v++ ) {" << std::endl
  << "int j_index = j* " << vecExtent << " + v;" << std::endl
  << "int one = " << bufName << "[j_index];" << std::endl
  << "int i = j - 1; "
  << "int i0_index = i*" << vecExtent << " + v; "
  << "int i1_index = (i+1)*" << vecExtent << " + v; "
  << R"(
    while( i >= 0 && )" << bufName << R"([i0_index] > one)
    {
   )" << bufName << "[i1_index] = " << bufName <<"[i0_index];" << std::endl
    <<  "i--; "
    << " i0_index = i*" << vecExtent << " + v; i1_index = (i+1)*" << vecExtent << " + v; " << std::endl
    << "   }" << std::endl
    << bufName << "[i1_index] = one;" << R"(
  }
 }
}
)";
          }
          return true;
        }
        case IRElemType::Call:
        {
         // randだけ差し替え
          auto op = elem->As<Call>();
          switch( op->_funcType )
          {
            case Call::RAND_F32:
              _stream << "RandomGenerator_FNext()";
              return true;
            case Call::MIX:
            {
              auto args = op->GetArgs()->ShallowCopy<BaseExprElem>();
              GenMixCall( "lerp", op, args);
              return true;
            }
            case Call::FRACT:
            {
              auto args = op->GetArgs()->ShallowCopy<BaseExprElem>();
              GenFunCall( "frac", args );
              return true;
            }
            default:
              return false;
          }
        }
        case IRElemType::VectorLet:
        {
          auto vlet = elem->As<VectorLet>();
          auto value = vlet->GetValue();

          // このケースはcommonで平気
          if (value->_elemType == IRElemType::Call)
            return false;

          assert( value->_elemType == IRElemType::LoadExpr );
          /*
            letv v = load(c)からv.0, v.1, v.2, v.3などを生成。
            u8v4, u16v4は特別扱い。
          */
          auto loadOp = value->As<LoadExpr>();

          // このケースもcommonで平気
          if (loadOp->IsLocal())
            return false;

          if (loadOp->IsU8V4())
          {
            /*
              u8v4は以下に展開
              const uint v = Buffer0.Load(4*c);
              const uint v.0 = v&0xff;
              const uint v.1 = (v>>8)&0xff;
              const uint v.2 = (v>>16)&0xff;
              const uint v.3 = (v>>24)&0xff;
            */
            _stream << "const uint " << _nconv.Ident( vlet->_name ) << " = "
              << _nconv.Ident( loadOp->_bufName ) << ".Load( 4* ";
            GenVExpr( loadOp->GetIndexArg() );
            _stream << ");" << std::endl;

            for (auto idx: NRange(4))
            {
              _stream << "const uint " << _nconv.Ident( Variable::TupleItemName( vlet->_name, idx ) ) << " = ";
              if (idx == 0)
              {
                _stream << _nconv.Ident( vlet->_name ) << "&0xff;" << std::endl;
              } 
              else
              {
                _stream << "(" << _nconv.Ident( vlet->_name ) << ">>" << 8*idx << ")&0xff;" << std::endl;
              }           
            }
          }
          else if (loadOp->IsU16V4())
          {
            /*
              u16v4は以下に展開
              uint v = Buffer0.Load(4*2*c);
              uint v1 = Buffer0.Load(4*(2*c+1));
              uint v.0 = v&0xffff;
              uint v.1 = (v>>16)&0xffff;
              uint v.2 = v1&0xffff;
              uint v.3 = (v1>>16)&0xff;
            */
            std::string tmpname = vlet->_name + ".t";

            _stream << "const uint " << _nconv.Ident( vlet->_name ) << " = "
              << _nconv.Ident( loadOp->_bufName ) << ".Load( 4*2* ";
            GenVExpr( loadOp->GetIndexArg() );
            _stream << ");" << std::endl;
            _stream << "const uint " << _nconv.Ident( tmpname ) << " = "
              << _nconv.Ident( loadOp->_bufName ) << ".Load( 4*(2* ";
            GenVExpr( loadOp->GetIndexArg() );
            _stream << "+1));" << std::endl;

            // vかv.tかを表す変数。
            for (auto idx1: NRange(2))
            {
              std::string& vname = idx1 == 0 ? vlet->_name : tmpname;
              for (auto idx2: NRange(2))
              {
                size_t idx = idx1*2+idx2;
                _stream << "const uint " << _nconv.Ident( Variable::TupleItemName( vlet->_name, idx ) ) << " = ";
                if (idx2 == 0)
                {
                  _stream << _nconv.Ident( vname ) << "&0xffff;" << std::endl;
                } 
                else
                {
                  _stream << "(" << _nconv.Ident( vname ) << ">>" << 16*idx2 << ")&0xffff;" << std::endl;
                }           

              }
            }
          }
          else
          {
            /*
              普通のベクトルは以下に展開
              const float v.0 = asfloat(Buffer0.Load(4*(4*c)))
              const float v.1 = asfloat(Buffer0.Load(4*(4*c+1)))
              const float v.2 = asfloat(Buffer0.Load(4*(4*c+2)))
              const float v.3 = asfloat(Buffer0.Load(4*(4*c+3)))
            */
            auto vdim = loadOp->_type.VectorExtent();
            auto itemType = loadOp->_type.ElemType( 0 );
            auto tpstr = _nconv.Type( itemType );
            auto asop = ToAsOp( itemType.AsNumeric() );

            for( auto idx: NRange(vdim) )
            {
              _stream << "const " << tpstr << " " << _nconv.Ident( Variable::TupleItemName( vlet->_name, idx ) ) << " = ";
              _stream << asop << "(" << _nconv.Ident( loadOp->_bufName ) << ".Load( 4*(" << vdim << "*";
              GenVExpr( loadOp->GetIndexArg() );
              if (idx != 0)
                _stream << "+" << idx;
              _stream << ")));" << std::endl;
            }
          }
          return true;
        }
        case IRElemType::LoadExpr:
        {
          /*
            だいたい以下のようなコードを生成。
            asint(_input_u8.Load(4*i))
          */
          auto op = elem->As<LoadExpr>();

          // localの場合は通常のC言語の配列アクセスなのでcommonで十分
          if (op->IsLocal())
            return false;

          // ベクトルのケースはVectorLetで処理済みのはず。
          assert( !op->IsVector() );

          auto np = op->_type.AsNumeric();
          // DirectXは4の倍数でしかLoad出来ない。
          assert( np.Bytes() == 4 );
          auto asop = ToAsOp( np );
          _stream << asop << "(" << _nconv.Ident( op -> _bufName );
          _stream << ".Load(" << np.Bytes() << "*";
          GenVExpr( op->GetIndexArg() );
          _stream << "))";
          return true;
        }

        default:
          return false;
      }
    };
  }
};

/*
  HLSLKernelCommonGenerator:
    HLSLTensorGeneratorとHLSLTopLevelBlockGeneratorの共通部分
*/
struct HLSLKernelCommonGenerator
{
  TLRoot& _tle;
  std::stringstream& _stream;
  NameConverter _nconv;

  HLSLKernelCommonGenerator( TLRoot& tle, std::stringstream& stream ) : _tle( tle ), _stream( stream ) {}

  std::vector<GlobalTensorLike*>& GetTensorRefs()
  {
    return _tle.GetTensorRefs();
  }

  std::vector<ReferenceParamInfo>& GetParamRefs()
  {
    return _tle.GetParamRefs();
  }

  void GenInputBuffers()
  {
    _stream << "ByteAddressBuffer paramBuf : register(t0);" << std::endl;

    auto& tensors = GetTensorRefs();
    size_t regIdx = 1;
    for( auto tidx : NRange(tensors.size()) )
    {
      auto ts = tensors[tidx]->GetTensorInfo();
      for( auto i : NRange( ts.ElemBufferNum() ))
      {
        _stream << "ByteAddressBuffer " << _nconv.Ident( ts.ElemBufferName( i ) ) << ": register(t" << regIdx++ <<");" << std::endl;
      }
    }
  }

  void GenFuncPrologue()
  {
    _stream << "[numthreads(1, 1, 1)]" << std::endl;
    _stream << "void CSMain( uint3 DTid : SV_DispatchTHreadID ){" << std::endl;
  }

  /*
    以下のようなコードを生成。インデックスはparamOriginから（tensorのdimなどがカーネルによってはこの前に生成されるので）

    const int _param1 = asint(paramBuf.Load(4*2));
    const float _param2 = asfloat(paramBuf.Load(4*3));
  */
  void GenParamSetup( size_t paramOrigin )
  {
    auto& params = GetParamRefs();
    for( auto pidx : NRange(params.size()) )
    {
      auto& pinfo = params[pidx];
      auto pname = _nconv.Ident( pinfo._name );
      std::string deftype;
      std::string astype;
      switch (pinfo._ptype)
      {
        case ReferenceParamInfo::PARAM_FLOAT32:
          deftype = "float";
          astype = "asfloat";
          break;
        case ReferenceParamInfo::PARAM_INT32:
          deftype = "int";
          astype = "asint";
          break;
        case ReferenceParamInfo::PARAM_UINT32:
          deftype = "uint";
          astype = "asuint";
          break;
      }
      _stream << "  const " << deftype << " " << pname << " = " << astype <<"(paramBuf.Load(4*" << (paramOrigin+pidx) << "));" << std::endl;
    }
  }

  /*
    乱数対応
    
    structにメソッドが定義出来ないのでグローバル変数とフリースタンディング関数にした。
    floatをintmaxで割ったら何故か答えがゼロになるので、intmaxに.0をつけてfloatにした。
    グローバル変数は変更出来ないと怒られたのでstaticをつけてSeedGeneratorで初期化する。
    今回は1カーネル1エントリなので複数のカーネルで同じ値になってしまう問題が無いのでこれで構わないはず。
  */
  void GenRandomPrologue( SeedGenerator& sgen )
  {
    _stream << R"(
static uint g_rand_seed = )" << sgen.Next() << R"(;

uint RandomGenerator_Next() {
  uint x = g_rand_seed;
  x ^= x << 13;
  x ^= x >> 17;
  x ^= x << 5;
  g_rand_seed = x;
  return x;
}
void RandomGenerator_AddNextSeed( uint nextSeed ) {
  RandomGenerator_Next();
  g_rand_seed *= (1+nextSeed);
}

float RandomGenerator_FNext()
{
  uint v = RandomGenerator_Next();
  return ((float)v)/)"
     << std::numeric_limits<uint32_t>::max() << R"(.0;
}

)";
  }
};

// floatの時はasuintでラップする。
struct AsUIntGuard
{
  std::stringstream& _stream;
  bool _isFloat;

  AsUIntGuard( std::stringstream& stream, Type tp ) : _stream( stream ), _isFloat( tp.IsFloat() )
  {
    if (_isFloat)
      _stream << "asuint(";
  }
  ~AsUIntGuard()
  {
    if (_isFloat)
      _stream << ")";
  }
};

/*
  HLSLTensorGenerator:
　  Tensorに対応したHLSLのカーネルを生成する。
    だいたい以下のコードを生成。

ByteAddressBuffer paramBuf : register(t0);
ByteAddressBuffer _input : register(t1);
RWByteAddressBuffer resultBuf0 : register(u0); 

[numthreads(1, 1, 1)]
vois CSMain( uint3 DTid : SV_DispatchTHreadID )
{
  const int _x = (int)DTid.x;
  const int _y = (int)DTid.y;
  const int _f1_out_extent_0 = asint(paramBuf.Load(4*0));
  const int _f1_out_extent_1 = asint(paramBuf.Load(4*1));

  resultBuf0.Store( _y*_f1_out_extent_0 + _x, _x + _y);
}
*/
struct HLSLTensorGenerator
{
  TLTensor& _tensor;
  TensorInfo _tsinfo;
  std::stringstream _stream;
  NameConverter _nconv;
  ExprToHLSL _exprConv;
  HLSLKernelCommonGenerator _kernelCommon;

  HLSLTensorGenerator( TLTensor& tensor ) : _tensor( tensor ), _tsinfo( tensor.GetTensorInfo() ), _exprConv( _stream ), _kernelCommon( _tensor, _stream ) {}

  bool HasRandom()
  {
    RandomFinder finder;
    return finder.HasRandom( _tensor );
  }

  void GenBuffers()
  {
    _kernelCommon.GenInputBuffers();
    for (auto i : NRange( _tsinfo.ElemBufferNum() ))
    {
      _stream << "RWByteAddressBuffer resultBuf" << i <<" : register(u" << i << ");" << std::endl;
    }
  }

  std::string ToString() const { return _stream.str(); }

  std::string LoopVarName( int index ) const
  {
    return _nconv.Ident( _tensor.LoopVarName( index ) );
  }

  std::string GetExtentName( int index ) const
  {
    return _nconv.Ident( _tsinfo.GetExtentName(index) );
  }

  void GenBodyWithoutReturn( BodyElem* vblock )
  {
    ForEachFn( vblock->GetExprsWithoutReturn(), [this](BaseExprElem* s) {
      _stream << "    ";
      _exprConv.GenExpr( s );
    });
  }

  void GenOneAssign( size_t bufIdx, Expr& idxExpr, BaseExprElem* rexpr )
  {
    _stream << "  resultBuf"<< bufIdx << ".Store(";
    _exprConv.GenVExpr( idxExpr.GetBase() );
    _stream << "*4, "; // sizeof(int)を想定
    {
      AsUIntGuard guard( _stream, rexpr->_type );
      _exprConv.GenVExpr( rexpr );
    }
    _stream << ");" << std::endl;

  }

  // TensorのReturnからassignを生成。以下のような文
  // resultBuf.Store( _y*_f1_out_extent_0 + _x, _x + _y);
  void GenFinalAssigns( BaseExprElem* retExpr )
  {
    auto indexVars = _tensor.ArgVars();

    TensorIRFactory factory( _tsinfo );

    // Lowerで無くなったはずの処理。assertしておく。
    assert( retExpr->_elemType != IRElemType::ReduceSum );

    auto idxExpr = factory.NoClampFlattenIdx( std::vector<Expr>( indexVars ) );
    if (_tsinfo.IsVector())
    {
      if (_tsinfo.GetType().IsU8V4())
      {
        /*      
          resultBuf.Store( _y*_f1_out_extent_0 + _x, (r[0]&0xff) |  ((r[1]&0xff)<<8) | ((r[2]&0xff)<<16) | ((r[3]&0xff)<<24) 
        */
        auto tupElem =  retExpr->As<TupleElem>();
        assert( tupElem != nullptr );
        auto tupItems = tupElem->ExpandTuple();
        assert( tupItems.size() == 4 );

        _stream << "  resultBuf0.Store(";
        _exprConv.GenVExpr( idxExpr.GetBase() );
        _stream << "*4, "; // sizeof(int)を想定

        // 最初だけシフト要らないので別扱い
        _stream << "(";
        _exprConv.GenVExpr( tupItems[0].GetBase() );
        _stream << "&0xff)";
        
        for (auto tupIdx : NRange( tupItems.size() - 1))
        {
          _stream << "|((";
          _exprConv.GenVExpr( tupItems[tupIdx+1].GetBase() );
          _stream << "&0xff)<<";
          _stream << (tupIdx+1)*8 << ")";
        }

        _stream << ");" << std::endl;
      }
      else if (_tsinfo.GetType().IsU16V4())
      {
        /*      
          resultBuf.Store( 4*(2*(_y*_f1_out_extent_0 + _x)+0), (r[0]&0xffff) |  ((r[1]&0xffff)<<16) ) 
          resultBuf.Store( 4*(2*(_y*_f1_out_extent_0 + _x)+1), (r[2]&0xffff) | ((r[3]&0xffff)<<16)) 
        */
        auto tupElem =  retExpr->As<TupleElem>();
        assert( tupElem != nullptr );
        auto tupItems = tupElem->ExpandTuple();
        assert( tupItems.size() == 4 );

        // resultBufのindex側のidx
        for( auto didx : NRange(2))
        {
          _stream << "  resultBuf0.Store(4*(2*";
          _exprConv.GenVExpr( idxExpr.GetBase() );
          _stream << "+" << didx << "), ";

          // 最初だけシフト要らない
          _stream << "(";
          _exprConv.GenVExpr( tupItems[didx*2].GetBase() );
          _stream << "&0xffff)";

          // 次はシフト
          _stream << "|((";
          _exprConv.GenVExpr( tupItems[didx*2+1].GetBase() );
          _stream << "&0xffff)<<16)";

          _stream << ");" << std::endl;

        }
      }
      else
      {
        IRBuildDSL d;
        auto mulBase = d._Mul( std::move(idxExpr), _tsinfo.VectorExtent() );
        // u8v4, u16v4以外のベクトルはindexのmuladdで表現
        TupleOrElemEach( retExpr, [this, &d, &mulBase]( size_t idx, BaseExprElem* one ) {
          auto vidxExpr = idx == 0 ? mulBase.Clone() : d._Add( mulBase.Clone(), d._I32(idx) );
          GenOneAssign( 0, vidxExpr, one );
        });
      }
    }
    else
    {
      TupleOrElemEach( retExpr, [this, &idxExpr]( size_t idx, BaseExprElem* one ) {
        GenOneAssign( _tsinfo.BufIndex( idx ), idxExpr, one );
      });

    }
  }

  void GenBody()
  {
    BodyElem* vblock = _tensor.GetBody();
    GenBodyWithoutReturn( vblock );

    GenFinalAssigns( vblock->GetReturnExpr() );
  }

  
  void GenFunction()
  {
    _kernelCommon.GenFuncPrologue();

    assert( _tsinfo.Dimensions() == 1 || _tsinfo.Dimensions() == 2 );
    _stream << "  const int " << LoopVarName( 0 ) << " = (int)DTid.x;" << std::endl;
    if (_tsinfo.Dimensions() == 2)
    {
      _stream << "  const int " << LoopVarName( 1 ) << " = (int)DTid.y;" << std::endl;
    }


    for( auto i : NRange( _tsinfo.Dimensions() ))
    {
      _stream << "  const int " << GetExtentName( i ) << " = asint(paramBuf.Load(4*" << i << "));" << std::endl;
    }

    // paramのセットアップ
    size_t paramOrigin = _tsinfo.Dimensions();
    _kernelCommon.GenParamSetup( paramOrigin );

    if (HasRandom())
    {
      /*
        RandomGenerator_AddNextSeed( DTid.x );
        RandomGenerator_AddNextSeed( DTid.y );
      */
     _stream << "  RandomGenerator_AddNextSeed( DTid.x );" << std::endl;
     if (_tsinfo.Dimensions() == 2)
     {
       _stream << "  RandomGenerator_AddNextSeed( DTid.y );" << std::endl;
     }
    }

    GenBody();

    _stream << "}" << std::endl;
  }

  LibraryIRInfo CollectIRInfo()
  {
    LibraryIRInfoCollector collector;
    collector.Collect( _tensor.GetRootElem() );
    return collector._info;
  }

  void GenAll( SeedGenerator& sgen )
  {
    auto info = CollectIRInfo();
    if (info._useRandom)
      _kernelCommon.GenRandomPrologue( sgen );

    GenBuffers();
    _stream << std::endl;
    GenFunction();
  }
};

/*
  HLSLTopLevelBlockGenerator:
　  TopLevelBlockに対応したHLSLのカーネルを生成する。
    だいたい以下のコードを生成。

ByteAddressBuffer paramBuf : register(t0);
ByteAddressBuffer _input : register(t1);
RWByteAddressBuffer resultBuf : register(u0); 

[numthreads(1, 1, 1)]
vois CSMain( uint3 DTid : SV_DispatchTHreadID )
{
  // ...stmt...

  resultBuf.Store( 4*1, _varname0 );
  resultBuf.Store( 4*2, _varname1 );
  ...
}
*/
struct HLSLTopLevelBlockGenerator
{
  TLBlock& _tlblock;
  std::stringstream _stream;
  NameConverter _nconv;
  ExprToHLSL _exprConv;
  std::vector<std::pair<Type, std::string>> _varsInfo;
  HLSLKernelCommonGenerator _kernelCommon;

  HLSLTopLevelBlockGenerator( TLBlock& tlblock ) : _tlblock( tlblock ), _exprConv( _stream ), _varsInfo( _tlblock.CollectGlobalVarsInfo() ), _kernelCommon( _tlblock, _stream ) {}

  std::string ToString() const { return _stream.str(); }

  void GenBuffers()
  {
    _kernelCommon.GenInputBuffers();
    _stream << "RWByteAddressBuffer resultBuf : register(u0);" << std::endl;
  }

  void GenBody()
  {
    _tlblock.GetIRBody()->GetExprs()->ForEach<BaseExprElem>( [this]( BaseExprElem* child ){
      _exprConv.GenExpr( child );
    });
  }

  void GenLastAssigns()
  {
    for (auto idx : NRange(_varsInfo.size()))
    {
      auto& pair = _varsInfo[idx];
      _stream << "  resultBuf.Store( 4*" << idx << ", ";
      {
        AsUIntGuard guard( _stream, pair.first );
        _stream << _nconv.Ident(pair.second);
      }
      _stream << " );" << std::endl;
    }    
  }

  void GenFunction()
  {
    _kernelCommon.GenFuncPrologue();
    _kernelCommon.GenParamSetup( 0 );

    GenBody();
    GenLastAssigns();

    _stream << "}" << std::endl;
  }

  LibraryIRInfo CollectIRInfo()
  {
    LibraryIRInfoCollector collector;
    collector.Collect( _tlblock.GetRootElem() );
    return collector._info;
  }
  
  void GenAll( SeedGenerator& sgen )
  {
    auto info = CollectIRInfo();
    if (info._useRandom)
      _kernelCommon.GenRandomPrologue( sgen );

    GenBuffers();
    _stream << std::endl;
    GenFunction();
  }
};

} ///< mfg_hlslconverter


/*
  DirectComputeのランタイム関連。
*/


#define NOMINMAX
#include <windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>

#include <cstdint>

// LoadImageをLoadImageAかLoadImageWにするマクロがあるがResourceLoaderと名前がかぶってるのでundefしておく。
// NOMINMAX的なのはなさそう。
#undef LoadImage

/*
  MFGのDirectCompute実行時のランタイムのnamespace。
*/
namespace mfg_runtime_d3d {

using namespace mfg_internal;
using namespace mfg_binary;
using namespace mfg_runtime_common;

extern PFN_D3D11_CREATE_DEVICE g_D3DCreateDevice;
extern pD3DCompile g_D3DCompile;
extern pD3DDisassemble g_D3DDisassemble;

// 型名の11はあとで12に変えるかもしれないので、型が広く使われるものは
// aliasを定義して使う。RAIIラッパの中に閉じるものはaliasを定義してないものもある。
// 関数名やenumはそのまま11のを呼ぶ。
using ID3DDeviceContext = ID3D11DeviceContext;
using ID3DDevice = ID3D11Device;
using ID3DBuffer = ID3D11Buffer;
using ID3DUnorderedAccessView = ID3D11UnorderedAccessView;

struct DllLoader
{
  HMODULE _lib;

  DllLoader( const char* dllname ) : _lib(NULL)
  {
    _lib = LoadLibraryA( dllname );
    if (_lib == NULL)
      throw FmtSystemError( ResId::DLL_LOAD_FAIL, std::string(dllname) );
  }

  ~DllLoader()
  {
    if (_lib != NULL)
    {
      FreeLibrary( _lib );
      _lib = NULL;
    }
  }

  template<typename T>
  T GetFuncPtr( const char* funcName )
  {
    return (T)GetProcAddress( _lib, funcName );
  }
};

template<typename T>
void SafeCOMRelease( T p )
{
  if (p != nullptr)
  {
    p->Release();
  }
}

inline void ThrowIfFail( HRESULT hr, ResId rid )
{
  if ( !SUCCEEDED( hr ) )
  {
    throw FmtSystemError( rid );
  }
}

struct D3DAllDllLoader
{
  DllLoader _d3d;
  DllLoader _d3dcompiler;

  D3DAllDllLoader() : _d3d("d3d11.dll"), _d3dcompiler("d3dcompiler_47.dll")
  {
    g_D3DCreateDevice = _d3d.GetFuncPtr<PFN_D3D11_CREATE_DEVICE>( "D3D11CreateDevice" );
    g_D3DCompile = _d3dcompiler.GetFuncPtr<pD3DCompile>( "D3DCompile" );
    g_D3DDisassemble = _d3dcompiler.GetFuncPtr<pD3DDisassemble>( "D3DDisassemble" );
  }
};

void EnsureD3DAllDllLoaded();
void ReleaseD3DAllDll();

struct D3DBuffer;
struct D3DShader;

/*
  DirectComputeのDeviceとContextの両方を持つ
*/
struct D3DContext
{
  ID3DDeviceContext* _context;
  ID3DDevice* _device;

  D3DContext() : _context(NULL), _device(NULL)
  {
    HRESULT hr = S_OK;

    static const D3D_FEATURE_LEVEL flvl[] = { D3D_FEATURE_LEVEL_11_0 };

    hr = g_D3DCreateDevice( nullptr,
                            D3D_DRIVER_TYPE_HARDWARE,
                            nullptr,
                            D3D11_CREATE_DEVICE_SINGLETHREADED,
                            flvl,
                            1,
                            D3D11_SDK_VERSION,
                            &_device,
                            NULL,
                            &_context );

    ThrowIfFail( hr, ResId::D3D_CONTEXT_CREATION_FAILURE );
  }


  ~D3DContext()
  {
    SafeCOMRelease( _device );
    SafeCOMRelease( _context );
  }

  inline void Dispatch( D3DShader& shader, const std::vector<D3DBuffer*>& rbufs, const std::vector<D3DBuffer*>& rwbufs, size_t groupX, size_t groupY );

  void EndClear( size_t srNum, size_t uaNum )
  {
    _context->CSSetShader( nullptr, nullptr, 0 );

    std::vector<ID3D11UnorderedAccessView*> uaView( uaNum );
    _context->CSSetUnorderedAccessViews( 0, uaNum, uaView.data(), nullptr );

    std::vector<ID3D11ShaderResourceView*> srView( srNum );
    _context->CSSetShaderResources( 0, srNum, srView.data() );

    ID3D11Buffer* ppCBnullptr[1] = { nullptr };
    _context->CSSetConstantBuffers( 0, 1, ppCBnullptr );
  }

  void VerifyDeviceNotRemoved()
  {
    HRESULT hr = _device->GetDeviceRemovedReason();
    if (hr == S_OK)
      return;
    
    switch( hr )
    {
      case DXGI_ERROR_DEVICE_HUNG:
        throw FmtUserError( ResId::D3D_KERNEL_TIMEOUT );
      default:
        throw FmtSystemError( ResId::D3D_DEVICE_REMOVED, hr );
    }
  }

};


struct D3DShader
{
  ID3D11ComputeShader *_shader;
  D3DShader( D3DContext& ctx, const std::string& src, bool debugDisasm = false ) : _shader( NULL )
  {
    const D3D_SHADER_MACRO defines[] = 
    {
        nullptr, nullptr
    };
    ID3DBlob* pErrorBlob = nullptr;
    ID3DBlob* pBlob = nullptr;

    auto hr = g_D3DCompile( src.c_str(), src.size(), NULL, defines, D3D_COMPILE_STANDARD_FILE_INCLUDE, "CSMain", "cs_5_0", D3DCOMPILE_ENABLE_STRICTNESS, 0, &pBlob, &pErrorBlob );

    auto guradE = ScopeGuard([&] { 
      SafeCOMRelease( pErrorBlob );
      SafeCOMRelease( pBlob );
    });
    if ( FAILED(hr))
    {
      std::string errMsg = "Can't compile HLSL: ";
      if (pErrorBlob)
      {
        errMsg += (char*)pErrorBlob->GetBufferPointer();
      }
      // 開発の都合でしばらくエラーメッセージはロギングしておく。
      std::cerr << errMsg << std::endl;

      // どっちのせいか分からないのでUserErrorにして開発者に情報を与える。
      throw UserError( errMsg );
    }

    if (debugDisasm) DisasmDump( pBlob );

    hr = ctx._device->CreateComputeShader( pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &_shader );
    if (FAILED(hr))
    {
      throw FmtSystemError( ResId::SHADER_CREATION_FAIL );
    }
  }

  void DisasmDump( ID3DBlob* pBlob )
  {
    ID3DBlob *pAsmBlob = nullptr;
    auto hr = g_D3DDisassemble(pBlob->GetBufferPointer(),pBlob->GetBufferSize(),
                    D3D_DISASM_ENABLE_INSTRUCTION_NUMBERING,"",&pAsmBlob);
    ThrowIfFail( hr, ResId::D3D_DISASM_FAIL );
    auto guradASM = ScopeGuard([&] { 
      SafeCOMRelease( pAsmBlob );
    });

    std::cerr << "asm:" << std::string( (char*)pAsmBlob->GetBufferPointer(), pAsmBlob->GetBufferSize() ) << std::endl;
  }

  ~D3DShader()
  {
    SafeCOMRelease( _shader );
  }
};

struct D3DBuffer
{
  ID3DBuffer *_buffer;
  size_t _elemSize;
  size_t _byteSize;

  void SetupDesc( D3D11_BUFFER_DESC& desc )
  {
    desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
    desc.ByteWidth = _byteSize;
    desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;
  }

  D3DBuffer( D3DContext& ctx, size_t elemSize, size_t elemNum ) : _buffer(NULL), _elemSize( elemSize ), _byteSize( elemSize*elemNum )
  {
    HRESULT hr = S_OK;

    D3D11_BUFFER_DESC desc = {};
    SetupDesc( desc );

    // D3DBufferはテンソル由来じゃない時は作成時にゼロクリアする。
    std::vector<uint8_t> zeroData( _byteSize, 0 );
    D3D11_SUBRESOURCE_DATA argData;
    argData.pSysMem = zeroData.data();

    hr = ctx._device->CreateBuffer( &desc, &argData, &_buffer );

    // メモリ不足は別のエラーの方がいいかも
    ThrowIfFail( hr, ResId::GPU_BUFFER_ALLOCATION_FAILURE );
  }

  D3DBuffer( D3DContext& ctx, size_t elemSize, size_t elemNum, void *initData ) : _buffer(NULL), _elemSize( elemSize ), _byteSize( elemSize*elemNum )
  {
    HRESULT hr = S_OK;

    /*
      パラメータが無い時のparamBufなどは、サイズ0で作ろうとするとエラーになる。
      他のコード変えたくないので要素1のダミーのバッファとする。
    */
    if ( _byteSize == 0 )
    {
      _byteSize = elemSize*1;
      initData = &_byteSize; // initDataはnullがやってくるのでサイズが1以上ある何かを渡しておく。
    }

    D3D11_BUFFER_DESC desc = {};
    SetupDesc( desc );

    D3D11_SUBRESOURCE_DATA argData;
    argData.pSysMem = initData;
    hr = ctx._device->CreateBuffer( &desc, &argData, &_buffer );

    // メモリ不足は別のエラーの方がいいかも
    ThrowIfFail( hr, ResId::GPU_BUFFER_ALLOCATION_FAILURE );
  }

  D3DBuffer( D3DBuffer&& src ) : _buffer( src._buffer ), _elemSize( src._elemSize), _byteSize( src._byteSize )
  {
    src._buffer = NULL;
  }

  ~D3DBuffer()
  {
    SafeCOMRelease( _buffer );
  }

  /*
    マニュアルのリリース。RAIIとして使う分には呼ぶ必要は無い。
  */
  void ManualRelease()
  { 
    SafeCOMRelease(_buffer );
    _buffer = NULL;
  }

  bool IsReleased() const { return _buffer == NULL; }

  static size_t PixelNumOf( mfg_pal::Image32& src ) { return src.Width()*src.Height(); }


  /*
    destTypeに合わせたD3DBufferを作り、CImageTileXXXの内容をコピーする。

    バッファはいつもuint32の配列として生成とアクセスの所でだけ特別扱いをする。

    destTypeがu8v4 ... バッファは uint32_t*W*H
    destTypeがu16v4 ... バッファは (uint32_t)*2*W*H
  */
  static D3DBuffer FromTile( D3DContext& ctx, NumericType destType, TileReference& src )
  {
    assert( destType == NumericType::UInt(8) || destType == NumericType::UInt(16) );
    size_t unitSize = (destType == NumericType::UInt(8)) ? 1 : 2;
    auto handleSize = unitSize*sizeof(uint32_t)*src.Width()*src.Height();
    mfg_pal::MemHandle* tmphandle = mfg_pal::HandleAlloc( handleSize );
    if (tmphandle == nullptr)
    {
      throw FmtSystemError( ResId::HOST_BUFFER_ALLOCATION_FAILURE );
    }

    auto guard = mfg_pal::ScopeGuard( [&]{ mfg_pal::HandleFree( tmphandle, handleSize ); } );
    mfg_pal::HandleLockGuard<void*> locker( tmphandle );
    auto dest = locker.Ptr();

    CImageTileToArray::FromTile( destType, dest, src );

    // D3Dではバイト単位のアクセスは出来ないので、4バイト単位しかサポートしない。
    // そこで内部でu8v4なものはバッファとしては4バイト単位のバッファとみなす。
    // u16v4は4バイト単位でサイズを倍とする。（ResourceViewの型とかが面倒なので）
    return D3DBuffer( ctx, sizeof(uint32_t), unitSize*src.Width()*src.Height(), dest );
  }

  static D3DBuffer FromConstTensor( D3DContext& ctx, ConstTensor& src )
  {
    return D3DBuffer( ctx, sizeof(uint32_t), src._size, src._data.data() );
  }

  inline void ToTile( D3DContext& ctx, NumericType btype, TileReference& dest );
};

/*
  RWできるバッファのVIew。UnorderedAccessViewのラッパ。
*/
struct D3DBufferView
{
  ID3DUnorderedAccessView *_view;

  D3DBufferView( ID3DDevice* device, D3DBuffer& buffer ) : _view( NULL )
  {
    D3D11_BUFFER_DESC descBuf = {};
    buffer._buffer->GetDesc( &descBuf );
        
    D3D11_UNORDERED_ACCESS_VIEW_DESC desc = {};
    desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
    desc.Buffer.FirstElement = 0;
    desc.Format = DXGI_FORMAT_R32_TYPELESS;
    desc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_RAW;
    desc.Buffer.NumElements = descBuf.ByteWidth / buffer._elemSize; 
    
    auto hr = device->CreateUnorderedAccessView( buffer._buffer, &desc, &_view );
    ThrowIfFail( hr, ResId::D3D_CREATE_BUFFER_VIEW_FAILURE );
  }

  ~D3DBufferView()
  {
    SafeCOMRelease( _view );
  }

  D3DBufferView( D3DBufferView&& src )
  {
    _view = src._view;
    src._view = NULL;
  }
};

/*
  ReadOnlyのバッファのView。SRVのラッパ
*/
struct D3DReadOnlyBufferView
{
  ID3D11ShaderResourceView *_view;

  D3DReadOnlyBufferView( ID3DDevice* device, D3DBuffer& buffer ) : _view( NULL )
  {
    D3D11_BUFFER_DESC descBuf = {};
    buffer._buffer->GetDesc( &descBuf );
        
    D3D11_SHADER_RESOURCE_VIEW_DESC desc = {};
    desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
    desc.BufferEx.FirstElement = 0;

    // 32bit単位じゃないとCreateShaderResourceViewで失敗するので、
    // これ以外は未サポート
    assert( buffer._elemSize == 4);
    desc.Format = DXGI_FORMAT_R32_TYPELESS;
    desc.BufferEx.Flags = D3D11_BUFFEREX_SRV_FLAG_RAW;
    desc.BufferEx.NumElements = descBuf.ByteWidth / buffer._elemSize; 
    
    auto hr = device->CreateShaderResourceView( buffer._buffer, &desc, &_view );
    ThrowIfFail( hr, ResId::D3D_CREATE_RO_BUFFER_VIEW_FAILURE );
  }

  ~D3DReadOnlyBufferView()
  {
    SafeCOMRelease( _view );
  }

  D3DReadOnlyBufferView( D3DReadOnlyBufferView&& src )
  {
    _view = src._view;
    src._view = NULL;
  }
};


void D3DContext::Dispatch( D3DShader& shader, const std::vector<D3DBuffer*>& rbufs, const std::vector<D3DBuffer*>& rwbufs, size_t groupX, size_t groupY )
{
  _context->CSSetShader( shader._shader, nullptr, 0  );

  std::vector<D3DReadOnlyBufferView> rviews;
  std::vector<ID3D11ShaderResourceView*> rViewPtrs;
  for( auto buf : rbufs )
  {
    rviews.emplace_back( _device, *buf );
    rViewPtrs.push_back( rviews.back()._view );
  }
  _context->CSSetShaderResources( 0, rViewPtrs.size(), rViewPtrs.data() );


  std::vector<D3DBufferView> rwviews;
  std::vector<ID3DUnorderedAccessView*> rwViewPtrs;
  for( auto buf : rwbufs )
  {
    rwviews.emplace_back( _device, *buf );
    rwViewPtrs.push_back( rwviews.back()._view );
  }
  _context->CSSetUnorderedAccessViews( 0, rwViewPtrs.size(), rwViewPtrs.data(), nullptr );

  _context->Dispatch( groupX, groupY, 1 );

  // SRViewとUAViewは終わったあとにNULLをセットしないと次のシェーダーからは見えない。
  // いくつセットしたかはここでしか分からないのでDispatchの都度クリアする事にする。
  EndClear( rviews.size(), rwviews.size() );

}

/*
  D3DのBufferをホスト側CPUで読むためのアクセサ
*/
struct D3DCpuBuffer
{
  D3DContext& _ctx;
  ID3DBuffer* _cpuBuf;
  size_t _byteSize;
  void* _bptr;
  BufferAccessor _accessor;

  D3DCpuBuffer( D3DContext& ctx, D3DBuffer& gpuBuf, const std::vector<size_t>& extents ) :  _ctx( ctx ), _cpuBuf( nullptr ), _byteSize( gpuBuf._byteSize ), _bptr( nullptr ), _accessor( nullptr, extents )
  {
    D3D11_BUFFER_DESC desc = {};
    gpuBuf._buffer->GetDesc( &desc );
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    desc.Usage = D3D11_USAGE_STAGING;
    desc.BindFlags = 0;
    desc.MiscFlags = 0;
    auto hr = _ctx._device->CreateBuffer(&desc, nullptr, &_cpuBuf);
    ThrowIfFail( hr, ResId::D3D_CPU_READABLE_BUFFER_ALLOCATION_FAILURE );
    _ctx._context->CopyResource( _cpuBuf, gpuBuf._buffer );

    D3D11_MAPPED_SUBRESOURCE mappedResource; 
    hr = _ctx._context->Map( _cpuBuf, 0, D3D11_MAP_READ, 0, &mappedResource );
    if (hr == DXGI_ERROR_DEVICE_REMOVED)
    {
      hr = _ctx._device->GetDeviceRemovedReason();
      if (hr == DXGI_ERROR_DEVICE_HUNG)
      {
        // このケースは前のDispatchのタイムアウトなのでユーザーエラーとしてメッセージもそのむね伝える。
        throw FmtUserError( ResId::D3D_KERNEL_TIMEOUT );
      }
      throw FmtSystemError( ResId::MAP_BUFFER_FAIL_DEVICE_REMOVED, hr );
    }
    ThrowIfFail( hr, ResId::D3D_CPU_READABLE_BUFFER_MAP_FAILURE );
    _bptr = mappedResource.pData;
    _accessor._buf = _bptr;

    // デバイスによってはDispatchのタイムアウトの後のMapは成功してしまうが
    // Deviceはremoveされている、という状態になる。それが検出できるのはMapの後だけなのでここでチェック。
    _ctx.VerifyDeviceNotRemoved();
  }

  template<typename... ARGS>
  int32_t& Int32( ARGS... args )
  {
    return _accessor.Int32( std::forward<ARGS>(args)... );
  }

  template<typename... ARGS>
  float& Float32( ARGS... args )
  {
    return _accessor.Float32( std::forward<ARGS>(args)... );
  }

  template<typename... ARGS>
  uint32_t& UInt32( ARGS... args )
  {
    return _accessor.UInt32( std::forward<ARGS>(args)... );
  }

  ~D3DCpuBuffer()
  {
    if( _bptr != nullptr)
      _ctx._context->Unmap( _cpuBuf, 0 );
    SafeCOMRelease( _cpuBuf );
  }
};

void D3DBuffer::ToTile( D3DContext& ctx, NumericType btype, TileReference& dest )
{
  D3DCpuBuffer result( ctx, *this, { (size_t)dest.Width(), (size_t)dest.Height() } );
  ArrayToCImageTile::ToTile( dest, btype, result._bptr );
}

// バックエンド関連
using mfg_internal::Type;
using mfg_internal::TLTensor;
namespace conv = mfg_hlsl_converter;

// カーネルに使う為のBuffer関連クラス。
// Metal版のParam32BufferやTensorBufferと似ている。


// TopLevelBlockから値を引き上げてくる為のバッファ。
struct Param32OutBuffer
{
  D3DContext& _ctx;
  size_t _paramNum;
  D3DBuffer _buffer;
  Param32OutBuffer( D3DContext& ctx, size_t paramNum ) : _ctx( ctx ), _paramNum( paramNum ), _buffer( ctx, 4, paramNum ) {}

  void CopyToEnv( const std::vector<std::pair<Type, std::string>>&varsInfo, REnv& env )
  {
    D3DCpuBuffer accessor( _ctx, _buffer, { _paramNum } );
    for (auto i : NRange(varsInfo.size()))
    {
      auto &pair = varsInfo[i];
      auto tp = pair.first;
      auto& name = pair.second;

      if (tp.IsFloat())
      {
        env.Assign( name, { accessor.Float32(i) });
      }
      else if(tp.IsInt())
      {
        env.Assign( name, { accessor.Int32(i) });
      }
      else if(tp.IsUInt())
      {
        env.Assign( name, { accessor.UInt32(i) });
      }
      else
      {
        assert( false );
      }
    }
  }
};

// Tensorに対応するD3DBufferを保持する。
// Metal版のTensorBufferとほとんど同じ。
struct TensorBuffer
{
  std::vector<NumericType> _types;
  std::vector<D3DBuffer> _buffers;

  TensorBuffer( D3DContext& ctx, std::vector<NumericType> types, size_t vectorDim, std::vector<size_t> dims ) : _types( std::move(types) )
  {
    size_t elemNum = std::accumulate( dims.begin(), dims.end(), (size_t)1, std::multiplies<size_t>{} );
    if (_types.size() == 1)
    {
      if (_types[0].Bytes() == 1 || _types[0].Bytes() == 2)
      {
        // D3Dではバッファは4バイト単位のアクセスのみサポート。
        // そこで1バイトと2バイトはu8v4とu16v4のケースだけは4バイト単位のバッファとしてサポートし、
        // それ以外のu8, u16はサポートしない。
        // MFGコンパイル時のエラーにするべきだが、とりあえずはここでassertで落としておく。
        // バッファとしては4バイト単位のバッファとしておくが、コード生成の所で辻褄をあわせる。
        assert( vectorDim == 4 );
        _buffers.emplace_back( ctx, vectorDim, elemNum*_types[0].Bytes() );
      }
      else
      {
        // u8v4, u8v16以外ではベクトルはelemNumとみなす。
        _buffers.emplace_back( ctx, _types[0].Bytes(), elemNum*vectorDim );
      }
    }
    else
    {
      assert(vectorDim == 1 );
      for( const auto& type : _types )
      {
        _buffers.emplace_back( ctx, type.Bytes(), elemNum );
      }
    }
  }

  // Metalと違いD3DBufferは作成時にバッファを渡す必要があるのでTensorInfoだけでは不十分
  TensorBuffer( D3DContext& ctx, OneInputInfo& input, TileReference& tile ) : _types( input.GetTensorInfo().GetElemBufferTypes() )
  {
    assert( _types.size() == 1 ); // CImageTile関連はu8v4かu16v4なのでベクトルだからtypesは一つ。
    _buffers.push_back( D3DBuffer::FromTile( ctx, _types[0], tile ) );
  }

  void ToTile( D3DContext& ctx, TileReference& dest )
  {
    _buffers[0].ToTile( ctx, _types[0], dest );
  }

  TensorBuffer( D3DContext& ctx, ConstTensor& cts ) : _types( { AtomType(cts._type) } )
  {
    _buffers.push_back( D3DBuffer::FromConstTensor( ctx, cts ) );
  }

  /*
    U8V4はバッファとしてはU8のバッファとして扱うので、少しトリッキーな判定が必要。
    それをここで行う。
  */
  static NumericType AtomType( const Type& tp )
  {
    if(tp.IsTuple())
    {
      // assume tp is vector, so return first elem type.
      return tp.ExpandNumericTypes()[0];
    }
    else
    {
      return tp.AsNumeric();
    }
  }


  TensorBuffer( D3DContext& ctx, const TensorInfo& tsinfo, std::vector<size_t> dims ) : TensorBuffer( ctx, tsinfo.GetElemBufferTypes(), tsinfo.VectorExtent(), std::move(dims) ) {}
  TensorBuffer( D3DContext& ctx, TLTensor& ts, std::vector<size_t> dims ) : TensorBuffer( ctx, ts.GetTensorInfo(), std::move(dims) ) {}

  size_t BufferNum() const { return _buffers.size(); }

  D3DBuffer& operator[]( size_t idx )
  {
    return _buffers[ idx ];
  }

  void ManualRelease()
  {
    for( auto& buf: _buffers )
    {
      buf.ManualRelease();
    }    
  }

  bool IsReleased() const 
  {
    // １つ目がリリースされてればリリースされてるとする。
    return _buffers[0].IsReleased();
  }

};

// Tensorの名前をキーにTensorBufferを保持する。
// Metal版のBufferMapと似ている。
struct BufferMap
{
  D3DContext& _ctx;
  std::map<std::string, TensorBuffer> _tbufMap;

  BufferMap( D3DContext& ctx ) : _ctx( ctx ) {}

  TensorBuffer& SetupTensorBuf( const TensorInfo& ts, std::vector<size_t> dims )
  {
    auto tname = ts.Name();
    assert( nullptr == LookupTensorBuf( tname ) );
    auto res = _tbufMap.insert( { tname, TensorBuffer{ _ctx, ts.GetElemBufferTypes(), ts.VectorExtent(), std::move(dims) } });
    return res.first->second;
  }

  TensorBuffer* LookupTensorBuf( const std::string& tname )
  {
    auto iter = _tbufMap.find( tname );
    if (iter == _tbufMap.end())
      return nullptr;
    return &iter->second;
  }

  void SetupTileBuffer( InputsInfo& iinfo, TLTensor& /* result */, InputTiles& inputTiles, int resultWidth, int resultHeight )
  {
    iinfo.ForEachInput( [&]( InputIndex ii, OneInputInfo& one ) {
      TileReference* oneTile = inputTiles.Lookup( ii._layerIndex );
      if (oneTile == nullptr)
      {
        _tbufMap.insert( { one.TsName(), TensorBuffer( _ctx, one.GetTensorInfo(), { (size_t)resultWidth, (size_t)resultHeight } ) } );
      }
      else 
      {
        _tbufMap.insert( { one.TsName(), TensorBuffer( _ctx, one, *oneTile ) } );
      }
    });
  }

  void SetupConstTensor( ConstTensor& cts )
  {
    _tbufMap.insert( { cts.Name(), TensorBuffer( _ctx, cts ) } );
  }

};

/*
  TLTensorとTopLevelBlockのExecutorの共通処理。
*/
struct TLCommonExecutor
{
  D3DContext& _ctx;
  BufferMap& _bufMap;
  REnv& _renv;
  IRBinary& _binary;
  conv::HLSLKernelCommonGenerator& _commonGen;

  TLCommonExecutor( D3DContext& ctx, BufferMap& bufMap, REnv& renv, IRBinary& binary, conv::HLSLKernelCommonGenerator& common )
  : _ctx( ctx ), _bufMap( bufMap ), _renv( renv ), _binary( binary ), _commonGen( common ) {}

  TensorBuffer& EnsureBuf( const TensorInfo& ts )
  {
    auto res = _bufMap.LookupTensorBuf( ts.Name() );
    if (res != nullptr)
      return *res;

    auto bounds = _binary.Resolve( ts, _renv );
    return _bufMap.SetupTensorBuf( ts, std::move(bounds) );
  }

  void SetupParams( std::vector<RVal32>& outParams )
  {
    auto& paramRefs = _commonGen.GetParamRefs();
    for( auto& param : paramRefs )
    {
      outParams.emplace_back();
      outParams.back().u = _renv.GetRVal32( param._name ).u;
    }
  }

  void AddBuffers( std::vector<D3DBuffer*>& result, TensorBuffer& tsbuf )
  {
    for( auto i : NRange( tsbuf.BufferNum() ) )
    {
      result.push_back( &tsbuf[i] );
    }
  }

  void SetupInputBuffers( std::vector<D3DBuffer*>& result )
  {
    auto& tsRefs = _commonGen.GetTensorRefs();
    for( auto one : tsRefs )
    {
      auto& tsbuf = EnsureBuf( one->GetTensorInfo() );
      AddBuffers( result, tsbuf );
    }
  }
};

struct TLTensorKernelExecutor
{
  D3DContext& _ctx;
  BufferMap& _bufMap;
  REnv& _renv;
  IRBinary& _binary;
  TLTensor& _target;
  conv::HLSLTensorGenerator _tgen;
  TLCommonExecutor _common;
  DebugLogger& _debLogger;

  TLTensorKernelExecutor( D3DContext& ctx, BufferMap& bufMap, REnv& renv, IRBinary& binary, TLTensor& target, DebugLogger& debLogger )
  : _ctx( ctx ), _bufMap( bufMap ), _renv( renv ), _binary( binary ), _target( target ), _tgen( _target ), _common( ctx, bufMap, renv, binary, _tgen._kernelCommon ), _debLogger( debLogger ) {}

  std::vector<RVal32> SetupParams( const std::vector<size_t>& bounds )
  {
    std::vector<RVal32> params;
    params.emplace_back();
    params.back().u._ival = bounds[0];
    if (bounds.size() == 2)
    {
      params.emplace_back();
      params.back().u._ival = bounds[1];
    }

    _common.SetupParams( params );
    return params;
  }

  void DebugLog( const std::string& msg )
  {
    _debLogger.Log( msg );
  }

  std::string GenSource()
  {
    _tgen.GenAll( _binary.GetSeedGen() );
    return _tgen.ToString();
  }

  void Run()
  {
    auto shaderSrc = GenSource();
    DebugLog( shaderSrc );

    auto targetInfo = _target.GetTensorInfo();
    auto bounds = _binary.Resolve( targetInfo, _renv );
    _renv.AddExtent( targetInfo, bounds );

    assert( bounds.size() == 1 || bounds.size() == 2 );
    auto dimWidth = bounds[0];
    auto dimHeight = bounds.size() >= 2 ? bounds[1] : 1;     

    auto params = SetupParams( bounds );
    D3DBuffer paramBuf( _ctx, sizeof(RVal32), params.size(), params.data() );

    std::vector<D3DBuffer*> inputBufs{ &paramBuf };
    _common.SetupInputBuffers( inputBufs );

    auto& tsbuf = _common.EnsureBuf( targetInfo );
    std::vector<D3DBuffer*> outBufs;
    _common.AddBuffers( outBufs, tsbuf );

    // アセンブリのダンプは普段は低レベル過ぎて通常のDebugLogで有効にしたくないので、
    // 必要に応じてここでハードコードで有効にする事にする。
    D3DShader shader( _ctx, shaderSrc /* , true */ );
    _ctx.Dispatch( shader, inputBufs, outBufs, dimWidth, dimHeight );
  }
};


struct TopLevelBlockKernelExecutor
{
  D3DContext& _ctx;
  std::ostream& _logStream;
  REnv& _renv;
  TLBlock& _target;
  conv::HLSLTopLevelBlockGenerator _topGen;
  TLCommonExecutor _common;
  DebugLogger& _debLogger;
  SeedGenerator& _sgen;

  TopLevelBlockKernelExecutor( D3DContext& ctx, std::ostream& logStream, BufferMap& bufMap, REnv& renv, IRBinary& binary, TLBlock& target, DebugLogger& debLogger, SeedGenerator& sgen )
  : _ctx( ctx ), _logStream(logStream), _renv( renv ), _target( target ), _topGen( _target ), _common( ctx, bufMap, renv, binary, _topGen._kernelCommon ), _debLogger( debLogger ), _sgen( sgen ) {}

  void DebugLog( const std::string& msg )
  {
    _debLogger.Log( msg );
  }

  std::string GenSource()
  {
    _topGen.GenAll( _sgen );
    return _topGen.ToString();
  }

  void Run()
  {
    auto shaderSrc = GenSource();
    DebugLog( shaderSrc );

    std::vector<RVal32> params;
    _common.SetupParams( params );

    D3DBuffer paramBuf( _ctx, sizeof(RVal32), params.size(), params.data() );

    std::vector<D3DBuffer*> inputBufs{ &paramBuf };
    _common.SetupInputBuffers( inputBufs );

    Param32OutBuffer paramOut( _ctx, _topGen._varsInfo.size() );

    D3DShader shader( _ctx, shaderSrc );
    _ctx.Dispatch( shader, inputBufs, { &paramOut._buffer }, 1, 1 );

    paramOut.CopyToEnv( _topGen._varsInfo, _renv );
    _renv.LogVars( _logStream, _topGen._tlblock._logEntries );
  }
};

// EnsureD3DAllDllLoadedをコンストラクタの最初に呼ぶ為だけのクラス。
struct DllEnsurer
{
  DllEnsurer() { EnsureD3DAllDllLoaded(); }
};

struct D3DBackend
{
  DllEnsurer _ensure;
  D3DContext _ctx;
  std::ostringstream _logStream;
  REnv _renv;
  BufferMap _bufMap;
  bool _debug = false;
  DebugLogger _debLogger;
  PerfCounter _counter;

  D3DBackend() : _bufMap( _ctx ), _debLogger( _logStream ) {}

  void EnableDebugLog( bool enable ) { _debug = enable; }
  
  void ReleaseDeadTensorBuffer( const GlobalTensorLike& tensor, size_t currentTLEPos )
  {
    if( tensor.LastReference() < (int)currentTLEPos )
    {
      auto tsbuf = _bufMap.LookupTensorBuf( tensor.TsName() );
      assert( tsbuf != nullptr );

      tsbuf->ManualRelease();
    }
  }

  // カーネルの参照しているバッファが不要になっていたら解放する。
  void ReleaseDeadReferencedBuffers( TLRoot& tle, size_t currentTLEPos )
  {
    for( auto refpTensor : tle.GetTensorRefs() )
    {
      ReleaseDeadTensorBuffer( *refpTensor, currentTLEPos );
    }    
  }

  /*
    以下はUnitTestのためにpublicにされているが、
    普通の使い方としてはRunを呼ぶだけ。
  */
  void SetupInputResult( mfg_binary::IRBinary& target, InputTiles& inputTiles, TileReference& resTile )
  {
    int rwidth = resTile.Width();
    int rheight = resTile.Height();

    target.RegisterBounds()
      .OfTileInfo( target.Input(), target.Result(), resTile );

    
    _renv.AddIOExtent( target.Input(), target.Result(), (size_t)rwidth, (size_t)rheight );

    _bufMap.SetupTileBuffer( target.Input(), target.Result(), inputTiles, rwidth, rheight );
  }

  void SetupConstTensors( mfg_binary::IRBinary& target )
  {
    target.SetupConstsTSExtents( _renv );
    for( auto& cts : target._constTensors )
    {
      _bufMap.SetupConstTensor( cts );
    }
  }

  DebugLogger& GetDebugLogger()
  {
    _debLogger._debug = _debug;
    return _debLogger;
  }

  void RunOneTLE( mfg_binary::IRBinary& target, size_t tlePos )
  {
    auto& tle = target._tles[tlePos];

    switch (tle._etype)
    {
      case TLEntityRef::TENSOR:
      {
        auto tltensor = tle.u._tensor;

        // 使われていないテンソルのカーネルはスキップする。
        if (tltensor->LastReference() >= 0)
        {
          TLTensorKernelExecutor exec( _ctx, _bufMap, _renv, target, *tltensor, GetDebugLogger() );
          exec.Run();

          ReleaseDeadReferencedBuffers( *tltensor, tlePos+1 );
          // これが消されることはない気もするが。
          ReleaseDeadTensorBuffer( exec._target, tlePos+1 ); 
        }

        break;
      }
      case TLEntityRef::TOP_LEVEL_BLOCK:
      {
        auto tblock = tle.u._tblock;

        TopLevelBlockKernelExecutor exec( _ctx, _logStream, _bufMap, _renv, target, *tblock, GetDebugLogger(), target.GetSeedGen() );
        exec.Run();

        ReleaseDeadReferencedBuffers( *tblock, tlePos+1 );
        break;
      }
      case TLEntityRef::UNBOUND:
        assert(false);
        break;
    }

    target.ForEachInput( [&]( InputIndex, OneInputInfo& one ) {
      ReleaseDeadTensorBuffer( one, tlePos+1 );
    });
  }

  // キャンセルされたらfalse、ちゃんと終わればtrue
  bool WaitKernel( TProgressCallback callback )
  {
    ID3D11Query* pQuery = nullptr;
    D3D11_QUERY_DESC desc = { D3D11_QUERY_EVENT, 0 };
    // desc.Query = D3D11_QUERY_EVENT;
    HRESULT hr = _ctx._device->CreateQuery( &desc, &pQuery );

    ThrowIfFail( hr, ResId::D3D_CREATE_QUERY_FAIL );
    _ctx._context->End( pQuery );

    auto releaseGuard = ScopeGuard([&]{ pQuery->Release(); });

    DWORD count = 0;
    while( S_OK != _ctx._context->GetData( pQuery, nullptr, 0, 0 ) )
    {
      if(!CallbackPercent( callback, 0 ))
        return false;
      // 最初1msecで5回まち、次に101msecで5回、201msecで5回...と進んでいき、501msec以降はずっと毎回501msecで待つ。
      Sleep( 1+std::min( 100*(count/5), (DWORD)500 ) );
      count++;
    }
    return true;
  }

  void CopyBackResult( mfg_binary::IRBinary& target, TileReference& resTile )
  {
    auto& bufout = *_bufMap.LookupTensorBuf( target.Result().Name() );
    bufout.ToTile( _ctx, resTile );
  }

  // キャンセルされたらfalse、正常終了したらtrueを返す
  bool Run( mfg_binary::IRBinary& target, InputTiles& inputTiles, TileReference& resTile, TProgressCallback callback = nullptr )
  {
    _counter.Reset();
    target.ResetSeedGen();

    _counter.BeginInput();
    SetupInputResult( target, inputTiles, resTile );
    SetupConstTensors( target );
    _counter.EndInput();

    // ホスト側コードもそれなりに含んじゃうが、大雑把に計測。
    _counter.BeginKernel();
    for( auto tlePos: NRange(target._tles.size()) )
    {
      RunOneTLE( target, tlePos );
    }
    auto ret =WaitKernel( callback );
    _counter.EndKernel();

    if (ret)
    {
      _counter.BeginResult();
      CopyBackResult( target, resTile );
      _counter.EndResult();
    }

    return ret;
  }

  std::string DrainLogs()
  {
    auto ret = _logStream.str();
    _logStream.str( std::string() );
    return ret;
  }
  
  const mfg::PerfInfo& GetPerfInfo() const { return _counter._info; }

  /* UnitTest用 */
  bool IsBufferReleased( const GlobalTensorLike& ts )
  {
    auto pbuf = _bufMap.LookupTensorBuf( ts.TsName() );
    return pbuf->IsReleased();
  }
};

} ///< mfg_runtime_d3d


//
// DirectComputeをデフォルトとするためのコードを以下に置く
//

#include "mfg_runtime_common.hpp"

namespace mfg {

// インテグレート向けのfacade、DirectComputeをデフォルトとする。
using MFG = MFGFacade<mfg_runtime_d3d::D3DBackend>;


}///< mfg

#endif

