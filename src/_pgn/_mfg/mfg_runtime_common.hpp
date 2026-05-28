/* -*- coding: utf-8 -*- マルチバイト */

// Copyright 2026 Kazuma Arino. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#ifndef MFG_RUNTIME_COMMON_HPP_
#define MFG_RUNTIME_COMMON_HPP_

#include <mfg_pal/pal.hpp>

#include "mfg_elem.hpp"
#include "mfg_tensor.hpp"

#include "mfg_ir_util.hpp"

#include "mfg_binary.hpp"

#include <istream>

/*
  MetalやD3Dなどのランタイムを作るのに共通で使えるコードを置く場所。
*/
namespace mfg_runtime_common
{
using namespace mfg_binary;
using namespace mfg_pal;

struct InputTiles
{
  // layerのindexとタイルのmap。
  std::map<int, TileReference> _inputMap;

  // TILEはmfg_pal::Image32かmfg_pal::Image64
  template<typename TILE>
  void Insert( int layerIndex, TILE* tile )
  {
    _inputMap.emplace( layerIndex, TileReference( tile ) );
  }

  /*
    現在のレイヤーからの相対位置のindexでTileReferenceを取り出す。
    存在しなかったり未サポートのレイヤー形式だったらnullptrが返る。
  */
  TileReference* Lookup( int index )
  {
    auto iter = _inputMap.find( index );
    if (iter == _inputMap.end())
      return nullptr;

    return &iter->second;
  }
};

/*
  乱数やsortなど、prologueで生成する必要がある要素を持つかどうかを調べるクラス群。
  LibraryIRInfoCollectorを使って調べて、LibraryIRInfoが結果。
*/

struct LibraryIRInfo
{
  bool _useRandom = false;
  bool _useSort = false;
};

struct RandomFinder
{
  bool IsRandom( IRElem* elem )
  {
    auto call = elem->As<Call>();
    if (call == nullptr)
      return false;
    return call->_funcType == Call::RAND_F32;
  }

  bool HasRandom( IRElem* root )
  {
    if (IsRandom( root ))
      return true;

    bool res = false;
    root->ForEachLeadingElem<Call>( [this, &res]( Call* op ){
      if (IsRandom( op ))
        res = true;
    });
    return res;
  }

  bool HasRandom( TLTensor& ts )
  {
    return HasRandom( ts.GetBody() );
  }

  bool HasRandom( TLBlock& tblock )
  {
    return HasRandom( tblock.GetIRBody() );
  }
};

// sort_int32_1dなどがあるかどうか。
struct SortFinder
{
  bool IsSort( const IRElem* elem )
  {
    auto call = elem->As<CallStmt>();
    if (call == nullptr)
      return false;

    // 今のところsort_int32_1dしか無いが、
    // floatはサポートするかもしれないので入れておく（metalでは生成はされているので）
    return call->_name == "sort_int32_1d" ||
            call->_name == "sort_float32_1d";
  }
};

struct LibraryIRInfoCollector
{
  LibraryIRInfo _info;
  
  void Collect( const TLEntityRef& ref )
  {
    switch( ref._etype )
    {
      case TLEntityRef::UNBOUND:
        break;
      case TLEntityRef::TENSOR:
      case TLEntityRef::TOP_LEVEL_BLOCK:
        Collect( ref.AsTopLevel()->GetRootElem() );
        break;
    }
  }

  void Collect( BodyElem* root )
  {
    RandomFinder rfinder;
    SortFinder sfinder;

    for( auto iter = root->_node->begin(); iter != root->_node->end(); iter++ )
    {
      if( iter.IsLeading() )
      {
        auto& elem = iter.GetContent();
        if (rfinder.IsRandom( elem.get() ))
        {
          _info._useRandom = true;
        }
        if (sfinder.IsSort( elem.get() ))
        {
          _info._useSort = true;
        }
      }
    }
  }
};

/*
  D3DやMetalなどのGPUバッファへの転送に関するutility.
*/

/*
   mfg_pal::Image32やmfg_pal::Image64から、連続領域のバッファへのコピー
   FromTileが外部から呼ばれるメソッド
*/
struct CImageTileToArray
{
  // TBppXXXからuint32_tやuint64_tへと変換するコンバーター
  template<typename TBPP>
  struct SameBppConv
  {
    static auto Value( const TBPP& src ) -> decltype(src.Value)
    {
      return src.Value;
    }
  };
  struct MakeSmallerConv
  {
    static uint32_t Value( const TBpp64& src )
    {
      return Bpp32( src ).Value;
    }
  };
  struct MakeLargerConv
  {
    static uint64_t Value( const TBpp32& src )
    {
      return Bpp64( src ).Value;
    }
  };


  /*
    CONV: TILEのTBppからELEMへの変換関数、Valueを含むクラス
    DEST: uint32_tかuint64_t
    TILE: mfg_pal::Image32かmfg_pal::Image64
  */
  template<typename CONV, typename DEST, typename TILE>
  static void CopyFromWithConv( DEST* dest, TILE& src )
  {
    auto guard = PixelLocker( &src );

    for (int y = 0; y < src.Height(); y++)
    {
      int rowOrg = y*src.Width();
      for (int x = 0; x < src.Width(); x++)
      {
        dest[rowOrg+x] = CONV::Value( src.PixelGet( x, y ) );
      }
    } 
  }

  // facade的なstaticメソッド郡
  static void FromSameBppTile( uint32_t* dest, mfg_pal::Image32& src )
  {
    CopyFromWithConv<SameBppConv<TBpp32>>( dest, src );
  }

  static void FromSameBppTile( uint64_t* dest, mfg_pal::Image64& src )
  {
    CopyFromWithConv<SameBppConv<TBpp64>>( dest, src );
  }

  // CImageTIie64 -> U8ベクトル
  static void FromLargerBppTile( uint32_t* dest, mfg_pal::Image64& src )
  {
    CopyFromWithConv<MakeSmallerConv>( dest, src );
  }

  // mfg_pal::Image32 -> U16ベクトル
  static void FromSmallerBppTile( uint64_t* dest, mfg_pal::Image32& src )
  {
    CopyFromWithConv<MakeLargerConv>( dest, src );
  }

  /*
    FromTileの実装。
    mfg_pal::Image32とmfg_pal::Image64の多重定義とする事でテンプレートから使えるようにする。
  */
  static void FromTile( NumericType dtype, void* dest, mfg_pal::Image32& src )
  {
    if (dtype == NumericType::UInt(8))
    {
      CImageTileToArray::FromSameBppTile( static_cast<uint32_t*>(dest), src );
    }
    else
    {
      assert( dtype == NumericType::UInt(16) );
      CImageTileToArray::FromSmallerBppTile( static_cast<uint64_t*>(dest), src );
    }
  }

  static void FromTile( NumericType dtype, void* dest, mfg_pal::Image64& src )
  {
    if (dtype == NumericType::UInt(16))
    {
      CImageTileToArray::FromSameBppTile( static_cast<uint64_t*>(dest), src );
    }
    else
    {
      assert( dtype == NumericType::UInt(8) );
      CImageTileToArray::FromLargerBppTile( static_cast<uint32_t*>(dest), src );
    }
  }

  static void FromTile( NumericType dtype, void* dest, TileReference& src )
  {
    switch( src._tileType )
    {
      case TileReference::TILE32:
        FromTile( dtype, dest, *src.u._tile32 );
        return;
      case TileReference::TILE64:
        FromTile( dtype, dest, *src.u._tile64 );
        return;
    }
  }
};

/*
  uint32_t*かuint64_tの連続領域からCImageTileXXへのコピー
  ToTileを外部からは使う。
*/
struct ArrayToCImageTile
{
  // uintXXX_tからTBpp32やTBpp64へと変換するコンバーター関連

  // ELEM: uint32かuint64
  // TBPP: TBpp32かTBpp64
  template<typename ELEM, typename TBPP>
  struct SameBppConv
  {
    static TBPP Color( ELEM src )
    {
      TBPP color;
      color.Value = src;
      return color;
    }
  };

  struct MakeLargerConv
  {
    static TBpp64 Color( uint32_t src )
    { 
      TBpp32 color;
      color.Value = src;
      return Bpp64( color );
    }
  };
  struct MakeSmallerConv
  {
    static TBpp32 Color( uint64_t src )
    { 
      TBpp64 color;
      color.Value = src;
      return Bpp32( color );
    }
  };

  template<typename CONV, typename DTILE, typename SELEM>
  static void CopyWithConv( DTILE& dest, SELEM* src )
  {
    auto guard = PixelLocker( &dest );
    for (int y = 0; y < dest.Height(); y++)
    {
      int rowOrg = y*dest.Width();
      for (int x = 0; x < dest.Width(); x++)
      {
        dest.PixelSet( x, y, CONV::Color( src[rowOrg+x] ) );
      }
    } 
  }

  static void FromSameBpp( mfg_pal::Image32& dest, uint32_t* src )
  {
    CopyWithConv<SameBppConv<uint32_t, TBpp32>>( dest, src );
  }

  static void FromSameBpp( mfg_pal::Image64& dest, uint64_t* src )
  {
    CopyWithConv<SameBppConv<uint64_t, TBpp64>>( dest, src );
  }

  static void FromLargerBpp( mfg_pal::Image32& dest, uint64_t* src )
  {
    CopyWithConv<MakeSmallerConv>( dest, src );
  }

  static void FromSmallerBpp( mfg_pal::Image64& dest, uint32_t* src )
  {
    CopyWithConv<MakeLargerConv>( dest, src );
  }

  /*
    ToTileの実装。
    mfg_pal::Image32とmfg_pal::Image64の多重定義とする事でテンプレートから使えるようにする。
  */
  static void ToTile( mfg_pal::Image32& dest, NumericType stype, void* src )
  {
    if (stype == NumericType::UInt(8))
    {
      ArrayToCImageTile::FromSameBpp( dest, static_cast<uint32_t*>(src) );
    }
    else
    {
      assert( stype == NumericType::UInt(16) );
      ArrayToCImageTile::FromLargerBpp( dest, static_cast<uint64_t*>(src) );
    }
  }

  static void ToTile( mfg_pal::Image64& dest, NumericType stype, void* src )
  {
    if (stype == NumericType::UInt(16))
    {
      ArrayToCImageTile::FromSameBpp( dest, static_cast<uint64_t*>(src) );
    }
    else
    {
      assert( stype == NumericType::UInt(8) );
      ArrayToCImageTile::FromSmallerBpp( dest, static_cast<uint32_t*>(src) );
    }
  }

  static void ToTile( TileReference& dest, NumericType stype, void* src )
  {
    switch( dest._tileType )
    {
      case TileReference::TILE32:
        ToTile( *dest.u._tile32, stype, src );
        return;
      case TileReference::TILE64:
        ToTile( *dest.u._tile64, stype, src );
        return;
    }
  }
};

/*
  デバッグ用Logger。バックエンドに_debugがセットされると使われる。
  _debugはコンストラクト時では無くあとからセットされる。
*/
struct DebugLogger
{
  bool _debug = false;
  std::ostream& _ostream;
  DebugLogger( std::ostream& ostream ) : _ostream( ostream ) {}
  void Log( const std::string& msg )
  {
    if (_debug)
      _ostream << msg << std::endl;
  }
};

/*
  フィルタ実行時にどこでどれだけかかったのかを保持するクラス。
  計測自体はバックエンドごとに測り方が異なるが、結果は同じ構造体にする事で表示周りは共通になるようにここに置く。
*/
struct PerfInfo
{
  size_t _inputCopy = 0;
  size_t _kernelRun = 0;
  size_t _resultCopy = 0;
  size_t Total() const { return _inputCopy+_kernelRun+_resultCopy; }
};

struct StopWatch
{
  size_t& _dest;
  size_t _begin = 0;
  StopWatch( size_t& dest ) : _dest( dest ) {}
  void Start() { _begin = mfg_pal::Tick(); }
  void Stop()
  {
    size_t dur = mfg_pal::Tick() - _begin;
    _dest += dur;
  }
};

struct PerfCounter
{
  PerfInfo _info;
  StopWatch _inputWatch;
  StopWatch _kernelWatch;
  StopWatch _resultWatch;

  PerfCounter() : _inputWatch( _info._inputCopy ), _kernelWatch( _info._kernelRun ), _resultWatch( _info._resultCopy ) {} 

  void Reset()
  {
    _info._inputCopy = 0;
    _info._kernelRun = 0;
    _info._resultCopy = 0;
  }

  void BeginInput() { _inputWatch.Start(); }
  void EndInput() { _inputWatch.Stop(); }
  void BeginKernel() { _kernelWatch.Start(); }
  void EndKernel()  { _kernelWatch.Stop(); }
  void BeginResult() { _resultWatch.Start(); }
  void EndResult() { _resultWatch.Stop(); }
};

} ///<mfg_runtime_common

namespace mfg_clike_generator {
using namespace mfg_internal;

// MSL, HLSLなどCLikeなGPGPUコードの共通部分をここに間借り。
// 増えてきたらネームスペースを分けたい

// tupleの時はその要素一つひとつに対して、それ以外はelemに対してfunを呼ぶ。
// Fは std::function<void(size_t, BaseExprElem*)>
template<typename F>
void TupleOrElemEach( BaseExprElem* elem, F fun )
{
  if (elem->_elemType == IRElemType::TupleElem)
  {
    auto tup = elem->As<TupleElem>();
    auto exprs = tup->GetItemsSC();
    for (auto i: NRange( exprs.size() ))
    {
      fun( i, exprs[i] );
    }
  }
  else
  {
    fun( 0, elem );
  }
}

// IRElemなどの要素をMSLの名前に変換する
struct NameConverter
{
  std::string NType( const NumericType& mfgNType ) const
  {
    #ifdef MFG_BACKEND_D3D
    // DirectXでは全て32bitの数値として扱う。(uint8_tとか無い)
    // またint32_tなどは無いのでintとかで扱う。（intはD3Dでは32ビット固定）
    assert( mfgNType._bits <= 32 );
    if (mfgNType.IsFloat())
    {
      return "float";
    }
    else if (mfgNType.IsUInt())
    {
      return "uint";
    }
    else
    {
      assert( mfgNType.IsInt() );
      return "int";
    }
    #else
    if (mfgNType.IsFloat())
    {
      return "float";
    }
    else if (mfgNType.IsUInt())
    {
      return "uint" + std::to_string( mfgNType._bits ) + "_t";
    }
    else
    {
      assert( mfgNType.IsInt() );
      return "int" + std::to_string( mfgNType._bits ) + "_t";      
    }
    #endif
  }

  std::string Type( const Type& mfgType ) const
  {
    assert( mfgType.IsNumeric() );
    return NType( mfgType.AsNumeric() );
  }

  // Cのidentifierとしてvalidなものに変換する。
  // 先頭を_で始めて、.を_にする。
  // 基本的にはパーサーの時点でalnumで作られたidentを.でつなげたものくらいしか来ないので、
  // この処理で十分。
  std::string Ident( const std::string& name ) const
  {
    std::stringstream ss;
    ss << "_";

    for (char c: name)
    {
      if (c == '.')
      {
        ss << "_";
      }
      else
      {
        ss << c;
      }
    }
    return ss.str();
  }
};

/*
  ExprをCLikeなコードに変換する。
*/
struct ExprToCLike
{
  std::stringstream &_stream;
  NameConverter _nconv;

  // ExprToCLikeは、微妙に挙動を変えたい事があるが、仮想関数を使うほどでも無いので
  // 横取りする関数を挿せるようにする。trueを返したら横取り。
  std::function<bool(ExprToCLike&, BaseExprElem*)> _overwriter;

  ExprToCLike( std::stringstream& stream ) : _stream( stream ) {}

  void GenFunCall( const std::string& fname, const std::vector<BaseExprElem*>& args )
  {
    _stream << fname << "(";
    Separator sep( _stream );
    for( auto arg : args )
    {
      sep.MaySep();
      GenVExpr( arg );
    }
    _stream << ")";
  }

  // float4を出したい時のfloatを返す。
  // このケースではint4でint32_tを表す
  // 4bitのケースしか考えない
  std::string VectorBase( NumericType ntype )
  {
    if (ntype.IsFloat())
    {
      return "float";
    }
    else if (ntype.IsUInt())
    {
      return "uint";
    }
    else
    {
      assert( ntype.IsInt() );
      return "int";
    }
  }

  void GenVectorType( const std::vector<NumericType>& ntypes )
  {
    _stream << VectorBase( ntypes[0] ) << ntypes.size();
  }

  // タプルのTypeから、float4とかを生成する。
  void GenVectorType( const Type& tupType )
  {
    auto ntypes = tupType.ExpandNumericTypes();
    GenVectorType( ntypes );
  }

  // タプルのtypeから、同じ次元のboolのベクトル、bool3とかを生成
  void GenBVectorType( const Type& tupType )
  {
    _stream << "bool" << tupType.VectorExtent();
  }

  // CallのTypeからfloat4などを生成する。
  // boolの場合もある特殊なケース。
  void GenInternalVectorType( Call* callOp )
  {
    auto ntypes = callOp->_type.ExpandNumericTypes();
    if (callOp->IsInternalTypeBool())
    {
      _stream << "bool" << ntypes.size();
      return;
    }
    GenVectorType( ntypes );
  }

  // [1, 2, 3]というタプルから(1, 2, 3)を出力
  void GenParenTuple( const TupleElem* tup )
  {
    _stream << "(";
    auto children = tup->GetItemsSC();

    Separator sep( _stream );
    for( auto one : children )
    {
      sep.MaySep();
      GenVExpr( one );
    }
    _stream << ")";
  }

  // [1.0, 1.1, 1.2]などを、 float3(1.0, 1.1, 1.2)として出力
  void GenAsVectorExpr( BaseExprElem* arg )
  {
    if (arg->_type.IsNumeric())
    {
      // スカラーの場合はそのまま
      GenVExpr( arg );
      return;
    }
    assert( arg->_elemType == IRElemType::TupleElem );
    auto tup = arg->As<TupleElem>();

    GenVectorType( tup->_type );
    GenParenTuple( tup );
  }

  // ベクトルの時はbool3(...)とかboolのベクトルとして、
  // それ以外はそのまま出力
  void GenAsBVectorExpr( BaseExprElem* arg )
  {
    if (arg->_type.IsNumeric())
    {
      // スカラーの場合はそのまま
      GenVExpr( arg );
      return;
    }
    assert( arg->_elemType == IRElemType::TupleElem );
    auto tup = arg->As<TupleElem>();

    GenBVectorType( tup->_type );
    GenParenTuple( tup );
  }


  bool Overwrite( BaseExprElem* elem )
  {
    if (!_overwriter)
      return false;
    return _overwriter( *this, elem );
  }

  #ifdef MFG_BACKEND_D3D
  const char* _int32Name = "int";
  #else
  const char* _int32Name = "int32_t";
  #endif

  // boostのios_flags_saver相当
  struct FSaver
  {
    std::ostream& _out;
    std::ostream::fmtflags _old;

    FSaver( std::ostream& ostream ) : _out( ostream ), _old( ostream.flags() ) {}
    ~FSaver() { _out.flags( _old );  }
  };

  // mixやclampなど、後ろの引数がscalarの場合は前の引数と同じだけdupしたベクトルにする時の処理。
  void GenMayDupVector( const Type& mayVecType, BaseExprElem* target )
  {
    // 三番目の引数はスカラーか他の２つのベクトルと同じ次元のベクトル。
    // スカラーの場合は同じ次元のベクトルにここで展開する（HLSLがスカラー引数をサポートしてないため）
    if (mayVecType.IsTuple() && target->_type.IsFloat())
    {
      GenVectorType( mayVecType );
      auto vecDim = mayVecType.ExpandNumericTypes().size();
      _stream << "(";
      Separator sep( _stream );
      for( auto i : NRange(vecDim) )
      {
        sep.MaySep();
        GenVExpr( target ); // 同じexprをベクトルの次元分繰り返す。
      }
      _stream << ")";
    }
    else
    {
      GenAsVectorExpr( target );
    }
  }

  void GenMixCall( const std::string& fname, Call* /* callOp */, std::vector<BaseExprElem*>& args )
  {
    _stream << fname << "(";
    GenAsVectorExpr( args[0] );
    _stream << ", ";
    GenAsVectorExpr( args[1] );
    _stream << ", ";
    GenMayDupVector( args[0]->_type, args[2] );
    _stream << ")";
  }

  // Exprをmetalの式に変換
  void GenVExpr( BaseExprElem* elem )
  {
    if (Overwrite(elem)) return;

    auto elemType = elem->_elemType;
    switch( elemType )
    {
      case IRElemType::Variable:
      {
        auto op = elem->As<Variable>();
        _stream << _nconv.Ident( op->_name );
        return;
      }
      case IRElemType::ImmElem:
      {
        auto op = elem->As<ImmElem>();

        if (op->_type.IsInt())
        {
          _stream << op->Value<int32_t>();
        }
        else if (op->_type.IsUInt())
        {
          FSaver fs( _stream );
          _stream << "0x" << std::hex << op->Value<uint32_t>();
        }
        else
        {
          assert( op->_type.IsFloat() );
          _stream << std::fixed << op->Value<float>() << "f";
        }
        return;
      }
      case IRElemType::IfEl:
      {
        auto op = elem->As<IfEl>();
        _stream << "(";
        GenVExpr( op->GetCondition() );
        _stream << " ? ";
        GenVExpr( op->GetTrueValue() );
        _stream << " : ";
        GenVExpr( op->GetFalseValue() );
        _stream << ")";
        return;
      }
      case IRElemType::BinOp:
      {
        auto binOp = elem->As<BinOp>();        
        auto op = binOp->DisplayName();
        _stream << "(";
        GenVExpr( binOp->GetLeft() );
        _stream << op;
        GenVExpr( binOp->GetRight() );
        _stream << ")";
        return;
      }
      case IRElemType::LoadExpr:
      {
        auto op = elem->As<LoadExpr>();
        if (op->IsVector())
        {
          /*
            以下を生成。
            uint4( load[4*index+0], load[4*index+1], load[4*index+2], load[4*index+3])
          */
          Expr baseIndex( op->GetIndexArg()->CloneBaseExpr() );
          IRBuildDSL d;
          auto ntypes = op->_type.ExpandNumericTypes();
          auto mulBase = d._Mul( std::move(baseIndex), ntypes.size() );

          GenVectorType( op->_type );
          _stream << "(";
          mfg_internal::Separator sep( _stream );
          for( auto vidx : NRange( ntypes.size() ) )
          {
            sep.MaySep();
            _stream << _nconv.Ident( op->_bufName ) << "[";
            GenVExpr( mulBase.GetBase() );
            if (vidx != 0)
            {
              _stream << "+" << vidx;
            }
            _stream << "]";
          }
          _stream << ")";
          return;
        }
        else
        {
          _stream << _nconv.Ident( op->_bufName ) << "[";
          GenVExpr( op->GetIndexArg() );
          _stream << "]";
          return;
        }
      }
      case IRElemType::Call:
      {
        auto op = elem->As<Call>();
        auto fname = op->DisplayName();
        auto args = op->GetArgs()->ShallowCopy<BaseExprElem>();
        switch( op->_funcType)
        {
          case Call::ABS:
          case Call::COS_F32:
          case Call::CEIL:
          case Call::EXP_F32:
          case Call::EXP2:
          case Call::FLOOR:
          case Call::FRACT:
          case Call::ISINF:
          case Call::ISNAN:
          case Call::LOG:
          case Call::LOG2:
          case Call::ROUND:
          case Call::SIN_F32:
          case Call::SQRT_F32:
          case Call::TAN_F32:
          case Call::TRUNC:
          {
            GenFunCall( fname, args );
            return;
          }
          // ベクトル系関数
          case Call::ALL:
          case Call::ANY:
          {
            _stream << fname << "(";
            GenAsBVectorExpr( args[0] );
            _stream << ")";
            return;
          }
          case Call::CROSS:
          {
            _stream << fname << "(";
            GenAsVectorExpr( args[0] );
            _stream << ", ";
            GenAsVectorExpr( args[1] );
            _stream << ")";
            return;
          }
          case Call::LENGTH:
          case Call::NORMALIZE:
          {
            _stream << fname << "(";
            GenAsVectorExpr( args[0] );
            _stream << ")";
            return;
          }
          case Call::ATAN2_F32:
          case Call::DOT:
          case Call::DISTANCE:
          case Call::MAX:
          case Call::MIN:
          {
            _stream << fname << "(";
            GenAsVectorExpr( args[0] );
            _stream << ", ";
            GenAsVectorExpr( args[1] );
            _stream << ")";
            return;
          }
          case Call::CLAMP:
          {
            _stream << fname << "(";
            GenAsVectorExpr( args[0] );
            _stream << ", ";
            GenMayDupVector( args[0]->_type, args[1] );
            _stream << ", ";
            GenMayDupVector( args[0]->_type, args[2] );
            _stream << ")";
            return;
          }
          case Call::MIX:
          {
            GenMixCall( fname, op, args );
            return;
          }
          case Call::SATURATE:
          case Call::SIGN:
          {
            _stream << fname << "(";
            GenAsVectorExpr( args[0] );
            _stream << ")";
            return;
          }
          case Call::SMOOTHSTEP:
          {
            _stream << fname << "(";
            GenMayDupVector( args[2]->_type, args[0] );
            _stream << ", ";
            GenMayDupVector( args[2]->_type, args[1] );
            _stream << ", ";
            GenAsVectorExpr( args[2] );
            _stream << ")";
            return;
          }
          case Call::STEP:
          {
            _stream << fname << "(";
            GenMayDupVector( args[1]->_type, args[0] );
            _stream << ", ";
            GenAsVectorExpr( args[1] );
            _stream << ")";
            return;
          }
          // その他
          case Call::RAND_F32:
          {
            _stream << "rgen.FNext()";
            return;
          }
          case Call::POW:
          {
            if (op->_type.IsInt())
            {
              // powは多くのGPGPUでfloatとして定義されている。
              // intの時のPowはキャストで実装してみる。
              _stream << "((int)pow((float)(";
              GenVExpr( op->GetArg(0) );
                _stream << "), (float)(";
              GenVExpr( op->GetArg(1) );
              _stream << ")))";
              return;
            }
            else
            {
              GenFunCall( fname, args );
              return;
            }
          }
          case Call::CAST:
          {
            auto target = op->GetArg( 0 );
            _stream << "((" << _nconv.Type( op->_type ) << ")";
            GenVExpr( target );
            _stream << ")";
            return;
          }
          case Call::LOGICAL_NOT:
          {
            auto target = op->GetArg( 0 );
            _stream << "!(";
            GenVExpr( target );
            _stream << ")";
            return;
          }
          default:
            assert( false );
        }
        return;
      }
      default:
        throw InternalError("Never reached here.");
    };
  }

  /*
    tmpというletから、以下を生成（.は_nconv.Identを通す）
    キャストを生成した方がいいかもしれない。

    const float tmp.0 = tmp.x;
    const float tmp.1 = tmp.y;
    const float tmp.2 = tmp.z;
  */
  void GenExpandedVectorAssign( VectorLet* letOp, Type tp )
  {
    const char* swizzle[] = { "x", "y", "z", "w" };

    IRBuildDSL d;
    auto vari = d._Var( tp, letOp->_name );
    auto expandVari = vari.GetElem()->ExpandTuple<ExprV>();
    assert( expandVari.size() <= 4 );

    auto ntypes = tp.ExpandNumericTypes();
    for( auto i : NRange(expandVari.size()) )
    {
      _stream << "const " << _nconv.NType( ntypes[i] ) << " "
        << _nconv.Ident( expandVari[i].GetElem()->_name )  << " = "
        << _nconv.Ident( letOp->_name ) << "." << swizzle[i] << ";" << std::endl;
    }
  }

  /*
     letv tmp = normalize(c)

     を、

     float3 tmp = normalize( float3(c.0, c.1, c.2) );
     const float tmp.0 = tmp.x;
     const float tmp.1 = tmp.y;
     const float tmp.2 = tmp.z;

     にする。（.は_nconv.Identを通す）
  */
  void GenVectorCall( VectorLet* letOp, Call* callOp )
  {
    GenInternalVectorType( callOp );
    _stream << " " << _nconv.Ident( letOp->_name ) << " = ";
    GenVExpr( callOp );
    _stream << ";" << std::endl;

    GenExpandedVectorAssign( letOp, callOp->_type );
  }

  /*
     letv tmp = load(c)

     を、

     int4 tmp = load( c );
     const int tmp.0 = tmp.x;
     const int tmp.1 = tmp.y;
     const int tmp.2 = tmp.z;

     にする。（.は_nconv.Identを通す）
     loadはバックエンドごとにいろいろ違う。
  */
  void GenVectorLoadExpr( VectorLet* letOp, LoadExpr* loadOp )
  {
    GenVectorType( loadOp->_type );
    _stream << " " << _nconv.Ident( letOp->_name ) << " = ";
    GenVExpr( loadOp );
    _stream << ";" << std::endl;

    GenExpandedVectorAssign( letOp, loadOp->_type );
  }

  void GenExpr( BaseExprElem* stmt )
  {
    if (Overwrite(stmt)) return;

    auto elemType = stmt->_elemType;
    switch( elemType )
    {
      case IRElemType::Let:
      {
        auto op = stmt->As<Let>();
        auto value = op->GetValue();
        assert( !value->_type.IsTuple() );

        /*
          例えば以下みたいな文
          const uint32_t name = value;
        */
        _stream << "const " << _nconv.Type( value->_type ) << " " << _nconv.Ident( op->_name ) << " = ";
        GenVExpr( value );
        _stream << ";" << std::endl;

        break;
      }
      case IRElemType::VectorLet:
      {
        auto op = stmt->As<VectorLet>();
        auto value = op->GetValue();
        if (value->_elemType == IRElemType::Call)
        {
          GenVectorCall( op, value->As<Call>() );
        }
        else
        {
          auto loadExpr = value->As<LoadExpr>();
          assert( loadExpr != nullptr );
          GenVectorLoadExpr( op, loadExpr );
        }
        break;
      }
      case IRElemType::VarLet:
      {
        auto op = stmt->As<VarLet>();
        auto value = op->GetValue();
        assert( !value->_type.IsTuple() );

        /*
          例えば以下みたいな文(constが無い)
          uint32_t name = value;
        */
        _stream << _nconv.Type( value->_type ) << " " << _nconv.Ident( op->_name ) << " = ";
        GenVExpr( value );
        _stream << ";" << std::endl;
        break;
      }
      case IRElemType::BodyElem:
      {
        // トップレベルのBodyElem、つまりVoid Block的なもの。
        auto op = stmt->As<BodyElem>();
        assert( op->_type.IsVoid() );
        op->GetExprs()->ForEach<BaseExprElem>([this](BaseExprElem* child){
          GenExpr( child );
        });
        break;
      }
      case IRElemType::ExecTensorInitialize:
      case IRElemType::TransformTensor:
      case IRElemType::DefByReduce:
      case IRElemType::ReduceToScalar:
      case IRElemType::TensorIterator:
      {
        // Lowerで消えるはずなのでここには来ないはず。
        assert(false);
        break;
      }
      case IRElemType::AllocateStmt:
      {
        // allocate r0:int32[256]
        auto op = stmt->As<AllocateStmt>();
        _stream << _nconv.Type( op->_bufType ) << " "
          << _nconv.Ident( op->_bufName ) << "[";
        
        GenVExpr( op->GetArg() );
        _stream << "];" << std::endl;
        break;
      }
      case IRElemType::StoreStmt:
      {
        // store r0[v] = 255 - v
        // ベクトルの場合の処理もここで行う（バックエンドによって違うので）
        auto op = stmt->As<StoreStmt>();
        if (op->IsVector())
        {
          /*
           store r0[v] = [1, 2, 3, 4]
           から、以下を生成
           r0[v*4+0] = 1;
           r0[v*4+1] = 2;
           r0[v*4+2] = 3;
           r0[v*4+3] = 4;
          */
          Expr baseIndex( op->GetIndexArg()->CloneBaseExpr() );
          auto right = op->GetRight()->As<TupleElem>();
          assert( right != nullptr );
          auto ritems = right->ExpandTuple();

          IRBuildDSL d;
          auto mulBase = d._Mul( std::move(baseIndex), ritems.size() );

          for (auto vidx : NRange( ritems.size() ))
          {
            _stream << _nconv.Ident( op->_bufName ) << "[";
            GenVExpr( mulBase.GetBase() );
            if (vidx != 0)
            {
              _stream << "+" << vidx;
            }
            _stream << "] = ";
            GenVExpr( ritems[vidx].GetBase() );
            _stream << ";" << std::endl;
          }
          break;
        }
        else
        {
          _stream << _nconv.Ident( op->_bufName ) << "[";
          GenVExpr( op->GetIndexArg() );
          _stream << "] = ";
          GenVExpr( op->GetRight() );
          _stream << ";" << std::endl;
          break;
        }
      }
      case IRElemType::CallStmt:
      {
        // callstmt: sort_int32_1d(r0, extent.r0.0)
        auto op = stmt->As<CallStmt>();

        // _nameはバックエンドの名前に合わせて最初から入っているのでconvする必要は無い。        
        GenFunCall( op->_name, op->GetArgs()->ShallowCopy<BaseExprElem>() );
        _stream << ";" << std::endl;
        break;
      }
      case IRElemType::VarStore:
      {
        // store r0 = 255 - v
        auto op = stmt->As<VarStore>();
        _stream << _nconv.Ident( op->_varName ) << " = ";
        GenVExpr( op->GetRight() );
        _stream << ";" << std::endl;
        break;
      }
      case IRElemType::ForStmt:
      {
        auto op = stmt->As<ForStmt>();
        auto vname = _nconv.Ident( op->_varName );

        _stream << "for(" << _int32Name << " " << vname << " = ";
        GenVExpr( op->GetBeginExpr() );
        _stream << "; ";
        
        _stream << vname << " < ";
        GenVExpr( op->GetEndExpr() );
        _stream << "; ";

        _stream << vname << "++ )" << std::endl;
        _stream << "    {" << std::endl;
        GenExpr( op->GetBody() );
        _stream << "    }" << std::endl;
        break;
      }
      case IRElemType::CompoundAssignment:
      {
        auto op = stmt->As<CompoundAssignment>();
        assert( op->IsAfterLower() );
        auto tsload = op->GetLeftAfterLower();
        auto rexpr = op->GetRight();

        // 左辺の生成
        _stream <<  _nconv.Ident( tsload->_bufName ) << "[";
        GenVExpr( tsload->GetIndexArg() );
        _stream << "]";

        // 右辺の生成
        _stream << " += ";
        GenVExpr( rexpr );
        _stream << ";" << std::endl;;
        break;
      }
      default:
      {
        GenVExpr( stmt );
        break;
      }
    }
  }
};


}///< mfg_clike_generator

#endif

