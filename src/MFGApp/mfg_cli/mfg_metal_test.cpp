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
#include "mfg_runtime_metal.hpp"
#include <limits>

using namespace std;
using namespace mfg_ir_util;
using namespace mfg_internal;
using namespace mfg_msl_converter;
using namespace mfg_runtime_metal;

// 1 kernelのMetal Context
using MCtx1K = MetalTestKernelContext;



////////////////////////////////////
// ハードコードしたMSLを実行するテストコード。
// デバッグ時の雛形などに使う。
// 
// ネガポジ反転
/////////////////////////////////////
static const char* negaposi_source = R"(
#include <metal_stdlib>
using namespace metal;
kernel void negaposi_inverse(device const uint32_t* bufA,
                       device uint32_t* result,
                       constant int32_t* iparam,
                       uint2 grid_idx [[thread_position_in_grid]])
{
  int32_t width = iparam[0];
  int32_t yindex = grid_idx.y;
  int32_t x = grid_idx.x;

  int index = yindex*width+x;
  uint32_t val = bufA[index];

  uint32_t A = (0xff) & (val >> 24);
  uint32_t R = (0xff) & (val >> 16);
  uint32_t G = (0xff) & (val >> 8);
  uint32_t B = (0xff) & val;

  R = 255-R;
  G = 255-G;
  B = 255-B;

  result[index] = (A << 24) | (R<<16) | (G<<8) | B;
}
)";

static const char* localbuf_test_source = R"(
#include <metal_stdlib>
using namespace metal;
kernel void localbuf_test(device const uint32_t* bufA,
                       device uint32_t* result,
                       constant int32_t* iparam,
                       uint2 grid_idx [[thread_position_in_grid]])
{
  int32_t yindex = grid_idx.y;
  int32_t x = grid_idx.x;
  int32_t width = iparam[0];
  float buf2[1024][1024][100*1024];
  float buf[3][256];
  for (int i = 0; i < 256; i++)
  {
    buf[0][i] = (float)i;
    buf[1][i] = (float)(1000+i);
    buf[2][i] = (float)(10000+i);
    buf2[1023][1023][999*1023+i*4] = (float)i;
  }

  int index = yindex*width+x;
  uint32_t val = bufA[index];

  uint32_t A = (0xff) & (val >> 24);
  uint32_t R = (0xff) & (val >> 16);
  uint32_t G = (0xff) & (val >> 8);
  uint32_t B = (0xff) & val;

  // R = 255-(uint8_t)buf[0][R];
  R = 255-(uint8_t)buf2[1023][1023][999*1023+yindex];
  G = 255-(uint8_t)(256*buf[1][G]/1255);
  B = 255-(uint8_t)(256*buf[2][B]/10255);

  result[index] = (A << 24) | (R<<16) | (G<<8) | B;
}
)";



/*
  モザイクフィルタ。scriptのBackendに生成させてみたカーネル
*/
static const char* mosaic_source = R"(
#include <metal_stdlib>
using namespace metal;
kernel void _avg(
  device int32_t* _avg_out_0,
  device int32_t* _avg_out_1,
  device int32_t* _avg_out_2,
  device int32_t* _avg_out_3,
  device const uint32_t* _input_u8,
  constant int32_t* param32_in,
  uint2 grid_idx [[thread_position_in_grid]])
{
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable"
  int32_t _x = (int32_t)grid_idx.x;
  int32_t _y = (int32_t)grid_idx.y;
  int32_t _avg_out_extent_0 = param32_in[0];
  int32_t _avg_out_extent_1 = param32_in[1];
  int32_t _MOSAIC_WIDTH = param32_in[2];
  int32_t _extent_input_u8_0 = param32_in[3];
  int32_t _extent_input_u8_1 = param32_in[4];
#pragma clang diagnostic pop
  int32_t _r0_min_0 = 0;
  int32_t _extent_r0_0 = _MOSAIC_WIDTH;
  int32_t _r0_min_1 = 0;
  int32_t _extent_r0_1 = _MOSAIC_WIDTH;
  int32_t _r0_sum_0 = 0;
  int32_t _r0_sum_1 = 0;
  int32_t _r0_sum_2 = 0;
  int32_t _r0_sum_3 = 0;
  for(int32_t _ry = _r0_min_1; _ry < _r0_min_1+_extent_r0_1; _ry++ )
  {
  for(int32_t _rx = _r0_min_0; _rx < _r0_min_0+_extent_r0_0; _rx++ )
  {
    uint32_t _t0 = _input_u8[max(min(((_MOSAIC_WIDTH*_y)+_ry), (_extent_input_u8_1-1)), 0)*_extent_input_u8_0+max(min(((_MOSAIC_WIDTH*_x)+_rx), (_extent_input_u8_0-1)), 0)];
    int32_t _a = ((int32_t)(255&(_t0>>24)));
    int32_t _r = ((int32_t)(255&(_t0>>16)));
    int32_t _g = ((int32_t)(255&(_t0>>8)));
    int32_t _b = ((int32_t)(255&_t0));
    _r0_sum_0 +=_a;
    _r0_sum_1 +=(_a*_r);
    _r0_sum_2 +=(_a*_g);
    _r0_sum_3 +=(_a*_b);
  }
  }
    _avg_out_0[((_y)*_avg_out_extent_0)+_x] = _r0_sum_0;
    _avg_out_1[((_y)*_avg_out_extent_0)+_x] = _r0_sum_1;
    _avg_out_2[((_y)*_avg_out_extent_0)+_x] = _r0_sum_2;
    _avg_out_3[((_y)*_avg_out_extent_0)+_x] = _r0_sum_3;
}
kernel void _result(
  device uint32_t* _result_out_0,
  device const int32_t* _avg_0,
  device const int32_t* _avg_1,
  device const int32_t* _avg_2,
  device const int32_t* _avg_3,
  constant int32_t* param32_in,
  uint2 grid_idx [[thread_position_in_grid]])
{
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable"
  int32_t _x2 = (int32_t)grid_idx.x;
  int32_t _y2 = (int32_t)grid_idx.y;
  int32_t _result_out_extent_0 = param32_in[0];
  int32_t _result_out_extent_1 = param32_in[1];
  int32_t _MOSAIC_WIDTH = param32_in[2];
  int32_t _avg_extent_0 = param32_in[3];
  int32_t _avg_extent_1 = param32_in[4];
#pragma clang diagnostic pop
    int32_t _a2 = _avg_0[((_y2/_MOSAIC_WIDTH)*_avg_extent_0)+(_x2/_MOSAIC_WIDTH)];
    int32_t _r2 = _avg_1[((_y2/_MOSAIC_WIDTH)*_avg_extent_0)+(_x2/_MOSAIC_WIDTH)];
    int32_t _g2 = _avg_2[((_y2/_MOSAIC_WIDTH)*_avg_extent_0)+(_x2/_MOSAIC_WIDTH)];
    int32_t _b2 = _avg_3[((_y2/_MOSAIC_WIDTH)*_avg_extent_0)+(_x2/_MOSAIC_WIDTH)];
    _result_out_0[((_y2)*_result_out_extent_0)+_x2] = ((_a2==0) ? 0 : (((255&(_a2/(_MOSAIC_WIDTH*_MOSAIC_WIDTH)))*16777216)|(((255&(_r2/_a2))*65536)|(((255&(_g2/_a2))*256)|(255&(_b2/_a2))))));
}
)";

static size_t IntBufferSize( mfg_pal::Image32& tile )
{
  return tile.Width() * tile.Height();
}

static void CopyFromTile( MetalBuffer& dst, mfg_pal::Image32 &src )
{
  auto guard = PixelLocker( &src );
  uint32_t* dest = dst.Contents<uint32_t>();

  for (int y = 0; y < src.Height(); y++)
  {
    int rowOrg = y*src.Width();
    for (int x = 0; x < src.Width(); x++)
    {
      dest[rowOrg+x] = src.PixelGet( x, y ).Value;
    }
  }
}

static void CopyToTile( mfg_pal::Image32& dest, MetalBuffer& srcBuf )
{
    uint32_t* src = srcBuf.Contents<uint32_t>();

    auto guard = PixelLocker( &dest );
    for (int y = 0; y < dest.Height(); y++)
    {
      int rowOrg = y*dest.Width();
      for (int x = 0; x < dest.Width(); x++)
      {
        TBpp32 color;
        color.Value = src[rowOrg+x];      
        dest.PixelSet( x, y, color );
      }
    } 

}

void MetalTestOneKernelFilter( const char* src, const char* kname, mfg_pal::Image32& srcTile, mfg_pal::Image32& destTile )
{
  AutoreleasePool pool;

  // setup env
  mfg_objc::NSObject device = MTLCreateSystemDefaultDevice();
  MetalLibrary lib( device, src );
  auto kernel = lib.CreateKernel( kname );
  MetalDispatcher dispatcher( device );
  auto command = dispatcher.CreateExecCommand( kernel, destTile.Width(), destTile.Height() );

  // バッファのセットアップ
  size_t byteBufSize = 4*srcTile.Width() * srcTile.Height();
  MetalBuffer bufferIn( device, byteBufSize );
  MetalBuffer bufferOut( device, byteBufSize );
  CopyFromTile( bufferIn, srcTile );

  std::vector<int32_t> iparam { srcTile.Width(), srcTile.Height() };
  command.ArgsSetup()
    .AddBuffer( bufferIn )
    .AddBuffer( bufferOut )
    .AddInts( iparam );

  command.RunBlock();

  CopyToTile( destTile, bufferOut );
}

static void NegaPosiInvertFunc( mfg_pal::Image32& srcTile, mfg_pal::Image32& destTile )
{
  MetalTestOneKernelFilter( negaposi_source, "negaposi_inverse", srcTile, destTile );
}

static void MosaicFunc( mfg_pal::Image32& inputTile, mfg_pal::Image32& destTile )
{
  const int MOSAIC_WIDTH = 16;
  AutoreleasePool pool;

  // setup env
  NSObject device = MTLCreateSystemDefaultDevice();
  MetalLibrary lib( device, mosaic_source );
  MetalDispatcher dispatcher( device );

  auto kernel = lib.CreateKernel( "_avg" );
  auto avgWidth = (inputTile.Width()-1)/MOSAIC_WIDTH+1;
  auto avgHeight =  (inputTile.Height()-1)/MOSAIC_WIDTH+1;


  auto command1 = dispatcher.CreateExecCommand( kernel, avgWidth, avgHeight );

  size_t avgSize = 4*avgWidth*avgHeight;
  MetalBuffer avgBuf0( device, avgSize );
  MetalBuffer avgBuf1( device, avgSize );
  MetalBuffer avgBuf2( device, avgSize );
  MetalBuffer avgBuf3( device, avgSize );

  // バッファのセットアップ
  size_t byteBufSize = 4*inputTile.Width() * inputTile.Height();
  MetalBuffer bufferIn( device, byteBufSize );
  MetalBuffer bufferOut( device, byteBufSize );
  CopyFromTile( bufferIn, inputTile );

  command1.ArgsSetup()
    .AddBuffer( avgBuf0 )
    .AddBuffer( avgBuf1 )
    .AddBuffer( avgBuf2 )
    .AddBuffer( avgBuf3 )
    .AddBuffer( bufferIn )
    .AddInts( { avgWidth, avgHeight, MOSAIC_WIDTH, inputTile.Width(), inputTile.Height()} );

  command1.RunBlock();

  auto kernel2 = lib.CreateKernel( "_result" );


  auto command2 = dispatcher.CreateExecCommand( kernel2, inputTile.Width(), inputTile.Height() );

  command2.ArgsSetup()
    .AddBuffer( bufferOut )
    .AddBuffer( avgBuf0 )
    .AddBuffer( avgBuf1 )
    .AddBuffer( avgBuf2 )
    .AddBuffer( avgBuf3 )
    .AddInts( { inputTile.Width(), inputTile.Height(), MOSAIC_WIDTH, avgWidth, avgHeight } );

  command2.RunBlock();
  CopyToTile( destTile, bufferOut );


}

extern void SetupNegaPosi( mfg_pal::Image32& inputTile, mfg_pal::Image32& resTile );
extern void AssertNegaPosi( mfg_pal::Image32& resTile );

extern bool LoadPngAsTile( const string& path, mfg_pal::Image32& dest );
extern bool SaveTileAsPng( const string& path, mfg_pal::Image32& tile );


static std::vector<TestPair> test_cases = {
{"Metalのローカルバッファのテスト", []{
  mfg_pal::Image32 inputTile;
  mfg_pal::Image32 resTile;

  REQUIRE( LoadPngAsTile( "test_data/simple_small.png", inputTile ) );
  resTile.Resize( inputTile.Width(), inputTile.Height() );

  MetalTestOneKernelFilter( localbuf_test_source, "localbuf_test", inputTile, resTile );

  // SaveTileAsPng( "tmp_test_localbuf.png", resTile );

}},
{"Metalのネガポジ反転", []{
  mfg_pal::Image32 inputTile;
  mfg_pal::Image32 resTile;

  SetupNegaPosi( inputTile, resTile );

  NegaPosiInvertFunc( inputTile, resTile );

  auto guard_r = PixelLocker( &resTile );
  AssertNegaPosi( resTile );
}},
{"Metalのモザイク、ハードコード版", []{
  mfg_pal::Image32 inputTile;
  mfg_pal::Image32 resTile;

  REQUIRE( LoadPngAsTile( "test_data/simple_small.png", inputTile ) );
  resTile.Resize( inputTile.Width(), inputTile.Height() );

  MosaicFunc( inputTile, resTile );

  auto guard_r = PixelLocker( &resTile );
  // 255 10 10 44
  REQUIRE( 0x2cff0a0a == resTile.PixelGet( 150, 100 ).Value );
  // 181 10 255 33
  REQUIRE( 0x21b50aff == resTile.PixelGet( 38, 126 ).Value );
}},
{"metalバックエンドのVer 0.1テスト", []{
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

  // ここはVer 0.1と関係ないけどついでに。
  if (SECTION("NameConverterのテスト")) {SG g;
    NameConverter nconv;

    REQUIRE( "_f1" == nconv.Ident( "f1" ) );
    REQUIRE( "_f1_out_extent_0" == nconv.Ident( "f1.out.extent.0" ) );
    REQUIRE( "int32_t" == nconv.Type( Int(32) ) );
    REQUIRE( "uint32_t" == nconv.Type( UInt(32) ) );
    REQUIRE( "float" == nconv.Type( Float(32) ) );
  }
  if (SECTION("TensorNameConverterのテスト")) {SG g;
    TensorNameConverter conv( ts );

    REQUIRE( "_k_f1" == conv.Kernel() );
    REQUIRE( "_b_f1_out_1" == conv.OutputBuf( 1 ) );
    REQUIRE( "_extent_f1_0" == conv.OutputBufExtent( 0 ) );
    REQUIRE( "_y" == conv.InitLoopVarName( 1 ) );
  }

  if (SECTION("MSLTensorGeneratorのテスト")) {SG g;
    MSLTensorGenerator tgen( ts );
    SeedGenerator sgen( 123 );

    if (SECTION("カーネルの宣言が正しく生成されるか？")) {SG g;
      tgen.GenSignature();
      auto actual = tgen.ToString();
      // cout << actual << endl << endl;

      size_t pos = 0;
      CheckContains( actual, "kernel void _k_f1(", pos );
      CheckContains( actual, "device int32_t* _b_f1_out_0,", pos );
      CheckContains( actual, "constant int32_t* param32_in,", pos );
      CheckContains( actual, "uint2 grid_idx [[thread_position_in_grid]])", pos );
    }
    
    if (SECTION("カーネルのトップレベルのfor文までが正しく生成されるか？")) {SG g;
      tgen.GenFuncPrologue( sgen );
      auto actual = tgen.ToString();
      // cout << actual << endl << endl;

      size_t pos = 0;
      CheckContains( actual, "{", pos );      
      CheckContains( actual, "int32_t _x = (int32_t)grid_idx.x;", pos );
      CheckContains( actual, "int32_t _y = (int32_t)grid_idx.y;", pos );
      CheckContains( actual, "int32_t _extent_f1_0 = param32_in[0];", pos );
      CheckContains( actual, "int32_t _extent_f1_1 = param32_in[1];", pos );
      CheckNotContains( actual, "{", pos );

      // 乱数を使ってないので生成されないはず。
      CheckNotContains( actual, "RandomGenerator" );
    }

    if (SECTION("InitDefのbody部が正しく生成されるか？")) {SG g;
      tgen.GenInitDefBody();
      auto actual = tgen.ToString();
      // cout << actual << endl << endl;

      CheckContains( actual, "_b_f1_out_0[((_y*_extent_f1_0)+_x)] = (_x+_y);" );
    }

    if (SECTION("Metalでの実行")) {SG g;
      tgen.GenLibrarySource( sgen );
      // cout << tgen.ToString() << endl << endl;

      MCtx1K mctx( tgen, 2, 3 );
      id device = mctx._device;

      MetalBuffer bufferOut( device, sizeof(int32_t)*2*3 );
      std::vector<int32_t> iparam { 2, 3 };
      mctx.ArgsSetup()
        .AddBuffer( bufferOut )
        .AddInts( iparam );


      mctx.RunBlock();

      auto res = bufferOut.Accessor( {2, 3} );

      REQUIRE( res.Int32( 0, 0 ) == 0 );
      REQUIRE( res.Int32( 1, 0 ) == 1 );
      REQUIRE( res.Int32( 0, 1 ) == 1 );
      REQUIRE( res.Int32( 1, 1 ) == 2 );
      REQUIRE( res.Int32( 0, 2 ) == 2 );
      REQUIRE( res.Int32( 1, 2 ) == 3 );
    }
     
  }

}},
{"mfgでリークが無いかの確認。", []{
  REQUIRE( g_node_alloc_count == 0 );
  REQUIRE( g_irelem_alloc_count == 0 );
}}
};

void RegisterMFGMetalTest(std::vector<TestPair>& testCases)
{
  testCases.insert(testCases.end(), test_cases.begin(), test_cases.end());
}