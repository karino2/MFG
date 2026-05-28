/* -*- coding: utf-8 -*- マルチバイト */

// Copyright 2026 Kazuma Arino. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#ifndef MFG_ELEM_HPP_
#define MFG_ELEM_HPP_

#include <mfg_pal/pal.hpp>

#include "forest.hpp"
#include "mfg_tool.hpp"
#include <algorithm> // for_each
#include <cassert>
#include <cstring>
#include <functional>
#include <iostream>
#include <ostream>
#include <stdexcept>
#include <string>
#include <sstream>
#include <tuple>
#include <vector>
#include <type_traits>

namespace mfg_internal
{
using namespace mfg_pal;
using namespace mfg_forest;

//////////////////////
// IRElem関連
//////////////////////

enum class IRElemType
{
  ImmElem,
  EnumElem, // パースの途中にだけ存在するIR。途中で解決されてIntImmになる。
  Variable,
  BinOp,
  IfEl,
  Call,
  TensorCall, // メソッドコールだが今の所selfに相当するものはテンソルのみなので、TensorCallとしてTensorInfoを持たせる。
  TupleElem,
  SwizzleCall,
  RangeElem,
  ReduceSum,
  Reduce,
  TensorIterator, // これはexpr的なものとstmtなもの２つがある（REDUCE_SUMかEXEC_FOREACHかなどで）
  SamplerElem, // パースの途中にだけ存在するIR。Lowerした結果には残らない。
  SpreadElem, // パースの途中にだけ存在するIR。最終的なツリーには残らない。
  BodyElem,
  BlockElem, // 引数とBodyを持つブロック定義、実行はされないのでExprとして扱う。
  LoadExpr, // Lowerで作られるExpr。Tensorの一次元化されたロード。
  ReduceToScalar,
  SamplerCall, // Lowerした結果には残らない
  // 中間で登場するstruct、IRElemにするかはあとで検討
  VectorElem,
  // Stmtっぽいもの (VoidExpr)
  Let,
  VectorLet,
  TupleLet,
  InlineFuncLet,
  VarLet,
  ExecTensorInitialize,
  TransformTensor,
  DefByReduce,
  CompoundAssignment,
  AllocateStmt, // 名前が一般的すぎるものにはStmtをつける、以下同じ
  StoreStmt,
  VarStore,
  CallStmt, 
  ForStmt
};

struct IRElemClonner;

struct IRElem;
using FNode = Forest<std::unique_ptr<IRElem>>;

extern int g_irelem_alloc_count;

/*
数値の型。Int(32)とか。
Tupleの要素になる。Tupleはネストはしない。
*/
struct NumericType
{
  enum Code
  {
    INT,
    UINT,
    FLOAT
  };
  Code _code; 
  uint8_t _bits;
  NumericType( Code code, uint8_t bits ) : _code( code ), _bits( bits ) {}

  static NumericType UInt( uint8_t bits ) { return NumericType( UINT, bits ); }
  static NumericType Int( uint8_t bits ) { return NumericType( INT, bits ); }
  static NumericType Float( uint8_t bits ) { return NumericType( FLOAT, bits ); }

  bool IsInt() const { return _code == INT; }
  bool IsUInt() const { return _code == UINT; }
  // uintかint
  bool IsUSInt() const { return IsInt() || IsUInt(); }

  bool IsFloat() const { return _code == FLOAT; }
  bool IsNumeric() const { return IsInt() || IsUInt() || IsFloat();  }

  int Bytes() const { return (_bits + 7) / 8; }

  bool operator==( const NumericType& other ) const
  {
    return ( _code == other._code ) && ( _bits == other._bits );
  }

  bool operator!=( const NumericType& other ) const
  {
    return !(*this == other );
  }
};


/*
  型を表す。
*/
struct Type
{
  enum Code
  {
    INT,
    UINT,
    FLOAT,    
    TUPLE,
    UNIT,
    RANGE,
    ENUM,
    OBJECT
  };
  Code _code; 

  // Numericの時は要素数1、Tupleの時は各要素の型が入る。
  // TupleはNumericのみを子供に持つ。
  // UNITやOBJECTの時はempty。
  std::vector<NumericType> _items;

  static NumericType ToNumeric( Code code, uint8_t bits )
  {
    switch( code )
    {
      case INT:
        return NumericType( NumericType::INT, bits );
      case UINT:
        return NumericType( NumericType::UINT, bits );
      case FLOAT:    
        return NumericType( NumericType::FLOAT, bits );
      default:
        throw InternalError( "Numeric type conversion of non numeric type" );
    }
  }

  Type() : _code( UNIT ) {}
  Type( Code code, uint8_t bits ) : _code( code ), _items{ ToNumeric( code, bits ) } {}
  // Tupleのコンストラクタ
  Type( std::vector<NumericType> items ) : _code( TUPLE ), _items( std::move(items) ) {}
  // Object, Unitなど。
  Type( Code code ) : _code( code ) {}

  // Enumのコンストラクタ
  static Type Enum(  size_t enumSymId )
  {
    // 今のところユーザー定義は出来ないのでそんなに大きなシンボルIDにはならない前提。
    assert( enumSymId <= 255 );
    // データは_itemsにNumericTypeのINTとしてビット幅のところにこそっと持つ
    Type tp( Code::ENUM );
    tp._items.emplace_back( NumericType::INT, (uint8_t) enumSymId );
    return tp;
  }

  Type( NumericType ntype ) : _code( UNIT ), _items { ntype }
  {
    switch( ntype._code )
    {
      case NumericType::INT:
        _code = INT;
        return;
      case NumericType::UINT:
        _code = UINT;
        return;
      case NumericType::FLOAT:
        _code = FLOAT;
        return;
    }
  }

  bool IsInt() const { return _code == INT; }
  bool IsUInt() const { return _code == UINT; }
  bool IsFloat() const { return _code == FLOAT; }
  bool IsEnum() const { return _code == ENUM; }

  bool IsVectorND( size_t N ) const
  {
    if (!IsVector())
      return false;
    return _items.size() == N;
  }

  bool IsVector4D() const
  {
    return IsVectorND( 4 );
  }

  bool IsVector3D() const
  {
    return IsVectorND( 3 );
  }

  // BGRA8を表すu8の4次元ベクトル
  bool IsU8V4() const
  {
    if (!IsVector4D())
      return false;
    return _items[0] == NumericType::UInt(8);    
  }

  // BGRA16を表すu16の4次元ベクトル
  bool IsU16V4() const
  {
    if (!IsVector4D())
      return false;
    return _items[0] == NumericType::UInt(16);
  }

  bool IsF32VN( size_t N ) const
  {
    if (!IsVectorND( N ))
      return false;
    return _items[0] == NumericType::Float(32);
  }


  bool IsF32V4() const { return IsF32VN( 4 ); }
  bool IsF32V3() const { return IsF32VN( 3 ); }

  bool IsTuple() const { return _code == TUPLE; }
  bool IsObject() const { return _code == OBJECT; }
  bool IsRange() const { return _code == RANGE; }

  // unsigned または signedのint
  bool IsUSInt() const { return IsInt() || IsUInt(); }

  bool IsNumeric() const { return IsInt() || IsUInt() || IsFloat();  }

  bool IsVoid() const { return _code == UNIT; }

  /*
    ベクトルはタプルのうち、同じ数値型で2〜4要素のもの。
  */
  bool IsVector() const
  {
    if (!IsTuple())
      return false;
    if (_items.size() == 1 || _items.size() > 4)
      return false;
    
    auto first = _items[0];
    return std::all_of( _items.begin(), _items.end(), [first]( NumericType nt ){ return nt == first; } );
  }

  size_t ItemNum() const
  {
    assert( IsTuple() );
    return _items.size();
  }

  // ベクトルで無い時は1次元とする。
  size_t VectorExtent() const
  {
    if (IsVector()) 
      return ItemNum();
    return 1;
  }

  size_t EnumTypeId() const
  {
    assert( IsEnum() );
    return (size_t)_items[0]._bits;
  }


  /*
    タプルの時は要素の型を、Numericの時は0指定で自身を返す。
    要素1のタプルとNumericの区別は曖昧。
  */
  Type ElemType( size_t tupIdx ) const
  {
    if (IsTuple())
      return Type( _items[tupIdx] );
    assert( tupIdx == 0 );
    return *this;
  }

  NumericType AsNumeric() const
  {
    assert( IsNumeric() );
    return _items[0];
  }

  /*
    Numericの時は要素1の自身のNumericTypeを、
    タプルの時はitemsのNumericTypesを返す。
    今の所_itemsと同じ。
  */
  std::vector<NumericType> ExpandNumericTypes() const
  {
    return _items;
  }

  bool operator==( const Type& other ) const
  {
    return ( _code == other._code ) && ( _items == other._items );
  }

  bool operator!=( const Type& other ) const
  {
    return !(*this == other );
  }
};

/*
  ツリーの要素を表す型。
  ただしForestノード自体はIRElemを引数に持つForest型なので、
  名前はIRNodeでは無くIRElemとする。
*/
struct IRElem
{
  IRElemType _elemType;
  // 自身を持つノードへのポインタ。
  mutable FNode* _node;
  Type _type;

  IRElem( IRElemType elemType ) : _elemType( elemType ), _node( nullptr ) { g_irelem_alloc_count++; }
  IRElem( IRElemType elemType, Type type ) : _elemType( elemType ), _node( nullptr ), _type( type ) { g_irelem_alloc_count++; }
  // virtual ~IRElem() = default;
  virtual ~IRElem() { g_irelem_alloc_count--; }

  FNode* GetNode() const { return _node; }

  virtual IRElem* CloneSelf() const = 0;

  /*
    このノードをrootとしてツリー全体をコピーする。
    各要素のIRElemはCloneSelf()でコピーされ、FNodeは元のツリーと同じ形になるようにnewしていく。
  */
  inline IRElem* Clone() const;

  FNode* EnsureNode() const
  {
    if (_node == nullptr)
    {
      // ここでconstを取るのを避ける方法が良く分からないのでキャスト。
      // 本質的には最初に代入しておくものをlazyにやっているだけなので問題ないはず。
      _node = new FNode( std::unique_ptr<IRElem>( (IRElem*)this ) );
    }
    return _node;
  }

  void
  AppendChild( IRElem *elem )
  {
    EnsureNode();
    _node->AppendChild( elem->EnsureNode() );
  }

  FNode *
  GetChildNode( int nth ) const
  {
    if (_node == nullptr)
      return nullptr;
    return _node->GetChild( nth );
  }

  FNode *
  GetLastChildNode() const
  {
    if (_node == nullptr)
      return nullptr;
    return _node->GetLastChild();
  }

  /*
    子供のIRElem*を返す。
    子供が居なかったらnullptrを返す。
  */
  IRElem* GetChild( int nth ) const
  {
    auto node = GetChildNode( nth );
    if (node == nullptr )
      return nullptr;
    return node->_data.get();
  }

  IRElem* GetLastChild() const
  {
    auto node = GetLastChildNode();
    if (node == nullptr )
      return nullptr;
    return node->_data.get();
  }

  /*
    子供のIRElemを型にキャストして返す。
    _elemTypeは一致している必要がある（一致しない場合はassert fail）。
    ただしnullptrは許す。
  */
  template<typename T>
  T* GetChild( int nth ) const
  {
    auto elem = GetChild( nth );
    if (elem == nullptr)
      return nullptr;
    assert( elem->_elemType == T::_elemType);
    return (T*)elem;
  }

  template<typename T>
  T* GetLastChild() const
  {
    auto elem = GetLastChild();
    if (elem == nullptr)
      return nullptr;
    assert( elem->_elemType == T::_elemType);
    return (T*)elem;
  }

  bool HasChildren() const
  {
    if (_node == nullptr)
      return false;
    return _node->HasChildren();
  }

  void Unchain()
  {
    if (_node == nullptr)
      return;
    _node->begin().Unchain();
  }

  template<typename T>
  const T *As() const
  {
    if (_elemType == T::_elemType)
    {
        return (const T *)this;
    }
    return nullptr;
  }

  template<typename T>
  T *As()
  {
    if (_elemType == T::_elemType)
    {
        return (T *)this;
    }
    return nullptr;
  }

  /*
    elemとelemTypeを受けつつforeachを回す。iterでToTrailingなどしても良い。
    Fの型は以下:
    fn( iter, IRElemUP& elem, IRElemeType elemType );
  */
  template<typename F>
  void ForEach( F fn )
  {
    EnsureNode()->ForEach([&fn](FNode::iterator& iter)
    {
      auto& elem = iter.GetContent();
      auto elemType = elem->_elemType;
      fn( iter, elem, elemType );
    });
  }

  /*
    elemとelemTypeを受けつつforeachを回す。iterでToTrailingなどしても良い。
    Fの型は以下:
    fn( iter, IRElemUP& elem, IRElemeType elemType );
  */
  template<typename F>
  void ForEachLeading( F fn )
  {
    EnsureNode()->ForEachLeading([&fn](FNode::iterator& iter)
    {
      auto& elem = iter.GetContent();
      auto elemType = elem->_elemType;
      fn( iter, elem, elemType );
    });
  }

  // ForEachLeadingElem<Let>( []( Let* op ){ ... });
  // などのように使う。今の所iterは無しで。
  template<typename E, typename F>
  void ForEachLeadingElem( F fn )
  {
    EnsureNode()->ForEachLeading([&fn](FNode::iterator& iter)
    {
      auto& elem = iter.GetContent();
      auto target = elem->As<E>();
      if (target != nullptr)
      {
        fn( target );
      }
    });
  }
};

using IRElemUP = std::unique_ptr<IRElem>;

struct IRElemClonner
{
  static std::unique_ptr<IRElem> Clone( const std::unique_ptr<IRElem>& elem ) { return std::unique_ptr<IRElem>( elem->CloneSelf() ); }
};

IRElem* IRElem::Clone() const
{
  if (_node == nullptr)
  {
    return CloneSelf();
  }

  auto newTree = _node->Clone<IRElemClonner>();

  // 全IRElemのnodeを新しいツリーのノードにセットする。
  for (auto& edge : *newTree)
  {
    if (edge.IsLeading())
      (*edge)->_node = edge._node;
  }
  return newTree->_data.get();
}

/*
  elemがnodeを持っていたらノードごと削除、持っていなければelemだけを削除
*/
inline void DeleteElem( IRElem* elem )
{
  // unique_ptr用
  if ( elem == nullptr )
    return;

  if (elem->_node == nullptr)
  {
    delete elem;
    return;
  }
  delete elem->_node;
}

// PrinterでもBitCodeのダンプでも使うのでここで。
inline std::ostream &operator<<(std::ostream &out, const Type &type) {
    switch (type._code) {
    case Type::INT:
        out << "int";
        break;
    case Type::UINT:
        out << "uint";
        break;
    case Type::FLOAT:
        out << "float";
        break;
    case Type::UNIT:
        out << "(void)";
        break;
    case Type::TUPLE:
        if (type.IsU8V4())
          out << "(u8v4)";
        else
          out << "(tuple)";
        break;
    case Type::RANGE:
        out << "(range)";
        break;
    case Type::ENUM:
        out << "(enum)";
        break;
    case Type::OBJECT:
        out << "(object)";
        break;
    }
    if (type.IsNumeric())
    {
      out << (int)type.AsNumeric()._bits;
    }
    return out;
}

struct Variable;
class Expr;

struct BaseExprElem : public IRElem
{
  BaseExprElem( IRElemType elemType ) : IRElem( elemType ) {}
  BaseExprElem( IRElemType elemType, Type type ) : IRElem( elemType, type ) {}

  bool IsConst() const
  {
    return (_elemType == IRElemType::ImmElem);
  }

  // IsConstの時にintの値として取り出す。
  // 主にboundとかintの小さな値である事が分かっているケース。
  inline int AsInt() const;

  BaseExprElem* CloneBaseExpr() const
  {
    return (BaseExprElem*)Clone();
  }
};

using BExprUP = std::unique_ptr<BaseExprElem, decltype(&DeleteElem)>;
inline BExprUP MakeBUP( BaseExprElem* elem ) { return BExprUP( elem, DeleteElem ); }

/*
  Typeがvoidのもの。Stmtっぽいもの。
  ただしts.for_eachなどパースの時点ではExprに見えるものでstmtのものもあるので、
  BaseVoidElemのサブクラスでないStmtはありうるから完全に型では分離出来ない。

  ただしBaseVoidElemのサブクラスならStmt的なもの、というのは保証出来る。
*/
struct BaseVoidElem : public BaseExprElem
{
  BaseVoidElem( IRElemType elemType ) : BaseExprElem( elemType ) {}
  ~BaseVoidElem() override = default;
};

/* 
  Expr関連。ExprはParameterなどで使うので最初の方に実体の定義が欲しいが、
  Variable等を使うモノは後じゃないと定義出来ないので、実体を作るのに必要なモノだけここに置いておく。
*/

template <typename T> class ExprT;

using ExprVOID = ExprT<BaseVoidElem>;

/*
  ExprはBaseExprElemを保持するクラス。直接ツリーに入るIRElemでは無くて、それのホルダー。
  C++のAPIとして直接使われる事もある。

  暗黙のコピーは出来ず、コピーコンストラクタはクローンする。
  クローンでBaseExprElemはツリーごとクローンされる。
*/
class Expr
{
protected:
  BaseExprElem* _elem;
  std::unique_ptr<FNode> _nodePtr;

public:
  Expr() = default ;
  explicit Expr( BaseExprElem* ptr ) :  _elem( ptr ), _nodePtr( ptr->EnsureNode() ) {}
  Expr( Expr&& src ) = default;
  explicit Expr( const Expr& src ) : Expr( src._elem->CloneBaseExpr() ) {}
  Expr& operator=( Expr&& src ) = default;

  template<typename T>
  inline Expr( ExprT<T>&& );

  inline Expr( int32_t x );

  // 外からしか使わないけれど、ここに置く以外の解決策がわからなかった…
  inline Expr( float x );

  template<typename T>
  ExprT<T> AsExprT() &&;

  /*
    このホルダーがIRElemを持っていたらtrue、持っていなかったらfalseを返す。
  */
  bool IsDefined() const
  {
    return _nodePtr.get() != nullptr;
  }

  BaseExprElem* GetBase() { return _elem; }
  const BaseExprElem* GetBase() const { return _elem; }

  /*
    オーナーシップを解放し、保持している要素を返す。
    IsDefined()でなければnullptrを返す。
  */
  BaseExprElem* Detach()
  {
    if (!IsDefined())
      return nullptr;
    auto res = _elem;
    _elem = nullptr;
    _nodePtr.release();
    return res;
  }

  void Attach( BaseExprElem* newElem )
  {
    assert( !IsDefined() );
    _elem = newElem;
    _nodePtr.reset( newElem->EnsureNode() );
  }

  // 保持している要素を削除してemptyの状態に戻す。
  void Erase()
  {
    if (IsDefined())
    {
      _nodePtr.reset( nullptr );
      _elem = nullptr;
    }
  }

  /*
    保持しているBaseExprElemをツリーごとクローンする。
  */
  Expr Clone() const
  {
    if(!IsDefined())
    {
      return Expr();
    }
    return Expr( _elem->CloneBaseExpr() );
  }

  template<typename K>
  const K* As() const
  {
    if (IsDefined() && _elem->_elemType == K::_elemType)
      return (const K *)_elem;
    
    return nullptr;
  }

  template<typename K>
  K* As()
  {
    if (IsDefined() && _elem->_elemType == K::_elemType)
      return (K *)_elem;
    
    return nullptr;
  }

  /*
    指しているポインタが同じかどうかを判定する。
    意味的に同じでもポインタが違えばfalseを返す。
  */
  bool IsSamePtr(const Expr& other) const { return _elem == other._elem; } 

  /*
    指しているポインタが同じかどうかを判定する。
    意味的に同じでもポインタが違えばfalseを返す。
  */
  bool IsSamePtr(const BaseExprElem* other) const { return _elem == other; } 


  Type GetType() const
  {
    return GetBase()->_type;
  }

  IRElemType GetElemType() const
  {
    return GetBase()->_elemType;
  }

  bool IsTuple() const { return GetType().IsTuple(); }

  bool IsConst() const
  {
    if (!IsDefined())
      return false;
    return GetBase()->IsConst();
  }

  bool IsVariable() const
  {
    if (!IsDefined())
      return false;
    return GetElemType() == IRElemType::Variable;
  }

  // IsConstの時に、intとして値を取り出す。
  // IsConstが確定しているので当然IsDefined()でもある。
  int AsInt() const
  {
    assert(IsDefined());
    return GetBase()->AsInt();
  }

  /*
    IntかUIntのconstで0の時。この場合は範囲チェックなどを省く事が出来る場合がある。
  */
  bool IsZero()  const
  {
    if (!IsConst())
      return false;
    if (!GetType().IsUSInt())
      return false;
    return AsInt() == 0;
  }
};

// Spreadのときは展開した型を、それ以外は要素の型を返す。
inline std::vector<Type> ToSpreadedTypes( const std::vector<Expr>& items );

inline std::vector<Expr> CloneExprs( const std::vector<Expr>& src )
{
  return MapFn( src, [](const Expr& expr){ return expr.Clone(); } );
}

inline std::vector<Expr> CloneBaseExprs( const std::vector<BaseExprElem*>& src )
{
  return MapFn( src, [](const BaseExprElem* expr){ return Expr( expr->CloneBaseExpr() ); } );
}

inline  std::vector<Expr> WrapExpr( std::vector<BaseExprElem*>&& bvec )
{
  return MapFn( bvec, [](BaseExprElem* be) { return Expr(be); } );
}

// 型指定のあるExpr。基本的にはExprと相互乗り入れが出来る。
// ETはBaseExprElemのサブクラスで、_elemTypeが取れる必要がある
template<typename ET>
class ExprT : public Expr
{
public:
  ExprT( ET* ptr ): Expr( ptr ) {}
  explicit ExprT(): Expr() {}

  ET* GetElem() { return (ET*)GetBase(); }
  const ET* GetElem() const { return (const ET*)GetBase(); }
  ExprT<ET> CloneT() const { return ExprT( (ET*) GetElem()->Clone() ); }

  template<typename T=ET>
  typename std::enable_if<std::is_base_of<BaseVoidElem, T>::value, ExprVOID>::type
  AsVOID() &&
  {
    return ExprVOID( (BaseVoidElem*)Detach() );
  }
};

template<typename T>
Expr::Expr( ExprT<T>&& src ) : Expr( src.Detach() ) {}

template<typename T>
ExprT<T> Expr::AsExprT() &&
{
  return ExprT<T>( Detach()->As<T>() );
}

using ExprV = ExprT<Variable>;

template<typename ST>
std::vector<Expr> ToExprVec( std::vector<ExprT<ST>> && tvec )
{
  return std::vector<Expr>( std::make_move_iterator( tvec.begin() ), std::make_move_iterator( tvec.end() ) );
}

template<typename ST>
std::vector<Expr> ToExprVec( ExprT<ST>&& one )
{
  std::vector<Expr> ret;
  ret.emplace_back( one.Detach() );
  return ret;
}

inline std::vector<Expr> ToExprVec( Expr&& one )
{
  std::vector<Expr> ret;
  ret.emplace_back( one.Detach() );
  return ret;
}

inline std::vector<ExprVOID> ToVOIDVec( ExprVOID&& one )
{
  std::vector<ExprVOID> ret;
  ret.push_back( std::move(one) );
  return ret;
}

/*
  BaseVoidElemのサブクラスのTだけ使えるメソッド。
*/
template<typename T>
typename std::enable_if<std::is_base_of<BaseVoidElem, T>::value, std::vector<ExprVOID>>::type
ToVOIDVec( ExprT<T>&& one )
{
  std::vector<ExprVOID> ret;
  ret.push_back( std::move(one).AsVOID() );
  return ret;
}


/*
  BaseVoidElemのサブクラスのTだけ使えるメソッド。
*/
template<typename T>
typename std::enable_if<std::is_base_of<BaseVoidElem, T>::value, std::vector<ExprVOID>>::type
ToVOIDVec( std::vector<ExprT<T>> && tvec )
{
  std::vector<ExprVOID> ret;
  for( auto& one: tvec )
  {
    ret.push_back( std::move(one).AsVOID() );
  }
  return ret;
}

template<typename T>
void AppendTail( std::vector<Expr>& dest, std::vector<ExprT<T>>&& tail )
{
  dest.insert( dest.end(), std::make_move_iterator( tail.begin() ), std::make_move_iterator( tail.end() ) );
}

/*
  "curiously recurring template pattern"で継承する為の基底クラス
*/
template<typename T>
struct ExprElem : public BaseExprElem
{
  ExprElem() : BaseExprElem( T::_elemType ) {}
  ExprElem( Type type ) : BaseExprElem( T::_elemType, type ) {}
  ~ExprElem() override = default;
};

template<typename T>
struct VoidExprElem : public BaseVoidElem
{
  VoidExprElem() : BaseVoidElem( T::_elemType ) {}
  ~VoidExprElem() override = default;
};

/*
  ExprでもStmtでも無い、内部で使われる中間のデータ構造の基底クラス。
*/
template<typename T>
struct OtherElem : public IRElem
{
  OtherElem() : IRElem (T::_elemType ) {}
};

/*
  IRElemのリストを持つ親ノード。
*/
struct VectorElem : public OtherElem<VectorElem>
{
  static const IRElemType _elemType = IRElemType::VectorElem;
  VectorElem() : OtherElem<VectorElem>() {}

  /*
    要素を一つも持ってなかったらtrue、要素を一つでも持っていたらfalseを返す。
  */
  bool IsEmpty() const
  {
    if (_node == nullptr)
      return true;
    return !_node->HasChildren();
  }

  /*
    子どもたちはツリーのCloneとしてコピーされるので、自分だけクローンすれば十分。
  */
  IRElem* CloneSelf() const override
  {
    return new VectorElem();
  }

  /*
    elemのオーナーシップは以後このベクトルが持つ。
  */
  void
  Append( IRElem* elem )
  {
    EnsureNode();
    _node->AppendChild( elem->EnsureNode() );
  }

  FNode::child_iterator begin()
  {
    if (_node == nullptr)
    {
      return FNode::child_iterator( FNode::iterator( nullptr, FEdgeDir::LEADING ));
    }
    return _node->BeginChild();
  }

  FNode::child_iterator end()
  {
    return begin().end();
  }

  FNode::const_child_iterator begin() const
  {
    if (_node == nullptr)
    {
      return FNode::const_child_iterator( FNode::const_iterator( nullptr, FEdgeDir::LEADING ));
    }
    return _node->CBeginChild();
  }

  FNode::const_child_iterator end() const
  {
    return begin().end();
  }

  template<typename T=IRElem, typename F>
  void ForEach( F fn )
  {
    for( auto& elem: *this)
    {
      fn( (T*)elem.get() );
    }
  }

  template<typename T=IRElem, typename F>
  auto MapFn( F fun ) -> std::vector<decltype( fun((T*)nullptr) )>
  {
     std::vector<decltype( fun((T*)nullptr) )> ret;
     ForEach<T>([&fun, &ret](T* elem) {
       ret.push_back( fun( elem ) );
     });
     return ret;
  }

  template<typename T=IRElem, typename F>
  void ForEachWithIndex( F fn )
  {
    size_t i = 0;
    for( auto& elem: *this)
    {
      fn( i, (T*)elem.get() );
      i++;
    }
  }

  // ポインタだけをコピーする。
  template<typename T=IRElem>
  std::vector<T*> ShallowCopy()
  {
    std::vector<T*> ret;
    for( auto& elem: *this)
    {
      ret.push_back( (T*)elem.get() );
    }
    return ret;
  }

  // 中の要素をcloneする
  template<typename T=IRElem>
  std::vector<T*> DeepCopy()
  {
    std::vector<T*> ret;
    for( auto& elem: *this)
    {
      ret.push_back( (T*)(elem.get()->Clone()) );
    }
    return ret;
  }

  size_t CountSize() const
  {
    size_t ret = 0;
    std::for_each( begin(), end(), [&ret](const IRElemUP&) { ret++; } );
    return ret;
  }

};

// IRElemとしてはVectorElemと区別しない。
// 便利メソッドを追加するだけのクラス。
struct ExprVector : public VectorElem
{
  ExprVector() = default;

  void AppendExpr( Expr&& expr )
  {
    Append( expr.Detach() );
  }

  // 子供をDetachし、Exprのベクトルとして返す
  std::vector<Expr> DetachChildren()
  {
    auto children = ShallowCopy<BaseExprElem>();
    for( auto one : children )
    {
      one->Unchain();
    }
    return WrapExpr( std::move( children ) );
  }

  void AttachChildren( std::vector<Expr>&& newChildren )
  {
    for( auto &expr: newChildren )
    {
      AppendExpr( std::move(expr) );
    }
  }

  /*
    各子要素に対してconvを実行し、結果のelemに差し替える。
  */
  void Transform( std::function<Expr(Expr&&)> conv )
  {
    auto children = DetachChildren();

    for( auto& one : children )
    {
      AppendExpr( conv( std::move( one ) ) );
    }
  }

  static ExprVector* FromExprs( std::vector<Expr>&& exprs )
  {
    auto ret = new ExprVector();
    ret->AttachChildren( std::move(exprs) );
    return ret;
  }

  template<typename T>
  static ExprVector* FromExprs( std::vector<ExprT<T>>&& exprs )
  {
    return FromExprs( ToExprVec( std::move(exprs) ) );
  }

};

struct ImmElem : public ExprElem<ImmElem>
{
  union ImmValue
  {
    int32_t _ival;
    uint32_t _uival;
    float _fval;

    ImmValue( int32_t ival ) : _ival(ival) {}
    ImmValue( uint32_t uival ) : _uival(uival) {}
    ImmValue( float fval ) : _fval(fval) {}

    bool operator==( const ImmValue& other ) const
    {
      return _ival == other._ival;
    }
  } u;

  ImmElem( Type t, int32_t ival ) : ExprElem<ImmElem>( t ), u(ival) {}
  ImmElem( Type t, uint32_t uival ) : ExprElem<ImmElem>( t ), u(uival) {}
  ImmElem( Type t, float fval ) : ExprElem<ImmElem>( t ), u(fval) {}

  static const IRElemType _elemType = IRElemType::ImmElem;

  ~ImmElem() override = default;

  IRElem* CloneSelf() const override
  {
    return new ImmElem( _type, u._ival );
  }

  bool operator==( const ImmElem& other ) const
  {
    return _type == other._type && u == other.u;
  }

  template<typename T> T Value() const;
};

template<> inline int32_t ImmElem::Value() const { return u._ival; }
template<> inline uint32_t ImmElem::Value() const { return u._uival; }
template<> inline float ImmElem::Value() const { return u._fval; }

// この辺はstatic methodにすべき。

inline Type Int( int bits )
{
  return Type( Type::INT, bits );
}

inline Type UInt( int bits )
{
  return Type( Type::UINT, bits );
}

inline Type Float( int bits )
{
  return Type( Type::FLOAT, bits );
}

inline Type TupleType( const std::vector<Type>& items )
{
  auto numerics = MapFn(items, [](const Type& tp) { return tp.AsNumeric(); } );
  return Type( std::move(numerics) );
}

inline Type RangeType()
{
  return Type( Type::RANGE );
}

inline Type ObjectType()
{
  return Type( Type::OBJECT );
}

// BGRA8に相当するu8の4次元ベクトルの型。よく使うので専用のファクトリを用意
inline Type U8V4Type()
{
  return TupleType( { UInt(8), UInt(8), UInt(8), UInt(8) } );
}

// BGRA16に相当するu16の4次元ベクトルの型。
inline Type U16V4Type()
{
  return TupleType( { UInt(16), UInt(16), UInt(16), UInt(16) } );
}

inline Type F32V4Type()
{
  return TupleType( { Float(32), Float(32), Float(32), Float(32) } );
}

inline Type F32V3Type()
{
  return TupleType( { Float(32), Float(32), Float(32) } );
}

// Cのuint32_t, int32_t, floatなどから対応するTypeを返す。
template<typename T> Type CType2Type();
template<> inline Type CType2Type<float>() { return Float(32); }
template<> inline Type CType2Type<uint32_t>() { return UInt(32); }
template<> inline Type CType2Type<int32_t>() { return Int(32); }

// ImmElemのコンストラクタを使うのでここで定義
Expr::Expr( int32_t x ) : Expr( new ImmElem( Int(32), x) ) {}
Expr::Expr( float x ) : Expr( new ImmElem( Float(32), x) ) {}

template<typename T>
Expr MakeConst( Type t, T val )
{
  if (t.IsInt())
  {
    return Expr( new ImmElem( t, (int32_t)val ));
  }
  else if (t.IsUInt())
  {
    return Expr( new ImmElem( t, (uint32_t)val ));
  }
  else if (t.IsFloat())
  {
    return Expr( new ImmElem( t, (float)val ));
  }
  else
    throw std::runtime_error("Unknown type");
}

inline Expr MakeZero( Type t )
{
  assert( t._code != Type::TUPLE ); // NYI
  return MakeConst( t, 0 );
}

template<typename T>
Expr MakeConst( NumericType nt, T val )
{
  return MakeConst( Type(nt), val );
}

inline Expr MakeZero( NumericType nt )
{
  return MakeConst( nt, 0 );
}

// パースの途中に現れる未解決のenum。
// ツリーにも入らずにExprとして返すためだけに存在する。
struct EnumElem : public ExprElem<EnumElem>
{
  size_t _symId;

  // この時点ではどのEnum型かは分かってないので、enumTypeIdにはダミーで0を入れておく。
  EnumElem( size_t symId )  : ExprElem<EnumElem>( Type::Enum( 0 ) ), _symId(symId) {}

  static const IRElemType _elemType = IRElemType::EnumElem;

  ~EnumElem() override = default;

  IRElem* CloneSelf() const override
  {
    return new EnumElem( _symId );
  }
};

/*
   Variable。Parameterなどに依存するのでここに置く。
*/
struct Variable : public ExprElem<Variable>
{
  std::string _name;
  bool _freeV; // free variableの参照（つまり現在のカーネルの外で定義された変数、典型的にはグローバル変数）の時はtrue。

  Variable( Type t, const std::string& name, bool freeV = false ) : ExprElem<Variable>(t), _name( name ), _freeV( freeV )
  {
  }

  static const IRElemType _elemType = IRElemType::Variable; 

  ~Variable() override = default;
  Variable( Variable&& other ) = default;

  IRElem* CloneSelf() const override
  {
    return new Variable( _type, _name, _freeV );
  }

  const std::string& Name() const { return _name; }

  bool operator==( const Variable& other ) const
  {
    if( _type != other._type )
      return false;
    return ( _name == other._name ); 
  }

  bool operator!=( const Variable& other ) const
  {
    return !( *this == other );
  }

  /*
    変数vがTupleの時に、変数のタプル、
    [v.0, v.1, v.2, ...]
    を返す。
    TはExprかExprV
  */
  template<typename T>
  std::vector<T> ExpandTuple() const
  {
    std::vector<T> ret;
    for( auto i : NRange( _type.ItemNum() ) )
    {
      ret.emplace_back( new Variable( _type._items[i], TupleItemName( Name(), i ), _freeV ) );
    }
    return ret;
  }

  // vからv.0などの名前を返す。
  // letなどでも使われるのでstaticで。
  static std::string TupleItemName( const std::string& baseName, size_t tupidx )
  {
    return baseName + "." + std::to_string(tupidx);
  }

};

int BaseExprElem::AsInt() const
{
  const ImmElem* imm = As<ImmElem>();
  if (imm->_type.IsInt())
    return imm->Value<int32_t>();

  assert( imm->_type.IsUInt() );
  return (int)imm->Value<uint32_t>();
}

std::ostream &operator<<(std::ostream &stream, const Expr &ir);

struct IfEl : public ExprElem<IfEl>
{
  /*
  子の内訳
  0: BaseExprElem* condition
  1: BaseExprElem* true_value
  2: BaseExprElem* false_value
  */
  BaseExprElem* GetCondition() const { return (BaseExprElem*) GetChild(0); }
  BaseExprElem* GetTrueValue() const { return (BaseExprElem*) GetChild(1); }
  BaseExprElem* GetFalseValue() const { return (BaseExprElem*) GetChild(2); }

  std::tuple<BaseExprElem*, BaseExprElem*, BaseExprElem*>
  DetachChildren()
  {
    auto cond = GetCondition();
    auto tval = GetTrueValue();
    auto fval = GetFalseValue();

    cond->Unchain();
    tval->Unchain();
    fval->Unchain();
    return std::make_tuple( cond, tval, fval );
  }

  void AttachChildren( Expr&& cond, Expr&& tval, Expr&& fval )
  {
    AppendChild( cond.Detach() );
    AppendChild( tval.Detach() );
    AppendChild( fval.Detach() );
  }

  static const IRElemType _elemType = IRElemType::IfEl; 

  IfEl( Expr&& cond,  Expr&& tvalue, Expr&& fvalue )
   : ExprElem<IfEl>( tvalue.GetType() )
  {
    AppendChild( cond.Detach() );
    AppendChild( tvalue.Detach() );
    AppendChild( fvalue.Detach() );
  }

  ~IfEl() override = default;

  IRElem* CloneSelf() const override
  {
    return new IfEl( _type );
  }

private:
  IfEl( Type type ) : ExprElem<IfEl>( type ) {}
};

// タプル。
//
// ツリーとして中身の要素が確定していて、いつでもツリーの段階で展開出来る。
//
// コンパイル結果は単にExprが複数コンパイルされる
struct TupleElem : public ExprElem<TupleElem>
{
  /*
    子の内訳。
    0: ExprVector items
  */
  ExprVector* GetItems() const
  {
    return GetChild<ExprVector>( 0 );
  }

  std::vector<Type> GetTypes() const
  { 
    return GetItems()->MapFn<BaseExprElem>(
        []( BaseExprElem* elem ){ return elem->_type; }
    );
  }

  /*
    各子要素に対してconvを実行し、その結果に差し替える。
  */
  void Transform( std::function<Expr(Expr&&)> conv )
  {
    GetItems()->Transform( conv );
  }

  std::vector<Expr> DetachChildren()
  {
    return GetItems()->DetachChildren();
  }


  BaseExprElem* Get( size_t argIdx ) const
  {
    return (BaseExprElem*)GetItems()->GetChild( argIdx );
  }

  // shallow copyを返す
  std::vector<BaseExprElem*> GetItemsSC() const
  {
    return GetItems()->ShallowCopy<BaseExprElem>();
  }

  // 可変じゃないのでコンストラクタえ持っておけば良いのだけど、
  // パフォーマンス的に問題が出るまでは毎回計算する（たぶん出ないので）
  size_t Size() const
  {
    return GetItems()->CountSize();
  }

  static const IRElemType _elemType = IRElemType::TupleElem; 



  TupleElem( std::vector<Expr>&& items ) : ExprElem<TupleElem>( TupleType( ToSpreadedTypes(items) ) )
  {
    AppendChild( ExprVector::FromExprs( std::move(items)) );
  }

  ~TupleElem() override = default;

  IRElem* CloneSelf() const override
  {
    return new TupleElem( _type );
  }

  std::vector<Expr> ExpandTuple()
  {
    return CloneBaseExprs( GetItemsSC() );
  }


private:
  TupleElem( Type type ) : ExprElem<TupleElem>( type ) {}
};

/*
  tup.xywy などを表すIRElem。
  tup.10なども表すので、indicesは3より大きい値のこともある。
*/
struct SwizzleCall : public ExprElem<SwizzleCall>
{
  /*
    tup.xywyの時には0, 1, 4, 1を持つ
  */
  std::vector<size_t> _indices;

  /*
    子の内訳。
    0: BaseExprElem* tupleTarget

    tupleTargetは必ずTuple型
  */
  BaseExprElem* GetTargetTuple() const
  {
    return (BaseExprElem*)GetChild( 0 );
  }

  static const IRElemType _elemType = IRElemType::SwizzleCall; 

  static Type ToType( const Expr& tupleTarget, const std::vector<size_t>& indices )
  {
    assert( tupleTarget.GetType().IsTuple() );
    std::vector<NumericType> tupTypes = tupleTarget.GetType().ExpandNumericTypes();

    if( indices.size() == 1 )
    {
      return Type( tupTypes[ indices[0] ] );
    }
    else
    {
      std::vector<NumericType> ntypes;

      for( auto tidx : indices )
      {
        assert( tidx < tupTypes.size() );
        ntypes.push_back( tupTypes[tidx] );
      }
      return Type( std::move(ntypes) );
    }
  }

  SwizzleCall( Expr&& tupleTarget, std::vector<size_t>&& indices ) : ExprElem<SwizzleCall>( ToType( tupleTarget, indices ) ), _indices( std::move(indices) )
  {
    AppendChild( tupleTarget.Detach() );
  }

  ~SwizzleCall() override = default;

  IRElem* CloneSelf() const override
  {
    auto ret = new SwizzleCall( _type );
    ret->_indices = _indices;
    return ret;
  }

private:
  SwizzleCall( Type type ) : ExprElem<SwizzleCall>( type ) {}
};

/*
  Exprのリストを持つElement。

  最後のExpr以外はVoid Expr。
  最後のExprはこのBodyElemのreturn valueとなる。
  最後のBodyElemがVoidの場合はこのBodyElemはstmt的なものだけを持つ要素という事になる。

  ブロックのbody、Forのbodyなど。
*/
struct BodyElem : public ExprElem<BodyElem>
{
  /*
    子供内訳
    0: ExprVector* exprs
  */
  ExprVector* GetExprs() const { return GetChild<ExprVector>(0); }

  BaseExprElem* GetReturnExpr() const { return (BaseExprElem*)GetExprs()->GetLastChild(); }
  std::vector<BaseExprElem*> GetExprsWithoutReturn() const
  {
    auto whole = GetExprs()->ShallowCopy<BaseExprElem>();
    whole.pop_back();
    return whole;
  }

  /*
    GetExprs()のchildrenをdetachしてExprとして返す。
  */
  std::vector<Expr> DetachExprs()
  {
    return GetExprs()->DetachChildren();
  }

  static const IRElemType _elemType = IRElemType::BodyElem;

  // bodyの最後がReturnExpr。
  BodyElem( std::vector<Expr>&& body ) : ExprElem<BodyElem>( body.back().GetType() )
  {
    assert( body.size() >= 1 );
    auto bodyVec = new ExprVector();
    for( auto& stmt: body)
    {
      bodyVec->Append( stmt.Detach() );
    }
    AppendChild( bodyVec );
  }

  ~BodyElem() override = default;

  IRElem* CloneSelf() const override
  {
    return new BodyElem( _type );
  }

private:
  BodyElem( Type t ) : ExprElem<BodyElem>( t ){}
};

using ExprBODY = ExprT<BodyElem>;

// 引数とBodyを持つブロックを表す。これはExprで、実行する方は別のノードとなる。
// バックエンドによってはCallをそのまま実行しこれを経由しない。
// シェーダーなどのreturn valueなどに制約のある環境では何らかの形に展開して実行する。
struct BlockElem : public ExprElem<BlockElem>
{
  std::vector<std::pair<std::string, Type>> _args;

  // _argsのうちbegからendまでの範囲の"_"をtmpVarの名前に置き換える。
  // endは含まない。
  void BlankId2TmpVar( size_t beg, size_t end )
  {
    assert( _args.size() >= end );
    for (auto i = beg; i < end; i++)
    {
      if (_args[i].first == "_")
      {
        _args[i] = std::make_pair( UniqueName('t'), _args[i].second );
      }
    }
  }

  /*
    子供の内訳

    0: BodyElem* body
  */
  BodyElem* GetBody() const { return GetChild<BodyElem>( 0 ); }

  ExprBODY DetachBody()
  {
    auto body = GetBody();
    body->Unchain();
    return ExprBODY( body );
  }

  void AttachBody( ExprBODY&& body )
  {
    AppendChild( body.Detach() );
  }

  BaseExprElem* GetReturnExpr() const
  {
    return GetBody()->GetReturnExpr();
  }

  Type GetReturnType() const { return GetReturnExpr()->_type; }

  static const IRElemType _elemType = IRElemType::BlockElem;

  BlockElem( std::vector<std::pair<std::string, Type>>&& args, ExprBODY&& vblock ) : ExprElem<BlockElem>( ObjectType() ), _args( std::move(args) )
  {
    AppendChild( vblock.Detach() );
  }

  ~BlockElem() override = default;

  IRElem* CloneSelf() const override
  {
    auto ret = new BlockElem();
    ret->_args = _args;
    return ret;
  }

private:
  // クローン用
  BlockElem() : ExprElem<BlockElem>( ObjectType() ) {}
};

// 今の所BlockElemなので別名にしておく。
// ただしBlockElemはRSumなどでも使われているので、区別はしたい。
using TensorElem = mfg_internal::BlockElem;

struct BlockConsumer
{
  virtual BlockElem* GetBlock() const = 0;
  virtual const std::string& Name() const = 0;

  Type GetReturnType() const
  {
    return GetBlock()->GetReturnType();
  }
};

struct RangeElem : public ExprElem<RangeElem>
{
  /*
  子の内訳
  0: BaseExprElem* begin
  1: BaseExprElem* end(exclusive)
  */
  BaseExprElem* GetBegin() const { return (BaseExprElem*) GetChild(0); }
  BaseExprElem* GetEnd() const { return (BaseExprElem*) GetChild(1); }

  static const IRElemType _elemType = IRElemType::RangeElem; 

  RangeElem( Expr&& beginExpr,  Expr&& endExpr )
   : RangeElem()
  {
    AppendChild( beginExpr.Detach() );
    AppendChild( endExpr.Detach() );
  }

  ~RangeElem() override = default;

  IRElem* CloneSelf() const override
  {
    return new RangeElem();
  }

private:
  RangeElem() : ExprElem<RangeElem>( RangeType() ) {}
};

using ExprRANGE = ExprT<RangeElem>;

struct ReduceSum : public ExprElem<ReduceSum>, public BlockConsumer
{
  std::string _name;

  /*  
  子の内訳
  0: ExprVector* ranges
  1: BlockElem* block

  rangesは次元 個の要素で、RangeElemが入っている。 
  */
  ExprVector* GetRanges() const { return GetChild<ExprVector>( 0 ); }
  BlockElem* GetBlock() const override { return GetChild<BlockElem>( 1 ); }
  const std::string& Name() const override { return _name; }

  static const IRElemType _elemType = IRElemType::ReduceSum; 
  size_t Dimensions() const { return GetRanges()->CountSize(); }


  ReduceSum( std::vector<ExprRANGE>&& ranges, ExprT<BlockElem>&& fobj ) : ReduceSum( UniqueName( 'r' ), fobj.GetElem()->GetReturnType() )
  {
    AppendChild( ExprVector::FromExprs( std::move(ranges) ) );
    AppendChild( fobj.Detach() );
  }

  ~ReduceSum() override = default;

  IRElem* CloneSelf() const override
  {
    return new ReduceSum( _name, _type );
  }

private:
  ReduceSum( const std::string& name, Type t ) : ExprElem<ReduceSum>( t ), _name( name ) {}
};


// ReduceSumと似ているが、initがある所だけ違う
// 共通化した方がいいかも。
struct Reduce : public ExprElem<Reduce>, public BlockConsumer
{
  std::string _name;

  /*  
  子の内訳
  0: BaseExprElem* init
  1: ExprVector* ranges
  2: BlockElem* block

  rangesは次元個の要素で、それぞれRangeElemが入っている。 
  */
  BaseExprElem* GetInitValue() const { return (BaseExprElem*)GetChild( 0 ); }
  ExprVector* GetRanges() const { return GetChild<ExprVector>( 1 ); }
  BlockElem* GetBlock() const override { return GetChild<BlockElem>( 2 ); }
  const std::string& Name() const override { return _name; }

  static const IRElemType _elemType = IRElemType::Reduce; 
  size_t Dimensions() const { return GetRanges()->CountSize(); }


  Reduce( Expr&& initValue, std::vector<ExprRANGE>&& ranges, ExprT<BlockElem>&& fobj ) : Reduce( UniqueName( 'r' ), fobj.GetElem()->GetReturnType() )
  {
    AppendChild( initValue.Detach() );
    AppendChild( ExprVector::FromExprs( std::move(ranges) ) );
    AppendChild( fobj.Detach() );
  }

  ~Reduce() override = default;

  IRElem* CloneSelf() const override
  {
    return new Reduce( _name, _type );
  }

private:
  Reduce( const std::string& name, Type t ) : ExprElem<Reduce>( t ), _name( name ) {}
};

// Tensorに関する情報を保持するPOJO。
struct TensorInfo
{
  std::string _name;
  bool _isLocal;
  size_t _dim;
  Type _type;

  TensorInfo( std::string name, bool isLocal, size_t dim, Type tp ) : _name( std::move(name) ), _isLocal( isLocal ), _dim( dim ), _type( std::move(tp) ) {}
  TensorInfo( const TensorInfo& ) = default;
  TensorInfo( TensorInfo&& ) = default;
  TensorInfo& operator=( const TensorInfo& ) = default;

  const std::string &Name() const{ return _name; }

  Type GetType() const
  {
    return _type;
  }  

  size_t Dimensions() const { return _dim; }

  bool IsLocal() const { return _isLocal; }

  bool operator==( const TensorInfo& dst ) const {
    // 名前が同じテンソルはいつも同じだろう。
    return _name == dst.Name();
  }

  bool IsVector() const { return GetType().IsVector(); }

  // ベクトルで無い時は1次元とする。
  size_t VectorExtent() const
  {
    return GetType().VectorExtent();
  }

  // かつてインターフェースを使った便利メソッド達だったもの
  // 今やほとんどTypeのmethodなので廃止しても良いかも。
  // テンソル関連は要素一つのタプルとNumericを同一視する所だけ違う。
  std::vector<NumericType> GetElemTypes() const { return GetType().ExpandNumericTypes(); }
  size_t ElemNum() const { return GetType()._items.size(); }
  Type ElemType( size_t idx ) const  { return GetType().ElemType( idx ); }

  // バックエンドのバッファの数。
  // ここにあるべきでは無い気もするが、ElemBufferName同様いまいち良い置き場が無いのでここに置く。
  size_t ElemBufferNum() const
  {
    if (IsVector())
      return 1;
    return ElemNum();
  }

  // バックエンドのバッファの種類。
  // ベクトルの場合は同じ型を一つのバッファに詰めるので一つになる。
  std::vector<NumericType> GetElemBufferTypes() const
  { 
    if (IsVector())
      return { ElemType(0).AsNumeric() };

    return GetElemTypes();
  }

  std::string GetExtentName( size_t dim ) const
  {
    return "extent." + Name() + "." + std::to_string( dim );
  }

  // to_ncoordなど、自身のextentとわかっているケースではこちらを呼ぶ。
  // 作られる変数はフリーvariableでは無いと判断出来るので、Referenceで二重定義になるのを防げる。
  Expr GetSelfExtent( size_t dim ) const
  {
    return Expr( new Variable( Int(32), GetExtentName(dim), false ) );
  }

  Expr GetExtent( size_t dim ) const
  {
    // 自身のExtentの時にこれを呼ぶとFreeVariableとしてしまうので、
    // そのケースは呼び出し側でGetSelfExtentを呼ぶことにする
    return Expr( new Variable( Int(32), GetExtentName(dim), !IsLocal() ) );
  }

  size_t BufIndex( size_t tupIndex ) const
  {
    return IsVector() ? 0 : tupIndex;
  }

  size_t VectorIndex( size_t tupIndex ) const
  {
    return IsVector() ? tupIndex : 0;
  }

  std::string ElemBufferName( size_t tupIdx ) const
  {
    return "b_" + Name() + "." + std::to_string( BufIndex(tupIdx) );
  }
};

/*
  テンソルをイテレーションするIRElem。
  ts.sumはexprで、ts.for_eachなどはstmt的なので、このIRElemは_typeを見て挙動が大きく変わる事になる。
*/
struct TensorIterator : public ExprElem<TensorIterator>, public BlockConsumer
{
  enum IterationType { REDUCE_SUM, EXEC_FOREACH };
  IterationType _itype;

  std::string _name;
  TensorInfo _tsinfo;

  const std::string& TensorName() const { return _tsinfo.Name(); }
  const TensorInfo& GetTensorInfo() const { return _tsinfo; }

  bool IsStmtLike() const
  {
    return _itype == EXEC_FOREACH;
  }

  /*  
  子の内訳
  0: BlockElem* block
  */
  BlockElem* GetBlock() const override { return (BlockElem*)GetChild( 0 ); }
  const std::string& Name() const override { return _name; }

  static const IRElemType _elemType = IRElemType::TensorIterator; 

  TensorIterator( IterationType itype,  const TensorInfo& ts, ExprT<BlockElem>&& fobj ) : TensorIterator( itype, UniqueName('r'), ts, fobj.GetElem()->GetReturnType() )
  {
    AppendChild( fobj.Detach() );
  }

  ~TensorIterator() override = default;

  IRElem* CloneSelf() const override
  {
    auto tsinfo = _tsinfo;
    return new TensorIterator( _itype, _name, std::move(tsinfo), _type );
  }

  const char* DisplayName() const
  {
    const char* const disp_names[] = {
      "tensor_sum",
      "for_each"
    };
    return disp_names[_itype];
  }

private:
  TensorIterator( IterationType itype, const std::string& name, const TensorInfo& tsinfo, Type t ) : ExprElem<TensorIterator>( t ), _itype(itype),  _name( name ), _tsinfo( tsinfo ) {}
};

struct SamplerElem: public ExprElem<SamplerElem>
{
  enum AddressType
  {
    NORMAL_EDGE,
    CLAMP_TO_EDGE,
    CLAMP_TO_BORDER_VALUE
  };

  enum CoordType
  {
    PIXEL_COORD,
    NORMALIZED_COORD_NEARESTNEIGHBOR,
    NORMALIZED_COORD_BILINEAR
  };

  AddressType _addressType;
  CoordType _coordType;
  TensorInfo _target;

  /*
    子の内訳。
    0: ExprVector* edgeArgs
  */
  ExprVector* GetEdgeArgs() const
  {
    return GetChild<ExprVector>( 0 );
  }

  BaseExprElem* GetEdgeArg( int argIdx ) const
  {
    return (BaseExprElem*)GetEdgeArgs()->GetChild( argIdx );
  }


  static const IRElemType _elemType = IRElemType::SamplerElem;

  SamplerElem( AddressType addressType, CoordType coordType, const TensorInfo& target, std::vector<Expr>&& edgeArgs ) : SamplerElem( addressType, coordType, target )
  {
    AppendChild( ExprVector::FromExprs( std::move(edgeArgs) ) );
  }

  ~SamplerElem() override = default;

  IRElem* CloneSelf() const override
  {
    return new SamplerElem( _addressType, _coordType, _target );
  }

private: 
  SamplerElem( AddressType addressType, CoordType coordType, const TensorInfo& target ) : ExprElem<SamplerElem>( ObjectType() ), _addressType( addressType ), _coordType( coordType ), _target( target )
  {
  }
};

struct SamplerCall: public ExprElem<SamplerCall>
{
  std::string _name; // Hoistする時の変数名。Hoistしない時は使わない。lazyに初期化。

  /*
    子の内訳
    0: SamplerElem* def
    1: ExprVector* args
  */

 SamplerElem* GetDef() const
 {
  return GetChild<SamplerElem>( 0 );
 }

  ExprVector* GetArgs() const
  {
    return GetChild<ExprVector>( 1 );
  }

  const std::string& Name()
  {
    if (_name == "")
      _name = UniqueName( 'r' );
    return _name;
  }

  static const IRElemType _elemType = IRElemType::SamplerCall;

  SamplerCall( const SamplerElem* sampDef, std::vector<Expr>&& args ) : SamplerCall( sampDef->_target.GetType(), std::string() )
  {
    AppendChild( sampDef->Clone() );
    AppendChild( ExprVector::FromExprs( std::move(args) ) );
  }

  SamplerCall( ExprT<SamplerElem>&& sampDef, std::vector<Expr>&& args ) : SamplerCall( sampDef.GetElem()->_target.GetType(), std::string() )
  {
    AppendChild( sampDef.Detach() );
    AppendChild( ExprVector::FromExprs( std::move(args) ) );
  }


  IRElem* CloneSelf() const override
  {
    return new SamplerCall( _type, _name );
  }

  ~SamplerCall() override = default;

private:
  SamplerCall( Type type, const std::string& name ) : ExprElem<SamplerCall>( type ), _name( name )
  {
  }
};


struct SpreadElem: public ExprElem<SpreadElem>
{
  /*
    子の内訳。
    0: BaseExprElem target

    targetはタプル型
  */
  BaseExprElem* GetTarget() const
  {
    return (BaseExprElem*)GetChild( 0 );
  }

  /*  
  BaseExprElem* DetachTarget()
  {
    auto target = GetTarget();
    target->Unchain();
    return target;
  }
  */

  static const IRElemType _elemType = IRElemType::SpreadElem;

  SpreadElem( Expr&& target ) : SpreadElem()
  {
    AppendChild( target.Detach() );
  }

  ~SpreadElem() override = default;

  IRElem* CloneSelf() const override
  {
    return new SpreadElem();
  }

private: 
  SpreadElem() : ExprElem<SpreadElem>( ObjectType() )
  {
  }
};

// Spreadのときは展開した型を、それ以外は要素の型を返す。
inline std::vector<Type> ToSpreadedTypes( const std::vector<Expr>& items )
{
  std::vector<Type> ret;
  for( auto &expr : items )
  {
    if (expr.GetElemType() == IRElemType::SpreadElem)
    {
      auto expandedNTypes = expr.As<SpreadElem>()->GetTarget()->_type.ExpandNumericTypes();
      for( auto ntype : expandedNTypes )
      {
        ret.emplace_back( ntype ); // NumericTypeのコンストラクタがある
      }
    }
    else
    {
      ret.push_back( expr.GetType() );
    }
  }
  return ret;
}

// Spreadのときは展開した型を、それ以外は要素の型を返す。
inline std::vector<Type> ToSpreadedTypes( const std::vector<BaseExprElem*>& items )
{
  std::vector<Type> ret;
  for( auto elem : items )
  {
    if (elem->_elemType == IRElemType::SpreadElem)
    {
      auto expandedNTypes = elem->As<SpreadElem>()->GetTarget()->_type.ExpandNumericTypes();
      for( auto ntype : expandedNTypes )
      {
        ret.emplace_back( ntype ); // NumericTypeのコンストラクタがある
      }
    }
    else
    {
      ret.push_back( elem->_type );
    }
  }
  return ret;
}


/*
  算術演算関連。
*/

// BinOpの子供のポインタを保持するだけの構造体。
// Detachなどで使う。
struct BinOpChildren
{
  BaseExprElem* _left;
  BaseExprElem* _right;
};

struct BinOp: public ExprElem<BinOp>
{
  // コンベンション的には大文字だが、
  // IRElemTypeから持ってきたのでしばらくここだけ大文字始まりのコンベンションで。
  enum BinOpType
  {
    Add,
    Sub,
    Mul,
    Div,
    Mod,
    BitwiseAnd,
    BitwiseOr,
    BitwiseXor,
    ShiftRight,
    ShiftLeft,
    Eq,
    Neq, // not equal
    Lt,
    Le,
    Ge,
    Gt,
    Or,
    And
  };

  BinOpType _opType;


  /*
  子供の内訳
  0: left (BaseExprElem)
  1: right (BaseExprElem)
  */
  BaseExprElem* GetLeft() const { return (BaseExprElem*) IRElem::GetChild(0); }
  BaseExprElem* GetRight() const { return (BaseExprElem*) IRElem::GetChild(1); }


  /*
    [a, b, c]*d 的なケース
  */
  bool IsLeftTuple() const
  {
    return GetLeft()->_type.IsTuple();
  }

  /*
    a*[b, c, d] 的なケース
  */
  bool IsRightTuple() const
  {
    return GetRight()->_type.IsTuple();
  }

  bool IsVectorize() const
  {
    return IsLeftTuple() || IsRightTuple();
  }

  std::pair<BaseExprElem*, BaseExprElem*> DetachChildren()
  {
    auto left = GetLeft();
    auto right = GetRight();
    left->Unchain();
    right->Unchain();
    return std::make_pair( left, right );
  }

  BinOpChildren
  DetachChlidrenAsStruct()
  {
    BinOpChildren children;
    std::tie( children._left, children._right ) = DetachChildren();
    return children;
  }

  void AttachChildren( BaseExprElem* left, BaseExprElem* right )
  {
    IRElem::AppendChild( left );
    IRElem::AppendChild( right );    
  }

  static const IRElemType _elemType = IRElemType::BinOp; 
  BinOp( Type tp, BinOpType opType, Expr&& a, Expr&& b ) : BinOp( tp, opType )
  {
    AppendChild( a.Detach() );
    AppendChild( b.Detach() );
  }

  BinOp( BinOpType opType, Expr&& a, Expr&& b ) : BinOp(  a.GetType(), opType, std::move(a), std::move(b) )
  {
  }

  ~BinOp() override = default;

  IRElem* CloneSelf() const override
  {
    return new BinOp( _type, _opType );
  }

  static std::string ToDisplayName( BinOpType binOpType )
  {
    switch( binOpType )
    {
      {
        case Add:
          return "+";
        case Sub:
          return "-";
        case Mul:
          return "*";
        case Div:
          return "/";
        case Mod:
          return "%";
        case Eq:
          return "==";
        case Neq:
          return "!=";
        case BitwiseAnd:
          return "&";
        case BitwiseOr:
          return "|";
        case BitwiseXor:
          return "^";
        case ShiftRight:
          return ">>";
        case ShiftLeft:
          return "<<";
        case Or:
          return "||";
        case And:
          return "&&";
        case Lt:
          return "<";
        case Le:
          return "<=";
        case Gt:
          return ">";
        case Ge:
          return ">=";
        default:
          assert(false);
          return "";
      }
    }
  }

  static bool IsBoolOp( BinOpType opType )
  {
    switch( opType )
    {
      case Eq:
      case Neq:
      case Or:
      case And:
      case Lt:
      case Le:
      case Gt:
      case Ge:
        return true;
      default:
        return false;
    }
  }

  std::string DisplayName() const
  {
    return ToDisplayName( _opType );
  }

protected:
  explicit BinOp( Type t, BinOpType opType ) : ExprElem<BinOp>( t ), _opType( opType ) {}
};

/* VoidExpr関連(Stmtっぽいもの） */

struct Let : public VoidExprElem<Let>
{
  std::string _name;
  bool _public;

  bool IsPublic() const { return _public; }

  /*
  子の内訳
  0: BaseExprElem* value
  */
  BaseExprElem* GetValue() const { return (BaseExprElem*) GetChild(0); }

  static const IRElemType _elemType = IRElemType::Let; 
  Let( const std::string& name, Expr&& value, bool isPublic = true ) : Let( name, isPublic )
  {
    AppendChild( value.Detach() );
  }

  ~Let() override = default;

  static Let* MakePrivate( const std::string& name, Expr&& value )
  {
    return new Let( name, std::move(value), false );
  }

  IRElem* CloneSelf() const override
  {
    return new Let( _name, _public );
  }

  const Type& GetValueType() const
  {
    return GetValue()->_type;
  }

private:
  Let( const std::string& name, bool isPublic ) : VoidExprElem<Let>(), _name( name ), _public( isPublic ) {}
};

using ExprLET = ExprT<Let>;

/*
  HoistされたVectorのLet。
  _nameの名前に.0, .1, ... とつけた名前の変数をバックエンドで生成する責任を持つ。
  データとしてはLetと一緒だが、役割が違う。
  Valueとしてはベクトルを返すCallやLoadExprなどしか来ない。
*/
struct VectorLet : public VoidExprElem<VectorLet>
{
  std::string _name;

  /*
  子の内訳
  0: BaseExprElem* value
  */
  BaseExprElem* GetValue() const { return (BaseExprElem*) GetChild(0); }

  static const IRElemType _elemType = IRElemType::VectorLet; 
  VectorLet( const std::string& name, Expr&& value ) : VectorLet( name )
  {
    assert( value.GetType().IsVector() );
    AppendChild( value.Detach() );
  }

  ~VectorLet() override = default;

  IRElem* CloneSelf() const override
  {
    return new VectorLet( _name );
  }

  const Type& GetValueType() const
  {
    return GetValue()->_type;
  }

private:
  VectorLet( const std::string& name ) : VoidExprElem<VectorLet>(), _name( name ) {}
};


struct TupleLet : public VoidExprElem<TupleLet>
{
  std::vector<std::string> _names;

  /*
  子の内訳
  0: BaseExprElem* value
  */
  BaseExprElem* GetValue() const { return (BaseExprElem*) GetChild(0); }

  Expr DetachValue()
  {
    auto rbase = GetValue();
    rbase->Unchain();
    return Expr( rbase );
  }

  static const IRElemType _elemType = IRElemType::TupleLet; 
  TupleLet( std::vector<std::string>&& names, Expr&& value ) : TupleLet( std::move( names ) )
  {
    AppendChild( value.Detach() );
  }

  ~TupleLet() override = default;

  IRElem* CloneSelf() const override
  {
    auto dup = _names;
    return new TupleLet( std::move(dup) );
  }

  const Type& GetType() const
  {
    return GetValue()->_type;
  }

private:
  TupleLet( std::vector<std::string>&& names ) : VoidExprElem<TupleLet>(), _names( std::move(names) ) {}
};

/*
  mutableな変数を定義するlet。
  var a = rexpr
*/
struct VarLet : public VoidExprElem<VarLet>
{
  std::string _name;
  /*
  子の内訳
  0: BaseExprElem* value
  */
  BaseExprElem* GetValue() const { return (BaseExprElem*) GetChild(0); }

  static const IRElemType _elemType = IRElemType::VarLet; 
  VarLet( const std::string& name, Expr&& value ) : VarLet( name )
  {
    AppendChild( value.Detach() );
  }

  ~VarLet() override = default;

  IRElem* CloneSelf() const override
  {
    return new VarLet( _name );
  }

  const Type& GetType() const
  {
    return GetValue()->_type;
  }

private:
  VarLet( const std::string& name ) : VoidExprElem<VarLet>(), _name( name ) {}
};

/*
  ローカルTensorの初期化の実行を表すstmt。
*/
struct ExecTensorInitialize : public VoidExprElem<ExecTensorInitialize>, public BlockConsumer
{
  std::string _rname;
  std::vector<size_t> _bounds;

  /*  
  子の内訳
  0: TensorElem* funcobj
  */

  TensorElem* GetTensor() const { return GetChild<TensorElem>( 0 ); }

  static const IRElemType _elemType = IRElemType::ExecTensorInitialize;

  ExecTensorInitialize( std::vector<size_t>&& bounds, ExprT<TensorElem>&& fobj ) : VoidExprElem<ExecTensorInitialize>(), _rname( UniqueName( 'r' ) ), _bounds( std::move(bounds) )
  {
    AppendChild( fobj.Detach() );
  }

  ~ExecTensorInitialize() override = default;

  IRElem* CloneSelf() const override
  {
    return new ExecTensorInitialize( _rname, _bounds );
  }

  TensorInfo GetTensorInfo() const
  {
    // これはいつでもローカルテンソル
    return TensorInfo( _rname, true, GetTensor()->_args.size(), GetTensor()->GetReturnType() );
  }

  /*
    BlockConsumerの実装
  */
  BlockElem* GetBlock() const override { return (BlockElem*)GetTensor(); }
  const std::string& Name() const override { return _rname; }

private:
  ExecTensorInitialize( const std::string& name, const std::vector<size_t>& bounds ) : VoidExprElem<ExecTensorInitialize>(), _rname( name ), _bounds( bounds )
  {}
};

/*
  今の所Transformはaccumulationのみ。
*/
struct TransformTensor : public VoidExprElem<TransformTensor>
{
  std::string _name;
  TensorInfo _tsinfo;

  enum MethodType { ACCM, SORT };

  MethodType _method;

  const std::string& TensorName() const { return _tsinfo.Name(); }
  const TensorInfo& GetTensorInfo() const { return _tsinfo; }

  /*  
  子の内訳
  0: ExprVector args
  */
  ExprVector* GetArgs() const
  {
    return GetChild<ExprVector>( 0 );
  }

  BaseExprElem* GetArg( int argIdx ) const
  {
    return (BaseExprElem*)GetArgs()->GetChild( argIdx );
  }

  const std::string& Name() const { return _name; }

  static const IRElemType _elemType = IRElemType::TransformTensor; 

  TransformTensor( TensorInfo&& ts, MethodType mtype, std::vector<Expr>&& args ): TransformTensor( UniqueName('r'), mtype, std::move(ts) )
  {
    AppendChild( ExprVector::FromExprs( std::move(args) ) );
  }

  ~TransformTensor() override = default;

  IRElem* CloneSelf() const override
  {
    auto tsinfo = _tsinfo;
    return new TransformTensor( _name, _method, std::move(tsinfo) );
  }

private:
  TransformTensor( const std::string& name, MethodType mtype, TensorInfo&& tsinfo ) : VoidExprElem<TransformTensor>(), _name( name ), _tsinfo( std::move(tsinfo) ), _method( mtype ) {}
};

struct TransAccmAccessor : public BlockConsumer
{
  TransformTensor& _trans;
  /*
    argsの内訳
    0: BaseExprElem* TargetDim
      今の所これはi32のImmElemのみ。だがIRElemとしてはBaseExprElemで持っておく。
    1: BlockElem* block
  */

  BaseExprElem* GetTargetDim() const { return _trans.GetArg( 0 ); }
  int32_t GetTargetDimAsImm() const { return GetTargetDim()->As<ImmElem>()->Value<int32_t>(); }

  BlockElem* GetBlock() const override { return (BlockElem*)_trans.GetArg( 1 ); }

  const std::string& Name() const override { return _trans.Name(); }

  TransAccmAccessor( TransformTensor& trans ) : _trans( trans )
  {
  }

  static void SkipToBlock( FNode::iterator& iter )
  {
    iter++; // ExprVector
    iter.SkipNChildren( 1 ); // targetDimを飛ばす。
  }
};

/*
  side effect専用のCall。
  backendの関数名をそのまま_nameに持つ。
*/
struct CallStmt : public VoidExprElem<CallStmt>
{
  std::string _name;

  /*  
  子の内訳
  0: ExprVector args
  */
  ExprVector* GetArgs() const
  {
    return GetChild<ExprVector>( 0 );
  }

  BaseExprElem* GetArg( int argIdx ) const
  {
    return (BaseExprElem*)GetArgs()->GetChild( argIdx );
  }

  const std::string& Name() const { return _name; }

  static const IRElemType _elemType = IRElemType::CallStmt; 

  CallStmt( const std::string& name, std::vector<Expr>&& args ): CallStmt( name )
  {
    AppendChild( ExprVector::FromExprs( std::move(args) ) );
  }

  ~CallStmt() override = default;

  IRElem* CloneSelf() const override
  {
    return new CallStmt( _name );
  }

private:
  CallStmt( const std::string& name ) : VoidExprElem<CallStmt>(), _name( name ) {}
};

/*
  ReduceTensorUtil:

  DefByReduceとReduceToZeroの共通部分のクラス。
  戻りの型が違う以外はほとんど同じ。
  だがVoidExprElemとExprElemを区別したいので、継承では無くコンポジションで共通部分の処理を行う。

  本来は継承でやりたい事を多重継承を避けるためにコンポジションをしているため、
  コンストラクタで子どもをappendしたり、Clone用のコンストラクタがあったりと使う側が注意して使う必要がある。

  TransformTensorとExecTensorInitializeの両方を持った感じ。
  特にTransformTensorとは類似しているが、共通化はもうちょっと必要なものがはっきりしてからにしたいので分けておく。
  TransformTensorとはinitがある所が違う。

  _rnameはBlockConsumerのidであってdefに出てくるidはこのIRElemは持たない（内部でもパース時のlookupでしか使われないので）。

*/
struct ReduceTensorUtil
{
  BaseExprElem* _elem;
  std::string _rname;
  TensorInfo _tsinfo;

  const std::string& GetSrcTensorName() const { return _tsinfo.Name(); }
  const TensorInfo& GetSrcTensorInfo() const { return _tsinfo; }

  /*  
  子の内訳
  0: BaseExprElem* TargetDim
     今の所これはi32のImmElemのみ。だがIRElemとしてはBaseExprElemで持っておく。
  1: BaseExprElem* init
  2: BlockElem* block
  */
  BaseExprElem* GetTargetDim() const { return (BaseExprElem*)_elem->GetChild( 0 ); }
  int32_t GetTargetDimAsImm() const { return (int32_t)GetTargetDim()->As<ImmElem>()->Value<int32_t>(); }

  BaseExprElem* GetInitValue() const { return (BaseExprElem*)_elem->GetChild( 1 ); }

  BlockElem* GetBlock() const { return (BlockElem*)_elem->GetChild( 2 ); }
  const std::string& RName() const { return _rname; }

  // dim次元のextentの対応するsrcのextentのdim
  // const int32_t dest_extent_0 = src_extent_0;
  // のような文を作る時に使う。
  //
  // reduceでtargetDimが潰れるので、その前後で
  // 返す値が変わる。 
  //
  // targetDimより前は引数をそのまま返し、
  // targetDimより後ろは+1したものを返す。
  size_t GetSrcExtentDim( size_t dim ) const
  {
    if (dim < (size_t)GetTargetDimAsImm())
      return dim;
    return dim + 1;
  }

  ReduceTensorUtil( BaseExprElem* elem, const std::string& name, TensorInfo&& tsinfo ) : _elem( elem ), _rname( name ), _tsinfo( std::move(tsinfo) ) {}

  ReduceTensorUtil( BaseExprElem* elem, TensorInfo&& ts, Expr&& targetDim, Expr&& initValue, ExprT<BlockElem>&& fobj ) : ReduceTensorUtil( elem, UniqueName('r'), std::move(ts) )
  {
    assert( targetDim.GetElemType() == IRElemType::ImmElem && targetDim.GetType().IsInt() );
    _elem->AppendChild( targetDim.Detach() );
    _elem->AppendChild( initValue.Detach() );
    _elem->AppendChild( fobj.Detach() );
  }

  static void SkipToBlock( FNode::iterator& iter )
  {
    iter.SkipNChildren( 2 );
  }
};

/*
  Tensorのreduceのうち、reduceの結果が0次元、つまり変数になるケース。
  このケースは単なる値のあるExprとして扱える。
*/
struct ReduceToScalar : public ExprElem<ReduceToScalar>, public BlockConsumer
{
  static const IRElemType _elemType = IRElemType::ReduceToScalar; 
  ReduceTensorUtil _rutil;

  // Typeはinitかaccmの型。initの型を使う。
  ReduceToScalar( TensorInfo&& ts, Expr&& targetDim, Expr&& initValue, ExprT<BlockElem>&& fobj ) : ExprElem<ReduceToScalar>( initValue.GetType() ), _rutil( this, std::move(ts), std::move(targetDim), std::move(initValue), std::move(fobj) )
  {
  }

  ~ReduceToScalar() override = default;

  /*
    以下ReduceToSclarとDefByReduceでほとんど共通。
  */

  /* _rutilへデリゲート。 */
  const std::string& GetSrcTensorName() const { return _rutil.GetSrcTensorName(); }
  const TensorInfo& GetSrcTensorInfo() const { return _rutil.GetSrcTensorInfo(); }
  BaseExprElem* GetTargetDim() const { return _rutil.GetTargetDim(); }
  int32_t GetTargetDimAsImm() const { return _rutil.GetTargetDimAsImm(); }
  BaseExprElem* GetInitValue() const { return _rutil.GetInitValue(); }
  size_t GetSrcExtentDim( size_t dim ) const { return _rutil.GetSrcExtentDim( dim ); }

  static void SkipToBlock( FNode::iterator& iter ) { ReduceTensorUtil::SkipToBlock( iter ); }

  /*
    BlockConsumerの実装
  */
  BlockElem* GetBlock() const override { return _rutil.GetBlock(); }
  const std::string& Name() const override { return _rutil.RName(); }

  /*
    Clone実装
  */
  IRElem* CloneSelf() const override
  {
    auto tsinfo = _rutil._tsinfo;
    return new ReduceToScalar( _type, _rutil.RName(), std::move(tsinfo) );
  }

private:
  // クローンでしか使わない。ReduceTensorUtilのコンストラクタが違う
  ReduceToScalar( Type type, const std::string& rname, TensorInfo&& tsinfo ) : ExprElem<ReduceToScalar>( type ), _rutil( this, rname, std::move(tsinfo) ) {}
};


/*
  reduceの結果がテンソルになるケース。
  だからIRElemとしてはVoidExprとして扱い、生成の時にも特別扱いをする。

  だがVoidExpr自体を継承するのは継承ツリーが複雑になるので、単に_typeをUNITにしたBaseExprElemとしている。
  （dynamic castとか使わないのでこれで問題無い）
*/
struct DefByReduce : public VoidExprElem<DefByReduce>, public BlockConsumer
{
  static const IRElemType _elemType = IRElemType::DefByReduce; 
  ReduceTensorUtil _rutil;

  DefByReduce( TensorInfo&& ts, Expr&& targetDim, Expr&& initValue, ExprT<BlockElem>&& fobj ) : VoidExprElem<DefByReduce>(), _rutil( this, std::move(ts), std::move(targetDim), std::move(initValue), std::move(fobj) )
  {
  }

  ~DefByReduce() override = default;

  // DefByReduceで作られる方のTensorInfo
  TensorInfo GetTensorInfo() const
  {
    // dimはsrcから一つ減らしたもの。
    // elemの型はinitと同じ。
    // この場合はいつもLocal
    auto tp = _rutil.GetInitValue()->_type;
    return TensorInfo( _rutil.RName(), true, _rutil.GetSrcTensorInfo().Dimensions() - 1, { tp } );
  }

  /*
    以下ReduceToSclarとDefByReduceでほとんど共通。
  */

  /* _rutilへデリゲート。 */
  const std::string& GetSrcTensorName() const { return _rutil.GetSrcTensorName(); }
  const TensorInfo& GetSrcTensorInfo() const { return _rutil.GetSrcTensorInfo(); }
  BaseExprElem* GetTargetDim() const { return _rutil.GetTargetDim(); }
  int32_t GetTargetDimAsImm() const { return _rutil.GetTargetDimAsImm(); }
  BaseExprElem* GetInitValue() const { return _rutil.GetInitValue(); }
  size_t GetSrcExtentDim( size_t dim ) const { return _rutil.GetSrcExtentDim( dim ); }

  static void SkipToBlock( FNode::iterator& iter ) { ReduceTensorUtil::SkipToBlock( iter ); }

  /*
    BlockConsumerの実装
  */
  BlockElem* GetBlock() const override { return _rutil.GetBlock(); }
  const std::string& Name() const override { return _rutil.RName(); }


  /*
    Clone実装
  */
  IRElem* CloneSelf() const override
  {
    auto tsinfo = _rutil._tsinfo;
    return new DefByReduce( _rutil.RName(), std::move(tsinfo) );
  }

private:
  // クローンでしか使わない。ReduceTensorUtilのコンストラクタが違う
  DefByReduce( const std::string& rname, TensorInfo&& tsinfo ) : VoidExprElem<DefByReduce>(), _rutil( this, rname, std::move(tsinfo) ) {}

};

struct Call : public ExprElem<Call>
{
  enum FuncType
  {
    ABS,
    ALL,
    ANY,
    ATAN2_F32,
    CAST,
    CEIL,
    CLAMP,
    COS_F32,
    CROSS,
    DISTANCE,
    DOT,
    EXP_F32,
    EXP2,
    FLOOR,
    FRACT,
    INLINE,
    ISINF,
    ISNAN,
    LENGTH,
    LOG,
    LOG2,
    LOGICAL_NOT,
    MAX,
    MIN,
    MIX,
    NORMALIZE,
    POW,
    RAND_F32,
    ROUND,
    SATURATE,
    SIGN,
    SIN_F32, 
    SMOOTHSTEP,
    SQRT_F32,
    STEP,
    TAN_F32,
    TRUNC,
    VEC2,
    VEC3,
    VEC4
  };

  std::string DisplayName()
  {
    // staticなメンバのinitializerはinlineで定義出来ないのでローカルで定義するしかない…
    static const char* const extern_names[] = {
      "abs",
      "all",
      "any",
      "atan2",
      "cast",
      "ceil",
      "clamp",
      "cos",
      "cross",
      "distance",
      "dot",
      "exp",
      "exp2",
      "floor",
      "fract",
      "inline", // デバッグでしか使わない
      "isinf",
      "isnan",
      "length",
      "log",
      "log2",
      "!", //デバッグでしか使わない予定
      "max",
      "min",
      "mix",
      "normalize",
      "pow",
      "rand_f32",
      "round",
      "saturate",
      "sign",
      "sin",
      "smoothstep",
      "sqrt",
      "step",
      "tan",
      "trunc",
      "vec2",
      "vec3",
      "vec4"
    };
    if (_funcType == CAST)
    {
      std::stringstream ss;
      ss << _type;
      return ss.str();
    }
    else if (_funcType == INLINE)
    {
      std::stringstream ss;
      ss << "inline<";
      ss << _inlfId;
      ss << ">";
      return ss.str();

    }
    else
    {
      return extern_names[_funcType];
    }
  }

  /*
    Lowerでベクトル用　のHoistが必要かどうか。
    本質的にはベクトルを返す関数かどうかだが、
    TupleElemになるVEC2, VEC3, VEC4はfalseになる。
  */
  bool NeedVectorHoist() const
  {
    if (!_type.IsTuple())
      return false;
      
    switch( _funcType )
    {
      case ATAN2_F32:
      case CLAMP:
      case CROSS:
      case NORMALIZE:
      case MAX:
      case MIN:
      case MIX:
      case SMOOTHSTEP:
      case STEP:
        return true;
      default:
        return false;
    }
  }

  /*
    inline関数はlower周りでいろいろ特別なので判定メソッドを用意。
  */
  bool IsInlineFunc() const
  {
    return _funcType == INLINE;
  }

  // 単純にTupleElemに展開されるもの。
  bool IsExpandAsTuple() const
  {
    switch( _funcType )
    {
      case VEC2:
      case VEC3:
      case VEC4:
        return true;
      default:
        return false;
    }    
  }

  /*
    MFGはDRAMのバッファを中心とした言語なので、bool型は無い。
    だが多くのシェーダーでboolとして扱うものがある。
    場所としてはmfg_binary.hppに置くべきものだが、
    enumの定義と離れていると管理が面倒なのでここに置く。
  */
  bool IsInternalTypeBool() const
  {
    switch( _funcType )
    {
      case ISINF:
      case ISNAN:
        return true;
      default:
        return false;
    }
  }

  /*
    ベクトライズされている関数ならtrue

    ややこしいのはベクトルを返す関数は違う、という事。

    ベクトルを返すケース（この関数はfalseを返す）
    normalize([1.0, 2.0])

    通常の関数のベクトライズのケース（この関数はtrueを返す）
    sin([1.0, 2.0])
  */
  bool IsVectorized() const
  {
    return _type.IsTuple() && !IsInlineFunc() && !NeedVectorHoist();
  }

  /*
    max( a, b, c ) などを max( max(a, b), c ) などに展開すべきか。
  */
  bool ShouldExpandVarArgToBin() const
  {
    switch( _funcType )
    {
      case MIN:
      case MAX:
      {
        auto expanded = ToSpreadedTypes( GetArgs()->ShallowCopy<BaseExprElem>() );
        return expanded.size() >= 3;
      }
      default:
        return false;
    }
  }

  FuncType _funcType;
  size_t _inlfId; // INLINEの時しか使われない。

  /*
    子の内訳。
    0: ExprVector args
  */
  ExprVector* GetArgs() const
  {
    return GetChild<ExprVector>( 0 );
  }

  BaseExprElem* GetArg( int argIdx ) const
  {
    return (BaseExprElem*)GetArgs()->GetChild( argIdx );
  }

  static const IRElemType _elemType = IRElemType::Call; 

  Call( Type type, FuncType ftype, std::vector<Expr>&& args ) : Call( type, ftype, 0 )
  {
    AppendChild( ExprVector::FromExprs( std::move(args) ) );
  }

  ~Call() override = default;

  IRElem* CloneSelf() const override
  {
    return new Call( _type, _funcType, _inlfId );
  }

  std::vector<Expr> DetachArgs()
  {
    return GetArgs()->DetachChildren();
  }

private:
  // クローン用。
  Call( Type type, FuncType funcType, size_t inlfId ) : ExprElem<Call>( type ), _funcType( funcType ), _inlfId( inlfId )
  {
  }
};


/*
  inline関数がhoistされた時に作られるlet。
  引数の名前に結果を入れる。

  子どもはinline funcのcall。
*/
struct InlineFuncLet : public VoidExprElem<InlineFuncLet>
{
  std::string _name;

  /*
  子の内訳
  0: Call* value
  */
  Call* GetInlineFunc() const { return (Call*) GetChild(0); }
  size_t GetInlfIdx() const { return GetInlineFunc()->_inlfId; }

  static const IRElemType _elemType = IRElemType::InlineFuncLet; 
  InlineFuncLet( const std::string& name, Expr&& inlf ) : InlineFuncLet( name )
  {
    assert( inlf.GetElemType() == IRElemType::Call && inlf.As<Call>()->IsInlineFunc() );
    AppendChild( inlf.Detach() );
  }

  ~InlineFuncLet() override = default;

  IRElem* CloneSelf() const override
  {
    return new InlineFuncLet( _name );
  }

private:
  InlineFuncLet( const std::string& name ) : VoidExprElem<InlineFuncLet>(), _name( name ) {}
};

// tensorをselfとしたメソッドコールを表す。
struct TensorCall : public ExprElem<TensorCall>
{
  enum MethodType
  {
    LOAD, // ts(x, y) のようなもの。タプルの展開はしていない
    TUPLE_LOAD, // ts[1](x, y) などのように個々の要素のload
    TO_NCOORD,
    IS_INSIDE
  };

  TensorInfo _tinfo;
  MethodType _mtype;
  bool _isArgVerified; // LOADとTUPLE_LOADの時しか使わない、既に引数の範囲チェックが終わっているのであらたにclampする必要が無ければtrue。
  size_t _tupleIdx; // TUPLE_LOADの時しか使わない

  /*
    子の内訳。
    0: ExprVector args
  */
  ExprVector* GetArgs() const
  {
    return GetChild<ExprVector>( 0 );
  }

  BaseExprElem* GetArg( int argIdx ) const
  {
    return (BaseExprElem*)GetArgs()->GetChild( argIdx );
  }

  const TensorInfo& GetTensorInfo() const { return _tinfo; }

  // ベクトルのLoadExprになるべきLOADの時はtrue。
  bool IsVectorLoad() const
  {
    if (_mtype != LOAD)
      return false;
    return _tinfo.IsVector();
  }

  static const IRElemType _elemType = IRElemType::TensorCall; 

  // TupleIndexのある方のコンストラクタ
  TensorCall( Type type, const TensorInfo& tinfo, MethodType mtype, bool isArgVerified, size_t tupleIdx, std::vector<Expr>&& args ) : TensorCall( type, tinfo, mtype, isArgVerified, tupleIdx )
  {
    AppendChild( ExprVector::FromExprs( std::move(args) ) );
  }

  // isArgVerifiedはLOADかTUPLE_LOADの時だけ意味があるもの。
  // ただ直し忘れなどが無いかチェックするためこれの無い版のコンストラクタは用意しない。
  // 使わない人はtrueでも入れておいてください。
  TensorCall( Type type, const TensorInfo& tinfo, MethodType mtype, bool isArgVerified, std::vector<Expr>&& args ) : TensorCall( type, tinfo, mtype, isArgVerified, 0, std::move(args) ) {}

  ~TensorCall() override = default;

  IRElem* CloneSelf() const override
  {
    return new TensorCall( _type, _tinfo, _mtype, _isArgVerified, _tupleIdx );
  }


private:
  // クローン用。
  TensorCall( Type type, const TensorInfo& tinfo, MethodType mtype, bool isArgVerified, size_t tupleIdx ) : ExprElem<TensorCall>( type ), _tinfo( tinfo ), _mtype( mtype ), _isArgVerified( isArgVerified ), _tupleIdx( tupleIdx )
  {
  }
};

/*
  ts(x, y) などを展開してts[y*ts.extent.0+x]のような形で持つIRElem。
  IRElem生成時に一次元化する。
  u8v4の時は特別な処理をするバックエンドがあるので、u8v4のロードとするが、
  それ以外のタプルの場合は個々の要素のロードに展開されたものを表す。
*/
struct LoadExpr : public ExprElem<LoadExpr>
{
  std::string _bufName;
  TensorInfo _tsinfo;

  /*
    子の内訳。
    0: BaseExprElem* argIndex

    一次元化された添字のexpr。
  */
  BaseExprElem* GetIndexArg() const { return (BaseExprElem*)GetChild(0); }

  bool IsVector() const { return _type.IsVector(); }
  bool IsU8V4() const { return _type.IsU8V4(); }
  bool IsU16V4() const { return _type.IsU16V4(); }

  bool IsLocal() const { return _tsinfo.IsLocal(); }
  const TensorInfo& GetTensorInfo() const { return _tsinfo; }


  static const IRElemType _elemType = IRElemType::LoadExpr;

  ~LoadExpr() override = default;

  LoadExpr( Type type, const std::string& bufName, const TensorInfo& tinfo, Expr&& indexArg ) : LoadExpr( type, bufName, tinfo )
  {
    AppendChild( indexArg.Detach() );
  }

  IRElem* CloneSelf() const override
  {
    return new LoadExpr( _type, _bufName, _tsinfo );
  }


private:
  LoadExpr( Type t, const std::string& bufName, const TensorInfo& tinfo ) : ExprElem<LoadExpr>( t ), _bufName( bufName ), _tsinfo( tinfo ) {}

};

// t(a, b) += c 型のCompound Assignementを表すVoidExpr。
struct CompoundAssignment : public VoidExprElem<CompoundAssignment>
{
  /*  
  子の内訳
  0: TensorCall* leftCall 
  1: BaseExprElem* rexpr

  leftCallはLowerでLoadExprに変えられる。
  これはかなり特殊なケースなので、省エネで対応。
  */

  BaseExprElem* GetLeftUnknown() const { return (BaseExprElem*)GetChild( 0 ); }
  TensorCall* GetLeft() const { return GetChild<TensorCall>( 0 ); }
  LoadExpr* GetLeftAfterLower() const { return GetChild<LoadExpr>( 0 ); }

  BaseExprElem* GetRight() const { return (BaseExprElem*)GetChild( 1 ); }

  bool IsAfterLower() const
  {
    return GetLeftUnknown()->_elemType == IRElemType::LoadExpr;    
  }

  static const IRElemType _elemType = IRElemType::CompoundAssignment;

  CompoundAssignment( ExprT<TensorCall>&& lcall, Expr&& rexpr ) : VoidExprElem<CompoundAssignment>()
  {
    AppendChild( lcall.Detach() );
    AppendChild( rexpr.Detach() );
  }

  ~CompoundAssignment() override = default;

  IRElem* CloneSelf() const override
  {
    return new CompoundAssignment();
  }
private:
  CompoundAssignment() : VoidExprElem<CompoundAssignment>()
  {}
};

/*
  Lowerで生成される低レベルのIR
*/

/*
  int32_t buf[256];
  的なものを表すIRElem。
*/
struct AllocateStmt : public VoidExprElem<AllocateStmt>
{
  Type _bufType;
  std::string _bufName;


  /*  
  子の内訳
  0: BaseExprElem* arg

  argはconstの変数や定数の加減乗除で作られる式。
  */

  BaseExprElem* GetArg() const { return (BaseExprElem*)GetChild( 0 ); }

  static const IRElemType _elemType = IRElemType::AllocateStmt;

  AllocateStmt( Type bufType, const std::string& bufName, Expr&& arg ) : AllocateStmt( bufType, bufName )
  {
    AppendChild( arg.Detach() );
  }

  ~AllocateStmt() override = default;

  IRElem* CloneSelf() const override
  {
    return new AllocateStmt( _bufType, _bufName );
  }
private:
  AllocateStmt( Type tp, const std::string& bufName ) : VoidExprElem<AllocateStmt>(), _bufType( tp ), _bufName( bufName )
  {}
};

/*
  以下のような式を表す。大かっこの中は単なるExprとみなす。

  _r0[((_x)*256)+_y] = rexpr;

  ベクトルの場合はバックエンドごとに対応の仕方が異なるので、
  IRとしてはベクトルのStoreStmtのままにしておく。
*/
struct StoreStmt : public VoidExprElem<StoreStmt>
{
  std::string _bufName;
  /*  
  子の内訳
  0: BaseExprElem* indexArg 
  1: BaseExprElem* rexpr
  */

  BaseExprElem* GetIndexArg() const { return (BaseExprElem*)GetChild( 0 ); }
  BaseExprElem* GetRight() const { return (BaseExprElem*)GetChild( 1 ); }

  static const IRElemType _elemType = IRElemType::StoreStmt;

  StoreStmt( const std::string& bufName, Expr&& indexArg, Expr&& rexpr ) : StoreStmt( bufName )
  {
    AppendChild( indexArg.Detach() );
    AppendChild( rexpr.Detach() );
  }

  ~StoreStmt() override = default;

  IRElem* CloneSelf() const override
  {
    return new StoreStmt( _bufName );
  }

  bool IsVector() const { return GetRight()->_type.IsVector(); }

private:
  StoreStmt( const std::string& bufName ) : VoidExprElem<StoreStmt>(), _bufName( bufName )
  {}
};

/*
  VarLetで作った変数の更新。

  _v0 = rexpr;
*/
struct VarStore : public VoidExprElem<VarStore>
{
  std::string _varName;
  /*  
  子の内訳
  0: BaseExprElem* rexpr
  */

  BaseExprElem* GetRight() const { return (BaseExprElem*)GetChild( 0 ); }

  static const IRElemType _elemType = IRElemType::VarStore;

  VarStore( const std::string& varName, Expr&& rexpr ) : VarStore( varName )
  {
    AppendChild( rexpr.Detach() );
  }

  ~VarStore() override = default;

  IRElem* CloneSelf() const override
  {
    return new VarStore( _varName );
  }
private:
  VarStore( const std::string& bufName ) : VoidExprElem<VarStore>(), _varName( bufName )
  {}
};

/*
  以下を表すIRElem。

  for( int32_t varname = iniExpr; varName < iniExpr+extentExpr; varName++ )
  {
    body
  }
*/
struct ForStmt : public VoidExprElem<ForStmt>
{
  std::string _varName;

  /*  
  子の内訳
  0: BaseExprElem* begin
  1: BaseExprElem* end
  2: BaseExprElem* body

  bodyはVoidExprでBodyElemとかForStmtとか。Endはexclusive
  */
  BaseExprElem* GetBeginExpr() const { return (BaseExprElem*)GetChild( 0 ); }
  BaseExprElem* GetEndExpr() const { return (BaseExprElem*)GetChild( 1 ); }
  BaseExprElem* GetBody() const { return (BaseExprElem*)GetChild( 2 ); }

  static const IRElemType _elemType = IRElemType::ForStmt;

  ForStmt( const std::string& varName, Expr&& beginExpr, Expr&& endExpr, Expr&& body ) : ForStmt( varName )
  {
    AppendChild( beginExpr.Detach() );
    AppendChild( endExpr.Detach() );
    AppendChild( body.Detach() );
  }

  ~ForStmt() override = default;

  IRElem* CloneSelf() const override
  {
    return new ForStmt( _varName );
  }

  static void SkipToBody( FNode::iterator& iter )
  {
    iter.SkipNChildren( 2 );
  }


private:
  ForStmt( const std::string& varName ) : VoidExprElem<ForStmt>(), _varName( varName )
  {}
};


}///< mfg_internal


#endif

