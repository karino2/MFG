/* -*- coding: utf-8 -*- マルチバイト */

// Copyright 2026 Kazuma Arino. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#ifndef MFG_RUNTIME_METAL_HPP_
#define MFG_RUNTIME_METAL_HPP_

/*
 デフォルトではAutoreleasePoolを作り、MFGFacade終了時にdrainしている。
 
 ARCを使う場合は以下をdefineする事。（libneet_conf.hなどでdefineする事を想定）
 __NEET_METAL_ARC_ENABLED__

 この場合、呼び出し側でMFGFacadeのRunを @autorelease でくくる事。 
 */
#include <mfg_pal/pal.hpp>

#include "mfg_elem.hpp"
#include "mfg_printer.hpp"
#include "mfg_tensor.hpp"
#include "mfg_binary.hpp"
#include "mfg_resource.hpp"
#include "mfg_runtime_common.hpp"

#include <string>
#include <sstream>
#include <vector>
#include <numeric>
#include <thread>
#include <chrono>


/*
  MSLのGenerator関連

  TensorなどをMetal Shading Languageのカーネルのソースコードに変換する。
  単にテキストを生成するだけなので、これ自体はランタイムには依存しない。
*/
namespace mfg_msl_converter {
using namespace mfg_internal;
using namespace mfg_binary;
using namespace mfg_runtime_common;
using namespace mfg_clike_generator;

// MacでUIntがぶつるかるので明示的にimport。
using mfg_internal::UInt;

// Tensor関連、主にカーネル単位のNameConverter。
struct TensorNameConverter
{
  NameConverter _conv;
  TLTensor& _tensor;
  TensorInfo _tsinfo;
  TensorNameConverter( TLTensor& tensor ) : _tensor( tensor ), _tsinfo( _tensor.GetTensorInfo() ) {}

  std::string Kernel() const
  {
    return _conv.Ident( "k_" + _tensor.Name() );
  }

  std::string RawOutputBuf() const
  {
    return "b_" + _tensor.Name() + ".out";
  }

  std::string OutputBuf( size_t tupIndex )
  {
    return _conv.Ident( RawOutputBuf() + "." + std::to_string( _tsinfo.BufIndex( tupIndex ) ) );
  }

  std::string OutputBufExtent( int index ) const
  {
    return _conv.Ident( _tsinfo.GetExtentName(index) );
  }

  std::string InitLoopVarName( int index ) const
  {
    return _conv.Ident( _tensor.LoopVarName( index ) );
  }

  std::string MostOuterLoopVarName() const
  {
    return InitLoopVarName( _tsinfo.Dimensions() - 1 );
  }

  size_t Dimensions() const { return _tsinfo.Dimensions(); }
};

struct IgnoreUnusedVarGuard
{
  std::stringstream &_stream;
  IgnoreUnusedVarGuard( std::stringstream &stream ) : _stream( stream )
  {
    _stream << "#pragma clang diagnostic push" << std::endl;
    _stream << "#pragma clang diagnostic ignored \"-Wunused-variable\"" << std::endl;
  }

  ~IgnoreUnusedVarGuard()
  {
    _stream << "#pragma clang diagnostic pop" << std::endl;
  }
};

/*
  MSLKernelCommonGenerator:
    TensorやTopLevelBlockなどのカーネルを生成する共通処理。

*/
struct MSLKernelCommonGenerator
{
  std::string _kernelName; 
  std::stringstream& _stream;
  std::vector<GlobalTensorLike*>& _tensorRefs;
  std::vector<ReferenceParamInfo>& _paramRefs;
  NameConverter _nconv;
  ExprToCLike _exprConv;

  MSLKernelCommonGenerator( const std::string& kernelName, TLRoot& tle, std::stringstream& stream ) : _kernelName( kernelName ), _stream( stream ), _tensorRefs( tle.GetTensorRefs() ), _paramRefs( tle.GetParamRefs() ), _exprConv( _stream )
  {
    _exprConv._overwriter = [this]( ExprToCLike& bs, BaseExprElem* elem )
      {
        switch( elem->_elemType )
        {
          case IRElemType::BinOp:
          {
            auto op = elem->As<BinOp>();
            if (op->_opType == BinOp::Mod && op->_type.IsFloat())
            {
              // Metalではfloatの%は使えないのでfmodにする。
              // HLSLでは%が使えてfmodが無いのでデフォルトは%にしてMetalだけoverwrite
              auto& stream = bs._stream;
              stream << "fmod(";
              bs.GenVExpr( op->GetLeft() );
              stream << ", ";
              bs.GenVExpr( op->GetRight() );
              stream << ")";
              return true;
            }
            return false;
          }
          default:
            return false;
        }
      };
  }

  const std::string& GetKernelName() const
  {
    return _kernelName;
  }

  std::vector<GlobalTensorLike*>& GetTensorRefs()
  {
    return _tensorRefs;
  }

  std::vector<ReferenceParamInfo>& GetParamRefs()
  {
    return _paramRefs;
  }

  std::string ToString() const { return _stream.str(); }

  // kernel void _f1(
  void GenSignatureFirstLine()
  {
    _stream << "kernel void " << GetKernelName() << "(" << std::endl;
  }

  // device const uint32_t* _f1_in,
  // device const uint32_t* _f2_in,
  void GenSignatureInputBufs()
  {
    for( auto tsref : GetTensorRefs() )
    {
      auto tsinfo = tsref->GetTensorInfo();
      for( auto i : NRange( tsinfo.ElemBufferNum() ) )
      {
        _stream << "  device const " << _nconv.Type( tsinfo.ElemType(i) ) << "* " << _nconv.Ident( tsinfo.ElemBufferName( i ) ) << "," << std::endl;
      }
    }
  }

  /*
      constant int32_t* param32_in,
      uint grid_idx [[thread_position_in_grid]])
  */
  void GenSignatureEpilogue( size_t dim )
  {
    assert( dim == 1 || dim == 2 );
    _stream << "  constant int32_t* param32_in," << std::endl;
    _stream << "  " << (dim == 1 ? "uint" : "uint2") << " grid_idx [[thread_position_in_grid]])" << std::endl;
  }

  // 以下のような文を生成
  // const int32_t _input_u8_textent_0 = param32_in[2];
  // const int32_t _input_u8_textent_1 = param32_in[3];
  void ParamSetup( size_t paramInOrigin )
  {
    size_t paramBaseIndex = paramInOrigin; // 最初にoutBufのextent.0がある場合などは最初のいくつかのインデックスをスキップした値となる
    auto& params = GetParamRefs();
    for( auto pidx : NRange(params.size()) )
    {
      // 以下のような文を生成
      // const int32_t _extent_input_u8_0 = param32_in[2];
      auto& pinfo = params[pidx];
      auto pname = _nconv.Ident( pinfo._name );

      switch( pinfo._ptype )
      {
        case ReferenceParamInfo::PARAM_INT32:
          _stream << "  const int32_t " << pname << " = param32_in[" << std::to_string( paramBaseIndex ) << "];" << std::endl;
          paramBaseIndex += 1;
          break;
        case ReferenceParamInfo::PARAM_FLOAT32:
          _stream << "  const float " << pname << " = ((constant float*)param32_in)[" << std::to_string( paramBaseIndex ) << "];" << std::endl;
          paramBaseIndex += 1;
          break;
        case ReferenceParamInfo::PARAM_UINT32:
          _stream << "  const uint32_t " << pname << " = ((constant uint32_t*)param32_in)[" << std::to_string( paramBaseIndex ) << "];" << std::endl;
          paramBaseIndex += 1;
          break;
        default:
          assert( false );
          break;
      }
    }
  }

  /*
    以下を生成。12345はカーネルの名前から生成されるハッシュ。

    RandomGenerator rgen(12345);
  */
  void GenRandomDef( SeedGenerator& sgen )
  {
    const auto& kname = GetKernelName();
    uint32_t seed = Hash32( (const uint8_t*)kname.c_str(), kname.size(), sgen.Next() );
    _stream << "  RandomGenerator rgen(" <<  seed << ");" << std::endl;
  }

  static void GenRandomPrologue( std::stringstream& out )
  {
    out << R"(
struct RandomGenerator
{
  uint32_t _seed;

  RandomGenerator( uint32_t seed ) : _seed(seed) {}

  uint32_t Next()
  {
    uint32_t x = _seed;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    _seed = x;
    return x;
  }

  void AddNextSeed( uint32_t nextSeed )
  {
    Next();
    _seed *= (1+nextSeed);
  }

  float FNext()
  {
    auto v = Next();
    return ((float) v)/)"
     << std::numeric_limits<uint32_t>::max() << R"(;
  }
};
)";
  }

  static void GenSortPrologue( std::stringstream& out )
  {
    out << R"(
template<typename T>
void sort_1d( thread T* localbuf, int vecExtent, int extent )
{
  for( int j = 1; j < extent; j++ )
  {
    for( int v = 0; v < vecExtent; v++)
    {
      T one = localbuf[j*vecExtent+v];
      int i = j - 1;
      while( i >= 0 && localbuf[i*vecExtent+v] > one)
      {
        localbuf[(i+1)*vecExtent+v] = localbuf[i*vecExtent+v];
        i--;
      }
      localbuf[(i+1)*vecExtent+v] = one;
    }
  }
}

void sort_int32_1d( thread int* localbuf, int vecExtent, int extent ){ return sort_1d( localbuf, vecExtent, extent ); }
void sort_float32_1d( thread float* localbuf, int vecExtent, int extent ){ return sort_1d( localbuf, vecExtent, extent ); }

)";

  /*
    sort_int32_1dはベクトルの要素ごとに別々にsortする。
  */
  }

  static void GenLibraryPrologue( std::stringstream& out, LibraryIRInfo info )
  {
    out << "#include <metal_stdlib>" << std::endl;
    out << "using namespace metal;" << std::endl;
    if (info._useRandom)
      GenRandomPrologue( out );
    if (info._useSort)
      GenSortPrologue( out );
  }

  void GenLibraryInit( LibraryIRInfo info )
  {
    GenLibraryPrologue( _stream, info );
  }

};

/*
  MSLTensorGenerator:
  　　　　Tensorに対応したMSLのカーネルを生成する。
    だいたい以下のコードを生成。
    thread_position_in_gridはintは許されていなかった（コンパイルエラー）

kernel void _f1(
  device uint32_t *_f1_out_0,
  constant int32_t* param32_in,
  uint grid_idx [[thread_position_in_grid]])
{
  int32_t _y = (int32_t)grid_idx;
  int32_t _f1_out_extent_0 = param32_in[0];
  int32_t _f1_out_extent_1 = param32_in[1];
  for( int _x = 0; _x < _f1_out_extent_0; _x++ )
  {
    _f1_out_0[_y*_f1_out_extent_0+x] = _x+_y;
  }
}
*/
struct MSLTensorGenerator
{
  TLTensor& _tensor;
  TensorInfo _tsinfo;
  TensorNameConverter _tconv;
  std::stringstream _stream;
  MSLKernelCommonGenerator _kernelCommon;
  NameConverter _nconv;
  ExprToCLike& _exprConv;

  MSLTensorGenerator( TLTensor& tensor ) : _tensor( tensor ), _tsinfo( tensor.GetTensorInfo() ), _tconv( _tensor ), _kernelCommon( _tconv.Kernel(), _tensor, _stream ), _exprConv( _kernelCommon._exprConv )
  {
  }

  std::string GetKernelName() const
  {
    return _tconv.Kernel();
  }

  std::string ToString() const { return _stream.str(); }

  bool HasRandom()
  {
    RandomFinder finder;
    return finder.HasRandom( _tensor );
  }

  void GenOutputBufs()
  {
    for (auto i : NRange( _tsinfo.ElemBufferNum() ))
    {
      _stream << "  device " << _nconv.Type( _tsinfo.ElemType( i ) ) << "* " << _tconv.OutputBuf( i ) << "," << std::endl;
    }
  }

  /*
    kernel void _f1(
      device uint32_t *_f1_out,
      constant int32_t* param32_in,
      uint grid_idx [[thread_position_in_grid]])
  */
  void GenSignature()
  {
    _kernelCommon.GenSignatureFirstLine();
    GenOutputBufs();
    _kernelCommon.GenSignatureInputBufs();
    _kernelCommon.GenSignatureEpilogue( _tsinfo.Dimensions() );
  }

  /*
  {
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wunused-variable"
    const int32_t _x = (int32_t)grid_idx.x;
    const int32_t _y = (int32_t)grid_idx.y;
    const int32_t _f1_out_extent_0 = param32_in[0];
    const int32_t _f1_out_extent_1 = param32_in[1];
    #pragma clang diagnostic pop

  乱数がある場合は以下も生成（12345は毎回違うseed）
    RandomGenerator rgen(12345);
    rgen.AddNextSeed(grid_idx.x);
    rgen.AddNextSeed(grid_idx.y);
  */
  void GenFuncPrologue( SeedGenerator& sgen )
  {
    _stream << "{" << std::endl;

    // extentは使わない時も生成するので、未使用ワーニングをここだけ消す。
    {
      IgnoreUnusedVarGuard vg( _stream );
      assert( _tconv.Dimensions() == 1 || _tconv.Dimensions() == 2);
      if (_tconv.Dimensions() == 1 )
      {
        _stream << "  const int32_t " << _tconv.InitLoopVarName( 0 ) <<  " = (int32_t)grid_idx;" << std::endl;
      }
      else
      {
        _stream << "  const int32_t " << _tconv.InitLoopVarName( 0 ) <<  " = (int32_t)grid_idx.x;" << std::endl;
        _stream << "  const int32_t " << _tconv.InitLoopVarName( 1 ) <<  " = (int32_t)grid_idx.y;" << std::endl;
      }

      for( auto i : NRange( _tconv.Dimensions() ) )
      {
        _stream << "  const int32_t " << _tconv.OutputBufExtent( i ) << " = param32_in[" << i << "];" << std::endl;

      }

      _kernelCommon.ParamSetup( _tconv.Dimensions() );  // 最初にoutBufのextent.0があるので、続きはその次から。
    }


    if (HasRandom())
    {
      /*
        RandomGenerator rgen(12345);
        rgen.AddNextSeed(grid_idx.x);
        rgen.AddNextSeed(grid_idx.y);
      */
      _kernelCommon.GenRandomDef( sgen );
      if (_tconv.Dimensions() == 1 )
      {
        _stream << "  rgen.AddNextSeed(grid_idx);" << std::endl;
      }
      else
      {        
        _stream << "  rgen.AddNextSeed(grid_idx.x);" << std::endl;
        _stream << "  rgen.AddNextSeed(grid_idx.y);" << std::endl;
      }

    }

    // 今のところテンソルは2次元までですべてdispatchで対応されるので生成しない。
  }

  /*
    }
  */
  void GenFuncEpilogue()
  {
    // カーネル関数自体のclose
    _stream << "}" << std::endl;
  }

  void GenOneAssign( const std::string& bufName, Expr& idxExpr, BaseExprElem* rexpr )
  {
      _stream << "    ";
      _stream << bufName << "[";
      _exprConv.GenVExpr( idxExpr.GetBase() );
      _stream << "] = ";
      _exprConv.GenVExpr( rexpr );
      _stream << ";" << std::endl;

  }

  /*
    _tsinfoがベクトルの時は
    baseIndex*vectorDim + tupIdxを返す。
    それ以外の時は baseIndexのまま。
  */
  Expr VectorizedIndex( Expr&& baseIndex, size_t tupIdx ) const
  {
    if (!_tsinfo.IsVector())
      return std::move(baseIndex);
    
    IRBuildDSL d;
    return d._Add( d._Mul( std::move(baseIndex), _tsinfo.VectorExtent() ), d._I32( tupIdx ) );
  }

  // TensorのReturnからassignを生成。以下のような文
  // f1_out_0[y*f1_out_extent_0+x] = x+y;
  void GenFinalAssigns( BaseExprElem* retExpr )
  {
    auto indexVars = _tensor.ArgVars();
    TensorIRFactory factory( _tsinfo );
    auto idxExpr = factory.NoClampFlattenIdx( std::move( indexVars ) );

    // Lowerで無くなったはずの処理。assertしておく。
    assert( retExpr->_elemType != IRElemType::ReduceSum );

    TupleOrElemEach( retExpr, [this, &factory, &idxExpr]( size_t idx, BaseExprElem* one ) {      
      auto eachIdx = VectorizedIndex( idxExpr.Clone(), idx );
      GenOneAssign( _tconv.OutputBuf( idx ), eachIdx, one );
    });      
  }

  void GenBodyWithoutReturn( BodyElem* vblock )
  {
    ForEachFn( vblock->GetExprsWithoutReturn(), [this](BaseExprElem* s) {
      _stream << "    ";
      _exprConv.GenExpr( s );
    });
  }

  void GenInitDefBody()
  {
    BodyElem* vblock = _tensor.GetBody();
    GenBodyWithoutReturn( vblock );

    GenFinalAssigns( vblock->GetReturnExpr() );
  }

  // カーネルのコードを生成。
  // includeなどが要らない場合はこちら。
  void GenKernel( SeedGenerator& sgen )
  {
    GenSignature();
    GenFuncPrologue( sgen );
    GenInitDefBody();
    GenFuncEpilogue();
  }

  // incudeまで含めたカーネルのコードを生成
  // 主にUnitTest用
  void GenLibrarySource( SeedGenerator& sgen )
  {
    LibraryIRInfoCollector collector;
    collector.Collect( _tensor.GetRootElem() );
    _kernelCommon.GenLibraryInit( collector._info );
    GenKernel( sgen );
  }
};


/*
  MSLTopLevelBlockGenerator:
  トップレベルのブロックをカーネルとして実現するコードを生成する。
  トップレベルのLetをグローバルな変数のリストとして、そのやり取りをする。

  スレッド数1のグリッドとして実行し、param32_outに結果を入れる。
  param32_outはint32_tもuint32_tもfloatもすべて共用する。
  64bitは必要になったら考える。

  だいたい以下のようなコードを生成。

  kernel void _globalBlock1(
    device uint32_t* param32_out,
    constant int32_t* param32_in,
    uint grid_idx [[thread_position_in_grid]])
  {
    float _angle = ((constant float*)param32_in)[0];
    float _strength = ((constant float*)param32_in)[1];
    
    // ブロックのbody
    int32_t _varname0 = xxx;
    float _varname1 = yyyy;

    ((device int32_t*)param32_out)[0] = _varname0;
    ((device float*)param32_out)[1] = _varname1;
  }
*/
struct MSLTopLevelBlockGenerator
{
  NameConverter _nconv;
  std::string _name;
  std::stringstream _stream;
  MSLKernelCommonGenerator _kernelCommon;
  ExprToCLike& _exprConv;
  TLBlock &_topLevelBlock;
  std::vector<std::pair<Type, std::string>> _varsInfo;

  MSLTopLevelBlockGenerator( TLBlock& block ) : _name( UniqueName('k') + "_gblock" ), _kernelCommon( _name, block, _stream ), _exprConv( _kernelCommon._exprConv ), _topLevelBlock( block )
  {
  }

  const std::string& GetKernelName() const { return _kernelCommon.GetKernelName(); }

  std::string ToString() const { return _stream.str(); }

    bool HasRandom()
  {
    RandomFinder finder;
    return finder.HasRandom( _topLevelBlock );
  }


  void CollectGlobalVars()
  {
    _varsInfo = _topLevelBlock.CollectGlobalVarsInfo();
  }

  /*
    以下を生成。
    grid_idxは使わないが、あっても困らないのでいつも生成してしまう事に。

    kernel void _globalBlock1(
      device int32_t* param32_out,
      constant int32_t* param32_in,
      uint grid_idx [[thread_position_in_grid]])
  */
  void GenSignature()
  {
    _kernelCommon.GenSignatureFirstLine();
    _stream << "  device int32_t* param32_out," << std::endl;
    _kernelCommon.GenSignatureInputBufs();
    _kernelCommon.GenSignatureEpilogue(1);
  }

  /*
    以下を生成
    {
      float _angle = ((constant float*)param32_in)[0];
      float _strength = ((constant float*)param32_in)[1];

  乱数がある場合は以下も生成（12345は毎回違うseed）
    RandomGenerator rgen(12345);
  */
  void GenFuncPrologue( SeedGenerator& sgen )
  {
    _stream << "{" << std::endl;

    // extentは使わない時も生成するので、未使用ワーニングをここだけ消す。
    {
      IgnoreUnusedVarGuard vg( _stream );
      _kernelCommon.ParamSetup( 0 );
    }

    if (HasRandom())
    {
      _kernelCommon.GenRandomDef( sgen );
    }
  }

  Type As32( const Type& baseType )
  {
    auto ntype = baseType.AsNumeric();
    if (ntype.Bytes() == 4)
      return baseType;
    return Type( NumericType( ntype._code, 32 ) );
  }

  void GetOneAssign( size_t index, const Type& type, const std::string& varName )
  {
    auto t32 = As32(type);
    _stream << "  ((device " << _nconv.Type( t32 ) << "*)param32_out)[";
    _stream << index << "] = ";
    if (t32 != type)
    {
      _stream << "(" << _nconv.Type(t32) << ")";
    }
    _stream << _nconv.Ident( varName ) << ";" << std::endl;
  }

  /*
    以下を生成。int32_tはキャストは要らないが、簡単の為全部キャスト挟む

    ((device float*)param32_out)[0] = _deltaU;
    ((device float*)param32_out)[1] = _deltaV;
    ((device int32_t*)param32_out)[2] = _rbegin;

    また、u8などはuint32_tにキャストする。

    ((device uint32_t*)param_32_out)[0] = (uint32_t)_b;
    ((device uint32_t*)param_32_out)[1] = (uint32_t)_g;

  */
  void GenLastAssigns()
  {
    for (auto idx : NRange(_varsInfo.size()))
    {
      auto& pair = _varsInfo[idx];
      GetOneAssign( idx, pair.first, pair.second );
    }    
  }

  void GenRootStmt( BodyElem* voidBlock )
  {
    voidBlock->GetExprs()->ForEach<BaseExprElem>([this](BaseExprElem* child){
      _exprConv.GenExpr( child );
    });
  }

  void GenKernel( SeedGenerator& sgen )
  {
    GenSignature();
    GenFuncPrologue( sgen );
    GenRootStmt( _topLevelBlock.GetIRBody() );
    GenLastAssigns();
    _stream << "}" << std::endl;
  }

  // UnitTest用
  void GenLibrarySource( SeedGenerator& sgen )
  {
    LibraryIRInfoCollector collector;
    collector.Collect( _topLevelBlock.GetRootElem() );
    _kernelCommon.GenLibraryInit( collector._info );
    GenKernel( sgen );
  }
};

} ///< mfg_msl_converter


/*
  Metalのランタイム関連。Objcのラッパーなども含む。
  以下は別ファイルでも良いのだが、ファイルの取り回しの都合でとりあえず同じファイルに含めておく。
*/


#include <objc/runtime.h>
#include <objc/message.h>
#include <cstdint>

extern "C" {

// objc_msgSendは特殊なのでいつもキャストして呼ぶ
// 詳細は以下
// https://www.mikeash.com/pyblog/objc_msgsends-new-prototype.html
extern void objc_msgSend(void);

#ifndef __OBJC__
extern void NSLog(id /* NSString * */ format, ...);
#endif
extern id MTLCreateSystemDefaultDevice();

}

/*
  objcのラッパーのnamespace。ほとんど単純なラッパのみ。
*/
namespace mfg_objc {
using mfg_internal::FmtSystemError;
using mfg_resource::ResId;

// この辺の定義はmetal-cppを真似ている。
using NSInteger = std::intptr_t;
using NSUInteger = std::uintptr_t;

template<typename RET, typename... ARGS>
RET SendMessage( ARGS... args )
{
  using MethodType = RET(*)(ARGS...);
  MethodType method = (MethodType)&objc_msgSend;
  return method( args... );
}

inline id AllocObject( const char* klassName )
{
  return SendMessage<id>( objc_getClass( klassName ), sel_getUid( "alloc" ) );
}

inline id AutoreleasePool_Create()
{
  id pool = AllocObject( "NSAutoreleasePool" );
  pool = SendMessage<id>( pool, sel_getUid( "init" ) );
  return pool;
}

// AutoreleasePoolはdrainでdisposeされるらしい。
// https://developer.apple.com/documentation/foundation/nsautoreleasepool
inline void AutoreleasePool_Drain( id pool )
{
  SendMessage<id>( pool, sel_getUid( "drain" ) );
}



#ifdef __NEET_METAL_ARC_ENABLED__
/*
 ARCが有効の時はAutoreleasePoolでくくるのは呼ぶ側の責任とし、MFGFacadeではAutoreleasePoolはつくらない。
 そこでダミーのクラスに差し替える。
*/
struct AutoreleasePool
{
};
#else
// RAIIのNSAutoreleasePool
struct AutoreleasePool
{
  id _pool;
  AutoreleasePool() : _pool( AutoreleasePool_Create() ) {}
  ~AutoreleasePool() { AutoreleasePool_Drain( _pool ); }
};
#endif

inline void NSObject_Retain( id obj )
{
  SendMessage<id>( obj, sel_getUid( "retain" ) );
}

inline void NSObject_Release( id obj )
{
  SendMessage<id>( obj, sel_getUid( "release" ) );
}

// リテラルをラップする用。std::stringなどのように削除されるものには使ってはいけない。
inline id NSString_CreateNoCopy( const char* string, size_t len )
{
  id nsstr = AllocObject( "NSString" );
  return SendMessage<id>(
          nsstr,
          sel_getUid( "initWithBytesNoCopy:length:encoding:freeWhenDone:" ),
          string, len, 4, 0
        );
}

// no need to release.
inline id NSString_StringWithCString( const char* cstring )
{
  return SendMessage<id>(
          objc_getClass( "NSString" ),
          sel_getUid( "stringWithCString:encoding:" ),
          cstring, 4 // UTF8
        );
}

// RAII用のNSObjectのホルダー
struct NSObject
{
  // errorなどを受け取る時は直接ここをポインタとして渡す。
  id _id;

  NSObject( id obj ) : _id( obj ) {}
  NSObject() : _id( nullptr ) {}
  NSObject( NSObject&& src ) : _id( src._id ) { src._id = nullptr; }

  bool IsNull() const { return _id == nullptr; }
  operator id() const { return _id; }

  /*
    マニュアルのRelease。RAIIとしては呼ぶ必要は無い。
    これを呼ぶと対応するidは解放されてデストラクタでは何もされない。
  */
  void ManualRelease()
  {
    if (_id != nullptr)
    {
      NSObject_Release( _id );
    }
    _id = nullptr;
  }

  // コピーassignmentだけど、元のobjをreleaseしない。
  // これはObjective-Cのイディオムとして、initなどでselfを差し替えたものを返す場合に、
  // 差し替えた物の管理はinit側の責務となるため。
  NSObject& operator=( id newObj )
  { 
    _id = newObj;
    return *this;
  }

  ~NSObject()
  { 
    if (_id != nullptr)
    {
      NSObject_Release( _id );
    }
  }
};

inline void NSLog_Utf8( const std::string& str )
{
  NSObject format_string = NSString_CreateNoCopy( "%@", 2 );
  NSObject contents = NSString_CreateNoCopy( str.c_str(), str.size() );
  NSLog( format_string, contents._id );
}

inline void NSLog_NSObject( id obj )
{
    NSObject format = NSString_CreateNoCopy( "%@", 2 );
    NSLog( format, obj );
}

inline std::string NSError_ToString( id error )
{
  id str = SendMessage<id>(
            error,
            sel_getUid( "description" )
          );
  return std::string( SendMessage<const char*>(
          str,
          sel_getUid( "UTF8String" )
        ) );

}

inline id MTLDevice_NewLibraryWithSource( id device, const char* source )
{
  #ifdef MFG_IGNORE_METAL_SOURCE_CACHE
  // キャッシュを無効化するために一時的にsaltを加える
  std::string modified_source = std::string(source) + "\n// Salt: " + std::to_string(std::time(nullptr));
  id source_nsstr = NSString_StringWithCString( modified_source.c_str() );
  #else
  id source_nsstr = NSString_StringWithCString( source );
  #endif

  NSObject options = AllocObject( "MTLCompileOptions" );
  options = SendMessage<id>( options._id, sel_getUid( "init" ) );

  SendMessage<void>( options._id, sel_getUid( "setFastMathEnabled:" ), false );

  id error;
  id resultLib = SendMessage<id>( device, sel_getUid( "newLibraryWithSource:options:error:" ), source_nsstr, options._id, &error );

  if (resultLib == nullptr)
  {
    auto errMsg = NSError_ToString( error );
    // シェーダーのコンパイルに失敗するケースはUserErrorかもしれないけれど、なるべくパース時にverifyするという事でシステムエラーにしておく。
    throw FmtSystemError( ResId::METAL_COMPILATION_ERROR, errMsg );
  }

  return resultLib;
}

inline bool MTLDevice_SupportsMFGGPU( id device )
{
  /*
    MFGはdispatchThreadsを使うので、GPU Family Common3以上が必要
    [dispatchThreads:threadsPerThreadgroup: - Apple Developer Documentation](https://developer.apple.com/documentation/metal/mtlcomputecommandencoder/2866532-dispatchthreads?language=objc)
    のリンクから辿れるMetal Feature Set Tables参照（Non uniform thread gropu size）
    また、#541 も参照。
  */
  const NSInteger GPUFamilyCommon3 = 3003;
  return SendMessage<bool>( device, sel_getUid("supportsFamily:"), GPUFamilyCommon3 );
}

inline id MTLLibrary_NewFunctionWithName( id library, const char* cname )
{
  id namensstr = NSString_StringWithCString( cname );
  return SendMessage<id>( library, sel_getUid( "newFunctionWithName:" ), namensstr );
}

inline id MTLDevice_NewComputePipelineStateWithFunction( id device, id function )
{
  id error;
  id result = SendMessage<id>( device, sel_getUid( "newComputePipelineStateWithFunction:error:" ), function, &error );
  if (result == nullptr)
  {
    NSLog_NSObject( error );
  }
  return result;
}

inline id MTLDevice_NewCommandQueue( id device )
{
  return SendMessage<id>( device, sel_getUid("newCommandQueue") );
}

inline id MTLDevice_NewBufferWithLength( id device, size_t length )
{
  return SendMessage<id>( device, sel_getUid("newBufferWithLength:options:"), length, 0 /* options:MTLResourceStorageModeShared*/ );
}

inline id MTLCommandQueue_CommandBuffer( id queue ) 
{
  return SendMessage<id>( queue, sel_getUid( "commandBuffer" ) );
}

inline id MTLCommandBuffer_ComputeCommandEncoder( id buffer )
{
  return SendMessage<id>( buffer, sel_getUid( "computeCommandEncoder" ) );
}

inline id MTLCommandBuffer_Error( id buffer )
{
  return SendMessage<id>( buffer, sel_getUid( "error" ) );
}

inline void MTLComputeCommandEncoder_SetComputePipelineState( id encoder, id pipelineState )
{
  SendMessage<void>( encoder, sel_getUid( "setComputePipelineState:" ), pipelineState );
}

inline void MTLComputeCommandEncoder_SetBuffer( id encoder, id buffer, size_t offset, uint32_t index )
{
  SendMessage<void>( encoder, sel_getUid( "setBuffer:offset:atIndex:" ), buffer, offset, index );
}

inline void MTLComputeCommandEncoder_SetBytes( id encoder, const void* buffer, NSUInteger length, NSUInteger index )
{
  SendMessage<void>( encoder, sel_getUid( "setBytes:length:atIndex:" ), buffer, length, index );
}

// この辺の定義はmetal-cppを真似ている。
struct MTLSize
{
  NSUInteger width, height, depth;
} __attribute__((packed));

inline NSUInteger MTLComputePipelineState_GetMaxTotalThreadsPerThreadGroup( id pipelineState )
{
  return SendMessage<NSUInteger>( pipelineState, sel_getUid( "maxTotalThreadsPerThreadgroup" ) );
}

inline NSUInteger MTLComputePipelineState_GetThreadExecutionWidth( id pipelineState )
{
  return SendMessage<NSUInteger>( pipelineState, sel_getUid( "threadExecutionWidth" ) );
}


inline void MTLComputeCommandEncoder_DispatchThreadGroups( id encoder, int32_t gridW, int32_t gridH, int32_t gridD, int32_t threadW, int32_t threadH, int32_t threadD )
{
  MTLSize gridSize;
  MTLSize threadSize;

  gridSize.width = gridW;
  gridSize.height = gridH;
  gridSize.depth = gridD;

  threadSize.width = threadW;
  threadSize.height = threadH;
  threadSize.depth = threadD;

  SendMessage<void>( encoder, sel_getUid( "dispatchThreadgroups:threadsPerThreadgroup:" ), gridSize, threadSize );
}

inline void MTLComputeCommandEncoder_DispatchThreads( id encoder, int32_t gridW, int32_t gridH, int32_t gridD, int32_t threadW, int32_t threadH, int32_t threadD )
{
  MTLSize gridSize;
  MTLSize threadSize;

  gridSize.width = gridW;
  gridSize.height = gridH;
  gridSize.depth = gridD;

  threadSize.width = threadW;
  threadSize.height = threadH;
  threadSize.depth = threadD;

  SendMessage<void>( encoder, sel_getUid( "dispatchThreads:threadsPerThreadgroup:" ), gridSize, threadSize );
}


inline void MTLComputeCommandEncoder_EndEncoding( id encoder )
{
  SendMessage<void>( encoder, sel_getUid( "endEncoding" ) );
}

inline void MTLCommandBuffer_Commit( id buffer )
{
  return SendMessage<void>( buffer, sel_getUid( "commit" ) );
}

inline void MTLCommandBuffer_WaitUntilCompleted( id buffer )
{
  return SendMessage<void>( buffer, sel_getUid( "waitUntilCompleted" ) );
}

enum class MTLCommandBufferStatus : NSUInteger {
  NOT_ENQUEUED = 0,
  ENQUEUED = 1,
  COMMITTED = 2,
  SCHEDULED = 3,
  COMPLETED = 4,
  ERROR = 5
};

inline MTLCommandBufferStatus MTLCommandBuffer_GetStatus( id buffer )
{
  return SendMessage<MTLCommandBufferStatus>( buffer, sel_getUid( "status" ) );
}


inline void* MTLBuffer_Contents( id buffer )
{
  return SendMessage<void*>( buffer, sel_getUid( "contents" ) );
}
} ///< mfg_objc

/*
  MFGのmetal実行時のランタイムのnamespace。
*/
namespace mfg_runtime_metal {

using namespace mfg_objc;
using namespace mfg_binary;
using namespace mfg_runtime_common;

// Mac版Qtのincludeとconflictするので明示的にusing
using mfg_objc::NSObject;

using mfg_internal::Type;
using mfg_internal::TLTensor;
using mfg_msl_converter::RVal32;
using mfg_msl_converter::REnv;
using mfg_internal::FmtSystemError;

struct MetalKernel
{
  id _device;
  NSObject _func;
  NSObject _pipelineState;

  MetalKernel( id device, id func )
    : _device( device ), _func( func ),
      _pipelineState( MTLDevice_NewComputePipelineStateWithFunction( device, func ) )
  {
    if( _func.IsNull())
      throw FmtSystemError( ResId::METAL_CREATION_FAILURE, "Function" );
    
    if (_pipelineState.IsNull())
      throw FmtSystemError( ResId::METAL_CREATION_FAILURE, "PipelineState" );

  }

  NSUInteger GetMaxTotalThreadsPerThreadGroup()
  {
    return MTLComputePipelineState_GetMaxTotalThreadsPerThreadGroup( _pipelineState );
  }

  NSUInteger GetThreadExecutionWidth()
  {
    return MTLComputePipelineState_GetThreadExecutionWidth( _pipelineState );
  }
};

struct MetalLibrary
{
  id _device;
  NSObject _library;

  MetalLibrary( id device, const char* source_cstr )
    : _device( device ),
      _library( MTLDevice_NewLibraryWithSource( _device, source_cstr ) )
  {
    // ほとんどのケースでMTLDevice_NewLibraryWithSourceの中で例外が投げられるのでこれが投げられる事は無いはずだが。
    if (_library.IsNull())
      throw FmtSystemError( ResId::METAL_CREATION_FAILURE, "MetalLibrary" );
  }

  MetalLibrary( id device, const std::string& source ) : MetalLibrary( device, source.c_str() )
  {    
  }

  MetalKernel CreateKernel( const char* fname_cstr )
  {
    return MetalKernel(
            _device,
            MTLLibrary_NewFunctionWithName( _library, fname_cstr )
    );
  }

  MetalKernel CreateKernel( const std::string& kernelName )
  {
    return CreateKernel( kernelName.c_str() );
  }

  MetalKernel CreateKernel( mfg_msl_converter::MSLTensorGenerator& tgen )
  {
    return CreateKernel( tgen.GetKernelName() );
  }

  MetalKernel CreateKernel( mfg_msl_converter::MSLTopLevelBlockGenerator& topGen )
  {
    return CreateKernel( topGen.GetKernelName() );
  }
};

/*
  MTLBufferのラッパクラス。
*/
struct MetalBuffer
{
  NSObject _buffer;
  size_t _byteSize; // UnitTest用

  MetalBuffer( id device, size_t byteSize ) : _buffer( MTLDevice_NewBufferWithLength( device, byteSize ) ), _byteSize( byteSize ) {}

  template<typename T>
  T* Contents()
  {
    return (T*) MTLBuffer_Contents( _buffer );
  }

  operator id() const { return _buffer._id; }

  /*
    マニュアルのリリース。RAIIとして使う分には呼ぶ必要は無い。
  */
  void ManualRelease() { _buffer.ManualRelease(); }

  bool IsReleased() const { return _buffer._id == nullptr; }

  void CopyFromConstTensor( ConstTensor& cts )
  {
    assert( _byteSize/4 == cts._size );

    // 1次元としてアクセスする。accessorは2次元のものを1次元でアクセスしても問題無い前提。
    uint32_t* dest = Contents<uint32_t>();
    for( int x = 0; x < cts._size; x++ )
    {
      dest[x] = cts._accessor.UInt32( x );
    }
  }

  BufferAccessor Accessor( const std::vector<size_t>& extents )
  {
    return BufferAccessor( Contents<void*>(), extents );
  }
};

// 32Bitのサイズのバッファ。
// int32_tもuint32_tもfloatも全部これを使う。
struct Param32Buffer
{
  id _device;
  size_t _size; // パラメータの数
  MetalBuffer _buffer;

  Param32Buffer( id device, size_t size ): _device( device ), _size( size ), _buffer( _device, 4*size )
  {    
  }

  int32_t& Int32( size_t index )
  {
    return _buffer.Contents<int32_t>()[ index ];
  }

  uint32_t& UInt32( size_t index )
  {
    return _buffer.Contents<uint32_t>()[ index ];
  }

  float& Float32( size_t index )
  {
    return _buffer.Contents<float>()[ index ];
  }

  void ManualRelease() { _buffer.ManualRelease(); }
  bool IsReleased() const { return _buffer.IsReleased(); }

  void CopyToEnv( const std::vector<std::pair<Type, std::string>>&varsInfo, REnv& env )
  {
    for (auto i : NRange(varsInfo.size()))
    {
      auto &pair = varsInfo[i];
      auto tp = pair.first;
      auto& name = pair.second;

      if (tp.IsFloat())
      {
        env.Assign( name, { Float32(i) });
      }
      else if(tp.IsInt())
      {
        env.Assign( name, { Int32(i) });
      }
      else if(tp.IsUInt())
      {
        env.Assign( name, { UInt32(i) });
      }
      else
      {
        assert( false );
      }

    }
  }

};

// Tensorに対応するMetalBuffer
// ようするにTupleBufferの事
struct TensorBuffer
{
  id _device;
  std::vector<NumericType> _types;
  std::vector<MetalBuffer> _buffers;

  TensorBuffer( id device, std::vector<NumericType> types, size_t vectorDim, std::vector<size_t> dims ) : _device( device ), _types( std::move(types) )
  {
    size_t elemNum = std::accumulate( dims.begin(), dims.end(), (size_t)1, std::multiplies<size_t>{} );
    for( const auto& type : _types )
    {
      _buffers.emplace_back( device, type.Bytes()*elemNum*vectorDim );
      if (_buffers.back().IsReleased())
        throw FmtSystemError( ResId::GPU_BUFFER_ALLOCATION_FAILURE );
    }
  }
  TensorBuffer( id device, const TensorInfo& tsinfo, std::vector<size_t> dims ) : TensorBuffer( device, tsinfo.GetElemBufferTypes(), tsinfo.VectorExtent(), std::move(dims) )
  {    
  }

  // 2D tuple
  TensorBuffer( id device, TLTensor& ts, std::pair<size_t, size_t> dims ) : TensorBuffer( device, ts.GetTensorInfo(), { dims.first, dims.second } )
  {
  }

  // 1D
  TensorBuffer( id device, TLTensor& ts, size_t extent ) : TensorBuffer( device, ts.GetTensorInfo(), { extent } )
  {
  }

  size_t BufferNum() const { return _buffers.size(); }

  MetalBuffer& operator[]( size_t idx )
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

  void FillZero()
  {
    for( auto& buf: _buffers )
    {
      std::memset( buf.Contents<void*>(), 0, buf._byteSize );
    }
  }

  void CopyFromTile( TileReference& src )
  {
    // ベクトルでなくてはおかしい
    assert( _types.size() == 1 );
    CImageTileToArray::FromTile( _types[0], _buffers[0].Contents<void>(), src );
  }

  void CopyToTile( TileReference& dest )
  {
    // ベクトルでなくてはおかしい
    assert( _types.size() == 1 );

    ArrayToCImageTile::ToTile( dest, _types[0], _buffers[0].Contents<void>() );
  }
};

// 引数のセットアップに使われるsetup-er
// 流れるインターフェース　
struct ExecArgs
{
  NSUInteger _nextIndex = 0;
  id _computeCommandEncoder;
  ExecArgs( id computeCommandEncoder ) : _computeCommandEncoder( computeCommandEncoder ) {}

  ExecArgs& AddBuffer( id buffer )
  {
    MTLComputeCommandEncoder_SetBuffer( _computeCommandEncoder, buffer, 0, _nextIndex++ );
    return *this;
  }

  ExecArgs& AddTensorBuffer( TensorBuffer& bufs )
  {
    for( auto i : NRange( bufs.BufferNum() ))
    {
      AddBuffer( bufs[i] );      
    }
    return *this;
  }

  ExecArgs& AddBytes( const void* buffer, NSUInteger length )
  {
    MTLComputeCommandEncoder_SetBytes( _computeCommandEncoder, buffer, length, _nextIndex++ );
    return *this;
  }

  ExecArgs& AddInts( const std::vector<int32_t>& iparam )
  {
    return AddBytes( iparam.data(), sizeof(int32_t)*iparam.size() );
  }

  ExecArgs& AddParams32( const std::vector<RVal32>& params )
  {
    static_assert( sizeof(RVal32) == 4, "RVal32 only support for 32bit." );
    return AddBytes( params.data(), sizeof(RVal32)*params.size() );
  }
};

// カーネルの実行一回あたりのコンテキスト
// 概念的にはMTLComputCommandEncoderに対応している。
struct KernelExecCommand
{
  MetalKernel &_kernel;
  int32_t _dispatchWidth;
  int32_t _dispatchHeight;
  id _commandBuffer;
  id _computeCommandEncoder;

  KernelExecCommand( MetalKernel& kernel, int32_t dispatchWidth, int32_t dispatchHeight, id commandQueue )
  : _kernel( kernel ), _dispatchWidth(dispatchWidth),  _dispatchHeight( dispatchHeight ),
    _commandBuffer( MTLCommandQueue_CommandBuffer( commandQueue ) ),
    _computeCommandEncoder( MTLCommandBuffer_ComputeCommandEncoder( _commandBuffer ) )
  {
    if (_commandBuffer == nullptr)
      throw FmtSystemError( ResId::METAL_CREATION_FAILURE, "CommandBuffer" );

    if (_computeCommandEncoder == nullptr)
      throw FmtSystemError( ResId::METAL_CREATION_FAILURE, "ComputeCommandEncoder" );

    MTLComputeCommandEncoder_SetComputePipelineState( _computeCommandEncoder, _kernel._pipelineState );
  }

  ExecArgs ArgsSetup()
  {
    return ExecArgs( _computeCommandEncoder );
  }

  NSUInteger GetMaxTotalThreadsPerThreadGroup()
  {
    return _kernel.GetMaxTotalThreadsPerThreadGroup();
  }

  NSUInteger GetThreadExecutionWidth()
  {
    return _kernel.GetThreadExecutionWidth();
  }

  NSUInteger ThreadsNum()
  {
    auto threads = GetMaxTotalThreadsPerThreadGroup();
    return std::min( (NSUInteger)_dispatchHeight, threads );
  }

  void RunAsync( int32_t gridW, int32_t gridH, int32_t groupW, int32_t groupH )
  {
    MTLComputeCommandEncoder_DispatchThreads( _computeCommandEncoder, gridW, gridH, 1, groupW, groupH, 1 );
    MTLComputeCommandEncoder_EndEncoding( _computeCommandEncoder );

    MTLCommandBuffer_Commit( _commandBuffer );
  }

  void RunAsync()
  {
    auto tgWidth = std::min( _dispatchWidth, (int32_t)GetThreadExecutionWidth() );
    auto tgHeight = std::min( _dispatchHeight, (int32_t)GetMaxTotalThreadsPerThreadGroup() / tgWidth );
    RunAsync( _dispatchWidth, _dispatchHeight, tgWidth, tgHeight );
  }

  void WaitUntilCompleted()
  {
    MTLCommandBuffer_WaitUntilCompleted( _commandBuffer );
  }

  void RunBlock()
  {
    RunAsync();
    WaitUntilCompleted();

    // debug
    /*
    id error = GetError();
    if (error != nullptr)
    {
      NSLog_NSObject( error );
    }
    */
  }

  id GetError()
  {
    return MTLCommandBuffer_Error( _commandBuffer );
  }
};

// 指定されたKernelExecCommandでスレッドをdispatchする。
// 概念的にはMTLCommandBufferに対応するもの。
struct MetalDispatcher
{
  id _device;
  NSObject _commandQueue;

  MetalDispatcher( id device )
  : _device( device ),
    _commandQueue( MTLDevice_NewCommandQueue( _device ) )
  {
    if (_commandQueue.IsNull())
      throw FmtSystemError( ResId::METAL_CREATION_FAILURE, "Command Queue" );
  }

  KernelExecCommand CreateExecCommand( MetalKernel& kernel, int32_t dispatchWidth, int32_t dispatcherHeight )
  {
    return KernelExecCommand( kernel, dispatchWidth, dispatcherHeight, _commandQueue );
  }
};


// AutoreleeasePoolやDevice, Dispatcherなどを持つContext。
// これだけはテスト以外でも使える程度の汎用性を持っているのでContextシリーズでもここに置く。
struct MetalContext
{
  AutoreleasePool _pool;
  NSObject _device;
  MetalDispatcher _dispatcher;

  MetalContext() : _device( MTLCreateSystemDefaultDevice() ), _dispatcher( _device )
  {
    if (_device.IsNull())
      throw FmtSystemError( ResId::METAL_CREATION_FAILURE, "Device" );

    if (!MTLDevice_SupportsMFGGPU( _device ))
      throw FmtSystemError( ResId::NON_SUPPORTED_GPU_FAMILY );
  }

  KernelExecCommand CreateExecCommand( MetalKernel& kernel, int32_t dispatchWidth, int32_t dispatcherHeight )
  {
    return _dispatcher.CreateExecCommand( kernel, dispatchWidth, dispatcherHeight );
  }
};

//
// MetalBackend関連
//

using namespace mfg_binary;
using namespace mfg_runtime_common;

using TLEIter = std::vector<TLEntityRef>::iterator;
namespace conv = mfg_msl_converter;

// tensorの名前をキーにMetalBufferを保持する
struct BufferMap
{
  id _device;
  std::map<std::string, TensorBuffer> _tbufMap;
  std::map<std::string, Param32Buffer> _tblockMap;


  BufferMap( id device ) : _device( device ) {}

  // nameがGenの方にしか無いので、Genを引数にする。
  // 名前とかvarsInfoは本来はTopLevelBlockが持つべきなのかもしれない。
  Param32Buffer& SetupTBlockBuf( const conv::MSLTopLevelBlockGenerator& tbgen )
  {
    assert( nullptr == LookupTBlockBuf( tbgen ) );
    auto res = _tblockMap.insert( { tbgen.GetKernelName(), Param32Buffer{ _device, tbgen._varsInfo.size() } } );
    return res.first->second;
  }

  Param32Buffer* LookupTBlockBuf( const conv::MSLTopLevelBlockGenerator& tbgen)
  {
    auto iter = _tblockMap.find( tbgen.GetKernelName() );
    if (iter == _tblockMap.end())
      return nullptr;
    return &iter->second;
  }

  TensorBuffer& SetupTensorBuf( const TensorInfo& tsinfo, std::vector<size_t> dims )
  {
    assert( nullptr == LookupTensorBuf( tsinfo.Name() ) );
    auto res = _tbufMap.insert( { tsinfo.Name(), TensorBuffer{ _device, tsinfo, std::move(dims) } });
    return res.first->second;
  }

  TensorBuffer* LookupTensorBuf( const std::string& tname )
  {
    auto iter = _tbufMap.find( tname );
    if (iter == _tbufMap.end())
      return nullptr;
    return &iter->second;
  }

  void SetupTileBuffer( InputsInfo& iinfo, TLTensor& result, InputTiles& inputTiles, int resultWidth, int resultHeight )
  {
    SetupTensorBuf( result.GetTensorInfo(), { (size_t)resultWidth, (size_t)resultHeight });
    iinfo.ForEachInput( [&]( InputIndex ii, OneInputInfo& one ){
      auto& bufferIn = SetupTensorBuf( one.GetTensorInfo(), { (size_t)resultWidth, (size_t)resultHeight });
      TileReference* oneTile = inputTiles.Lookup( ii._layerIndex );
      if (oneTile == nullptr)
      {
        // 無ければ0fill
        bufferIn.FillZero();
      }
      else 
      {
        bufferIn.CopyFromTile( *oneTile );
      }
    });
  }

  void SetupConstTensor( ConstTensor& cts )
  {
    auto& buffer = SetupTensorBuf( cts.GetTensorInfo(), cts.GetExtents() );
    buffer[0].CopyFromConstTensor( cts );
  }

};


struct LibraryExecutor
{
  MetalContext& _ctx;
  std::ostream& _logStream;
  BufferMap& _bufMap;
  REnv& _renv;
  IRBinary& _binary;
  std::vector<TLEntityRef>& _tles;
  const TLEIter& _libBegin;
  const TLEIter& _libEnd;

  // _libBeginがTLEEntityの何番目か。バッファのlifetimeを表す起点。
  size_t _originIndex;

  bool _isIgnoreUnusedVariable = true;

  std::vector<std::unique_ptr<conv::MSLTensorGenerator>> _tgens;
  std::vector<std::unique_ptr<conv::MSLTopLevelBlockGenerator>> _bgens;

  LibraryExecutor( MetalContext& ctx, std::ostream& logStream, BufferMap& bufMap, REnv& renv, IRBinary& binary, const TLEIter& libBeg, const TLEIter& libEnd, size_t originIndex )
  : _ctx( ctx ), _logStream( logStream ), _bufMap( bufMap ), _renv( renv ), _binary( binary ), _tles( binary._tles ), _libBegin( libBeg ), _libEnd( libEnd ), _originIndex( originIndex ) {}


  id Device() { return _ctx._device; }

  struct GeneratorHolder
  {
    TLEntityRef::TLEType _etype;
    union {
      conv::MSLTensorGenerator *_tgen = nullptr;
      conv::MSLTopLevelBlockGenerator *_bgen;
    } u;

    GeneratorHolder( conv::MSLTensorGenerator* tgen ) : _etype( TLEntityRef::TENSOR )
    {
      u._tgen = tgen;
    }

    GeneratorHolder( conv::MSLTopLevelBlockGenerator* bgen ) : _etype( TLEntityRef::TOP_LEVEL_BLOCK )
    {
      u._bgen = bgen;
    }

    TLRoot& GetTLE()
    {
      switch (_etype)
      {
        case TLEntityRef::TENSOR:
        {
          return u._tgen->_tensor;
        }
        case TLEntityRef::TOP_LEVEL_BLOCK:
        {
          return u._bgen->_topLevelBlock;
        }
        default:
          throw std::runtime_error( "Never reached here." );
      }

    }
  };

  std::vector<GeneratorHolder> _gens;

  std::vector<GeneratorHolder> CreateGenerators( const TLEIter& libBeg, const TLEIter& libEnd )
  {
    std::vector<GeneratorHolder> gens;
    auto cur = libBeg;
    while( cur != libEnd )
    {
      switch (cur->_etype)
      {
        case TLEntityRef::TENSOR:
        {
          auto& ts = *cur->u._tensor;
          _tgens.emplace_back( new conv::MSLTensorGenerator( ts ));
          auto &tgen = _tgens.back();
          gens.emplace_back( tgen.get() );
          break;
        }
        case TLEntityRef::TOP_LEVEL_BLOCK:
        {
          auto& tblock = *cur->u._tblock;
          _bgens.emplace_back( new conv::MSLTopLevelBlockGenerator( tblock ) );
          gens.emplace_back( _bgens.back().get() );
          break;
        }
        case TLEntityRef::UNBOUND:
          assert(false);
          break;
      }
      cur++;
    }
    return gens;
  }

  void SetupGenerator()
  {
    _gens = CreateGenerators( _libBegin, _libEnd );
  }

  /*
    ツリーの中にRandomやsort_int32_1dなどがあるかを調べる。
  */
  LibraryIRInfo CollectIRInfo( const TLEIter& libBeg, const TLEIter& libEnd )
  {
    LibraryIRInfoCollector collector;
    auto cur = libBeg;
    while( cur != libEnd )
    {
      collector.Collect( *cur );
      cur++;
    }
    return collector._info;
  }

  // UnitTest用にここだけ別関数に抜き出す。
  std::string CreateLibrarySource( DebugLogger& debLogger )
  {
    std::stringstream ss;

    if (_isIgnoreUnusedVariable)
    {
      ss << "#pragma clang diagnostic ignored \"-Wunused-variable\"" << std::endl;
    }

    LibraryIRInfo info = CollectIRInfo( _libBegin, _libEnd );
    conv::MSLKernelCommonGenerator::GenLibraryPrologue( ss, info );

    for (auto& genHolder : _gens)
    {
      switch (genHolder._etype)
      {
        case TLEntityRef::TENSOR:
          genHolder.u._tgen->GenKernel( _binary.GetSeedGen() );
          ss << genHolder.u._tgen->ToString();
          break;
        case TLEntityRef::TOP_LEVEL_BLOCK:
          genHolder.u._bgen->CollectGlobalVars();
          genHolder.u._bgen->GenKernel( _binary.GetSeedGen() );
          ss << genHolder.u._bgen->ToString();
          break;
        case TLEntityRef::UNBOUND:
          assert(false);
          break;
      }
    }

    debLogger.Log( "=== Metal Shader Source ===" );
    debLogger.Log( ss.str() );

    return ss.str();
  }

  MetalLibrary CreateLibrary( DebugLogger& debLogger )
  {
    auto src = CreateLibrarySource( debLogger );
    return MetalLibrary( Device(), src );
  }

  TensorBuffer& EnsureBuf( const TensorInfo& ts )
  {
    auto res = _bufMap.LookupTensorBuf( ts.Name() );
    if (res != nullptr)
      return *res;

    auto bounds = _binary.Resolve( ts, _renv );
    return _bufMap.SetupTensorBuf( ts, std::move(bounds) );
  }

  Param32Buffer& EnsureBuf( conv::MSLTopLevelBlockGenerator& tbgen )
  {
    auto res = _bufMap.LookupTBlockBuf( tbgen );
    if (res != nullptr)
      return *res;

    return _bufMap.SetupTBlockBuf( tbgen );
  }

  void AddBoundsToParam( const std::vector<size_t>& bounds, std::vector<RVal32>& params )
  {
    for (auto b: bounds)
    {
      params.emplace_back();
      params.back().u._uival = b;
    }
  }

  ExecArgs& AddTensorRefs( const std::vector<GlobalTensorLike*>& refs, ExecArgs& setup, std::vector<RVal32>& params )
  {
    for (auto pts: refs)
    {
      auto tsinfo = pts->GetTensorInfo();

      auto bounds = _binary.Resolve( tsinfo, _renv );
      _renv.AddExtent( tsinfo, bounds );
      AddBoundsToParam( bounds, params );
      
      setup.AddTensorBuffer( EnsureBuf( tsinfo ) );
    }
    return setup;
  }

  void AddParamRefsToParams( const std::vector<ReferenceParamInfo>& refs, std::vector<RVal32>& params )
  {
    for (auto& vari: refs)
    {
      // 本当はコンパイル時では無く実行時に解決すべきだが、
      // 現状はコンパイルする時には全てのパラメータが与えられた状態なので
      // この場で解決してしまう。
      // 本当はCImageTile32の幅や高さと合わせてこのparamsのセットアップは
      // 実行直前まで遅延すべき。
      params.emplace_back();
      params.back().u = _renv.GetRVal32( vari._name ).u;
    }
  }

  void SetupTLECommonArgs( TLRoot& tle, ExecArgs& setup, std::vector<RVal32>& params )
  {
    AddParamRefsToParams( tle.GetParamRefs(), params );
    AddTensorRefs( tle.GetTensorRefs(), setup, params )
      .AddParams32( params );
  }

  // MTLCommandBuffer_WaitUntilCompletedを呼ぶ為のcommandBufferのidを返す
  id ExecuteTensorKernel( MetalLibrary& lib, conv::MSLTensorGenerator& tgen )
  {
    auto &ts = tgen._tensor;
    auto kernel = lib.CreateKernel( tgen );
    auto tsinfo = ts.GetTensorInfo();
    auto bounds = _binary.Resolve( tsinfo, _renv );
    _renv.AddExtent( tsinfo, bounds );

    assert( bounds.size() == 1 || bounds.size() == 2 );
    auto dimWidth = bounds[0];
    auto dimHeight = bounds.size() >= 2 ? bounds[1] : 1;     

    auto com = _ctx.CreateExecCommand( kernel, dimWidth, dimHeight );
    std::vector<RVal32> params;
    params.emplace_back();
    params.back().u._ival = dimWidth;
    if (bounds.size() == 2)
    {
      params.emplace_back();
      params.back().u._ival = bounds[1];
    }

    auto setup = com.ArgsSetup();
    // IntParamの追加
    setup.AddTensorBuffer( EnsureBuf( ts.GetTensorInfo() ) );
    SetupTLECommonArgs( ts, setup, params );

    com.RunAsync();

    return com._commandBuffer;
  }

  // MTLCommandBuffer_WaitUntilCompletedを呼ぶ為のcommandBufferのidを返す
  id ExecuteTopLevelBlockKernel( MetalLibrary& lib, conv::MSLTopLevelBlockGenerator& topGen )
  {
    auto& tblock = topGen._topLevelBlock;
    auto kernel = lib.CreateKernel( topGen );
    auto com = _ctx.CreateExecCommand( kernel, 1, 1 );
    
    std::vector<RVal32> params;

    auto setup = com.ArgsSetup();

    setup.AddBuffer( EnsureBuf( topGen )._buffer );
    // IntParamの追加
    SetupTLECommonArgs( tblock, setup, params );

    com.RunAsync();

    return com._commandBuffer;
  }

  void ReleaseDeadTensorBuffer( const GlobalTensorLike& tensor, size_t currentTLEPos )
  {
    // referenceされてなければカーネル実行をスキップするので、
    // そもそもバッファはつくられないので何もしない。（前半の条件）
    if( tensor.LastReference() >= 0 && tensor.LastReference() < (int)currentTLEPos)
    {
      auto tsbuf = _bufMap.LookupTensorBuf( tensor.TsName() );
      assert( tsbuf != nullptr );

      tsbuf->ManualRelease();
    }
  }

  void ReleaseDeadBuffers()
  {
    // size()-1までの実行が終わったので次はsize()
    size_t currentTLEPos = _originIndex + _gens.size();

    _binary.ForEachInput([&]( InputIndex, OneInputInfo& one ) {
      ReleaseDeadTensorBuffer( one, currentTLEPos );
    });

    for( auto& genHolder : _gens )
    {
      auto& tle = genHolder.GetTLE();
      for( auto refpTensor : tle.GetTensorRefs() )
      {
          ReleaseDeadTensorBuffer( *refpTensor, currentTLEPos );
      }

      switch (genHolder._etype)
      {
        case TLEntityRef::TENSOR:
        {
          auto& ts = genHolder.u._tgen->_tensor;
          ReleaseDeadTensorBuffer( ts, currentTLEPos );
          break;
        }
        case TLEntityRef::TOP_LEVEL_BLOCK:
        {
          auto paramBuf = _bufMap.LookupTBlockBuf( *genHolder.u._bgen );
          assert( paramBuf != nullptr );
          paramBuf->ManualRelease();          
          break;
        }
        case TLEntityRef::UNBOUND:
          assert(false);
          break;
      }
    }
  }

  // キャンセルされたらfalse、それ以外はtrue、エラーは例外を投げる
  bool WaitPolling( id combuf, TProgressCallback callback )
  {
    int count = 0;
    while( true )
    {
      auto status = MTLCommandBuffer_GetStatus( combuf );
      switch(status)
      {
        case MTLCommandBufferStatus::COMPLETED:
          return true;
        case MTLCommandBufferStatus::COMMITTED:
        case MTLCommandBufferStatus::ENQUEUED:
        case MTLCommandBufferStatus::SCHEDULED:
          break;
        case MTLCommandBufferStatus::ERROR:
        case MTLCommandBufferStatus::NOT_ENQUEUED:
          // 両方ともエラーとする。後者は無いはずだが。
          throw FmtSystemError( ResId::METAL_COMMAND_BUFFER_ERROR, static_cast<int>(status) );
      }

      // キャンセルされた
      if(!CallbackPercent( callback, 0 ))
        return false;
    
      // 最初1msecで5回まち、次に101msecで5回、201msecで5回...と進んでいき、501msec以降はずっと毎回501msecで待つ。
      std::this_thread::sleep_for(std::chrono::milliseconds( 1 + std::min( 100*(count/5), 500 ) ) );
      count++;
    }
  }

  bool ExecuteKernels( MetalLibrary& lib, TProgressCallback callback )
  {
    id combuf = nullptr;
    for( auto& genHolder : _gens )
    {
      switch (genHolder._etype)
      {
        case TLEntityRef::TENSOR:
        {
          // 使われていないテンソルのカーネルはスキップする。
          if (genHolder.u._tgen->_tensor._lastReferenced >= 0)
            combuf = ExecuteTensorKernel( lib, *genHolder.u._tgen );
        }
        break;
        case TLEntityRef::TOP_LEVEL_BLOCK:
          combuf = ExecuteTopLevelBlockKernel( lib, *genHolder.u._bgen );
          break;
        case TLEntityRef::UNBOUND:
          assert(false);
          break;
      }
    }

    // すべてのtensorが使われてないケースではnullptrになる。
    if (combuf != nullptr)
    {
      if (!WaitPolling(combuf, callback))
        return false;
    }

    auto& last = _gens.back();
    if (last._etype == TLEntityRef::TOP_LEVEL_BLOCK)
    {
      auto &topGen = *last.u._bgen;
      auto paramBuf = _bufMap.LookupTBlockBuf( topGen );
      paramBuf->CopyToEnv( topGen._varsInfo, _renv );
      _renv.LogVars( _logStream, topGen._topLevelBlock._logEntries );
    }

    ReleaseDeadBuffers();
    return true;
  }
};

// IRBinaryを実行するクラス
// 将来的にはコンテキストを別持ちにしたいが、まずは全部をこのクラスの寿命に紐づけておく
struct MetalBackend
{
  MetalContext _ctx;
  std::ostringstream _logStream;
  BufferMap _bufMap;
  REnv _renv;
  bool _debug = false;

  // rsumのdestructuring letなどで不要な変数を作るのは正常動作。
  // コンパイルで消えるので基本的には無視する。
  // 使わないletはlowerの最後に削除するのが望ましいが、
  // 今はそういう処理をやっていないのでwarningの方をsuppressする。
  // これを無くしたい時はMFG_IGNORE_METAL_SOURCE_CACHEをdefineして
  // warningが出ないかを確認しながら作業する事。
  bool _isIgnoreUnusedVariable = true;

  DebugLogger _debLogger;
  PerfCounter _counter;

  MetalBackend() : _bufMap( _ctx._device ), _debLogger( _logStream )  /* _debLogger( std::cerr ) */ {}

  id Device() { return _ctx._device; }

  void EnableDebugLog( bool enable ) { _debug = enable; }

  // libraryの境界の一つ次のiterを返す
  // 基本的にはTopLevelBlockの次かend。
  TLEIter LookupLibraryBoundary( std::vector<TLEntityRef>& tles, const TLEIter& beg )
  {
    if (beg == tles.end())
      return tles.end();

    auto cur = beg;
    auto next = beg;
    next++;
    while( next != tles.end() )
    {
      if (cur->_etype == TLEntityRef::TOP_LEVEL_BLOCK)
        return next;
      cur++;
      next++;
    }
    return next;
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

    _bufMap.SetupTileBuffer( target.Input(), target.Result(), inputTiles,  rwidth, rheight );
  }

  DebugLogger& GetDebugLogger()
  {
    _debLogger._debug = _debug;
    return _debLogger;
  }

  bool RunLibrary( mfg_binary::IRBinary& target, const TLEIter& libBeg, const TLEIter& libEnd, size_t originIndex, TProgressCallback callback = nullptr  )
  {
    LibraryExecutor executor( _ctx, _logStream, _bufMap, _renv, target, libBeg, libEnd, originIndex );
    executor._isIgnoreUnusedVariable = _isIgnoreUnusedVariable;

    executor.SetupGenerator();
    auto lib = executor.CreateLibrary( GetDebugLogger() );
    return executor.ExecuteKernels( lib, callback );
  }

  void CopyBackResult( mfg_binary::IRBinary& target, TileReference& resTile )
  {
    auto& bufout = *_bufMap.LookupTensorBuf( target.Result().Name() );
    bufout.CopyToTile( resTile );
  }

  /* UnitTest用 */
  bool IsBufferReleased( const GlobalTensorLike& ts )
  {
    auto pbuf = _bufMap.LookupTensorBuf( ts.TsName() );
    return pbuf->IsReleased();
  }

  void SetupConstTensors( mfg_binary::IRBinary& target )
  {
    target.SetupConstsTSExtents( _renv );
    for( auto& cts : target._constTensors )
    {
      _bufMap.SetupConstTensor( cts );
    }
  }

  bool Run( mfg_binary::IRBinary& target, InputTiles& inputTiles, TileReference& resTile, TProgressCallback callback = nullptr )
  {
    _counter.Reset();
    target.ResetSeedGen();

    _counter.BeginInput();
    SetupInputResult( target, inputTiles, resTile );
    SetupConstTensors( target );
    _counter.EndInput();

    auto libBeg = target._tles.begin();
    auto originItr = libBeg;

    _counter.BeginKernel();
    bool success = true;
    while ( success && (libBeg != target._tles.end()) )
    {
      auto libEnd = LookupLibraryBoundary( target._tles, libBeg );
      success = RunLibrary( target, libBeg, libEnd, (size_t)(libBeg - originItr), callback );

      libBeg = libEnd;
    }
    _counter.EndKernel();

    if (success)
    {
      _counter.BeginResult();
      CopyBackResult( target, resTile );
      _counter.EndResult();
    }

    return success;
  }

  std::string DrainLogs()
  {
    auto ret = _logStream.str();
    _logStream.str( std::string() );
    return ret;
  }

  const mfg::PerfInfo& GetPerfInfo() const { return _counter._info; }
};



//////////////////////////////////////////////
// Unit Test用のコンテキスト関連の実装
//
// RAIIでMetal関連のコンテキストを持つ。
//
// コンテキストは包含関係になっていて、下に行くほどいろいろな事を決め打ち多くを持つ。
// - MetalContext ... 一番一般的なコンテキスト(AutoreleasePoolとMTLDeviceとDispatcherだけ持つ)
// - MetalTestLibraryContext ... ライブラリが一つなのを決め打ち
// - MetalTestKernelContext ... ライブラリとカーネルが一つなのを決め打ち
//
// コンテキスト系列は今の所、あくまで便利クラスであって無しでも使えるようにしてく。
// (将来的にAutoreleasePoolなどの持ち方はもっと上で持つかもしれないので)
//
// mfg_metal_test.cppとmfg_test.cppの両方で使いたいのでここに置く。
///////////////////////////////////////////////


// ライブラリとMetalContextの両方を持つコンテキスト
struct MetalTestLibraryContext
{
  MetalContext _mctx;
  id _device;
  MetalLibrary _lib;

  MetalTestLibraryContext( const std::string& libSrc ) : _device( _mctx._device), _lib( _device, libSrc ) {}

  MetalKernel CreateKernel( const std::string& kernelName )
  {
    return _lib.CreateKernel( kernelName );
  }

  MetalKernel CreateKernel( mfg_msl_converter::MSLTensorGenerator& tgen )
  {
    return _lib.CreateKernel( tgen );
  }

  KernelExecCommand CreateExecCommand( MetalKernel& kernel, int32_t dispatchWidth, int32_t dispatcherHeight )
  {
    return _mctx.CreateExecCommand( kernel, dispatchWidth, dispatcherHeight );
  }
};

// カーネル一つとライブラリとMetalContextを含むコンテキスト
struct MetalTestKernelContext
{
  MetalTestLibraryContext _lctx;
  id _device;
  MetalKernel _kernel;
  KernelExecCommand _command;

  MetalTestKernelContext(mfg_msl_converter::MSLTensorGenerator& tgen, size_t xWidth, size_t yHeight ) : _lctx( tgen.ToString() ), _device( _lctx._device ), _kernel( _lctx.CreateKernel( tgen ) ), _command( _lctx.CreateExecCommand( _kernel, xWidth, yHeight ) ){}

  ExecArgs ArgsSetup() { return _command.ArgsSetup(); }

  void RunBlock() { _command.RunBlock(); }
};

} ///< mfg_runtime_metal


//
// Metalをデフォルトとするためのコードを以下に置く
//

#include "mfg_runtime_common.hpp"

namespace mfg {

// インテグレート向けのfacade、現状はMetalをデフォルトとする。
using MFG = MFGFacade<mfg_runtime_metal::MetalBackend>;


}///< mfg

#endif

