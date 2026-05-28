// Copyright 2026 Kazuma Arino. All rights reserved.
// SPDX-License-Identifier: Apache-2.0


#include "forest.hpp"
#include <string>
#include <iostream>
#include <sstream>


#define _NFIFTEST_SUBTEST_
#include "nfiftest.hpp"
using namespace nfiftest;

// インテリセンスの効きがいまいちなので同じファイルに再定義
#undef REQUIRE
#define REQUIRE(expr) if(!(expr)) throw nfiftest::assert_fail_error(__FILE__, __LINE__, #expr)

using namespace mfg_forest;
using namespace std;


static string DumpTree( Forest<string>& node )
{
  stringstream actual;
  for (auto& edge : node)
  {
    if ( edge._direction == FEdgeDir::LEADING )
    {
      actual << "<" << *edge << ">" << endl;
    }
    else
    {
      actual << "</" << *edge << ">" << endl;
    }
  }
  return actual.str();
}

struct DestructorTracker
{
  DestructorTracker( bool& isDestructorCalled ) : _destructorCalled( isDestructorCalled ) {}
  DestructorTracker( DestructorTracker&& src ) : _destructorCalled( src._destructorCalled )
  {
    src._isMoved = true;
  } 
  ~DestructorTracker()
  { 
    if (!_isMoved)
      _destructorCalled = true;
  }
  bool& _destructorCalled;
  bool _isMoved = false;
};

struct StringCloner
{
  static string Clone( const string& src )
  {
    return std::string( src );
  }
};

static std::vector<TestPair> test_cases_forest = {
{"Forestの少し複雑なツリーのテスト", []{
  /*
  https://stlab.adobe.com/group__asl__tutorials__forest.html のDefault Construction and Insertと同じ例。
  */
  Forest<string> node( "grandmother" );
  auto i = node.begin().ToTrailing();
  {
    auto p = i.Insert( "mother" ).ToTrailing();
    p.Insert( "me" );
    p.Insert( "sister" );
    p.Insert( "brother" );
  }

  {
    auto p = i.Insert( "aunt" ).ToTrailing();
    p.Insert( "cousin" );
  }
  i.Insert( "uncle" );

  if (SECTION("ツリーが出来ているかをダンプして確認")) {SG g;
    auto expect = R"(<grandmother>
<mother>
<me>
</me>
<sister>
</sister>
<brother>
</brother>
</mother>
<aunt>
<cousin>
</cousin>
</aunt>
<uncle>
</uncle>
</grandmother>
)";
    auto actual = DumpTree( node );

    REQUIRE( actual == expect );
  }

  if (SECTION("SkipNChildrenのテスト、2つ飛ばす")) {SG g;
    auto iter = node.begin();
    iter.SkipNChildren( 2 ); // motherとauntをスキップ、auntのTrailing（つまり次の++でuncleに）。
    
    REQUIRE( iter.GetContent() == "aunt" );
    REQUIRE( iter.IsTrailing() );
  }

  if (SECTION("長男を削除")) {SG g;
    auto iter = node.begin();
    iter++; // mother
    iter++; //me
    iter.Erase();

    auto expect = R"(<grandmother>
<mother>
<sister>
</sister>
<brother>
</brother>
</mother>
<aunt>
<cousin>
</cousin>
</aunt>
<uncle>
</uncle>
</grandmother>
)";
    auto actual = DumpTree( node );

    REQUIRE( actual == expect );
  }

  if (SECTION("まんなかの子を削除")) {SG g;
    auto iter = node.begin();
    iter++; // mother
    iter++; //me
    iter++; //me-trail
    iter++; // sister
    iter.Erase();

    auto expect = R"(<grandmother>
<mother>
<me>
</me>
<brother>
</brother>
</mother>
<aunt>
<cousin>
</cousin>
</aunt>
<uncle>
</uncle>
</grandmother>
)";
    auto actual = DumpTree( node );

    REQUIRE( actual == expect );
  }

  if (SECTION("末子を削除")) {SG g;
    auto iter = node.begin();
    iter++; // mother
    iter++; //me
    iter++; //me-trail
    iter++; // sister
    iter++; // sister-trail
    iter++; // brother
    iter.Erase();

    auto expect = R"(<grandmother>
<mother>
<me>
</me>
<sister>
</sister>
</mother>
<aunt>
<cousin>
</cousin>
</aunt>
<uncle>
</uncle>
</grandmother>
)";
    auto actual = DumpTree( node );

    REQUIRE( actual == expect );
  }

  if (SECTION("一人っ子を削除")) {SG g;
    auto iter = node.begin();
    iter++; // mother
    iter++; //me
    iter++; //me-trail
    iter++; // sister
    iter++; // sister-trail
    iter++; // brother
    iter++; // brother
    iter++; // mother-trail
    iter++; // aunt
    iter++; // cousin
    iter.Erase();

    auto expect = R"(<grandmother>
<mother>
<me>
</me>
<sister>
</sister>
<brother>
</brother>
</mother>
<aunt>
</aunt>
<uncle>
</uncle>
</grandmother>
)";
    auto actual = DumpTree( node );

    REQUIRE( actual == expect );
  }

  if (SECTION("範囲削除、motherの子供全削除")) {SG g;
    auto iter = node.begin();
    iter++; // mother
    auto last = iter.TrailingOf();
    iter++; // first-child.
    iter.Erase( last );

    auto expect = R"(<grandmother>
<mother>
</mother>
<aunt>
<cousin>
</cousin>
</aunt>
<uncle>
</uncle>
</grandmother>
)";
    auto actual = DumpTree( node );

    REQUIRE( actual == expect );
  }

  if (SECTION("範囲削除、motherの子供全削除、lastが少し先のケース")) {SG g;
    auto iter = node.begin();
    iter++; // mother

    auto last = iter.TrailingOf(); // mother-trail
    last++; // aunt-leading 

    iter++; // first-childl
    iter.Erase( last );

    auto expect = R"(<grandmother>
<mother>
</mother>
<aunt>
<cousin>
</cousin>
</aunt>
<uncle>
</uncle>
</grandmother>
)";
    auto actual = DumpTree( node );

    REQUIRE( actual == expect );
  }

  if (SECTION("範囲削除、真ん中のサブツリーを削除")) {SG g;
    auto iter = node.begin();
    iter++; // mother
    iter.ToTrailing(); // mother-trail
    iter++; // aunt-leading 

    auto last = iter.TrailingOf();
    last++; // uncle-lead

    iter.Erase( last );

    auto expect = R"(<grandmother>
<mother>
<me>
</me>
<sister>
</sister>
<brother>
</brother>
</mother>
<uncle>
</uncle>
</grandmother>
)";
    auto actual = DumpTree( node );

    REQUIRE( actual == expect );
  }

  if (SECTION("auntをUnchain")) {SG g;
    auto iter = node.begin();
    iter++; // mother
    iter.ToTrailing(); // mother-trail
    iter++; // aunt-leading 

    auto auntTree = iter.Unchain();

    REQUIRE( iter.IsLeading() );
    REQUIRE( *(iter._edge) == "uncle");
    REQUIRE( auntTree->IsRoot() );

    auto expect1 = R"(<grandmother>
<mother>
<me>
</me>
<sister>
</sister>
<brother>
</brother>
</mother>
<uncle>
</uncle>
</grandmother>
)";
    auto expect2 = R"(<aunt>
<cousin>
</cousin>
</aunt>
)";
    auto actual1 = DumpTree( node );
    auto actual2 = DumpTree( *auntTree );

    REQUIRE( actual1 == expect1 );
    REQUIRE( actual2 == expect2 );

    delete auntTree;
  }

  if (SECTION("一人っ子をUnchain、iterは親のtrailingにならなくてはいけない")) {SG g;
    auto iter = node.begin();
    iter++; // mother
    iter.ToTrailing(); // mother-trail
    iter++; // aunt-leading 
    iter++; // cousin-leading 

    auto cousinTree = iter.Unchain();

    REQUIRE( iter.IsTrailing() );
    REQUIRE( *(iter._edge) == "aunt");
    REQUIRE( cousinTree->IsRoot() );

    auto expect1 = R"(<grandmother>
<mother>
<me>
</me>
<sister>
</sister>
<brother>
</brother>
</mother>
<aunt>
</aunt>
<uncle>
</uncle>
</grandmother>
)";
    auto expect2 = R"(<cousin>
</cousin>
)";
    auto actual1 = DumpTree( node );
    auto actual2 = DumpTree( *cousinTree );

    REQUIRE( actual1 == expect1 );
    REQUIRE( actual2 == expect2 );

    delete cousinTree;
  }
  if (SECTION("サブツリーをChain")) {SG g;
    auto subtree = new Forest<string>( "A" );
    auto i = subtree->begin().ToTrailing();
    i.Insert( "B" ).ToTrailing();
    i.Insert( "C" ).ToTrailing();

    auto iter = node.begin();
    iter.ToTrailing(); // grand-trail
    iter--; // uncle-trail
    iter.Chain( subtree );

    auto expect = R"(<grandmother>
<mother>
<me>
</me>
<sister>
</sister>
<brother>
</brother>
</mother>
<aunt>
<cousin>
</cousin>
</aunt>
<uncle>
<A>
<B>
</B>
<C>
</C>
</A>
</uncle>
</grandmother>
)";
    auto actual = DumpTree( node );

    REQUIRE( actual == expect );
  }

  if (SECTION("サブツリーをReplace、兄弟あり")) {SG g;
    auto subtree = new Forest<string>( "A" );
    auto i = subtree->begin().ToTrailing();
    i.Insert( "B" ).ToTrailing();
    i.Insert( "C" ).ToTrailing();

    auto iter = node.begin();
    iter.ToTrailing(); // grand-trail
    iter--; // uncle-trail
    iter--; // uncle-lead
    iter--; // aunt-trail
    iter.ToLeading();
    auto ret = iter.Replace( subtree );

    REQUIRE( ret->IsRoot() );
    REQUIRE( iter.IsTrailing() );
    REQUIRE( iter.GetContent() == "A" );

    auto expect1 = R"(<grandmother>
<mother>
<me>
</me>
<sister>
</sister>
<brother>
</brother>
</mother>
<A>
<B>
</B>
<C>
</C>
</A>
<uncle>
</uncle>
</grandmother>
)";
    auto expect2 = R"(<aunt>
<cousin>
</cousin>
</aunt>
)";

    auto actual1 = DumpTree( node );
    REQUIRE( actual1 == expect1 );

    auto actual2 = DumpTree( *(ret.get()) );
    REQUIRE( actual2 == expect2 );
  }

  if (SECTION("サブツリーをReplace、一人っ子")) {SG g;
    auto subtree = new Forest<string>( "A" );
    auto i = subtree->begin().ToTrailing();
    i.Insert( "B" ).ToTrailing();
    i.Insert( "C" ).ToTrailing();

    auto iter = node.begin();
    iter.ToTrailing(); // grand-trail
    iter--; // uncle-trail
    iter--; // uncle-lead
    iter--; // aunt-trail
    iter--; // cousin-trail
    iter.ToLeading();
    auto ret = iter.Replace( subtree );

    REQUIRE( ret->IsRoot() );

    auto expect1 = R"(<grandmother>
<mother>
<me>
</me>
<sister>
</sister>
<brother>
</brother>
</mother>
<aunt>
<A>
<B>
</B>
<C>
</C>
</A>
</aunt>
<uncle>
</uncle>
</grandmother>
)";
    auto expect2 = R"(<cousin>
</cousin>
)";

    auto actual1 = DumpTree( node );
    REQUIRE( actual1 == expect1 );

    auto actual2 = DumpTree( *(ret.get()) );
    REQUIRE( actual2 == expect2 );
  }

  if (SECTION("iterを途中でUnchainしてRechainした時のNextが期待通りかのテスト")) {SG g;
    auto iter = node.begin();
    iter++; // mother
    iter++; //me
    auto me = iter.Unchain();
    auto newiter = iter.Chain( me );
    
    // Chainはiterを変えない
    REQUIRE( iter.GetContent() == "sister" );

    // newiterは入れたノードのLeading
    REQUIRE( newiter.GetContent() == "me" );
    REQUIRE( newiter.IsLeading() );

    // copy assignで自分に出来る
    iter = newiter;
    REQUIRE( iter.GetContent() == "me" );
    REQUIRE( iter.IsLeading() );
  }

  if (SECTION("Cloneのテスト")) {SG g;
    auto expect = R"(<grandmother>
<mother>
<me>
</me>
<sister>
</sister>
<brother>
</brother>
</mother>
<aunt>
<cousin>
</cousin>
</aunt>
<uncle>
</uncle>
</grandmother>
)";

    auto cloned = node.Clone<StringCloner>();

    auto actual = DumpTree( *cloned );

    REQUIRE( actual == expect );
    REQUIRE( cloned != &node );
    delete cloned;
  }

  if (SECTION("ChildIteratorのテスト")) {SG g;
    auto iter = ChildIterator<string>( &node );
    auto end = iter.end();
    REQUIRE( *iter == "mother" );
    REQUIRE( iter != end );

    iter++;
    REQUIRE( *iter == "aunt" );
    REQUIRE( iter != end );

    iter++;
    REQUIRE( *iter == "uncle" );
    REQUIRE( iter != end );

    iter++;
    REQUIRE( iter == end );
  }

  if (SECTION("GetChildのテスト")) {SG g;
    auto child = node.GetChild( 0 );
    REQUIRE( child->_data  == "mother" );

    child = node.GetChild( 1 );
    REQUIRE( child->_data == "aunt" );

    child = node.GetChild( 2 );
    REQUIRE( child->_data == "uncle" );

    child = node.GetChild( 3 );
    REQUIRE( child == nullptr );
  }
  if (SECTION("GetLastChildのテスト")) {SG g;
    auto child = node.GetLastChild();
    REQUIRE( child->_data  == "uncle" );

    auto nochildren = child->GetLastChild();
    REQUIRE( nochildren == nullptr );
  }
}},
{"ForestのInsertのテスト", []{
  Forest<string> node( "A" );
  auto i = node.begin().ToTrailing();
  i.Insert( "B" ).ToTrailing();
  i.Insert( "C" ).ToTrailing();

  if (SECTION("ABCの親子関係が正しく出来ているかテスト")) {SG g;
    auto expect = R"(<A>
<B>
</B>
<C>
</C>
</A>
)";
    auto actual = DumpTree( node );

    REQUIRE( actual == expect );
  }
  /*
  https://github.com/pixiv/polygon/issues/220#issuecomment-715770985
  にある4通りのテスト。
  */
  if (SECTION("パターン1のテスト")) {SG g;
    auto iter = node.begin().NextOf();
    iter.Insert( "N" );

    auto expect = R"(<A>
<N>
</N>
<B>
</B>
<C>
</C>
</A>
)";
    auto actual = DumpTree( node );

    REQUIRE( actual == expect );
  }

  if (SECTION("パターン2のテスト")) {SG g;
    auto iter = node.begin();
    iter++;
    iter++;
    iter.Insert( "N" );

    auto expect = R"(<A>
<B>
<N>
</N>
</B>
<C>
</C>
</A>
)";
    auto actual = DumpTree( node );

    REQUIRE( actual == expect );
  }

  if (SECTION("パターン3のテスト")) {SG g;
    auto iter = node.begin();
    iter++;
    iter++;
    iter++;
    iter.Insert( "N" );

    auto expect = R"(<A>
<B>
</B>
<N>
</N>
<C>
</C>
</A>
)";
    auto actual = DumpTree( node );

    REQUIRE( actual == expect );
  }

  if (SECTION("パターン4のテスト")) {SG g;
    auto iter = node.begin();
    iter++;
    iter++;
    iter++;
    iter++;
    iter++;
    iter.Insert( "N" );

    auto expect = R"(<A>
<B>
</B>
<C>
</C>
<N>
</N>
</A>
)";
    auto actual = DumpTree( node );

    REQUIRE( actual == expect );
  }


}},
{"Forestでノードのデストラクタがちゃんと呼ばれるかのテスト", []{
  bool aCalled = false;
  bool bCalled = false;
  bool cCalled = false;
  auto root = new Forest<DestructorTracker>( DestructorTracker( aCalled ) );
  auto i = root->begin().ToTrailing();
  i.Insert( DestructorTracker( bCalled ) ).ToTrailing();
  i.Insert( DestructorTracker( cCalled ) ).ToTrailing();

  REQUIRE( !aCalled );
  REQUIRE( !bCalled );
  REQUIRE( !cCalled );

  delete root;

  REQUIRE( aCalled );
  REQUIRE( bCalled );
  REQUIRE( cCalled );
}},
{"Forestのノード一つ一つの条件を定めたテスト", []{
  Forest<string> node( "A" );
  if (SECTION("一つのノードのクローン")) {SG g;
    auto newNode = node.Clone<StringCloner>();

    auto expect = "<A>\n</A>\n";
    auto actual = DumpTree( node );

    REQUIRE( actual == expect );

    delete newNode;
  }

  if (SECTION("子供が居ない時のChildIteratorのテスト")) {SG g;
    auto iter = ChildIterator<string>( &node );
    auto end = iter.end();
    REQUIRE( iter == end );
  }

  if (SECTION("子供が一人だけの時のテスト")) {SG g;
    auto i = node.begin().ToTrailing();
    i.Insert( "B" );

    if (SECTION("子供が一人だけの時のクローン")) {SG g;
      auto expect = R"(<A>
<B>
</B>
</A>
)";

      auto newNode = node.Clone<StringCloner>();
      auto actual = DumpTree( node );

      REQUIRE( actual == expect );

      delete newNode;
    }

    if (SECTION("子供が一人だけの時のChildIteratorのテスト")) {SG g;
      auto iter = ChildIterator<string>( &node );
      auto end = iter.end();

      REQUIRE( *iter == "B" );
      REQUIRE( iter != end );

      iter++;
      REQUIRE( iter == end );
    }

  }  
}}
};

void RegisterForestTest(std::vector<TestPair>& testCases)
{
  testCases.insert(testCases.end(), test_cases_forest.begin(), test_cases_forest.end());
}