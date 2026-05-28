/* -*- coding: utf-8 -*- マルチバイト */

// Copyright 2026 Kazuma Arino. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#ifndef FOREST_HPP_
#define FOREST_HPP_

#include <mfg_pal/pal.hpp>
#include <algorithm> // max
#include <cassert>
#include <memory>
#include <map>
#include <set>

namespace mfg_forest
{
using namespace mfg_pal;

enum class FEdgeDir
{
  LEADING,
  TRAILING
};

enum class FPriorNext
{
  PRIOR,
  NEXT
};


template<typename T> class ForestIterator;
template<typename T> class ChildIterator;

extern int g_node_alloc_count;

/*
Forestのノード。ノードの集合体がForestで、集合体自身を表すclassは無い。
*/
template<typename T>
class Forest
{
  friend class ForestIterator<T>;

  // _edge[dir][prior_next]の順番。
  Forest<T>* _edge[2][2]; 

  void InitEdge()
  {
    /*
    leafはこの２つはthis
    */
    _edge[size_t(FEdgeDir::LEADING)][size_t(FPriorNext::NEXT)] = this;
    _edge[size_t(FEdgeDir::TRAILING)][size_t(FPriorNext::PRIOR)] = this;

    /*
    親は無し
    */
    _edge[size_t(FEdgeDir::LEADING)][size_t(FPriorNext::PRIOR)] = nullptr;
    _edge[size_t(FEdgeDir::TRAILING)][size_t(FPriorNext::NEXT)] = nullptr;
  }

public:
  using iterator = ForestIterator<T>;
  using const_iterator= ForestIterator<const T>;

  using child_iterator = ChildIterator<T>;
  using const_child_iterator = ChildIterator<const T>;

  explicit Forest( const T& data ) : _data( data ) 
  {
    g_node_alloc_count++;
    InitEdge();
  }

  explicit Forest( T&& data ) : _data( std::move( data ) )
  {
    g_node_alloc_count++;
    InitEdge();
  }

  ~Forest()
  {
    g_node_alloc_count--;
    if(IsRoot())
    {
      // 自分を除く子どもたちを削除。
      begin().Erase( begin().TrailingOf() );
      assert( !begin().HasChildren() );
    }
  }

  bool IsRoot() const
  {
    return _edge[size_t(FEdgeDir::LEADING)][size_t(FPriorNext::PRIOR)] == nullptr
      &&  _edge[size_t(FEdgeDir::TRAILING)][size_t(FPriorNext::NEXT)] == nullptr;
  }

  T _data;

  Forest<T>*& GetLink(FEdgeDir dir, FPriorNext link) { return _edge[size_t(dir)][size_t(link)]; }
  
  Forest<T>* GetLink(FEdgeDir dir, FPriorNext link) const { return _edge[size_t(dir)][size_t(link)]; }

  iterator begin() { return iterator( this, FEdgeDir::LEADING ); }

  /*
    const_iteratorにわたすthisがconst Forest<T>*になってしまうので強制的にconstを取るキャストをしている。
    本来はForestIteratorのノードを表す型をテンプレートにし Forest<T>* と const Forest<T>*を選べるようにするのが正しいが、
    そうするとForestIteratorの中でインテリセンスが効かなくなってしまうので、お行儀の悪いキャストで乗り切る事にする。
  */
  const_iterator begin() const { return const_iterator( (Forest<const T>*)this, FEdgeDir::LEADING ); }

  iterator end() { return iterator( this, FEdgeDir::TRAILING ).NextOf(); }
  const_iterator end() const { return const_iterator( (Forest<const T>*)this, FEdgeDir::TRAILING ).NextOf(); }

  child_iterator BeginChild() { return child_iterator( begin() ); }
  child_iterator EndChild() { return child_iterator( begin() ).end(); }
  const_child_iterator CBeginChild() const { return const_child_iterator( begin() ); }
  const_child_iterator CEndChild() const { return const_child_iterator( begin() ).end(); }

  /*
    すべてのエッジに対して関数fnを実行。
    fnはiterを引数に取る。iterをいじっても良い（ToLeading()とか）
  */
  template<typename F>
  void ForEach( F fn )
  {
    for( auto iter = begin(); iter != end(); iter++ )
    {
      fn( iter );
    }
  }

  /*
    すべてのエッジに対して、Leadingの時だけ関数fnを実行。
    fnはiterを引数に取る。iterをいじっても良い（ToLeading()とか）
  */
  template<typename F>
  void ForEachLeading( F fn )
  {
    for( auto iter = begin(); iter != end(); iter++ )
    {
      if (iter.IsLeading())
        fn( iter );
    }
  }

  /*
  nth番目の子供を返す。子供の数より多い場合はnullptrを返す。
  */
  Forest<T>*
  GetChild( int nth )
  {
    auto iter = ChildIterator<T>( this );
    for(auto i : NRange( nth ))
    {
      N_UNUSED( i );
      iter++;
    }
    if (iter == iter.end())
      return nullptr;
    return iter.GetNode();
  }

  /*
    最後の子供を返す。みなしごだったらnullptrを返す
  */
  Forest<T>*
  GetLastChild()
  {
    if (!HasChildren())
      return nullptr;
    auto iter = begin().ToTrailing();
    iter--;
    return iter.GetNode();
  }

  /*
  childを末っ子として追加。サブツリーもOK。
  */
  void
  AppendChild( Forest<T>* child )
  {
    begin().ToTrailing().Chain( child );    
  }

  bool
  HasChildren() const
  {
    return begin().HasChildren();
  }

  /*
  ツリーをクローンする。
  要素のTに対し、以下の関数が存在する場合だけ使えるメソッド。
  T C::Clone(const T&);
  各ノードはC::Cloneを使ってクローンしていく。
  Tがunique_ptrなどの時の為、メソッドでは無くテンプレートで指定する事にした。
  */
  template<typename C>
  typename std::enable_if<std::is_same<T, decltype(C::Clone( std::declval<T>() ))>::value, Forest<T>*>::type
  Clone() const
  {
    using new_iterator = ForestIterator<typename std::remove_const<T>::type>;

    std::map<Forest<T>*, Forest<T>*> alloced;
    auto newRoot = new Forest<T>( C::Clone( _data ) );

    // 単なるポインタの値をキーとして使いたいだけなのだが、
    // うまくconstつけてコンパイル通せなかったのでキャスト…
    alloced[ (Forest<T>*)this] = newRoot;

    auto prev = newRoot->begin();
    for( auto iter = begin().NextOf(); iter != end(); iter++ )
    {
      auto newNode = alloced[(Forest<T>*)iter.GetNode() ];
      if ( newNode == nullptr )
      {
        newNode = new Forest<T>( C::Clone( iter.GetNode()->_data ) );
        alloced[(Forest<T>*)iter.GetNode() ] = newNode;
      }

      // 新しいツリーの方の, iterと同じ場所を指すiterator
      // new_iterator newiter {iter};
      // newiter.SetNode( newNode );
      new_iterator newiter ( newNode, iter._edge._direction );

      prev.SetNext( newiter );
      prev++;
      assert( newiter == prev );
    }

    return newRoot;
  }
};

template<typename T>
struct Edge
{
  Forest<T> *_node;
  FEdgeDir _direction;
  Edge( Forest<T>* node, FEdgeDir dir ) : _node( node ), _direction( dir ) {}

  bool Equal( const Edge<T>& other ) const
  {
    return _node == other._node && _direction == other._direction;
  }

  bool IsLeading() const { return _direction == FEdgeDir::LEADING; }
  bool IsTrailing() const { return _direction == FEdgeDir::TRAILING; }

  T& operator*() const { return _node->_data; }
};

template<typename T>
class ForestIterator : public IteratorFacade<ForestIterator<T>, Edge<T>>
{
  friend class Forest<T>;
  static const auto NEXT = FPriorNext::NEXT;
  static const auto PRIOR = FPriorNext::PRIOR;
  static const auto LEADING = FEdgeDir::LEADING;
  static const auto TRAILING = FEdgeDir::TRAILING;

  Forest<T>*& GetLink(FEdgeDir dir, FPriorNext link) { return _edge._node->GetLink( dir, link ); }  
  Forest<T>* GetLink(FEdgeDir dir, FPriorNext link) const { return _edge._node->GetLink( dir, link ); }

  void SetLink( FEdgeDir dir, FPriorNext link, Forest<T> *node )
  {
    // みなし子のrootはnullptr。その場合は更新しない。
    if (GetNode() != nullptr)
      GetLink( dir, link ) = node;
  }

  void SetNext( ForestIterator<T>& y )
  {
    SetLink( _edge._direction, FPriorNext::NEXT, y.GetNode() );
    y.SetLink( y._edge._direction, FPriorNext::PRIOR, GetNode() );
  }

  bool _rootDeleteing = false;


public:
  Edge<T> _edge;

  ForestIterator( Forest<T>* node, FEdgeDir dir ) : _edge( node, dir ) {}

  ForestIterator( const ForestIterator& x ) : _edge( x._edge ) {}

  ForestIterator& operator=( const ForestIterator& x )
  {
    _edge = x._edge;
    return (*this);
  }

  Forest<T>* GetNode() const { return _edge._node; }
  T& GetContent() { return _edge._node->_data; }
  void SetNode( Forest<T> *node ) { _edge._node = node; }
  void SetDirection( FEdgeDir dir ) { _edge._direction = dir; }
  void SetTrailingOfNode( Forest<T> *node )
  {
    SetNode( node );
    SetDirection( FEdgeDir::TRAILING );
  }

  ////////////////////////////
  // IteratorFacade関連
  ////////////////////////////


  bool Equal( const ForestIterator<T>& other ) const
  {
    return _edge.Equal( other._edge );
  }

  Edge<T>& Dereference() { return _edge; }
  const Edge<T>& Dereference() const { return _edge; }

  /*
    ASLのドキュメントのiterationを見ながら読むとわかりやすい。
    https://stlab.adobe.com/group__asl__tutorials__forest.html
  */
  void Increment()
  {
    Forest<T>* next = GetLink( _edge._direction, NEXT );

    if ( _edge.IsLeading() )
    {
      // leafだったら反転。それ以外ならLEADINGのまま。
      _edge._direction = ( next == GetNode() ? TRAILING : LEADING );
    }
    else
    {
      // 兄弟に移動する場合は反転、親に戻る場合はそのまま。
      // 兄弟に移動したかどうかは、nextのleading-priorが移動元かどうかで判定
      // nextがendまで来るとnullptrになるが、その場合は反転はしない。
      if (next != nullptr )
        _edge._direction = ( next->GetLink( LEADING, PRIOR ) == GetNode() ? LEADING : TRAILING );
    }

    _edge._node = next;
  }

  /*
    end()から--は出来ない（nulllptrからは戻り方が分からないので）
  */
  void Decrement()
  {
    Forest<T>* prev = GetLink( _edge._direction, PRIOR );

    if ( _edge.IsLeading() )
    {
      // 兄弟に移動する場合は反転、親に戻る場合はそのまま。
      // 兄弟に移動したかどうかは、prevのtrailing-nextが移動元かどうかで判定
      // prevがnullptrの場合はrootの場合なので兄弟はいないからそのまま。

      _edge._direction = ( prev!= nullptr && prev->GetLink( TRAILING, NEXT ) == GetNode() ? TRAILING : LEADING );
    }
    else
    {
      // leafだったら反転。それ以外ならTRAILINGのまま。
      _edge._direction = ( prev == GetNode() ? LEADING : TRAILING );
    }

    _edge._node = prev;
  }

  ////////////////////////////
  // Iteratorのそのほかのメソッド
  ////////////////////////////

  // 自身をTrailにする
  ForestIterator& ToTrailing()
  {
    _edge._direction = TRAILING;
    return *this;
  }

  // 自身をLeadingにする
  ForestIterator& ToLeading()
  {
    _edge._direction = LEADING;
    return *this;
  }

  // Leadingのコピーを返す
  ForestIterator LeadingOf() const
  {
    auto res = *this;
    res._edge._direction = LEADING;
    return res;
  }

  // Trailingのコピーを返す
  ForestIterator TrailingOf() const
  {
    auto res = *this;
    res.ToTrailing();
    return res;
  }

  // nextのコピーを返す。nextは++で求める。
  ForestIterator NextOf() const
  {
    auto res = *this;
    res++;
    return res;
  }

  // priorのコピーを返す。priorは--で求める。
  ForestIterator PriorOf() const
  {
    auto res = *this;
    res--;
    return res;
  }

  bool IsLeading() const { return _edge._direction == LEADING; }
  bool IsTrailing() const { return _edge._direction == TRAILING; }

  /*
    num個の子供を飛ばして、次はnum+1個目の子供に行く状態にする。
    ようするにnum個目の子供のTrailingの状態にする。
  */
  void SkipNChildren( int num )
  {
    for( auto i : NRange( num ))
    {
      N_UNUSED( i );
      (*this)++;
      ToTrailing();
    }
  }

  /*
  現在のエッジにノードを挿入する。

  InsertをiteratorのメソッドとするのはSTL的では無いが、
  free floating関数にするとxの型のADLが効いてしまって使いづらいかったので
  iteratorのメソッドとした。

  Insertの実装は、
  https://gitlab.meganezaru.info/pixiv/polygon/-/issues/220#note_1286814
  の4パターンを見ながら読むとわかりやすい。
  */
  ForestIterator<T> Insert( const T& x )
  {
    return Chain( new Forest<T>(x) );
  }

  /*
  Insertのmoveバージョン。
  詳細はコピーコンストラクタバージョンを参照のこと。
  */
  ForestIterator<T> Insert( T&& x )
  {
    return Chain( new Forest<T>( std::move( x ) ) );
  }

  /*
    今さしているiteratorのノードに子供が要るかを返す。

    nodeに実装する方がいいか？
  */
  bool HasChildren() const
  {
    return GetNode() != LeadingOf().NextOf().GetNode();
  }

  /*
    自身の指しているノードを削除し、次の有効なイテレータを返す。指しているノードが葉の時しか呼んではいけない。thisの指すイテレータは以後使わない事。
  */
  ForestIterator Erase()
  {
    ForestIterator leading_prior( LeadingOf().PriorOf() );
    ForestIterator trailing_next( TrailingOf().NextOf() );

    assert( !HasChildren() );
    leading_prior.SetNext( trailing_next );

    // nullにすると誤ってend()と一致してしまうかもしれないので、deleteするだけにする。
    delete _edge._node;

    return  (_edge._direction == LEADING)  ? leading_prior.NextOf() : trailing_next;
  }

  /*
    現在の位置からlastまでのノードを削除する。
    lastが現在よりも上まで続いている場合はiteratorが2回通るノードのみ削除。

    詳細は以下
    https://stlab.adobe.com/group__asl__tutorials__forest.html
    の Node Deletionが参考になる。
  */
  ForestIterator Erase( const ForestIterator& last )
  {
    // Eraseの開始であるthisはleadingで無くてはいけない。
    assert( IsLeading() );

    int stack_depth = 0;
    ForestIterator cur( *this );

    while (cur != last)
    {
      if(cur.IsLeading())
      {
        stack_depth++;
        cur++;
      }
      else // 戻り
      {
        // 二度通っていたら削除
        if (stack_depth > 0)
        {          
          cur = cur.Erase();
        } 
        else
        {
          ++cur;
        }
        stack_depth = std::max(0, stack_depth - 1);
      }
    }
    return last;
  }

  /*
    現在の位置にサブツリーを挿入。
    挿入のルールはInsertと同様。
    引数のsubtreeの寿命は以後このツリーが管理するから呼び出し元で削除しない事。

    thisは変化せず、subtreeのルートのLEADINGを指すiteratorを返す。
    ループの中でChainしてそこに進みたい場合は明示的にコピーアサインを呼び出す事。
  */
  ForestIterator<T> Chain( Forest<T>* subtree )
  {
    ForestIterator<T> result( subtree, FEdgeDir::LEADING );

    ForestIterator<T> prev( PriorOf() );

    ForestIterator<T> newTrail( result.TrailingOf() );

    prev.SetNext( result );
    newTrail.SetNext( *this );

    return result;  
  }

  /*
    現在指しているノードとその子孫のサブツリーを、現在のツリーから切り離す。
    返されるサブツリーのルートの寿命管理は、呼び出し側が行う。
    iteratorは次に進む。簡単のため、thisはleadingじゃないと駄目としておく。
  */
  Forest<T>* Unchain()
  {
    assert( IsLeading() );
    assert( !GetNode()->IsRoot() );

    ForestIterator leading_prior( PriorOf() );
    ForestIterator trailing_next( TrailingOf().NextOf() );

    leading_prior.SetNext( trailing_next );

    // unchainするノードの親をnullptrに。
    GetLink( LEADING, PRIOR ) = nullptr;
    GetLink( TRAILING, NEXT ) = nullptr;

    auto ret = GetNode();

    // thisのイテレータを次に進める。unchain前のtrialingのnext
    _edge = trailing_next._edge;

    return ret;
  }

  /*
    現在指しているノードを、引数のnewNodeに差し替える。
    現在指しているノードはUnchainされてuniqu_ptrとして返される。
    thisは新しいノードのTRAILINGを指す。
  */
  std::unique_ptr<Forest<T>> Replace( Forest<T>* newNode )
  {
    auto oldNode = _edge._node;
    auto prevLead = oldNode->GetLink( LEADING, PRIOR );
    auto nextTrail = oldNode->GetLink( TRAILING, NEXT );
    newNode->GetLink( LEADING, PRIOR ) = prevLead;
    newNode->GetLink( TRAILING, NEXT ) = nextTrail;

    // prevLeadとnexttrailがnullptrならそもそもこのノードがルートなので差し替える必要は無い。
    // その場合はoldNodeをunique_ptrで返せば十分。
    if (prevLead != nullptr)
    {
      if (prevLead->GetLink( LEADING, NEXT ) == oldNode)
        prevLead->GetLink( LEADING, NEXT ) = newNode;
      else
        prevLead->GetLink( TRAILING, NEXT ) = newNode;
    }

    if (nextTrail != nullptr)
    {
      if (nextTrail->GetLink( TRAILING, PRIOR ) == oldNode)
        nextTrail->GetLink( TRAILING, PRIOR ) = newNode;
      else
        nextTrail->GetLink( LEADING, PRIOR ) = newNode;
    }

    oldNode->GetLink(LEADING, PRIOR) = nullptr;
    oldNode->GetLink(TRAILING, NEXT) = nullptr;

    _edge._node = newNode;
    _edge._direction = TRAILING;
    
    return std::unique_ptr<Forest<T>>( oldNode );
  }
};

template<typename T>
class ChildIterator : public IteratorFacade<ChildIterator<T>, T>
{
  ForestIterator<T> _curIterator;
  ForestIterator<T> _endIterator;

  explicit ChildIterator( const ForestIterator<T>& cur, const ForestIterator<T>& end ) : _curIterator( cur ), _endIterator( end )
  {
  }

public:
  ChildIterator( const ChildIterator& x ) = default;

  explicit ChildIterator( Forest<T> *node ) : ChildIterator( node->begin() ) {}
  explicit ChildIterator( const ForestIterator<T>& parentIter ) : _curIterator( parentIter ), _endIterator( parentIter.TrailingOf() )
  {
    _curIterator++;
  }

  /*
    IteratorFacade関連
  */

  void Increment()
  {
    if ( _curIterator == _endIterator )
      return;    
    _curIterator.ToTrailing();
    _curIterator++;
  }

  T& Dereference() const { return _curIterator.GetNode()->_data; }

  bool Equal( const ChildIterator<T>& other ) const
  {
    return other._curIterator == _curIterator;
  }

  ChildIterator<T> end() const
  {
    return ChildIterator( _endIterator, _endIterator );
  }

  /*
  そのほかのpublic method。
  */

  Forest<T>* GetNode()
  {
    return _curIterator.GetNode();
  }

  /*
    現在指しているノードを差し替える。
    ForestIteratorのReplaceと違い、指しているエッジはnewNodeのLeadingとなる（同じ場所）
  */
  std::unique_ptr<Forest<T>> Replace( Forest<T>* newNode )
  {
    auto curIter = _curIterator;
    auto ret = _curIterator.Replace( newNode );

    // 現在の指している場所を復元。
    _curIterator = curIter;
    _curIterator._edge._node = newNode;

    return ret;
  }

};

} ///< neet

#endif

