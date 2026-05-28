/* -*- coding: utf-8 -*- マルチバイト */

// Copyright 2026 Kazuma Arino. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#ifndef MFG_TENSOR_HPP_
#define MFG_TENSOR_HPP_


/*
    Tensor関連を置くヘッダファイル。

    ツリー自身の構成要素にはならず、それを保持したり操作したりするレイヤ。
    ランタイムの種類には依存せず共通で使われる。
*/


#include <mfg_pal/pal.hpp>
#include "mfg_elem.hpp"
#include "mfg_ir_util.hpp"
#include "mfg_lower.hpp"
#include <algorithm> // std::max等
#include <numeric> // accumulate等
#include <functional>

namespace mfg_internal
{
using namespace mfg_pal;

// TLTensorとOneInputInfoの共通のインターフェース
struct GlobalTensorLike
{
  // IRBinaryの何番目のentityから参照されているか。
  virtual void Referenced( size_t referencedIndex ) = 0;
  virtual int LastReference() const = 0;
  virtual TensorInfo GetTensorInfo() const = 0;

  std::string TsName() const { return GetTensorInfo().Name(); }
};

// TopLevelElementの境界を越えて依存するものを保持する。
struct Reference
{
  enum ReferenceType
  {
    TENSOR_LIKE,
    VARIABLE
  };

  union
  {
    GlobalTensorLike *_tslike = nullptr;
    Variable *_variable;
  } u;

  ReferenceType _rtype;

  Reference( GlobalTensorLike* input )
  {
    _rtype = TENSOR_LIKE;
    u._tslike = input;
  }
  Reference( Variable* variable )
  {
    _rtype = VARIABLE;
    u._variable = variable;
  }

  Reference( const Reference& ) = default;
  Reference& operator=( const Reference& ) = default;

  // templateで使う用
  bool IsEqual( GlobalTensorLike* input )
  {
    if (_rtype != TENSOR_LIKE)
      return false;
    return u._tslike == input;
  }

  bool IsEqual( Variable* variable )
  {
    if (_rtype != VARIABLE)
      return false;
    return u._variable == variable;
  }
};

/*
  Tensorの依存する外部パラメータ。
  Variableと同じものだが、extentなど保持する場所が無いものもあるので実体を持てるように別構造体に。
*/
struct ReferenceParamInfo
{
  enum ParamInfoType
  {
    PARAM_INT32,
    PARAM_UINT32,
    PARAM_FLOAT32
  };

  ParamInfoType _ptype;
  std::string _name;

  ReferenceParamInfo( ParamInfoType ptype, const std::string& name ) : _ptype( ptype ), _name( name ) {}

  static ParamInfoType ToParamInfoType( Type tp )
  {
    assert( tp.IsFloat() || tp.IsInt() || tp.IsUInt() );
    if (tp.IsFloat())
    {
      return PARAM_FLOAT32;
    }
    else if (tp.IsUInt())
    {
      return PARAM_UINT32;
    }
    else
    {
      return PARAM_INT32;
    }
  }
};

struct ReferenceVectorSet : public std::vector<Reference>
{
  // RefernceVectorSetは参照がある都度追加していくが、
  // 一度追加が終われば以後は参照されるだけになる。
  // 参照する時にextentの重複などをzapする必要があるが、
  // zapしたあとに追加されると都合が悪いため、それができないというのを
  // コード上で表現するために一度でも参照が取得されたらfreezeという状態として
  // 以後追加を禁止する。
  bool _freezed = false;
  std::vector<GlobalTensorLike*> _refTensors;
  std::vector<ReferenceParamInfo> _refParams;

  // TはGlobalTensorLikeかVariable
  template<typename T>
  void Add( T* input )
  {
    assert( !_freezed );

    for (auto& ref : (*this))
    {
      if (ref.IsEqual( input ))
        return;
    }
    emplace_back( input );
  }

  void EnsureFreeze()
  {
    if (_freezed)
      return;
    
    _freezed = true;
    auto zapped = ZapDuplicateExtent();

    for (auto& ref: zapped)
    {
      if (ref._rtype == Reference::TENSOR_LIKE)
      {
        auto tsptr = ref.u._tslike;
        _refTensors.push_back( tsptr );
        auto tsinfo = tsptr->GetTensorInfo();
        for( auto i : NRange( tsinfo.Dimensions() ))
        {
          _refParams.emplace_back( ReferenceParamInfo::PARAM_INT32, tsinfo.GetExtentName(i) );
        }        
      }
      else
      {
        assert( ref._rtype == Reference::VARIABLE );
        auto varptr = ref.u._variable;
        auto ptype = ReferenceParamInfo::ToParamInfoType( varptr->_type );

        _refParams.emplace_back( ptype, varptr->_name );
      }
    }
  }

  std::vector<GlobalTensorLike*>& GetTensorRefs()
  { 
    EnsureFreeze();
    return _refTensors;
  }

  std::vector<ReferenceParamInfo>& GetParamRefs()
  {
    EnsureFreeze();
    return _refParams;
  } 


  /*
    extentへの参照とtsへの参照が両方あると、
    大抵のバックエンドではtsへの参照でextentも生成する為重複定義となる。
    そこでtsへの参照がある時はextentの参照を除去するメソッド。
  */
  ReferenceVectorSet ZapDuplicateExtent() const
  {
    ReferenceVectorSet ret;
    std::set<std::string> varName;

    // tensorのextentを集める(extentのrefはtensorの参照があったら重複する事になるので抜く)
    for (const auto& ref : (*this))
    {
      if (ref._rtype == Reference::TENSOR_LIKE)
      {
        auto tsptr = ref.u._tslike;
        auto tsinfo = tsptr->GetTensorInfo();
        for( auto dim : NRange( tsinfo.Dimensions() ))
        {
          varName.insert( tsinfo.GetExtentName( dim ) );
        }
      }
    }

    for (const auto& ref : (*this))
    {
      if (ref._rtype == Reference::VARIABLE)
      {
        if (varName.find(ref.u._variable->_name) == varName.end())
        {
          varName.insert( ref.u._variable->_name );
          ret.push_back( ref );
        }
      }
      else
      {
        ret.push_back( ref );
      }
    }

    return ret;
  }

  void CallTensorReferenced( size_t refIndex )
  {
    // GetTensorRefsで呼ばれるがこの関数がFreeze前提なのをわかるようにここで呼ぶ。
    EnsureFreeze();
    for (auto pts : GetTensorRefs())
    {
      pts->Referenced( refIndex );
    }
  }
};

/*
  TLBlockとTLTensorの共通部分。
  外部へのリファレンスなど。

  IRElemのルートとなるトップレベルのEntity。
*/
struct TLRoot
{
  // 外部のTensor、外部のグローバル変数、InputXXXなどの外部で定義されるフリーVariable。
  ReferenceVectorSet _refs;

  std::vector<GlobalTensorLike*>& GetTensorRefs()
  {
    return _refs.GetTensorRefs();
  }

  std::vector<ReferenceParamInfo>& GetParamRefs()
  {
    return _refs.GetParamRefs();
  }

  // TはOneInputInfoかVariableかTensor
  template<typename T>
  void AddReference( T& input )
  {
    _refs.Add( &input );
  }

  void CallTensorReferenced( size_t refIndex )
  {
    _refs.CallTensorReferenced( refIndex );
  }

  GlobalTensorLike* LookupTensor( const std::string& tname ) const 
  {
    for (auto& ref : _refs)
    {
      if (ref._rtype == Reference::TENSOR_LIKE)
      {
        if (ref.u._tslike->TsName() == tname)
          return ref.u._tslike;
      }
    }
    return nullptr;
  }

  // このTopLevelが保持するIRElemのルートを返す。
  // TensorもTopLevelBlockもBodyElem
  virtual BodyElem* GetRootElem() = 0;

  virtual void Lower( InlineFuncRegistry& ifr ) = 0;

  template<typename T>
  T* FindElem()
  {
    auto root = GetRootElem();
    for( auto iter = root->_node->begin(); iter != root->_node->end(); iter++ )
    {
      if( iter.IsLeading() )
      {
        auto& elem = iter.GetContent();
        auto elemType = elem->_elemType;
        if (T::_elemType == elemType)
          return elem->As<T>();
      }
    }
    return nullptr;
  }

  /*
    グローバルへの参照で、このTLEntity内に定義が無いもの。
    カーネルとしては引数のparamで渡されなくてはいけないもの一覧。
    TLTensorへの参照とグローバル変数への参照
  */
  void CollectFreeRef( std::function<GlobalTensorLike*(const std::string&)> tslookup )
  {
    auto root = GetRootElem();
    for( auto iter = root->_node->begin(); iter != root->_node->end(); iter++ )
    {
      if( iter.IsLeading() )
      {
        auto& elem = iter.GetContent();
        auto elemType = elem->_elemType;
        if (elemType == IRElemType::Variable)
        {
          auto v = iter.GetContent()->As<Variable>();
          assert( v != nullptr );
          if (v->_freeV)
          {
            AddReferenceGV( *v );
          }
        }
        else if(elemType == IRElemType::SamplerCall)
        {
          auto sc = iter.GetContent()->As<SamplerCall>();
          MayAddTsRef( sc->GetDef()->_target, tslookup );
        }
        else if (elemType == IRElemType::TensorCall)
        {
          // Lowerが終わっているのでここには来ないはず。
          // このブロック自体不要のはずだが、
          // 勘違いしてコードを書いたくらいややこしいのでドキュメントの意味を込めてassertしておく。
          assert(false);
          throw InternalError("Never reached here");
        }
        else if (elemType == IRElemType::LoadExpr)
        {
          auto load = iter.GetContent()->As<LoadExpr>();
          MayAddTsRef( load->GetTensorInfo(), tslookup );
        }

      }
    }
  }

  void MayAddTsRef( const TensorInfo& tsinfo, std::function<GlobalTensorLike*(const std::string&)> tslookup )
  {
    if (!tsinfo.IsLocal())
    {
      auto tl = tslookup( tsinfo._name );
      assert( tl!= nullptr );
      AddReference( *tl );
    }
  }

  /*
    グローバル変数への参照の追加
  */
  void AddReferenceGV( Variable& vari )
  {
    // グローバル変数のタプルへの参照
    // 展開した子供の変数への参照とする。
    // つまりvへの参照があったら
    // v.0, v.1, ...への参照とする。
    if (vari._type.IsTuple())
    {
      auto items = vari.ExpandTuple<ExprV>();
      for (auto& item: items)
      {
        AddReference( *item.GetElem() );
      }
    }
    else
    {
      AddReference( vari );
    }
  }
};

// TopLevelのTensor。
// シェーダー環境ではカーネルと対応するテンソルであると同時に、
// プログラム的なIRElemツリーに対するインターフェースにもなっている。
struct TLTensor final : public TLRoot, public GlobalTensorLike
{
  std::string _name;
  size_t _dim;

  ExprT<TensorElem> _block;
  int _lastReferenced = -1;

  TLTensor( const std::string &name, size_t dim ) : _name( name ), _dim( dim )
  {
  }

  TLTensor( TLTensor&& src ) = default;

  const TensorElem* GetTensorElem() const { return _block.GetElem(); }
  TensorElem* GetTensorElem() { return _block.GetElem(); }

  const std::string &Name() const { return _name; }
  Type GetType() const
  {
    if (_block.IsDefined())
      return GetTensorElem()->GetReturnType();
    
    // テンソルの定義途中でまだ完成してないケース。
    // to_ncoordなどが呼ばれた時点ではreturnのtypeがまだ決まってないのでこの型は不定。
    // だがこのreturn typeはto_ncoordでは使われないので何かが変えればなんでも良い。
    // UInt32を返しておく。
    return UInt(32);
  }  


  /*
    GlobalTensorLikeの実装はじまり
  */
  void Referenced( size_t referenceIndex ) override
  {
    _lastReferenced = std::max( _lastReferenced, (int)referenceIndex );
  }
  int LastReference() const override { return _lastReferenced; }

  TensorInfo GetTensorInfo() const override
  {
    return TensorInfo(_name, false, _dim, GetType() );
  }
  /*
    GlobalTensorLikeの実装終わり
  */

  std::string LoopVarName( int index ) const
  {
    return GetTensorElem()->_args[index].first;
  }


  // argsをVariableのExprのvectorとして返す
  std::vector<Expr> ArgVars() const
  {
    IRBuildDSL d;
    return MapFn( GetTensorElem()->_args, [&d]( const std::pair<std::string, Type>& cur ){ return (Expr)d._Var( cur ); } );
  }



  // このtensor定義のblockのstmt部分を取得（必ずBodyElem）
  mfg_internal::BodyElem*
  GetBody()
  {
    return GetTensorElem()->GetBody();
  }

  bool IsDefined() const { return _block.IsDefined(); }

  void Define( ExprT<TensorElem>&& fobj )
  {
    assert( !IsDefined() );  
    _block.Attach( fobj.Detach() );
  }

  void Define( std::vector<std::string>&& args, ExprBODY&& vblock )
  {
    auto argPairs = MapFn( args, [](const std::string& name){ return std::make_pair( name, mfg_internal::Int(32) ); } );
    Define( ExprT<mfg_internal::TensorElem>( new mfg_internal::TensorElem( std::move(argPairs), std::move(vblock) ) ) );
  }

  BodyElem* GetRootElem() override
  {
    return GetBody();
  }

  void Lower( InlineFuncRegistry& ifr ) override
  {
    auto tlelem = GetTensorElem();
    auto prevBody = tlelem->DetachBody();
    auto newBody = LowerAll( std::move(prevBody), ifr );
    tlelem->AttachBody( std::move(newBody) );
  }
};

/*
  LogEntry:

  @print_expr(XXX)
  の情報を保持。

  @print_expr(thetaImg(255, 312))
  などは、

  let tmp_var = thetaImg(255, 312)
  @print_expr(tmp_var)

  に変換されて、LogEntryとしてはtmp_varの名前を文字列で保持するだけ。
  tmp_varはTLBlock::CollectGlobalVarsInfoに必ず含まれる
  （将来的にはREnvに入ってて参照しただけの場合にはホストに書き戻す必要が無いケースもあるが、まずはそういうケースでも簡単の為一時変数に代入してそれをREnvに書き戻す事にする）
*/
struct LogEntry
{
  Type _vtype;
  std::string _varName;

  LogEntry( Type vtype, const std::string& varName ) : _vtype(vtype), _varName( varName ) {}  
};

/*
  TLBlock:

  トップレベルのブロック。

  テンソル定義とテンソル定義の間のグローバルな式を表す。
  基本的にはletの集まりで、ここで定義された変数はテンソル定義の中で参照できるようにバックエンドがする。

  GPGPUではこのブロックも1x1のカーネルとなり、終わった後にパラメータバッファに結果を書き戻して、
  それをホスト側のREnvに退避する事で以後のテンソルなどで参照できるようにする。
*/
struct TLBlock final : public TLRoot
{
  ExprT<BodyElem> _block;
  std::vector<LogEntry> _logEntries;

  void Define( ExprT<BodyElem>&& block )
  {
    _block = std::move( block );
  }

  // let１文だけの　top level blockを簡単の為サポートしたい事があるので、その為の口。
  // ExecTensorInitializeの場合もあるのでExprLETではなくExprに。
  void DefineOne( Expr&& oneLet )
  {
    std::vector<Expr> vec;
    vec.push_back( std::move(oneLet) );
    _block = ExprT<BodyElem>( new BodyElem( std::move(vec) ) );
  }

  // このTLBlockを表すBodyElem、つまりIRElemのルートを返す。
  BodyElem* GetIRBody() { return _block.GetElem(); }

  std::vector<std::pair<Type, std::string>> CollectGlobalVarsInfo()
  {
    std::vector<std::pair<Type, std::string>> varsInfo;
    auto voidBlock = GetIRBody();
    voidBlock->GetExprs()->ForEach<BaseExprElem>( [&varsInfo]( BaseExprElem* child ){
      auto childLet = child->As<Let>();
      if (childLet != nullptr && childLet->IsPublic() && childLet->GetValueType().IsNumeric() )
      {
        varsInfo.emplace_back( childLet->GetValueType(), childLet->_name );          
      }
    });
    return varsInfo;

  }

  BodyElem* GetRootElem() override
  {
    return GetIRBody();
  }

  void Lower( InlineFuncRegistry& ifr ) override
  {
    _block = LowerAll( std::move(_block), ifr );
  }

  void AddLogEntry( Type vtype, const std::string& varName )
  {
    _logEntries.emplace_back( vtype, varName );
  }

  
};

// BufferAccessor
// 連続のメモリ領域に、読みやすいaccessorを提供
struct BufferAccessor
{
  void *_buf;
  std::vector<size_t> _extents;

  BufferAccessor( void* buf, const std::vector<size_t>& extents ) : _buf( buf ), _extents( extents ) {}

  size_t Stride( int dim ) const 
  {
    int32_t stride = 1;
    for( auto i : NRange( dim + 1 ) )
    {
      if (i != 0)
      {
        stride = stride * _extents[ i - 1 ];
      }
    }
    return stride;
  }

  template<typename... ARGS>
  ptrdiff_t OffsetOf( int d, int index, ARGS... rest ) const
  {
    return OffsetOf( d+1, rest... ) + Stride( d ) * index;
  }

  ptrdiff_t OffsetOf( int /* d */ ) const { return 0; }

  template<typename RT, typename... ARGS>
  RT& At( int index, ARGS... rest )
  {
    return *(((RT*)_buf) + OffsetOf( 0, index, rest... ));
  }

  template<typename... ARGS>
  int32_t& Int32( ARGS... args )
  {
    return At<int32_t>( std::forward<ARGS>(args)... );
  }

  template<typename... ARGS>
  float& Float32( ARGS... args )
  {
    return At<float>( std::forward<ARGS>(args)... );
  }

  template<typename... ARGS>
  uint32_t& UInt32( ARGS... args )
  {
    return At<uint32_t>( std::forward<ARGS>(args)... );
  }
};

// テンソルリテラルなどの定数のテンソル。
// 値もCPU上のヒープ上に持つ
struct ConstTensor : public GlobalTensorLike
{
  // RVal32のどれか
  Type _type;
  size_t _size; // 連続領域のサイズ

  // 32ビットの何か。値はアクセサ越しにアクセスするのでサイズさえあっていれば型はなんでも良い。
  std::vector<uint32_t> _data;
  BufferAccessor _accessor;

  std::string _name;
  int _lastReferenced = -1;

  explicit ConstTensor( Type t, const std::string &name, std::vector<uint32_t>&& data, const std::vector<size_t>& extents ) : _type(t), _size(std::accumulate( extents.begin(), extents.end(), (size_t)1, std::multiplies<size_t>{} ) ), _data( std::move(data) ), _accessor( _data.data(), extents ), _name( name )
  {
  }

  explicit ConstTensor( const std::string &name, const mfg_pal::Image32& src ) : _type( U8V4Type() ), _size( src.Width()*src.Height() ), _data( _size ), _accessor( _data.data(), { (size_t)src.Width(), (size_t)src.Height() } ), _name( name )
  {
    auto guard = PixelLocker( &src );
    for( auto y : NRange(src.Height()) )
    {
      for( auto x : NRange(src.Width()) )
      {
        _accessor.UInt32(x, y) = src.PixelGet( x, y ).Value;
      }
    }
  }

  ConstTensor( ConstTensor&& ) = default;

  const std::string &Name() const { return _name; }

  const std::vector<size_t>& GetExtents() const { return _accessor._extents; }

  /*
    GlobalTensorLikeの実装
  */  
  void Referenced( size_t referenceIndex ) override
  {
    _lastReferenced = std::max( _lastReferenced, (int)referenceIndex );
  }
  int LastReference() const override { return _lastReferenced; }

  TensorInfo GetTensorInfo() const override
  {
    return TensorInfo( _name, false, _accessor._extents.size(), _type );
  }

};

// input_u8とinput_u16のどちらが参照されたか。
// reuslt_u8とresult_u16もこれを使う。
enum class IOTensorType
{
  UNSPECIFIED,
  TENSOR_U8,
  TENSOR_U16
};

/*
  [BASE]が"input"か"result"として
  [BASE]_u8か[BASE]_u16の文字列にマッチする。
*/
struct IOTypeMatcher
{
  std::string _base; // "input" か "result"

  std::string _u8name; // input_u8かresult_u8
  std::string _u16name;

  IOTypeMatcher( const std::string& base ) : _base( base ), _u8name( base + "_u8" ), _u16name( base + "_u16" ) {}

  /*
    nameがマッチするかを調べるメソッド。
    一度マッチしたら以後はその型で確定したとみなす。

    [BASE]_u8か[BASE]_u16ならそれぞれのIOTensorTypeを返す。それ以外ならUNSPECIFIEDを返す。
    [BASE]はinputかresult。
  */
  IOTensorType MatchType( const std::string& name )
  {
    if (name == _u8name)
    {
      return IOTensorType::TENSOR_U8;
    }
    else if(name == _u16name)
    {
      return IOTensorType::TENSOR_U16;
    }
    return IOTensorType::UNSPECIFIED;
  }
};

/*
  input_u8とinput_u16を表すテンソル。
  layerIndexもmangleされて、input_u8_m1とかになる。なお、layerIndexが0の時はinput_u8と0がつかない。
*/
struct OneInputInfo : public GlobalTensorLike
{
  int _lastReferenced = -1;
  IOTensorType _tensorType;
  TensorInfo _tsinfo;

  OneInputInfo( IOTensorType ttype, const std::string& tsname, Type tp  ) : _tensorType( ttype ), _tsinfo( tsname, false, 2, tp ) {}
  
  static std::string Suffix( int layerIndex )
  {
    if (layerIndex == 0)
      return "";
    else if(layerIndex < 0)
    {
      return std::string("_m") + std::to_string( -layerIndex );
    }
    else
    {
      return std::string("_") + std::to_string( layerIndex );
    }
  }

  static OneInputInfo Create( IOTensorType ttype, int layerIndex )
  {
    switch (ttype)
    {
      case IOTensorType::TENSOR_U8:
        return OneInputInfo( ttype, "input_u8" + Suffix( layerIndex ), U8V4Type() );
      case IOTensorType::TENSOR_U16:
        return OneInputInfo( ttype, "input_u16" + Suffix( layerIndex ), U16V4Type() );
      default:
        throw InternalError( "Wrong IOTensorType for OneInputInfo creation. Should never happens." );
    }
  }

  /*
    GlobalTensorLikeの実装
  */  
  void Referenced( size_t referenceIndex ) override
  {
    _lastReferenced = std::max( _lastReferenced, (int)referenceIndex );
  }
  int LastReference() const override { return _lastReferenced; }

  TensorInfo GetTensorInfo() const override
  {
    return _tsinfo;
  }
};

struct InputIndex
{
  int _layerIndex; // currentを0として相対的な位置。上が+1で下が-1。
  IOTensorType _tensorType;

  InputIndex( int layerIndex, IOTensorType ttype ) : _layerIndex(layerIndex), _tensorType(ttype) {}
};

inline bool operator <( const InputIndex& l, const InputIndex& r )
{
  if (l._layerIndex != r._layerIndex)
    return l._layerIndex < r._layerIndex;
  return l._tensorType < r._tensorType;
}



struct InputsInfo
{
  std::map<InputIndex, OneInputInfo> _inputMap;
  std::map<std::string, OneInputInfo*> _created;
  IOTypeMatcher _matcher;

  InputsInfo() : _matcher( "input" ) {}

  // FはF( const InputIndex&, OneInputInfo& ) 型の関数。
  template<typename F>
  void ForEachInput( F fun )
  {
    for( auto iter: _inputMap )
    {
      fun( iter.first, iter.second );
    }
  }

  // FはF( InputIndex, const OneInputInfo& ) 型の関数。
  template<typename F>
  void ForEachInput( F fun ) const
  {
    for( auto iter: _inputMap )
    {
      fun( iter.first, iter.second );
    }
  }

  /*
    input_u8かinput_u16ならそのOneInputInfoのポインタを、それ以外ならnullptrを返す。
    input_u8[-1]などの時も対応。layerIndexはこの場合は-1。
  */
  OneInputInfo* Lookup( const std::string& name, int layerIndex )
  {
    IOTensorType mtype = _matcher.MatchType( name );
    if (mtype == IOTensorType::UNSPECIFIED)
      return nullptr;
    
    InputIndex index( layerIndex, mtype );
    auto iter = _inputMap.find( index );
    if (iter == _inputMap.end())
    {
      auto res = _inputMap.emplace( index, OneInputInfo::Create( mtype, layerIndex ) );
      auto& one = res.first->second;
      _created.emplace( one.TsName(), &one );
      return &one;
    }
    return &iter->second;
  }

  /*
    これまでに作られたinput_XX[YYY]か、input_u8かinput_u16なら
    そのOneInputInfoのポインタを、それ以外ならnullptrを返す。
  */
  OneInputInfo* Lookup( const std::string& name )
  {
    auto iter = _created.find( name );
    if (iter != _created.end())
      return iter->second;
    return Lookup( name, 0 );
  }

};

// resultのテンソルを保持する構造体
// 作られた時点ではresult_u8かresult_u16か確定していないのでインスタンスは無い。
struct ResultTensorHolder
{
  std::unique_ptr<TLTensor> _result;
  IOTypeMatcher _matcher;
  IOTensorType _tensorType;

  ResultTensorHolder() : _matcher( "result" ), _tensorType( IOTensorType::UNSPECIFIED ) {}

  bool IsDefined() const { return _result.get() != nullptr; }

  /*
    "result_u8"か"result_u16"の時はTLTensorを、それ以外はnullptrを返す。
    一度u8かu16のどちらかにマッチしたら、以後マッチしてない方はnullptrを返すようになる。
  */
  TLTensor* Lookup( const std::string& name )
  {
    if (IsDefined())
    {
      if (name == _result->TsName())
        return _result.get();
      else
        return nullptr;
    }

    _tensorType = _matcher.MatchType( name );
    switch( _tensorType )
    {
      case IOTensorType::TENSOR_U8:
      {
        _result.reset( new TLTensor("result_u8", 2 ) );
        return _result.get();
      }
      case IOTensorType::TENSOR_U16:
      {
        _result.reset( new TLTensor("result_u16", 2 ) );
        return _result.get();
      }
      case IOTensorType::UNSPECIFIED:
      default:
        return nullptr;
    }
  }

  TLTensor* GetTensor() { return _result.get(); }
};

}///< mfg_internal

#endif

