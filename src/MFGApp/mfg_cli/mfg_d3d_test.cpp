// Copyright 2026 Kazuma Arino. All rights reserved.
// SPDX-License-Identifier: Apache-2.0



#define _NFIFTEST_SUBTEST_
#include "nfiftest.hpp"
using namespace nfiftest;

#include "mfg_test_common.hpp"
using namespace mfg_test_common;

// インテリセンスの効きがいまいちなので同じファイルに再定義
#undef REQUIRE
#define REQUIRE(expr) if(!(expr)) throw nfiftest::assert_fail_error(__FILE__, __LINE__, #expr)

#include <iostream>
#include "forest.hpp"
#include "mfg.hpp"
#include "mfg_runtime_d3d.hpp"
#include <mfg_pal/pal.hpp>
#include <limits>


using namespace std;
using namespace mfg_ir_util;
using namespace mfg_internal;
using namespace mfg_hlsl_converter;
using namespace mfg_runtime_d3d;



////////////////////////////////////
// ハードコードしたHLSLを実行するテストコード。
// デバッグ時の雛形などに使う。
/////////////////////////////////////
static std::string adder_source = R"(
RWByteAddressBuffer Buffer0 : register(u0);
RWByteAddressBuffer Buffer1 : register(u1);
RWByteAddressBuffer BufferOut : register(u2);

[numthreads(1024, 1, 1)]
void CSMain( uint3 DTid : SV_DispatchThreadID )
{
    int input_u8 = asint( Buffer0.Load( DTid.x*4 ) );
    int i1 = asint( Buffer1.Load( DTid.x*4 ) );
    
    BufferOut.Store( DTid.x*4, asuint(input_u8 + i1) );
}
)";


static std::string negaposi_source = R"(
ByteAddressBuffer Buffer0 : register(t0);
RWByteAddressBuffer BufferOut : register(u0);

[numthreads(1, 1, 1)]
void CSMain( uint3 DTid : SV_DispatchThreadID )
{
  uint tmp = Buffer0.Load( DTid.x*4 );
  uint B = tmp&0xff;
  uint G = (tmp>>8)&0xff;
  uint R = (tmp>>16)&0xff;
  uint A = (tmp>>24)&0xff;

  R = 255-R;
  G = 255-G;
  B = 255-B;

  BufferOut.Store( DTid.x*4, ((A&0xff)<<24)|((R&0xff)<<16)| ((G&0xff)<<8) | (B&0xff) );
}
)";


extern void SetupNegaPosi( mfg_pal::Image32& inputTile, mfg_pal::Image32& resTile );
extern void AssertNegaPosi( mfg_pal::Image32& resTile );

extern bool LoadPngAsTile( const string& path, mfg_pal::Image32& dest );
extern bool SaveTileAsPng( const string& path, mfg_pal::Image32& tile );

#define REQUIRE_FEQUALS( expr1, expr2 ) if( std::abs((expr1) - (expr2)) > 0.001 )  throw nfiftest::assert_fail_error(__FILE__, __LINE__, std::string(": expr1(") + #expr1 + std::string("= ") + std::to_string((expr1)) + std::string(", expr2(") + (#expr2) + ")=" + std::to_string((expr2)))

static std::vector<TestPair> test_cases = {
{"DirecComputeのadderのテスト", []{
  D3DAllDllLoader d3druntime;

  D3DContext d3dctx;

  int32_t buf1[1024];
  int32_t buf2[1024];
  for( auto i : NRange(1024) )
  {
    buf1[i] = i;
    buf2[i] = 2048 - 2*i;
  }

  D3DBuffer d3d_buf1( d3dctx, sizeof(int32_t), 1024, buf1 );
  D3DBuffer d3d_buf2( d3dctx, sizeof(int32_t), 1024, buf2 );
  D3DBuffer d3d_bufOut( d3dctx, sizeof(int32_t), 1024 );

  /* コンパイル */
  D3DShader shader( d3dctx, adder_source );

  /* Dispatch */
  std::vector<D3DBuffer*> bufPtrs = { &d3d_buf1, &d3d_buf2, &d3d_bufOut };
  d3dctx.Dispatch( shader, {}, bufPtrs, 1, 1 );

  // CPUにコピーして結果をverify
  D3DCpuBuffer result( d3dctx, d3d_bufOut, { 1024 } );

  for( auto i : NRange(1024)) {
    REQUIRE( result.Int32(i) == (buf1[i]+buf2[i]) );
  }
}},
{"ハードコードしたネガポジ反転のテスト", []{
  D3DAllDllLoader d3druntime;
  D3DContext d3dctx;

  mfg_pal::Image32 inputTile;
  mfg_pal::Image32 resTile;

  SetupNegaPosi( inputTile, resTile );

  TileReference inputRes( &inputTile );
  TileReference resRes( &resTile );
  D3DBuffer d3d_buf1 = D3DBuffer::FromTile( d3dctx, NumericType::UInt(8), inputRes );
  D3DBuffer d3d_bufOut( d3dctx, sizeof(uint32_t), D3DBuffer::PixelNumOf( resTile ) );

  /* コンパイル */
  D3DShader shader( d3dctx, negaposi_source );

  /* Dispatch */
  d3dctx.Dispatch( shader, { &d3d_buf1 }, { &d3d_bufOut }, inputTile.Width()*inputTile.Height(), 1 );

  d3d_bufOut.ToTile( d3dctx, NumericType::UInt(8), resRes );

  auto guard_r = PixelLocker( &resTile );
  // fprintf(stderr, "PixelGet(0, 0) = %x\n", resTile.PixelGet( 0, 0 ).Value );
  AssertNegaPosi( resTile );

}},
{"D3DバックエンドのVer 0.1テスト", []{
  ResetUniqueName();

  TLTensor ts( "f1", 2 );
  VarE x( Int(32), "x" );
  VarE y( Int(32), "y" );
  ts.Define(
    {"x", "y"},
    Expr_BodyElem(
        Expr_Add( x, y )
      )    
  );

  SeedGenerator sgen( 123 );

  HLSLTensorGenerator tgen( ts );
  tgen.GenAll( sgen );


  auto shaderSrc = tgen.ToString();

  // 壊れてないか程度の軽いテスト
  if (SECTION("バッファのコード生成が期待通りか")) {SG g;
    // cout << shaderSrc << std::endl;
    CheckContains( shaderSrc, "ByteAddressBuffer paramBuf : register(t0);" );
    CheckContains( shaderSrc, "RWByteAddressBuffer resultBuf0 : register(u0);" );
  }

  if (SECTION("ランタイムで実行")) {SG g;
    D3DAllDllLoader d3druntime;
    D3DContext d3dctx;

    std::vector<int32_t> iparam { 2, 3 };

    D3DBuffer d3d_pramBuf( d3dctx, sizeof(uint32_t), iparam.size(), iparam.data() );
    D3DBuffer d3d_bufOut( d3dctx, sizeof(uint32_t), 2*3 );

    D3DShader shader( d3dctx, shaderSrc );

    /* Dispatch */
    d3dctx.Dispatch( shader, { &d3d_pramBuf }, { &d3d_bufOut }, 2, 3 );

    D3DCpuBuffer res( d3dctx, d3d_bufOut, {2, 3} );

    REQUIRE( res.Int32( 0, 0 ) == 0 );
    REQUIRE( res.Int32( 1, 0 ) == 1 );
    REQUIRE( res.Int32( 0, 1 ) == 1 );
    REQUIRE( res.Int32( 1, 1 ) == 2 );
    REQUIRE( res.Int32( 0, 2 ) == 2 );
    REQUIRE( res.Int32( 1, 2 ) == 3 );
  }


}},
{"D3Dで複数カーネルのテスト（0.4とだいたい同じ）", []{
  std::string source1 = R"(
RWByteAddressBuffer resultBuf : register(u0);

[numthreads(1, 1, 1)]
void CSMain( uint3 DTid : SV_DispatchTHreadID ){
  int _x = (int)DTid.x;
  int _y = (int)DTid.y;
  int _extent_ts_0 = 5;
  int _extent_ts_1 = 5;
  resultBuf.Store(((_y*_extent_ts_0)+_x)*4, (_x+_y));
})";

  std::string source2 = R"(
ByteAddressBuffer _ts: register(t0);
RWByteAddressBuffer resultBuf : register(u0);

[numthreads(1, 1, 1)]
void CSMain( uint3 DTid : SV_DispatchTHreadID ){
  int _x = (int)DTid.x;
  int _y = (int)DTid.y;
  int _e_result_extent_0 = 4;
  int _e_result_extent_1 = 4;
  int _extent_ts_0 = 5;
  int _extent_ts_1 = 5;
  resultBuf.Store(((_y*_e_result_extent_0)+_x)*4, ((((asint(_ts.Load(4*((_y*_extent_ts_0)+_x)))+asint(_ts.Load(4*(((_y+1)*_extent_ts_0)+_x))))+asint(_ts.Load(4*((_y*_extent_ts_0)+(_x+1)))))+asint(_ts.Load(4*(((_y+1)*_extent_ts_0)+(_x+1)))))/4));
  // resultBuf.Store(4*((_y*_e_result_extent_0)+_x), asint(_ts.Load(4*((_y*_extent_ts_0)+_x))));
}
)";

  D3DAllDllLoader d3druntime;
  D3DContext d3dctx;

  D3DBuffer tsbuf( d3dctx, sizeof(int32_t), 5*5 );
  D3DBuffer resbuf( d3dctx, sizeof(int32_t), 4*4 );

  D3DShader shader1( d3dctx, source1 );
  d3dctx.Dispatch( shader1, {}, {&tsbuf}, 5, 5 );
  
  D3DShader shader2( d3dctx, source2 );
  d3dctx.Dispatch( shader2, {&tsbuf}, {&resbuf}, 4, 4 );


  /*
  D3DCpuBuffer tsResult( d3dctx, tsbuf, {5, 5} );

  fprintf( stderr, "0, 0: %x\n", tsResult.Int32( 0, 0 ) );
  fprintf( stderr, "1, 0: %x\n", tsResult.Int32( 1, 0 ) );
  fprintf( stderr, "0, 1: %x\n", tsResult.Int32( 0, 1 ) );
  fprintf( stderr, "1, 1: %x\n", tsResult.Int32( 1, 1 ) );
  */
  
  D3DCpuBuffer result( d3dctx, resbuf, { 4, 4 } );

  /*
  fprintf( stderr, "0, 0: %x\n", result.Int32( 0, 0 ) );
  fprintf( stderr, "1, 0: %x\n", result.Int32( 1, 0 ) );
  fprintf( stderr, "0, 1: %x\n", result.Int32( 0, 1 ) );
  fprintf( stderr, "1, 1: %x\n", result.Int32( 1, 1 ) );
  cout << "debend" << std::endl;
  */

  REQUIRE( 1 == result.UInt32( 0, 0 ) );
  REQUIRE( 2 == result.UInt32( 1, 0 ) );
  REQUIRE( 2 == result.UInt32( 0, 1 ) );
  REQUIRE( 3 == result.UInt32( 1, 1 ) );

}},
{"D3Dのfloatのテスト", []{
  TLTensor ts( UniqueName('f'), 2 );
  VarE x( Int(32), "x" );
  VarE y( Int(32), "y" );

  ts.Define(
    {"x", "y"},
    Expr_BodyElem(
        Expr_FCallOne( Call::SIN_F32, Expr_CastFloat32( Expr_Mul( x, y ) ) ) 
    )
  );

  HLSLTensorGenerator tgen( ts );
  SeedGenerator sgen( 123 );
  tgen.GenAll( sgen );

  auto shaderSrc = tgen.ToString();
  if (SECTION("floatのStoreはasuintしないといけない")) {SG g;
    // cout << shaderSrc << endl;
    CheckContains( shaderSrc, "asuint(sin(" );
  }

  if (SECTION("D3Dで実行して結果が合うか")) {SG g;
    D3DAllDllLoader d3druntime;
    D3DContext d3dctx;

    D3DBuffer tbuf( d3dctx, sizeof(float), 2*3 );
    std::vector<int32_t> iparam1 { 2, 3 };
    D3DBuffer paramBuf1( d3dctx, sizeof(int32_t), iparam1.size(), iparam1.data() );

    D3DShader shader1( d3dctx, shaderSrc );
    d3dctx.Dispatch( shader1, { &paramBuf1}, { &tbuf }, 2, 3 );

    D3DCpuBuffer buf2( d3dctx, tbuf,  { 2, 3 } );
    // fprintf( stderr, "%f\n", buf2.Float32( 1, 1 ) );
    // fprintf( stderr, "%f, %f\n", buf2.Float32( 1, 2 ), std::sin( 2.0f ) );
    REQUIRE_FEQUALS( std::sin( 1.0f ), buf2.Float32( 1, 1 ) );
    REQUIRE_FEQUALS( std::sin( 2.0f ), buf2.Float32( 1, 2 ) );
  }

}},
{"mfgでリークが無いかの確認。", []{
  REQUIRE( g_node_alloc_count == 0 );
  REQUIRE( g_irelem_alloc_count == 0 );
}}
};

void RegisterMFGD3DTest(std::vector<TestPair>& testCases)
{
  testCases.insert(testCases.end(), test_cases.begin(), test_cases.end());
}