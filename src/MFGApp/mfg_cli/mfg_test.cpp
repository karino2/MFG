// Copyright 2026 Kazuma Arino. All rights reserved.
// SPDX-License-Identifier: Apache-2.0


#include "forest.hpp"
#include "mfg.hpp"
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>


#define _NFIFTEST_SUBTEST_
#include "nfiftest.hpp"
using namespace nfiftest;

#include "mfg_test_common.hpp"
using namespace mfg_test_common;

#include "mfg_resource.hpp"
using namespace mfg_resource;

#include "mfg_resource_expander.hpp"
using namespace mfg_resource_expander;

// インテリセンスの効きがいまいちなので同じファイルに再定義
#undef REQUIRE
#define REQUIRE(expr) if(!(expr)) throw nfiftest::assert_fail_error(__FILE__, __LINE__, #expr)

using namespace std;
using namespace mfg_internal;
using namespace mfg;
using namespace mfg_ir_util;
using namespace mfg_parser;

#ifdef MFG_BACKEND_METAL
#include "mfg_runtime_metal.hpp"
using namespace mfg_msl_converter;
using namespace mfg_runtime_metal;

// 1 kernelのMetal Context
using MCtx1K = MetalTestKernelContext;
using LibContext = MetalTestLibraryContext;
#endif

#ifdef MFG_BACKEND_D3D
#include "mfg_runtime_d3d.hpp"
using namespace mfg_hlsl_converter;
using namespace mfg_runtime_d3d;
#endif


static std::vector<TestPair> test_cases_mfg_misc = {
{"NRangeのテスト", []{// 置き場が無かったのでここに置く。
  std::stringstream ss;
  for (auto i : NRange(3))
  {
    ss << std::to_string(i) << " ";
  }
  REQUIRE( "0 1 2 " == ss.str() );
}},
{"RevRangeのテスト", []{
  std::stringstream ss;
  for (auto i : RevRange(3))
  {
    ss << std::to_string(i) << " ";
  }
  REQUIRE( "2 1 0 " == ss.str() );
}},
{"Containsのテスト", [] {
  REQUIRE(Contains("abcde", "abc"));
  REQUIRE(Contains("abcde", "bcde"));
  REQUIRE(Contains("abcde", "bc"));
  REQUIRE(!Contains("abcde", "abcdef"));
}},
{"CountContainsのテスト", [] {
  REQUIRE(CountContains("abcde", "abc") == 1);
  REQUIRE(CountContains("abcabc", "abc") == 2);
  REQUIRE(CountContains("abcdabc", "abc") == 2);
  REQUIRE(CountContains("abcde", "bc") == 1);
  REQUIRE(CountContains("abcde", "abcdef") == 0);
}},
{"Containsのposがあるケースのテスト", [] {
  size_t pos = 0;
  REQUIRE( Contains("abcde", "abc", pos ) );
  REQUIRE( pos == 3 );

  REQUIRE( !Contains("abcde", "abc", pos ) );
  REQUIRE( pos == 3 );

  pos = 0;
  REQUIRE( Contains("abcabc", "abc", pos ) );
  REQUIRE( pos == 3 );
  REQUIRE( Contains("abcabc", "abc", pos ) );
  REQUIRE( pos == 6 );
  pos = 1;
  REQUIRE( Contains("abcabc", "abc", pos ) );
  REQUIRE( pos == 6 );
  pos = 5;
  REQUIRE( !Contains("abcabc", "abc", pos ) );
}},
{"UniqueNameのテスト", []{
  ResetUniqueName();

  if (SECTION("一回目はr0")) {SG g;
    auto name = UniqueName('r');
    REQUIRE( name == "r0");

    if (SECTION("2回目はr1")) {SG g;
      auto name2 = UniqueName('r');
      REQUIRE(name2 == "r1");
    }
    if (SECTION("他のprefixなら0")) {SG g;
      auto name2 = UniqueName('v');
      REQUIRE(name2 == "v0");
    }
  }
}},
{"Symbolizerのテスト", []{
  Symbolizer symbolizer;

  if (SECTION("一回目はid 1")) {SG g;
    auto hoge1 = symbolizer.Intern("hoge");
    REQUIRE( hoge1 == 1 );

    if (SECTION("違う文字列をinternすると別のid")) {SG g;
      auto ika = symbolizer.Intern("ika");
      REQUIRE( ika != hoge1 );

      if (SECTION("逆引きのテスト")) {SG g;
        auto hogeName = symbolizer.ToName( hoge1 );
        auto ikaName = symbolizer.ToName( ika );
        REQUIRE( "hoge" == hogeName );
        REQUIRE( "ika" == ikaName );
      }
    }
    if (SECTION("同じ文字列を二回Internしても同じid")) {SG g;
      auto hoge2 = symbolizer.Intern("hoge");
      REQUIRE( hoge1 == hoge2 );
    }
  }
}},
{"EnumSystemのテスト", []{
  EnumSystem esym;

  esym.Register( "SamplerCoord", { "Pixel", "NormalizedNearest", "NormalizedLinear" } );
  esym.Register( "SamplerAddressMode", { "None", "ClampToEdge", "ClampToBorderValue" } );

  size_t coord;
  size_t amode;
  REQUIRE( esym.LookupSymbol("SamplerCoord", coord ) );
  REQUIRE( esym.LookupSymbol("SamplerAddressMode", amode ) );

  if (SECTION("基本的なResolve")) {SG g;
    size_t nearest;
    REQUIRE( esym.Resolve( coord, "NormalizedNearest", nearest ) );
    REQUIRE( nearest == 1 );

    size_t clampBorder;
    REQUIRE( esym.Resolve( amode, "ClampToBorderValue", clampBorder ) );
    REQUIRE( clampBorder == 2 );
  }
  if (SECTION("存在しないResolveが失敗するか")) {SG g;
    size_t dummy;
    // enumTypeId違い
    REQUIRE( !esym.Resolve( amode, "NormalizedNearest", dummy ) );
    REQUIRE( !esym.Resolve( coord, "ClampToBorderValue", dummy ) );
  }
}},
{"TensorLiteralData、intのケース", []{
  auto ldata = TensorLiteralData::Create2D( std::vector<int32_t>{1, 2, 3} );  
  REQUIRE( Int(32) == ldata._type );

  if (SECTION("intの適切なrowを足して期待通りか")) {SG g;
    REQUIRE( ldata.AddRow( std::vector<int32_t>{ 4, 5, 6 } ) );
    REQUIRE( ldata._extents[0] == 3 );
    REQUIRE( ldata._extents[1] == 2 );

    REQUIRE( 1 == ldata._data[ 0 + 0] );
    REQUIRE( 2 == ldata._data[ 1 + 3*0 ] );
    REQUIRE( 3 == ldata._data[ 2 + 3*0 ] );
    REQUIRE( 4 == ldata._data[ 0 + 3*1 ] );
    REQUIRE( 5 == ldata._data[ 1 + 3*1 ] );
    REQUIRE( 6 == ldata._data[ 2 + 3*1 ] );
  }
  if (SECTION("型の違うrowを足してfalseが返るか")) {SG g;
    REQUIRE( !ldata.AddRow( std::vector<int32_t>{ 4, 5, 6, 7 } ) );
    REQUIRE( !ldata.AddRow( std::vector<float>{ 4.0F, 5.0F, 6.0F } ) );
    REQUIRE( ldata._extents[1] == 1 );
  }

}},
{"mfgでリークが無いかの確認。", []{
  if (SECTION("リークが無いかの確認")) {SG g;
    REQUIRE( g_node_alloc_count == 0 );
    REQUIRE( g_irelem_alloc_count == 0 );
  }
}}
};

static std::vector<TestPair> test_cases_mfg_ir = {
{"Extentのテスト", []{
  ResetUniqueName();

  TLTensor ts( UniqueName('f'), 2 );
  VarE x( Int(32), "x" );
  VarE y( Int(32), "y" );

  auto tsinfo = ts.GetTensorInfo();
  VarE x_extent( Int(32), tsinfo.GetExtentName( 0 ) );
  VarE y_extent( Int(32), tsinfo.GetExtentName( 1 ) );

  ts.Define(
    {"x", "y"},
    Expr_BodyElem(
        Expr_Add( y_extent, Expr_Add( x_extent, Expr_Add( x, y ) ) )
      )    
  );

  if (SECTION("変数名は期待通りか")){SG g;
    REQUIRE( "extent.f0.0" == x_extent._name );
    REQUIRE( "extent.f0.1" == y_extent._name );
  }

  #ifdef MFG_BACKEND_METAL
  if (SECTION("metalのテスト")) {SG g;
    MSLTensorGenerator tgen( ts );
    SeedGenerator sgen( 123 );
    tgen.GenLibrarySource( sgen );

    MCtx1K mctx( tgen, 2, 3 );
    TensorBuffer tb( mctx._device, ts, { 2, 3 } );

    mctx.ArgsSetup()
      .AddTensorBuffer( tb )
      .AddInts( { 2, 3 } );
    mctx.RunBlock();

    auto buf = tb[0].Accessor( { 2, 3 } );

    REQUIRE( buf.Int32( 0, 0 ) == 0+5 );
    REQUIRE( buf.Int32( 1, 0 ) == 1+5 );
    REQUIRE( buf.Int32( 0, 1 ) == 1+5 );
    REQUIRE( buf.Int32( 1, 1 ) == 2+5 );
    REQUIRE( buf.Int32( 0, 2 ) == 2+5 );
    REQUIRE( buf.Int32( 1, 2 ) == 3+5 );
  }
  #endif
}},
{"Tupleのテスト", []{
  ResetUniqueName();
  IRBuildDSL d;

  TLTensor ts( UniqueName('f'), 2 );
  VarE x( Int(32), "x" );
  VarE y( Int(32), "y" );

  ts.Define(
    {"x", "y"},
    Expr_BodyElem(
        d._Tuple({ d._Add( x, y ), Expr_FCallOne( Call::SIN_F32, d._ToF32( d._Mul( x, y ) ) ) })
      )    
  );

  /*
  auto actual = PrintExprS( tensor.GetBody() );
  cout << actual << endl;
  */
  #ifdef MFG_BACKEND_D3D
  if (SECTION("D3Dのテスト")) {SG g;
    SeedGenerator sgen( 123 );
    HLSLTensorGenerator tgen( ts );
    tgen.GenAll( sgen );

    auto shaderSrc = tgen.ToString();
    // cout << shaderSrc << endl;

    if (SECTION("D3DでタプルのBufferが正しく生成されているか")) {SG g;
      CheckContains( shaderSrc, "RWByteAddressBuffer resultBuf0 : register(u0);" );
      CheckContains( shaderSrc, "RWByteAddressBuffer resultBuf1 : register(u1);" );
    }
    if (SECTION("D3DでタプルのStoreが正しく生成されているか")) {SG g;
      CheckContains( shaderSrc, "resultBuf0.Store(((_y*_extent_f0_0)+_x)*4, (_x+_y));");
      CheckContains( shaderSrc, "resultBuf1.Store(((_y*_extent_f0_0)+_x)*4, asuint(sin(((float)(_x*_y)))));");
    }
    if (SECTION("D3Dで実行して結果が合うか")) {SG g;
      // cout << shaderSrc << endl;

      D3DAllDllLoader d3druntime;
      D3DContext d3dctx;
      TensorBuffer tbuf( d3dctx, ts, { 2, 3 } );

      std::vector<int32_t> iparam1 { 2, 3 };
      D3DBuffer paramBuf1( d3dctx, sizeof(int32_t), iparam1.size(), iparam1.data() );

      D3DShader shader1( d3dctx, shaderSrc );
      d3dctx.Dispatch( shader1, { &paramBuf1}, { &tbuf[0], &tbuf[1] }, 2, 3 );

      D3DCpuBuffer buf1( d3dctx, tbuf[0], { 2, 3 } );

      REQUIRE( buf1.Int32( 0, 0 ) == 0 );
      REQUIRE( buf1.Int32( 1, 0 ) == 1 );
      REQUIRE( buf1.Int32( 0, 1 ) == 1 );
      REQUIRE( buf1.Int32( 1, 1 ) == 2 );
      REQUIRE( buf1.Int32( 0, 2 ) == 2 );
      REQUIRE( buf1.Int32( 1, 2 ) == 3 );

      D3DCpuBuffer buf2( d3dctx, tbuf[1],  { 2, 3 } );
      // fprintf( stderr, "%f\n", buf2.Float32( 1, 1 ) );
      // fprintf( stderr, "%f, %f\n", buf2.Float32( 1, 2 ), std::sin( 2.0f ) );
      REQUIRE( buf2.Float32( 0, 0 ) == 0.0f );
      REQUIRE( buf2.Float32( 1, 0 ) == 0.0f );
      REQUIRE( buf2.Float32( 0, 1 ) == 0.0f );
      REQUIRE( buf2.Float32( 1, 1 ) == std::sin( 1.0f ) );
      REQUIRE( buf2.Float32( 0, 2 ) == 0.0f );
      REQUIRE_FEQUALS( buf2.Float32( 1, 2 ), std::sin( 2.0f ) );
    }
  }
  #endif
  #ifdef MFG_BACKEND_METAL
  if (SECTION("metalのテスト")) {SG g;
    MSLTensorGenerator tgen( ts );
    SeedGenerator sgen( 123 );
    if (SECTION("カーネルの引数宣言でタプルが正しく展開されるか")) {SG g;
      tgen.GenSignature();
      auto actual = tgen.ToString();
      // cout << actual << std::endl;
      CheckContains( actual, "device int32_t* _b_f0_out_0," );
      CheckContains( actual, "device float* _b_f0_out_1," );
    }
    if (SECTION("カーネルのbody部の最後でタプルを展開したassignになっているか")) {SG g;
      tgen.GenInitDefBody();
      auto actual = tgen.ToString();
      // cout << actual << std::endl;

      // 細かい生成結果でfailしないように要点だけverify
      CheckContains( actual, "_b_f0_out_0[" );
      CheckContains( actual, "_b_f0_out_1[" );
      REQUIRE( 2 == CountContains( actual, "] =" ) );
    }

    if (SECTION("Metalで実行して結果が合うか")) {SG g;
      tgen.GenLibrarySource( sgen );
      // cout << tgen.ToString() << endl << endl;

      MCtx1K mctx( tgen, 2, 3 );
      id device = mctx._device;
      MetalBuffer bufInt( device, sizeof(int32_t) * 2* 3 );
      MetalBuffer bufFloat( device, sizeof(float) * 2* 3 );

      mctx.ArgsSetup()
        .AddBuffer( bufInt )
        .AddBuffer( bufFloat )
        .AddInts( { 2, 3, 2, 3 } );

      mctx.RunBlock();

      auto buf1 = bufInt.Accessor( { 2, 3 } );
      REQUIRE( buf1.Int32( 0, 0 ) == 0 );
      REQUIRE( buf1.Int32( 1, 0 ) == 1 );
      REQUIRE( buf1.Int32( 0, 1 ) == 1 );
      REQUIRE( buf1.Int32( 1, 1 ) == 2 );
      REQUIRE( buf1.Int32( 0, 2 ) == 2 );
      REQUIRE( buf1.Int32( 1, 2 ) == 3 );

      auto buf2 = bufFloat.Accessor( { 2, 3 } );
      REQUIRE( buf2.Float32( 0, 0 ) == 0.0f );
      REQUIRE( buf2.Float32( 1, 0 ) == 0.0f );
      REQUIRE( buf2.Float32( 0, 1 ) == 0.0f );
      REQUIRE_FEQUALS( buf2.Float32( 1, 1 ), std::sin( 1.0f ) );
      REQUIRE( buf2.Float32( 0, 2 ) == 0.0f );
      REQUIRE_FEQUALS( buf2.Float32( 1, 2 ), std::sin( 2.0f ) );
    }

  }
  #endif
}},
{"ReduceSumのテスト", []{
  ResetUniqueName();

  TLTensor ts( "temp", 2 ), dest( "dest", 2 );
  VarE x( Int(32), "x" );
  VarE y( Int(32), "y" );
  VarE rx( Int(32), "rx" );
  VarE ry( Int(32), "ry" );

  ts.Define(
    {"x", "y"},
    Expr_BodyElem(
        Expr_Add( x, Expr_Add( Expr_Mul(5, y), 3) )
      )    
  );

  dest.Define(
    {"x", "y"},
    Expr_BodyElem(
      Expr_ReduceSum( { 0, 2, 0, 3 },
        Expr_Block( {"rx", "ry"}, 
          Expr_BodyElem(
            Expr_TensorCall( ts.GetTensorInfo(), Expr_Add( x, rx ), Expr_Add( y, ry ) )
          )
        ) 
      )
    )
  );
  dest.AddReference( ts );

  if (SECTION("ツリーが期待通りか")) {SG g;
    auto actual = PrintExprS( dest.GetBody() );
    // cout << actual << endl;
    VerifyEquals(R"(vblock {
 reduce_sum r0 0..<2, 0..<3 by (func(rx:int32, ry:int32)
 vblock {
  temp(x + rx, y + ry)
 }
)
}
)", actual);
  }

  if (SECTION("LoopVExprHoisterrがrsumを正しく処理するかのテスト")) { SG g;
    StmtLikeVExprHoister hoister;
    auto block = dest.GetBody()->DetachExprs();
    REQUIRE( block.size() == 1 ); // rsumのみのはず。
    auto newElem = hoister.ReplaceAll( std::move( block[0] ) );

    auto newTree = PrintExprS( newElem );
    // cout << "(" << newTree << ")" << endl;
    VerifyEquals("rres.r0.0\n", newTree );


    REQUIRE( hoister._hoisted.size() == 1 );
    REQUIRE( hoister._hoisted[0].GetElemType() == IRElemType::ReduceSum );

    /*
    auto rsumTree = PrintExprS( hoister._hoisted[0] );
    cout << endl << "rsumtree: " << endl;
    cout << rsumTree << endl;
    */
  }

  if (SECTION("destをLowerしてrsumを展開")) {SG g;
    InlineFuncRegistry ifr;
    dest.Lower( ifr );

    if (SECTION("rsumのLowerが期待通りか？")) {SG g;
      auto actual = PrintExprS( dest.GetBody() );
      // cout << actual << endl;

      if (SECTION("rsumのLowerでboundsが正しく生成されているか")) {SG g;
        size_t pos = 0;
        CheckContains( actual, "let rbegin.r0.0 = 0", pos );
        CheckContains( actual, "let rend.r0.0 = 2", pos );
        CheckContains( actual, "let rend.r0.1 = 3", pos );

        if (SECTION("Forの順番が正しいか")) {SG g;
          // この順番が正しいのかちょっと自信が無いが、治すならtensor側も含めて一貫性をもたせるべきなので一旦このまま一通り作業を終える。
          CheckContains( actual, "for(ry", pos );
          CheckContains( actual, "for(rx", pos );
        }

        if (SECTION("rsumのLowerでAccmのdefが正しく生成されているか")) {SG g;
          CheckContains( actual, "var rres.r0.0 = 0", pos );

          if (SECTION("rsumのLowerでAccmの更新が正しく生成されているか")) {SG g;
            CheckContains( actual, "store rres.r0.0 = rres.r0.0 + (load:b_temp", pos );
          }
        }
      }

    }

    #ifdef MFG_BACKEND_METAL
    if (SECTION("metalのテスト")) {SG g;
      MSLTensorGenerator tgen( ts );
      MSLTensorGenerator tdGen( dest );
      SeedGenerator sgen( 123 );

      if (SECTION("MSL生成、ReduceSumの生成")) {SG g;
        tdGen.GenInitDefBody();
        auto actual = tdGen.ToString();
        // cout << actual << std::endl;

        if(SECTION("rsumのforの生成")) {SG g;
          size_t pos = 0;
          CheckContains( actual, "int32_t _rres_r0_0 = 0;", pos );
          CheckContains( actual, "for(int32_t _ry = _rbegin_r0_1; _ry < _rend_r0_1; _ry++ )", pos );
          CheckContains( actual, "{", pos );
          CheckContains( actual, "for(int32_t _rx = _rbegin_r0_0; _rx < _rend_r0_0; _rx++ )", pos );
          CheckContains( actual, "{", pos );
          CheckContains( actual, "}", pos );
          CheckContains( actual, "}", pos );
        }

        if (SECTION("rsumのaccmの更新")) {SG g;
          CheckContains( actual, "_rres_r0_0 = (_rres_r0_0+_b_temp_0[" );
        }

        if (SECTION("rsumの結果の使用")) {SG g;
          CheckContains( actual, "= _rres_r0_0;" );
        }
      }

      if (SECTION("ReduceSumをMetalで実行して結果があうか？")) {SG g;
        // 現状rdとバッファでboundsの指定の仕方が結構違う…

        tgen.GenLibrarySource( sgen );
        tdGen.GenKernel( sgen );
        // cout << tgen.ToString() + tdGen.ToString() << std::endl;


        LibContext lctx( tgen.ToString()+tdGen.ToString() );
        id device = lctx._device;

        MetalBuffer tempBuf( device, sizeof(int32_t)*4*5 );
        MetalBuffer destBuf( device, sizeof(int32_t)*2*2 );

        auto tkern = lctx.CreateKernel( tgen );
        auto tcom = lctx.CreateExecCommand( tkern, 4, 5 );
        tcom.ArgsSetup()
          .AddBuffer( tempBuf )
          .AddInts( { 4, 5 } );
        tcom.RunBlock();

        auto destKern = lctx.CreateKernel( tdGen );
        auto destCom = lctx.CreateExecCommand( destKern, 2, 2 );
        destCom.ArgsSetup()
          .AddBuffer( destBuf )
          .AddBuffer( tempBuf )
          .AddInts( { 2, 2, 4, 5 } );
        
        destCom.RunBlock();

        auto output = destBuf.Accessor( { 2, 2 } );
        REQUIRE( 51 == output.Int32( 0, 0 ) );
        REQUIRE( 57 == output.Int32( 1, 0 ) );
        REQUIRE( 81 == output.Int32( 0, 1 ) );
        REQUIRE( 87 == output.Int32( 1, 1 ) );
      }
    }
    #endif
  }

}},
// モザイク関連の切り分けの為のテスト。
{"タプルのReduceSumのテスト", []{
  ResetUniqueName();
  IRBuildDSL d;

  TLTensor dest( "dest", 2 );
  VarE x( Int(32), "x" );
  VarE y( Int(32), "y" );
  VarE rx( Int(32), "rx" );
  VarE ry( Int(32), "ry" );

  dest.Define(
    {"x", "y"},
    Expr_BodyElem(
      Expr_ReduceSum( { 0, 2, 0, 3 }, 
        Expr_Block(
          {"rx", "ry"},
          Expr_BodyElem(
            d._Tuple({ d._Add( x, ry ), d._Add( y, rx ) })
          )
        )
      )
    )
  );

  InlineFuncRegistry ifr;
  dest.Lower( ifr );

  if (SECTION("IR上でベクトルバッファになっているかのテスト")) {SG g;
    auto tsinfo = dest.GetTensorInfo();
    REQUIRE( tsinfo.IsVector() );
    REQUIRE( tsinfo.ElemBufferNum() == 1 );
    REQUIRE( tsinfo.VectorExtent() == 2 );

    auto bufTypes = tsinfo.GetElemBufferTypes();
    REQUIRE( bufTypes.size() == 1 );
  }

  #ifdef MFG_BACKEND_METAL
  if (SECTION("metalのテスト")) {SG g;
    MSLTensorGenerator tgen( dest );
    SeedGenerator sgen( 123 );

    tgen.GenLibrarySource( sgen );

    MCtx1K kctx( tgen, 2, 2 );
    TensorBuffer tbuf( kctx._device, dest, { 2, 2 } );

    kctx.ArgsSetup()
      .AddTensorBuffer( tbuf )
      .AddInts( { 2, 2 } );

    kctx.RunBlock();

    // ベクトルなので、最初の次元を2にして対応
    auto output1 = tbuf[0].Accessor( { 2, 2, 2 } );

    REQUIRE( 2*(0+1+2) == output1.Int32( 0, 0, 0 ) );
    REQUIRE( 2*(1+2+3) == output1.Int32( 0, 1, 0 ) );
    REQUIRE( 2*(0+1+2) == output1.Int32( 0, 0, 1 ) );
    REQUIRE( 2*(1+2+3) == output1.Int32( 0, 1, 1 ) );

    REQUIRE( 3*(0+1) == output1.Int32( 1, 0, 0 ) );
    REQUIRE( 3*(0+1) == output1.Int32( 1, 1, 0 ) );
    REQUIRE( 3*(1+2) == output1.Int32( 1, 0, 1 ) );
    REQUIRE( 3*(1+2) == output1.Int32( 1, 1, 1 ) );
  }
  #endif
}},
{"Realizer1Dのテスト、キャストが自動で挟まるかもついでにテスト", []{
  // expとaddで自動でキャストが挟まるかのテストから持ってきたので
  // expもテストされる。本当は分けた方がいい気もするが、まぁいいでしょう。

  ResetUniqueName();

  TLTensor ts( UniqueName('f'), 1 );
  VarE x( Int(32), "x" );

  ts.Define(
    {"x"},
    Expr_BodyElem(
      Expr_FCallOne( Call::EXP_F32, Expr_Add( x, 0.3f ) )
    )
  );

  if (SECTION("intとの+でキャストがちゃんと生成されるか")) {SG g;
    auto actual = PrintExprS( ts.GetBody() );
    // cout << actual << endl;
    CheckContains( actual, "float32(x)" );
  }

  #ifdef MFG_BACKEND_METAL
  if (SECTION("metalのテスト")) {SG g;
    MSLTensorGenerator tgen( ts );
    SeedGenerator sgen( 123 );

    if (SECTION("extentやループ変数の処理が正しく行えているか？")) {SG g;
      tgen.GenFuncPrologue( sgen );
      auto actual = tgen.ToString();
      // cout << actual << std::endl;
      CheckContains( actual, "int32_t _x = (int32_t)grid_idx;" );
      CheckContains( actual, "int32_t _extent_f0_0 = param32_in[0];" );
      CheckNotContains( actual, "int32_t _extent_f0_1 =" );
    }
    if (SECTION("添字の処理が正しく行えているか？")) {SG g;
      tgen.GenInitDefBody();
      // cout << tgen.ToString() << std::endl;
      CheckContains( tgen.ToString(), "_b_f0_out_0[_x] =" );
    }
    // 1Dはthread dispatchのみで中のfor文が要らないので、関数の中括弧だけ。
    if (SECTION("カーネルの中括弧の対応はとれているか？")) {SG g;
      tgen.GenKernel( sgen );
      auto actual = tgen.ToString();
      // cout << actual << std::endl;
      REQUIRE( 1 == CountContains( actual, "{" ) );
      REQUIRE( 1 == CountContains( actual, "}" ) );
    }
    if (SECTION("Metalで実行して結果が合うか？")) {SG g;
      tgen.GenLibrarySource( sgen );
      MCtx1K mctx( tgen, 3, 1 );

      TensorBuffer tb( mctx._device, ts, 3 );
      mctx.ArgsSetup()
        .AddTensorBuffer( tb )
        .AddInts( { 3 } );
      
      mctx.RunBlock();

      auto output = tb[0].Accessor( { 3 } );
      REQUIRE_FEQUALS( 1.349859, output.Float32(0) );
      REQUIRE_FEQUALS( 3.669297, output.Float32(1) );
      REQUIRE_FEQUALS( 9.974182, output.Float32(2) );
    }

  }
  #endif
}},
// レンズぼかし関連
{"TensorSumReducer1Dのテスト", []{
  ResetUniqueName();

  TLTensor ts( "ts", 1 );
  TLBlock block1;
  
  VarE x( Int(32), "x" );
  VarE v( Int(32), "val" );
  VarE result( Int(32), "res" );

  ts.Define(
    {"x"},
    Expr_BodyElem(
        Expr_Add( x, 2 )
      )    
  );
  
  block1.DefineOne(
    Expr_Let( result,
      Expr_ReduceTensorSum( ts,  
        Expr_TSDomainBlock( ts, {"i", "val"},
          Expr_BodyElem(
            Expr_Add( Expr_Mul( 2, v ), 1 )
          )
        )
      )
    )
  );
  block1.AddReference( ts );

  if (SECTION("LoopVExprHoisterrがts.sumを正しく処理するかのテスト")) { SG g;
    StmtLikeVExprHoister hoister;
    auto block = block1.GetIRBody()->DetachExprs();
    REQUIRE( block.size() == 1 ); // letのみ

    auto newElem = hoister.ReplaceAll( std::move( block[0] ) );
    auto newTree = PrintExprS( newElem );    
    // cout << "(" << newTree << ")" << endl;
    CheckContains( newTree, "let res = rres.r0.0" );

    REQUIRE( hoister._hoisted.size() == 1 );
    REQUIRE( hoister._hoisted[0].GetElemType() == IRElemType::TensorIterator );
    /*
    auto rsumTree = PrintExprS( hoister._hoisted[0] );
    cout << endl << "rsumtree: " << endl;
    cout << rsumTree << endl;
    */
  }

  if (SECTION("Lower")) {SG g;
    InlineFuncRegistry ifr;

    block1.Lower( ifr );

    if (SECTION("ts.sumのLowerが正しく展開されているか")) {SG g;
      auto actual = PrintExprS( block1.GetIRBody() );
      // cout << actual << endl;

      if (SECTION("ts.sumのLowerでboundsが正しく生成されているか")) {SG g;
        size_t pos = 0;
        CheckContains( actual, "let rbegin.r0.0 = 0", pos );
        CheckContains( actual, "let rend.r0.0 = extent.ts.0", pos );

        if (SECTION("ts.sumのLowerでForとloadが正しいか")) {SG g;
          CheckContains( actual, "for(i:", pos );
          CheckContains( actual, "let val = load:b_ts.0[clamp(i, 0, extent.ts.0 - 1)]", pos );
        }

        if (SECTION("ts.sumのLowerでAccmのdefが正しく生成されているか")) {SG g;
          CheckContains( actual, "var rres.r0.0 = 0", pos );

          if (SECTION("ts.sumのLowerでAccmの更新が正しく生成されているか")) {SG g;
            CheckContains( actual, "store rres.r0.0 = rres.r0.0 + ((2 * val) + 1)", pos );
          }
        }
      }

      #ifdef MFG_BACKEND_D3D
      if (SECTION("D3Dのテスト")) {SG g;
        SeedGenerator sgen( 123 );
        HLSLTopLevelBlockGenerator topGen( block1 );
        topGen.GenAll( sgen );

        auto topSrc = topGen.ToString();
        // cout << topSrc << endl;
        if (SECTION("D3DのTopLevelBlockでStoreが正しく生成されているか")) {SG g;
          CheckContains( topSrc, "resultBuf.Store( 4*0, _res );" );
        }
        if (SECTION("D3DのTopLevelBlockでpramのインデックスが正しく生成されているか")) {SG g;
          CheckContains( topSrc, "const int _extent_ts_0 = asint(paramBuf.Load(4*0));" );
        }

        if (SECTION("D3Dのバックエンドで実行")) {SG g;
          HLSLTensorGenerator tgen( ts );
          SeedGenerator sgen( 123 );
          tgen.GenAll( sgen );

          REnv env;
          D3DAllDllLoader d3druntime;
          D3DContext d3dctx;

          TensorBuffer tsbuf( d3dctx, ts, { 3 } );

          D3DShader shader1( d3dctx, tgen.ToString() );
          d3dctx.Dispatch( shader1, {}, { &tsbuf[0] }, 3, 1 );

          std::vector<int32_t> iparam { 3 };
          D3DBuffer paramIn( d3dctx, sizeof(int32_t), iparam.size(), iparam.data() );
          Param32OutBuffer paramOut( d3dctx, 1 );
          D3DShader shader2( d3dctx, topSrc );
          d3dctx.Dispatch( shader2, { &paramIn, &tsbuf[0] }, { &paramOut._buffer }, 1, 1 );

          paramOut.CopyToEnv( topGen._varsInfo, env );

          auto expect = 2*(2+3+4)+3;
          REQUIRE( expect ==  env.GetValue<int32_t>( result.Name() ) );
        }
      }
      #endif
    }

    #ifdef MFG_BACKEND_METAL
    if (SECTION("metalのテスト")) {SG g;
      MSLTensorGenerator tgen( ts );
      MSLTopLevelBlockGenerator topGen( block1 );
      SeedGenerator sgen( 123 );

      topGen.CollectGlobalVars();

      if (SECTION("TensorSumのMSL生成のテスト")) {SG g;
        topGen.GenKernel( sgen );

        auto actual = topGen.ToString();
        // cout << actual << endl;

        if (SECTION("TensorSumのループとvalが正しく生成されているか？")) {SG g;
          CheckContains( actual, "for(int32_t _i = _rbegin_r0_0; _i < _rend_r0_0; _i++");
          CheckContains( actual, "const int32_t _val = _b_ts_0[clamp(_i, 0, (_extent_ts_0-1))];");
        }

        if (SECTION("TensorSumの周辺が正しく生成されているか？")) {SG g;
          size_t pos = 0;
          CheckContains( actual, "int32_t _rres_r0_0 = 0;", pos );
          CheckContains( actual, "for(int32_t _i =", pos );
          CheckContains( actual, "int32_t _res = _rres_r0_0;", pos );
          CheckContains( actual, "((device int32_t*)param32_out)[0] = _res;", pos );
        }
      }
      
      if (SECTION("Metalで実行して結果が合うか")) {SG g;
        tgen.GenLibrarySource( sgen );
        topGen.GenLibrarySource( sgen );

        REnv env;
        MetalContext ctx;
        id device = ctx._device;

        MetalLibrary tsLib( device, tgen.ToString() );
        TensorBuffer tbuf( device, ts, 3 );

        auto tsKern = tsLib.CreateKernel( tgen.GetKernelName() );
        auto tsCom = ctx.CreateExecCommand( tsKern, 3, 1 );
        tsCom.ArgsSetup()
          .AddTensorBuffer( tbuf )
          .AddInts( { 3 } );
        
        tsCom.RunAsync();

        MetalLibrary topLib( device, topGen.ToString() );
        Param32Buffer paramBuf( device, 1 );

        auto topKern = topLib.CreateKernel( topGen.GetKernelName() );
        auto topCom = ctx.CreateExecCommand( topKern, 1, 1 );
        topCom.ArgsSetup()
          .AddBuffer( paramBuf._buffer )
          .AddTensorBuffer( tbuf )
          .AddInts( { 3 } );

        topCom.RunBlock();
        paramBuf.CopyToEnv( topGen._varsInfo, env );

        auto expect = 2*(2+3+4)+3;
        REQUIRE( expect ==  env.GetValue<int32_t>( result.Name() ) );
      }
    }
    #endif
  }

}},
{"SqrtとSubのテスト", []{
  TLTensor ts( UniqueName('f'), 1 );
  VarE x( Int(32), "x" );

  ts.Define(
    {"x"},
    Expr_BodyElem(
        Expr_Sub( Expr_FCallOne( Call::SQRT_F32, 16.0f ), x )
      )    
  );

  #ifdef MFG_BACKEND_METAL
  if (SECTION("metalのテスト")) {SG g;
    MSLTensorGenerator tgen( ts );
    SeedGenerator sgen( 123 );
    tgen.GenLibrarySource( sgen );
    // cout << tgen.ToString() << endl;

    MCtx1K mctx( tgen, 3, 1 );
    TensorBuffer tbuf( mctx._device, ts, 3 );

    mctx.ArgsSetup()
      .AddTensorBuffer( tbuf )
      .AddInts( { 3 } );

    mctx.RunBlock();

    auto output = tbuf[0].Accessor( { 3 } );

    REQUIRE_FEQUALS( 4.0, output.Float32(0) );
    REQUIRE_FEQUALS( 3.0, output.Float32(1) );
    REQUIRE_FEQUALS( 2.0, output.Float32(2) );
  }
  #endif
}},
{"SwizzleのToTypeのテスト", []{
  ResetUniqueName();
  IRBuildDSL d;

  VarE hoge( Int(32), "hoge" );
  VarE ika( Float(32), "ika" );

  // swizzleをベクトルじゃないケースでサポートするのか？という話はあるが、
  // MFGとしては今のところタプル相手ならなんでも使える。

  auto tuple = d._Tuple({ hoge, ika, hoge, ika } );

  auto res = SwizzleCall::ToType( tuple, { 0, 3, 1, 0} );
  REQUIRE( res.IsTuple() );
  auto ntypes = res.ExpandNumericTypes();

  REQUIRE( ntypes.size() == 4 );
  REQUIRE( ntypes[0].IsInt() );
  REQUIRE( ntypes[1].IsFloat() );
  REQUIRE( ntypes[2].IsFloat() );
  REQUIRE( ntypes[3].IsInt() );
}},
{"SeedGeneratorのテスト", []{
  int origin = 123;
  SeedGenerator gen( origin );
  if(SECTION("最初の二つのシードが異なるか")){ SG g;
    int seed1 = gen.Next();
    int seed2 = gen.Next();
    REQUIRE( seed1 != seed2 );
    if (SECTION("Restartしたら同じ系列か")) {SG g;
      gen.Restart();
      int s1 = gen.Next();
      int s2 = gen.Next();
      REQUIRE( s1 == seed1 && s2 == seed2 );
    }
    if (SECTION("別のoriginをセットしたら違うか")) {SG g;
      gen.NewOrigin( 456 );
      int s1 = gen.Next();
      REQUIRE( s1 != seed1 );
    }
    if (SECTION("コピーしても同じ系列か")) {SG g;
      auto gen2 = gen;
      gen2.Restart();
      int s1 = gen2.Next();
      int s2 = gen2.Next();
      REQUIRE( s1 == seed1 && s2 == seed2 );

    }
  }
}},
{"mfgでリークが無いかの確認。", []{
  REQUIRE( g_node_alloc_count == 0 );
  REQUIRE( g_irelem_alloc_count == 0 );
}}
};

template<typename E, typename F>
void ExpectException( const char* msg, F f )
{
  if (SECTION(msg)) {SG g;
    try
    {
      f();
    }
    catch( const E& )
    {
      return;
    }
    REQUIRE( false );
  }
}

static std::vector<TestPair> test_cases_mfg_resource = {
{"引数一つのFormatのテスト、引数がsize_t", []{
  size_t arg = 123;
  if (SECTION("間のケース")) {SG g;
    auto actual = Format( "hoge{0}ika", arg );
    REQUIRE( actual == "hoge123ika" );
  }
  if (SECTION("最初のケース")) {SG g;
    auto actual = Format( "{0}ika", arg );
    REQUIRE( actual == "123ika" );
  }
  if (SECTION("最後のケース")) {SG g;
    auto actual = Format( "hoge{0}", arg );
    REQUIRE( actual == "hoge123" );
  }
}},
{"引数一つのFormatのテスト、引数がstd::string", []{
  std::string arg("fuga");
  auto actual = Format( "hoge{0}ika", arg );
  REQUIRE( actual == "hogefugaika" );
}},
{"引数一つのFormatのテスト、エラーのケース", []{
  ExpectException<InternalError>("{0}が無い", [](){
    Format( "hogeika", 123 );
  });
  ExpectException<InternalError>("{1}しか無い", [](){
    Format( "hoge{1}ika", 123 );
  });
  ExpectException<InternalError>("{0しか無い", [](){
    Format( "hoge{0ika", 123 );
  });
  ExpectException<InternalError>("{ 0 }とスペースがある", [](){
    Format( "hoge{ 0 }ika", 123 );
  });
}},
{"引数2つのFormatのテスト", []{
  size_t argsz = 123;
  std::string argstr( "fuga" );
  if (SECTION("size, stringのケース")) {SG g;
    auto actual = Format( "hoge{0}ika{1}tako", argsz, argstr );
    REQUIRE( actual == "hoge123ikafugatako" );
  }
  if (SECTION("string, sizeのケース")) {SG g;
    auto actual = Format( "hoge{0}ika{1}tako", argstr, argsz );
    REQUIRE( actual == "hogefugaika123tako" );
  }
  if (SECTION("{1} {0}のケース")) {SG g;
    auto actual = Format( "hoge{1}ika{0}tako", argsz, argstr );
    REQUIRE( actual == "hogefugaika123tako" );
  }
  if (SECTION("{0}{1}と隙間が無いケース")) {SG g;
    auto actual = Format( "hoge{0}{1}tako", argsz, argstr );
    REQUIRE( actual == "hoge123fugatako" );
  }
  if (SECTION("{0}と{1}が最初と最後のケース")) {SG g;
    auto actual = Format( "{0}hoge{1}", argsz, argstr );
    REQUIRE( actual == "123hogefuga" );
  }
  if (SECTION("intとchar*のケース")) {SG g;
    auto actual = Format( "hoge{0}ika{1}tako", 123, "fuga" );
    REQUIRE( actual == "hoge123ikafugatako" );
  }
}},
{"引数２つのFormatのテスト、エラーのケース", []{
  ExpectException<InternalError>("{0}も{1}も無い", [](){
    Format( "hogeika", 123, "abc" );
  });
  ExpectException<InternalError>("{0}が無い", [](){
    Format( "hoge{1}ika{1}", 123, "abc" );
  });
  ExpectException<InternalError>("{1}が無い", [](){
    Format( "hoge{0}ika", 123, "abc" );
  });
}},
{"引数3つのFormatのテスト", []{
  size_t argsz = 123;
  size_t argsz2 = 456;
  std::string argstr( "fuga" );
  if (SECTION("{0}, {1}, {2}の順番")) {SG g;
    auto actual = Format( "hoge{0}ika{1}tako{2}ABC", argsz, argsz2, argstr );
    REQUIRE( actual == "hoge123ika456takofugaABC" );
  }
  if (SECTION("{2}, {1}, {0}の順番")) {SG g;
    auto actual = Format( "hoge{2}ika{1}tako{0}ABC", argsz, argsz2, argstr );
    REQUIRE( actual == "hogefugaika456tako123ABC" );
  }
}},
{"引数4つのFormatのテスト", []{
  size_t argsz = 12;
  size_t argsz2 = 34;
  size_t argsz3 = 56;
  std::string argstr( "fuga" );
  if (SECTION("{0}, {1}, {2}, {3}の順番")) {SG g;
    auto actual = Format( "hoge{0}ika{1}tako{2}ABC{3}DEF", argsz, argsz2, argstr, argsz3 );
    REQUIRE( actual == "hoge12ika34takofugaABC56DEF" );
  }
  if (SECTION("{2}, {1}, {3}, {0}の順番")) {SG g;
    auto actual = Format( "hoge{2}ika{1}tako{3}ABC{0}DEF", argsz, argsz2, argstr, argsz3 );
    REQUIRE( actual == "hogefugaika34tako56ABC12DEF" );
  }
}},
{"言語設定のテスト", []{
  auto guard = ScopeGuard([]{ SetLanguage( NL_ENGLISH ); });

  if(SECTION("英語のリソースが取れているか")) {SG g;
    auto actual = ResourceString( ResId::ONE_D_REDUCE_WITH_NONE_ZERO_DIM );
    REQUIRE( std::string("1D tensor reduce but dim is not 0: {0}") == std::string(actual) );
  }

  if (SECTION("日本語のリソースが取れているか")) {SG g;
    SetLanguage( NL_JAPANESE );
    auto actual = ResourceString( ResId::ONE_D_REDUCE_WITH_NONE_ZERO_DIM );
    REQUIRE( std::string("1次元テンソルのreduceで0以外のdimが指定されています： dim: {0}") == std::string(actual) );
  }

}},
{"リソースIDのLookupのテスト", []{
  if(SECTION("存在するリソースIDの場合はそのIDを返す")) {SG g;
    auto res = LookupResId( "NEWLINE_INSIDE_STRING" );
    REQUIRE( res == ResId::NEWLINE_INSIDE_STRING );
  }

  if (SECTION("存在しないリソースIDの場合はUserError")) {SG g;
    bool thrown = false;
    try {
      LookupResId("NON_EXISTING_RESOURCE_ID_XXXX");
    } catch( const UserError& ) {
      thrown = true;
    }
    REQUIRE( thrown );
  }
}},
{"リソースIDの存在確認のテスト", []{
  if(SECTION("存在するリソースIDの場合はtrue")) {SG g;
    auto res = ResIdExist( "NEWLINE_INSIDE_STRING" );
    REQUIRE( res );
  }

  if (SECTION("存在しないリソースIDの場合はfalse")) {SG g;
    auto res = ResIdExist( "NON_EXISTING_RES_ID_THIS_DOES_NOT_EXIST" );
    REQUIRE( !res );
  }
}},
{"TokenTypeToStrのテスト", []{
  // 自動生成がうまく行っているかを確認する程度
  REQUIRE( "DOLLAR" == TokenTypeToStr(TokenType::DOLLAR) );
}},
};

static std::map<std::string, std::string> g_resolverMap {
  { "TEST_ID", "これはテストのリソース文字列です" },
  { "MULTILINE_ID", "これは複数行のテストです。\nこれが二行目になります。" },
};

static std::string Resolver( const char* resId )
{
  auto res = g_resolverMap.find( resId );
  if (res == g_resolverMap.end())
    throw mfg_internal::InternalError("No resid found.");
  return res->second;
}

static ResourceExpander CreateExpander( std::string src )
{
  return ResourceExpander( src, Resolver );
}

static void VerifyMFGString( const std::string& expect, const MFGString& actual ) {
  REQUIRE( actual._type == MFGString::STRING );
  REQUIRE( expect == actual._stringVal );
};


// ResourceExpander関連のテスト
static std::vector<TestPair> test_cases_resource_expander = {
{"テスト用のResolverのテスト", []{
  REQUIRE( "これはテストのリソース文字列です" == Resolver("TEST_ID") );
}},
{"ConsumeCommitのテスト", []{
  if (SECTION("正常ケース")) {SG g;
    auto src = R"(# 234
let a = b
)";
      auto expander = CreateExpander( src );
      expander.ConsumeComment();
      REQUIRE( expander._result.str() == "# 234\n" );
      REQUIRE( expander.LookAt("let") );
  }
  if (SECTION("コメントで終わる場合")) {SG g;
    auto src = "# abc";
      auto expander = CreateExpander( src );
      expander.ConsumeComment();
      REQUIRE( expander.Pos() == 5 );
      REQUIRE( expander.Peek( 0 ) == -1 );
      REQUIRE( expander._result.str() == src );
  }
}},
{"ScanIdのテスト", []{
  auto src = "   THIS_IS_ID, this is not";
  auto expander = CreateExpander( src );
  expander.Advance( 3 );
  REQUIRE( expander.ScanId() == "THIS_IS_ID" );
  REQUIRE( expander.Pos() == 13 );
}},
{"ConsumeResourceのテスト", []{
  auto src = "$TEST_ID, this is not";
  auto expander = CreateExpander( src );
  expander.ConsumeResource();
  REQUIRE( expander._result.str() == "\"これはテストのリソース文字列です\"" ); 
  REQUIRE( expander.LookAt(",") );
}},
{"ConsumeStringLiteralのテスト", []{
  auto src = R"("String Literal"abc)";
  auto expander = CreateExpander( src );
  expander.ConsumeStringLiteral();
  REQUIRE( expander._result.str() == R"("String Literal")" ); 
  REQUIRE( expander.LookAt("abc") );
}},
{"ConsumeStringLiteral、エスケープのテスト", []{
  auto src = "\"String\\\" Literal\"abc";
  auto expander = CreateExpander( src );
  expander.ConsumeStringLiteral();
  REQUIRE( expander._result.str() == "\"String\\\" Literal\"" );
  REQUIRE( expander.LookAt("abc") );
}},
{"ConsumeResourceCommentのテスト", []{
  auto src = R"(#$MULTILINE_ID
let a = 2
)";
  auto expander = CreateExpander( src );
  expander.ConsumeResourceComment(2);
  // 1行目はインデントしない（すでにされてる）
  // 2行目はインデントする
  // 最後は改行で終わる
  REQUIRE( expander._result.str() ==
R"(# これは複数行のテストです。
  # これが二行目になります。
)" ); 
  REQUIRE( expander.LookAt("let") );
}},
{"ConsumeAllのテスト", []{
  auto src = R"(@title $TEST_ID
# this is test comment
# next line comment
#$MULTILINE_ID
def result_u8 |x, y| {
  #$TEST_ID
  let a = 2
  #$MULTILINE_ID
  u8[a, a, 0, 0]
}
)";
  auto expect = R"(@title "これはテストのリソース文字列です"
# this is test comment
# next line comment
# これは複数行のテストです。
# これが二行目になります。
def result_u8 |x, y| {
  # これはテストのリソース文字列です
  let a = 2
  # これは複数行のテストです。
  # これが二行目になります。
  u8[a, a, 0, 0]
}
)";

  auto expander = CreateExpander( src );
  auto actual = expander.ConsumeAll();
  REQUIRE( expect == actual );
}},
{"AttrScannerのScanTitleStringのテスト", []{
  if (SECTION("最初の行がtitle")) {SG g;
    auto firstline = R"(@title "これはテストのタイトルです"
# this is test comment
# next line comment
def result|x, y| {}
)";
    AttrScanner scanner( firstline );
    auto actual = scanner.ScanTitleString();
    REQUIRE( actual == "これはテストのタイトルです" );
  }
  if (SECTION("最初に空行")) {SG g;
    auto firstline = R"(
@title "これはテストのタイトルです"
# this is test comment
# next line comment
def result|x, y| {}
)";
    AttrScanner scanner( firstline );
    auto actual = scanner.ScanTitleString();
    REQUIRE( actual == "これはテストのタイトルです" );
  }
  if (SECTION("最初にコメント")) {SG g;
    auto firstline = R"(# this is test comment
@title "これはテストのタイトルです"
# next line comment
def result|x, y| {}
)";
    AttrScanner scanner( firstline );
    auto actual = scanner.ScanTitleString();
    REQUIRE( actual == "これはテストのタイトルです" );
  }
}},
{"AttrScannerのScanVersionのテスト", []{
  auto scanVersion = []( AttrScanner& scanner ) {
    auto mfgstr = scanner.ScanVersion();
    REQUIRE( mfgstr._type == MFGString::STRING );
    return mfgstr._stringVal;
  };

  if (SECTION("最初の行がversion")) {SG g;
    auto firstline = R"(@version "1.2.3"
# this is test comment
# next line comment
def result|x, y| {}
)";
    AttrScanner scanner( firstline );
    auto actual = scanVersion( scanner );
    REQUIRE( actual == "1.2.3" );
  }
  if (SECTION("versionとtitleの混在で、順不同でscan出来るか")) {SG g;
    auto firstline = R"(@version "1.2.3"
@title "abc"
# this is test comment
# next line comment
def result|x, y| {}
)";
    AttrScanner scanner( firstline );
    auto title = scanner.ScanTitleString();
    REQUIRE( title == "abc" );
    auto version = scanVersion( scanner );
    REQUIRE( version == "1.2.3" );
  }
}},
{"AttrScannerのScanStringAttrListのテスト", []{
  std::vector<std::string> attrNames { "title", "version" };

  if (SECTION("versionとtitleが両方あるケース、versionが先")) {SG g;
    auto src = R"(@version "1.2.3"
# this is test comment
# next line comment
@title "abc"
def result|x, y| {}
)";
    AttrScanner scanner( src );
    auto ret = scanner.ScanStringAttrList( attrNames );
    REQUIRE( ret.size() == 2 );
    VerifyMFGString( "abc", ret["title"] );
    VerifyMFGString( "1.2.3", ret["version"] );
  }
  if (SECTION("versionとtitleが両方あるケース、titleが先")) {SG g;
    auto src = R"(# this is test comment
@title "abc"
@version "1.2.3"
# next line comment
def result|x, y| {}
)";
    AttrScanner scanner( src );
    auto ret = scanner.ScanStringAttrList( attrNames );
    REQUIRE( ret.size() == 2 );
    VerifyMFGString( "abc", ret["title"] );
    VerifyMFGString( "1.2.3", ret["version"] );
  }
  if (SECTION("versionだけ")) {SG g;
    auto src = R"(# this is test comment
@version "1.2.3"
# next line comment
def result|x, y| {}
)";
    AttrScanner scanner( src );
    auto ret = scanner.ScanStringAttrList( attrNames );
    REQUIRE( ret.size() == 1 );
    VerifyMFGString( "1.2.3", ret["version"] );
  }
  if (SECTION("titleだけ")) {SG g;
    auto src = R"(@title "abc"
# this is test comment
# next line comment
def result|x, y| {}
)";
    AttrScanner scanner( src );
    auto ret = scanner.ScanStringAttrList( attrNames );
    REQUIRE( ret.size() == 1 );
    VerifyMFGString( "abc", ret["title"] );
  }
}},
};


static std::vector<TestPair> test_cases_json_resource = {
{"StringToNLanguageのテスト", []{
  if (SECTION("存在するロケールのケース")) {SG g;
    REQUIRE( NL_JAPANESE == StringToNLanguage( "ja" ) );
  }
  if (SECTION("存在しないロケールは例外")) {SG g;
    bool thrown = false;
    try
    {
      StringToNLanguage("jaNotFound");
    }catch(...)
    {
      thrown = true;
    }
    REQUIRE( thrown );
  }
}},
{"JsonToResStringMapのテスト", []{
std::string strings_json = R"(
{
  "ja": {"MY_TITLE": "日本語タイトル", "STRENGTH_LABEL": "強さ"},
  "en": {"MY_TITLE": "English or Default Title", "STRENGTH_LABEL": "Strength"}
}
)";

  auto rmap = JsonToResStringMap( strings_json );
  if (SECTION("フランス語は無いはず")) {SG g;
    REQUIRE( rmap.end() == rmap.find(mfg_pal::NL_FRENCH) );
  }
  if (SECTION("日本語はあるはず")) {SG g;
    auto jiter = rmap.find( mfg_pal::NL_JAPANESE );
    REQUIRE( jiter != rmap.end() );
    if (SECTION("存在するラベルのルックアップが成功する")) {SG g;
      auto& jmap = jiter->second;
      REQUIRE( "日本語タイトル" == jmap["MY_TITLE"] );
      REQUIRE( "強さ" == jmap["STRENGTH_LABEL"] );
    }
  }
  if (SECTION("英語はあるはず")) {SG g;
    auto eiter = rmap.find( mfg_pal::NL_ENGLISH );
    REQUIRE( eiter != rmap.end() );
    if (SECTION("存在するラベルのルックアップが成功する")) {SG g;
      auto& emap = eiter->second;
      REQUIRE( "English or Default Title" == emap["MY_TITLE"] );
      REQUIRE( "Strength" == emap["STRENGTH_LABEL"] );
    }
  }
}}
};

#include "mar_util.hpp"
using namespace mar_util;

static std::vector<TestPair> test_cases_mar_util = {
{"FindFirstSuffixFileのテスト", []{
  /*
  std::ifstream ifs("test_data/test_res.mar", std::ios::binary);
  if (!ifs) {
    std::cout << "fail!" << std::endl;
  }
  ifs.seekg(0, std::istream::end);
  */

  auto file = mfg_pal::GetZipFile( "test_data/test_res.mar" );
  uz::UnZipper unzipper(*file);

  auto lister = unzipper.listFiles();

  REQUIRE( lister.end() == FindFirstSuffixFile( lister, ".png" ) );
  REQUIRE( lister.end() != FindFirstSuffixFile( lister, ".mfg" ) );
  REQUIRE( lister.end() != FindFirstSuffixFile( lister, ".json" ) );
}},
{"MARFileのテスト", []{
  auto targetPath = "test_data/test_res.mar";
  if (SECTION("Verifyが成功するか")) {SG g;
    // failするケースを用意するのは面倒なので成功だけ。
    REQUIRE( MARUtil::Verify( targetPath ) );
  }
  if (SECTION("LookupTitleのテスト、タイトルが無いケース")) {SG g;
    // タイトルが無いがすでにcommitされてるデータで手頃だったので
    REQUIRE( "" == MARUtil::LookupTitle( "test_data/test_mfg.zip" ) );
  }
  if (SECTION("LookupTitleのテスト、jsonのリソースを使っているケース")) {SG g;
    REQUIRE( "English or Default Title" == MARUtil::LookupTitle( targetPath ) );
  }
}},
{"MARFileのLookupInfoのテスト", []{
  auto guard = ScopeGuard([]{ SetLanguage( NL_ENGLISH ); });
  /*
    "ja": {"TITLE_LABEL": "テストタイトル"},
  "en": {"TITLE_LABEL": "Test Title"}
*/
  if (SECTION("LookupInfoで@titleと@versionが正しくとれるか")){SG g;
    auto info = MARUtil::LookupInfo( "test_data/version_test.mar" );
    REQUIRE( info._title == "Test Title");
    REQUIRE( info._version == "1.0");
  }
  if (SECTION("日本語の解決も出来るか")) {SG g;
    SetLanguage( NL_JAPANESE);
    auto info = MARUtil::LookupInfo( "test_data/version_test.mar" );
    REQUIRE( info._title == "テストタイトル");
  }
  if (SECTION("versionが無いファイルでは空文字になるか？")) {SG g;
    auto info = MARUtil::LookupInfo( "test_data/test_mfg.zip" );
    REQUIRE( info._title == "" );
    REQUIRE( info._version == "" );
  }
}},
{"MARFileのLookupThumbnailのテスト", []{
  if (SECTION("thumbnail.pngが無いmarでは空のベクトル")) {SG g;
    auto thumb = MARUtil::LookupThumbnail("test_data/version_test.mar");
    REQUIRE( thumb.size() == 0 );
  }
  if (SECTION("thumbnail.pngがあるmarでは中身がある")) {SG g;
    auto thumb = MARUtil::LookupThumbnail("test_data/thumbnail_test.mar");
    REQUIRE( thumb.size() != 0 );
  }
}}
};
static std::vector<TestPair> test_cases_addhook = {
{"MFGのアドホックなテスト", []{
}}
};

void RegisterMFGTest(std::vector<TestPair>& testCases)
{
#if 0
  testCases.insert( testCases.end(), test_cases_addhook.begin(), test_cases_addhook.end() );
#else
  testCases.insert(testCases.end(), test_cases_mfg_ir.begin(), test_cases_mfg_ir.end());
  testCases.insert(testCases.end(), test_cases_mfg_misc.begin(), test_cases_mfg_misc.end());
  testCases.insert(testCases.end(), test_cases_mfg_resource.begin(), test_cases_mfg_resource.end());
  testCases.insert(testCases.end(), test_cases_resource_expander.begin(), test_cases_resource_expander.end());
  testCases.insert(testCases.end(), test_cases_json_resource.begin(), test_cases_json_resource.end());
  testCases.insert(testCases.end(), test_cases_mar_util.begin(), test_cases_mar_util.end());
#endif
}
