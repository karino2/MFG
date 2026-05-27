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
#define REQUIRE_FEQUALS( expr1, expr2 ) if( std::abs((expr1) - (expr2)) > 0.001 )  throw nfiftest::assert_fail_error(__FILE__, __LINE__, std::string(": expr1(") + #expr1 + std::string("= ") + std::to_string((expr1)) + std::string(", expr2(") + (#expr2) + ")=" + std::to_string((expr2)))

#include <iostream>
#include <fstream>
#include "forest.hpp"
#include "mfg.hpp"
#include "mfg_samples.h"
#include <mfg_pal/pal.hpp>

#ifdef MFG_BACKEND_METAL
#include "mfg_runtime_metal.hpp"
using namespace mfg_runtime_metal;
using TargetBackend = MetalBackend;
#endif

#ifdef MFG_BACKEND_D3D
#include "mfg_runtime_d3d.hpp"
using namespace mfg_runtime_d3d;
using TargetBackend = D3DBackend;

using namespace mfg_hlsl_converter;
#endif

#if defined(MFG_BACKEND_METAL) || defined(MFG_BACKEND_D3D)
#define MFG_BACKEND_ENABLED

template<typename TILE>
void RunBackend( TargetBackend& backend, mfg_binary::IRBinary& target, TILE& inputTile, TILE& resTile )
{
  InputTiles tiles;
  tiles.Insert( 0, &inputTile );
  TileReference resRef( &resTile );

  backend.Run( target, tiles, resRef );
}

template<typename TILE>
void RunFacade( mfg::MFGFacade<TargetBackend>& facade, mfg::MFGBinary& target, TILE& inputTile, TILE& resTile )
{
  InputTiles tiles;
  tiles.Insert( 0, &inputTile );
  TileReference resRef( &resTile );

  facade.Run( target, tiles, resRef );
}


#endif

using namespace std;
using namespace mfg_ir_util;
using namespace mfg_internal;
using namespace mfg_parser;

void PixelSetInt( mfg_pal::Image32& inputTile, int x, int y, int val )
{
  TBpp32 color;
  color.Value = (uint32_t)val;
  inputTile.PixelSet( x, y, color );
}

bool LoadPngAsTile( const string& path, mfg_pal::Image32& dest )
{
  auto vec = ReadContent( path );
  return mfg_pal::DecodePng32( vec, dest );
}

struct TestFolderLoader: ResourceLoader
{
  bool LoadImage( const std::string& name, mfg_pal::Image32& dest ) override
  {
    return LoadPngAsTile( "test_data/" + name, dest );
  }
};

bool SaveTileAsPng( const string& path, mfg_pal::Image32& image )
{
  return mfg_pal::SavePng32( image, path );
}


void SetupNegaPosi( mfg_pal::Image32& inputTile, mfg_pal::Image32& resTile )
{
  inputTile.Resize( 3, 2 );
  resTile.Resize( 3, 2 );
  auto guard_i = PixelLocker( &inputTile );

  TBpp32 color;

  color.A = 0xff;
  color.R = 0xff;
  color.G = 0;
  color.B = 0;
  inputTile.PixelSet( 0, 0, color );

  color.R = 0;
  color.G = 0xff;
  inputTile.PixelSet( 1, 0, color );

  color.G = 0;
  color.B = 0xff;
  inputTile.PixelSet( 2, 0, color );
  
  color.R = 0xee;
  color.B = 0x00;
  inputTile.PixelSet( 0, 1, color );

  color.R = 0;
  color.G = 0xee;
  inputTile.PixelSet( 1, 1, color );

  color.G = 0;
  color.B = 0xee;
  inputTile.PixelSet( 2, 1, color );
}

void AssertNegaPosi( mfg_pal::Image32& resTile )
{
  REQUIRE( 0xff00ffff == resTile.PixelGet( 0, 0 ).Value );
  REQUIRE( 0xffff00ff == resTile.PixelGet( 1, 0 ).Value );
  REQUIRE( 0xffffff00 == resTile.PixelGet( 2, 0 ).Value );
  REQUIRE( 0xff11ffff == resTile.PixelGet( 0, 1 ).Value );
  REQUIRE( 0xffff11ff == resTile.PixelGet( 1, 1 ).Value );
  REQUIRE( 0xffffff11 == resTile.PixelGet( 2, 1 ).Value );
}

void SetupNegaPosi64( mfg_pal::Image64& inputTile, mfg_pal::Image64& resTile )
{
  inputTile.Resize( 3, 2 );
  resTile.Resize( 3, 2 );
  auto guard_i = PixelLocker( &inputTile );

  TBpp64 color;

  color.A = 0xffff;
  color.R = 0xffff;
  color.G = 0;
  color.B = 0;
  inputTile.PixelSet( 0, 0, color );

  color.R = 0;
  color.G = 0xffff;
  inputTile.PixelSet( 1, 0, color );

  color.G = 0;
  color.B = 0xffff;
  inputTile.PixelSet( 2, 0, color );
  
  color.R = 0x1234;
  color.B = 0x0000;
  inputTile.PixelSet( 0, 1, color );

  color.R = 0;
  color.G = 0x1234;
  inputTile.PixelSet( 1, 1, color );

  color.G = 0;
  color.B = 0x1234;
  inputTile.PixelSet( 2, 1, color );
}

void AssertNegaPosi64( mfg_pal::Image64& resTile )
{
  REQUIRE( 0xffff0000ffffffff == resTile.PixelGet( 0, 0 ).Value );
  REQUIRE( 0xffffffff0000ffff == resTile.PixelGet( 1, 0 ).Value );
  REQUIRE( 0xffffffffffff0000 == resTile.PixelGet( 2, 0 ).Value );
  REQUIRE( 0xffffedcbffffffff == resTile.PixelGet( 0, 1 ).Value );
  REQUIRE( 0xffffffffedcbffff == resTile.PixelGet( 1, 1 ).Value );
  REQUIRE( 0xffffffffffffedcb == resTile.PixelGet( 2, 1 ).Value );
}

void SetupAddFilter0( mfg_pal::Image32& inputTile, mfg_pal::Image32& resTile )
{
  inputTile.Resize( 3, 2 );
  resTile.Resize( 3, 2 );
  auto guard_i = PixelLocker( &inputTile );

  TBpp32 color;

  color.A = 0xff;
  color.R = 0x0;
  color.G = 0;
  color.B = 0;
  inputTile.PixelSet( 0, 0, color );

  color.R = 1;
  inputTile.PixelSet( 1, 0, color );

  color.R = 2;
  inputTile.PixelSet( 2, 0, color );
  
  color.R = 3;
  inputTile.PixelSet( 0, 1, color );

  color.R = 4;
  inputTile.PixelSet( 1, 1, color );

  color.R = 5;
  inputTile.PixelSet( 2, 1, color );
}

void SetupAddFilter1( mfg_pal::Image32& inputTile )
{
  inputTile.Resize( 3, 2 );
  auto guard_i = PixelLocker( &inputTile );

  TBpp32 color;

  color.A = 0xff;
  color.R = 0x10;
  color.G = 0;
  color.B = 0;
  inputTile.PixelSet( 0, 0, color );

  color.R = 0x20;
  inputTile.PixelSet( 1, 0, color );

  color.R = 0x30;
  inputTile.PixelSet( 2, 0, color );
  
  color.R = 0x40;
  inputTile.PixelSet( 0, 1, color );

  color.R = 0x50;
  inputTile.PixelSet( 1, 1, color );

  color.R = 0x60;
  inputTile.PixelSet( 2, 1, color );
}

void AssertAddFilter( mfg_pal::Image32& resTile )
{
  auto guard_i = PixelLocker( &resTile );
  REQUIRE( 0xff100000 == resTile.PixelGet( 0, 0 ).Value );
  REQUIRE( 0xff210000 == resTile.PixelGet( 1, 0 ).Value );
  REQUIRE( 0xff320000 == resTile.PixelGet( 2, 0 ).Value );
  REQUIRE( 0xff430000 == resTile.PixelGet( 0, 1 ).Value );
  REQUIRE( 0xff540000 == resTile.PixelGet( 1, 1 ).Value );
  REQUIRE( 0xff650000 == resTile.PixelGet( 2, 1 ).Value );
}

// -1のレイヤーが存在しない時に0になるかのテスト
void AssertAddFilterOneLayer( mfg_pal::Image32& resTile )
{
  auto guard_i = PixelLocker( &resTile );
  REQUIRE( 0xff000000 == resTile.PixelGet( 0, 0 ).Value );
  REQUIRE( 0xff010000 == resTile.PixelGet( 1, 0 ).Value );
  REQUIRE( 0xff020000 == resTile.PixelGet( 2, 0 ).Value );
  REQUIRE( 0xff030000 == resTile.PixelGet( 0, 1 ).Value );
  REQUIRE( 0xff040000 == resTile.PixelGet( 1, 1 ).Value );
  REQUIRE( 0xff050000 == resTile.PixelGet( 2, 1 ).Value );
}


void CountSaltPepper( mfg_pal::Image32& resTile, size_t& whiteCnt, size_t& blackCnt, size_t& otherCnt )
{
  auto guard_r = PixelLocker( &resTile );

  whiteCnt = 0;
  blackCnt = 0;
  otherCnt = 0;

  for( auto y : NRange( resTile.Height() ))
  {
    for( auto x : NRange( resTile.Width() ))
    {
      auto col = resTile.PixelGet( x, y );
      if (col.Value == 0xff000000)
      {
        blackCnt++;
      }
      else if (col.Value == 0xffffffff)
      {
        whiteCnt++;
      }
      else
      {
        otherCnt++;
      }
    }
  }
}

/*
  Rが
  hist = [0, 3, 1, 1, 0, 1]
  になる感じのセットアップ。
*/
static void SetupHist( mfg_pal::Image32& inputTile, mfg_pal::Image32& resTile )
{
  inputTile.Resize( 3, 2 );
  resTile.Resize( 3, 2 );
  {
    auto guard_i = PixelLocker( &inputTile );

    TBpp32 color;
    color.A = 0xff;
    color.G = 0;
    color.B = 0;

    color.R = 1;
    inputTile.PixelSet( 0, 0, color );

    color.R = 1;
    inputTile.PixelSet( 1, 0, color );

    color.R = 2;
    inputTile.PixelSet( 2, 0, color );

    color.R = 3;
    inputTile.PixelSet( 0, 1, color );

    color.R = 1;
    inputTile.PixelSet( 1, 1, color );

    color.R = 5;
    inputTile.PixelSet( 2, 1, color );
  }
}

// Ver04のassert。metalとVMの両方で使うので関数にする。
template<typename T>
void AssertVer04( T& buf )
{
  int tmpBuf[5][5];
  int expect[4][4];
  for (auto y : NRange(5) )
  {
    for (auto x : NRange(5) )
    {
      tmpBuf[x][y] = x+y;
    }
  }

  for (auto y : NRange(4) )
  {
    for (auto x : NRange(4) )
    {
      expect[x][y] = (tmpBuf[x][y] +
                  tmpBuf[x][y + 1] +
                  tmpBuf[x + 1][y] +
                  tmpBuf[x + 1][y + 1]) / 4;
    }
  }

  for (auto y : NRange(4) )
  {
    for (auto x : NRange(4) )
    {
      REQUIRE( expect[x][y] == buf.Int32( x, y ) );
    }
  }
}

// mfg_pal::Image32をBufferと同じ口で見せる
// mfg_cliではハンドルは単なるメモリなのでほぼ何もしない。
struct ImageTileAccessor
{
  mfg_pal::Image32& _tile;
  ImageTileAccessor( mfg_pal::Image32& tile ) : _tile( tile ) {}
  int32_t Int32( int x, int y )
  {
    return (int32_t) _tile.PixelGet( x, y ).Value;
  }
};


#define ASSERT_CURRENT_TOKEN( tokenizer, ttype, offset, len ) { auto &token = (tokenizer)._current; \
REQUIRE( token._ttype == (ttype)); \
REQUIRE( token._offset == (offset) ); \
REQUIRE( token.Length() == (len) ); \
}

static void VerifyFirstToken( std::string src, mfg_parser::TokenType ttype, size_t len )
{
  Tokenizer tokenizer( src );
  tokenizer.Scan();

  try
  {
    ASSERT_CURRENT_TOKEN( tokenizer, ttype, 0, len );  
  }
  catch(const nfiftest::assert_fail_error& e)
  {
    throw nfiftest::assert_fail_error(__FILE__, __LINE__, "\""+ src + "\" : " + e.what() );
  }  
}

// parser用 test bed
struct TestPBed
{
  TreeBuilder _builder;
  Parser _parser;

  TestPBed( const char* src, bool withScan = true ) : _parser( _builder, src ) {
    if (withScan)
      _parser._tokenizer.Scan();
  }

  TestPBed( const std::string& src, bool withScan = true ) : TestPBed( src.c_str(), withScan ) {}

  void DefineVariable( const Token& varName, ExprV&& vari )
  {
    _builder.DefineVariable( varName, std::move(vari) );
  }

  void ParseAll()
  {
    _parser.ParseAll();
  }

  void BuildIR()
  {
    _parser.ParseAll();
    _builder._binary->LowerAndAnalyze();
  }

  TLTensor& Result()
  {
    return _builder._binary->Result();
  }

  IRBinary& IRBinary()
  {
    return *_builder._binary.get();
  }

  SeedGenerator& SeedGen() { return IRBinary().GetSeedGen(); }

  std::vector<std::unique_ptr<TLTensor>>& Tensors()
  {
    return _builder._binary->_tensors;
  }

  BaseExprElem* GetReturnExpr( TLTensor& ts )
  {
    return ts.GetBody()->GetReturnExpr();
  }

  // resultテンソルの最後のreturn expr。
  // テストではParseAllして最後のExprを返す、が良くあるので。
  BaseExprElem* GetReturnExpr()
  {
    return GetReturnExpr( _builder._binary->Result() );
  }

  std::string PrintResultTree() const
  {
    return PrintExprS( _builder._binary->Result().GetBody() );
  }
};

static void BuildIR( TreeBuilder& builder, Parser& parser )
{
  parser.ParseAll();
  builder._binary->LowerAndAnalyze();
}

static std::string ParseAndReturnTreeDump( const std::string& src )
{
  ResetUniqueName();

  TreeBuilder builder;
  Parser parser( builder, src );

  parser.ParseAll();

  return PrintExprS( builder._binary->Result().GetBody() );
}

static std::string ParseAndLowerAndReturnTreeDump( const std::string& src )
{
  ResetUniqueName();

  TreeBuilder builder;
  Parser parser( builder, src );

  BuildIR( builder, parser );

  return PrintExprS( builder._binary->Result().GetBody() );
}

/*
  先頭の型をパースしてTypeを返す。
  ようするにsrcは "f32" とか "f32v2" とかで始まる。
*/
static Type ParseType( const std::string src )
{
  TestPBed pbed( src );

  auto typeId = pbed._parser._tokenizer._current;
  return pbed._parser.TypeTkToType( typeId );
}

static OneInputInfo& GetFirstInput( InputsInfo& ii ) { return ii._inputMap.begin()->second; }

#ifdef MFG_BACKEND_METAL
// Resultテンソルのカーネルをダンプして返す。
static std::string GenMSLResultDump( const std::string& src )
{
  ResetUniqueName();
  TestPBed pbed( src );
  pbed.BuildIR();

  mfg_msl_converter::MSLTensorGenerator tgen( pbed.Result() );
  tgen.GenKernel( pbed.SeedGen() );
  return tgen.ToString();
}

#endif
#ifdef MFG_BACKEND_D3D
// Resultテンソルのカーネルをダンプして返す。
static std::string GenD3DResultDump( const std::string& src )
{
  ResetUniqueName();
  TestPBed pbed( src );
  pbed.BuildIR();

  mfg_hlsl_converter::HLSLTensorGenerator tgen( pbed.Result() );
  tgen.GenAll( pbed.SeedGen() );
  return tgen.ToString();
}

#endif


#ifdef MFG_BACKEND_ENABLED
static void ExecVerifyScript( const std::string& src, int width, int height, std::function<void(mfg_pal::Image32&)> verify )
{
  ResetUniqueName();
  TreeBuilder builder;
  builder.SetLoader( std::unique_ptr<ResourceLoader>( new TestFolderLoader() ) );

  Parser parser( builder, src );

  BuildIR( builder, parser );

  mfg_pal::Image32 inputTile;
  mfg_pal::Image32 resTile;

  inputTile.Resize( width, height );
  resTile.Resize( width, height );
  TargetBackend backend;

  RunBackend( backend, *builder._binary.get(), inputTile, resTile );

  auto guard_r = PixelLocker( &resTile );
  verify( resTile );
}


static ParseError VerifyParseError( const std::string& src )
{
  ParseError ret("dummy", Location( 0, 0 ) );
  bool thrown = false;
  try {
    ExecVerifyScript( src, 1, 1, [](mfg_pal::Image32& resTile) {   
      REQUIRE(false); // ここには来ない
    });
  }
  catch( const ParseError& err )
  {
    ret = err;
    thrown = true;
  }
  REQUIRE( thrown );
  return ret;
}
#endif

static std::vector<TestPair> test_cases = {
{"TStringのテスト", []{
  TString str("hoge");
  REQUIRE( str._len == 4 );
  REQUIRE( str == TString("hoge", 4));
  REQUIRE( !(str == TString("hoge", 3)) );
  REQUIRE( str.IsCharAt( 3, 'e' ) );
  REQUIRE( !str.IsCharAt( 3, 'b' ) );
  REQUIRE( !str.IsCharAt( 4, 'e' ) );
  REQUIRE( str.IsCharAt( 0, 'h' ) );
  REQUIRE( str.IsStrAt( 1, "og", 2 ) );
  REQUIRE( !str.IsStrAt( 1, "go", 2 ) );
  REQUIRE( !str.IsStrAt( 1, "ogea", 4 ) );
  REQUIRE( str.IsStrAt( 0, "hog", 3 ) );
  REQUIRE( str.IsStrAt( 2, "ge", 2 ) );
}},
{"Tokenizerの最初のtokenのテスト", []{
  VerifyFirstToken("@param_i32 WIDTH", TokenType::ATMARK, 1 );
  VerifyFirstToken("def ts |x, y|", TokenType::DEF, 3 );
  VerifyFirstToken("let x =", TokenType::LET, 3 );
  VerifyFirstToken(">> 3", TokenType::GTGT, 2 );
  VerifyFirstToken(">= 3", TokenType::GTEQ, 2 );
  VerifyFirstToken("> 3", TokenType::GT, 1 );
  VerifyFirstToken("by |rx,", TokenType::BY, 2 );
  VerifyFirstToken("deffer |x, y|", TokenType::IDENTIFIER, 6 ); // defから始まっているが続きがある場合
  VerifyFirstToken("_hist |x, y|", TokenType::IDENTIFIER, 5 );
  VerifyFirstToken("byt = ", TokenType::IDENTIFIER, 3 );
  VerifyFirstToken("_, y|", TokenType::BLANK_ID, 1 );
  VerifyFirstToken("=3+4", TokenType::EQUAL, 1 );
  VerifyFirstToken("==3+4", TokenType::EQEQ, 2 );
  VerifyFirstToken("255,", TokenType::INTEGER, 3 );
  VerifyFirstToken("255u+", TokenType::UINTEGER, 4 );
  VerifyFirstToken("0u", TokenType::UINTEGER, 2 );
  VerifyFirstToken("0+", TokenType::INTEGER, 1 );
  VerifyFirstToken("12.3\n", TokenType::FLOAT, 4 );
  VerifyFirstToken("0x12AF\n", TokenType::HEX, 6 );
  VerifyFirstToken("0x12Au\n", TokenType::UHEX, 6 );
  VerifyFirstToken("|x, y|", TokenType::BAR, 1 );
  VerifyFirstToken("|| 3 == ", TokenType::BAR2, 2 );
  VerifyFirstToken("&& 3 == ", TokenType::AMPERSAND2, 2 );
  VerifyFirstToken("&255", TokenType::AMPERSAND, 1 );
  VerifyFirstToken("%255", TokenType::PERCENT, 1 );
  VerifyFirstToken("! clamped =", TokenType::BANG, 1 );
  VerifyFirstToken("!= -1", TokenType::NEQ, 2 );
  VerifyFirstToken(".set_bound", TokenType::DOT, 1 );
  VerifyFirstToken("... )", TokenType::DOT3, 3 );
  VerifyFirstToken("..< ", TokenType::HALF_OPEN_RANGE, 3 );
  VerifyFirstToken("0..< ", TokenType::INTEGER, 1 ); // Floatと間違えないように。
  VerifyFirstToken("1..< ", TokenType::INTEGER, 1 ); // Floatと間違えないように。
  VerifyFirstToken("^3", TokenType::HAT, 1 );
  VerifyFirstToken("<3", TokenType::LT, 1 );
  VerifyFirstToken("<=3", TokenType::LTEQ, 2 );
  VerifyFirstToken("<< 3", TokenType::LTLT, 2 );
  VerifyFirstToken("\"hoge\")", TokenType::STRING, 6 );
  VerifyFirstToken("\"ho\\\"ge\")", TokenType::STRING, 8 );
  VerifyFirstToken("mut! ts(x, y) += 3", TokenType::MUT, 3 );
  VerifyFirstToken("$abc", TokenType::DOLLAR, 1 );
}},
{"TokenのToInt等のテスト", []{
  if (SECTION("intの即値")) {SG g;
    Token target( "255", TokenType::INTEGER, 0, 3 );
    REQUIRE( 255 == target.ToInt() );
  }
  if (SECTION("hexの即値")) {SG g;
    Token target( "0xff000000", TokenType::HEX, 0, 10 );
    REQUIRE( 0xff000000 == target.ToHex() );
  }
  if (SECTION("uhexの即値")) {SG g;
    Token target( "0xff000000u", TokenType::UHEX, 0, 11 );
    REQUIRE( 0xff000000 == target.ToUHex() );
  }
}},
{"TokenのToStringのテスト", []{
  if (SECTION("identifierの場合")) {SG g;
    Token target( "deffer ", TokenType::IDENTIFIER, 0, 6 );
    REQUIRE( "deffer" == target.ToString() );
  }
  if (SECTION("文字列、エスケープ無し")) {SG g;
    Token target( "\"hoge\"", TokenType::STRING, 0, 6 );
    REQUIRE( "hoge" == target.ToString() );
  }
  if (SECTION("文字列、エスケープあり")) {SG g;
    Token target( "\"ho\\\"ge\"", TokenType::STRING, 0, 8 );
    REQUIRE( "ho\"ge" == target.ToString() );
  }
}},
{"TokenizerのNextのテスト", []{
  Tokenizer tokenizer( "def ts |x, y|");

  tokenizer.Scan();
  ASSERT_CURRENT_TOKEN( tokenizer, TokenType::DEF, 0, 3 );

  tokenizer.NextWOE();
  ASSERT_CURRENT_TOKEN( tokenizer, TokenType::IDENTIFIER, 4, 2 );
  REQUIRE( tokenizer._current.IsEqual( "ts", 2) );

  tokenizer.NextWOE();
  ASSERT_CURRENT_TOKEN( tokenizer, TokenType::BAR, 7, 1 );
}},
{"Tokenizerのコメントののテスト", []{
  std::string src = R"(
    # comment
    def ts
)";
  Tokenizer tokenizer( src );

  tokenizer.Scan();
  
  ASSERT_CURRENT_TOKEN( tokenizer, TokenType::END_OF_LINE, 0, 1 );

  tokenizer.NextWOE();
  ASSERT_CURRENT_TOKEN( tokenizer, TokenType::END_OF_LINE, 14, 1 );

  tokenizer.NextWOE();
  ASSERT_CURRENT_TOKEN( tokenizer, TokenType::DEF, 19, 3 );
}},
{"TokenizerのNextで末尾のテスト", []{
  Tokenizer tokenizer( "def");

  tokenizer.Scan();
  tokenizer.NextWOE();
  ASSERT_CURRENT_TOKEN( tokenizer, TokenType::END_OF_SRC, 3, 0 );

  if (SECTION("末尾でNextを呼んでも結果が変わらない")) {SG g;
    tokenizer.NextWOE();
    ASSERT_CURRENT_TOKEN( tokenizer, TokenType::END_OF_SRC, 3, 0 );
  }
}},
{"SwizzleのToIndicesのテスト", []{
  using vec = std::vector<size_t>;
  TestPBed pbed( "dummy" );

  auto toIndices = [&pbed]( const char *str )
  { 
    Token tk(str, TokenType::IDENTIFIER, 0, strlen(str) );
    return pbed._parser.ToSwizzleIndices( tk );
  };

  if (SECTION("基本的な.xyzwのケース")) {SG g;
    REQUIRE( toIndices( "xyzw" ) == vec( { 0, 1, 2, 3 } ) );
  }
  if (SECTION("重複のテスト、yyのケース")) {SG g;
    REQUIRE( toIndices( "yy" ) == vec( { 1, 1 } ) );
  }
}},
{"Parserの一番基本的なParseExprWOEのテスト", []{
  ResetUniqueName();
  std::string ssrc = "input_u8(x, y)";

  auto src = ssrc.c_str();
  TestPBed pbed( src );
  pbed.DefineVariable( Token( src, TokenType::IDENTIFIER, 9, 1), new Variable( Int(32), "x" ) );
  pbed.DefineVariable( Token( src, TokenType::IDENTIFIER, 12, 1), new Variable( Int(32), "y" ) );

  auto actual = pbed._parser.ParseExprWOE();
  REQUIRE( actual.GetElemType() == IRElemType::TensorCall );
  REQUIRE( "input_u8(x, y)\n" == PrintExprS(actual) );
}},
{"引き算と即値のexprのパース", []{
  ResetUniqueName();
  std::string ssrc = "255-r";
  
  auto src = ssrc.c_str();
  TestPBed pbed( src );
  pbed.DefineVariable( Token( src, TokenType::IDENTIFIER, 4, 1), new Variable( Int(32), "r" ) );

  Expr res( pbed._parser.ParseExprWOE() );
  REQUIRE( res.GetElemType() == IRElemType::BinOp && res.As<BinOp>()->_opType == BinOp::Sub );
  CheckContains( PrintExprS( res.GetBase() ), "255 - r" );
}},
{"ネガポジ反転のBodyElemのパースのテスト", []{
  ResetUniqueName();
  std::string ssrc = "{ let [b, g, r, a] = input_u8(x, y)\nu8 [255-b, 255-g, 255-r, a] }";
  
  auto src = ssrc.c_str();
  TestPBed pbed( src );

  pbed.DefineVariable( Token( src, TokenType::IDENTIFIER, 30, 1), new Variable( Int(32), "x" ) );
  pbed.DefineVariable( Token( src, TokenType::IDENTIFIER, 33, 1), new Variable( Int(32), "y" ) );

  auto vblock = pbed._parser.ParseBody();
  auto actual = PrintExprS( vblock );

  // cout << actual << endl;

  size_t pos = 0;
  CheckContains( actual, "vblock {", pos );
  CheckContains( actual, "let [u0_b, u1_g, u2_r, u3_a] = input_u8(x, y)", pos );
  CheckContains( actual, "(u8v4)(tuple(255 - int32(u0_b), 255 - int32(u1_g),", pos );
}},
{"文字列の値のパースのテスト", []{

  auto parseStringFun = [](const std::string& src) {
    ResetUniqueName();
    TestPBed pbed( src );
    return pbed._parser.ParseStringValue();
  };

  if(SECTION("文字列リテラルのケース")) {SG g;
    auto res = parseStringFun("\"hogeika\"");
    REQUIRE( res == "hogeika" );
  }

  if(SECTION("文字列リソースのケース")) {SG g;
    auto res = parseStringFun("$TITLE_NEGAPOSI");
    REQUIRE( res == "Negaposi filter" );
  }

  if(SECTION("文字列リソースのケース、日本語のケース")) {SG g;
    auto guard = ScopeGuard([]{ mfg_resource::SetLanguage( NL_ENGLISH ); });
    mfg_resource::SetLanguage( NL_JAPANESE );

    auto res = parseStringFun("$TITLE_NEGAPOSI");
    REQUIRE( res == "ネガポジ反転フィルター" );
  }
}},
{"@titleのパースのテスト", []{
  ResetUniqueName();
  std::string src = "@title \"ネガポジ反転\"\n";  
  
  TestPBed pbed( src );
  pbed._parser.ParsePureAttribute();

  REQUIRE( "ネガポジ反転" == pbed._builder._binary->_title );
}},
{"@titleのパースのテスト、リソースバージョン", []{
  ResetUniqueName();
  std::string src = "@title $TITLE_NEGAPOSI\n";
  
  if(SECTION("デフォルトのリソース")){SG g;
    TestPBed pbed( src );
    pbed._parser.ParsePureAttribute();

    REQUIRE( "Negaposi filter" == pbed._builder._binary->_title );
  }

  if(SECTION("日本語のケース")) {SG g;
    auto guard = ScopeGuard([]{ mfg_resource::SetLanguage( NL_ENGLISH ); });
    mfg_resource::SetLanguage( NL_JAPANESE );

    TestPBed pbed( src );
    pbed._parser.ParsePureAttribute();

    REQUIRE( "ネガポジ反転フィルター" == pbed._builder._binary->_title );
  }
}},
{"@versionのパースのテスト", []{
  ResetUniqueName();
  std::string src = "@version \"1.2.3\"\n";
  
  TestPBed pbed( src );
  pbed._parser.ParsePureAttribute();

  REQUIRE( "1.2.3" == pbed._builder._binary->_version );
}},
{"type idのパースのテスト", []{
  REQUIRE( Float(32) == ParseType("f32") );
  REQUIRE( UInt(32) == ParseType("u32") );
  REQUIRE( Int(32) == ParseType("i32") );
  REQUIRE( UInt(8) == ParseType("u8") );
  REQUIRE( U8V4Type() == ParseType("u8v4") );
  REQUIRE( U16V4Type() == ParseType("u16v4") );
  REQUIRE( F32V3Type() == ParseType("f32v3") );
}},
{"ネガポジ反転のパーサーのテスト", []{
  ResetUniqueName();
  std::string src = R"(
# これはコメント

@title "ネガポジ反転"
def result_u8 |x, y| {
  let [b, g, r, a] = input_u8(x, y)
  u8[255-b, 255-g, 255-r, a]
}
)";
  
  TreeBuilder builder;
  Parser parser( builder, src );

  if (SECTION("ParseAttrDefが期待通り動くか？")) {SG g;
    parser._tokenizer.Scan();
    parser.SkipEOL();
    parser.ParsePureAttribute();

    REQUIRE( "ネガポジ反転" == builder._binary->_title );

    if (SECTION("ParseTensorDefが期待通り動くか？")) {SG g;
      parser.ParseTensorDef();

      auto &result = builder._binary->Result();
      auto resinfo = result.GetTensorInfo();
      REQUIRE( resinfo.Dimensions() == 2 );

      auto &args = result.GetTensorElem()->_args;
      REQUIRE( "u0_x" == args[0].first );
      REQUIRE( "u1_y" == args[1].first );

      REQUIRE( resinfo.ElemNum() == 4 );
      REQUIRE( resinfo.ElemType(0) == UInt(8) );

      auto actual = PrintExprS( result.GetBody() );
      // cout << actual << endl;
      // vblockより先は別のテストでカバーしているので、ここでは
      // vblockがちゃんと生成されている事だけチェックする。
      CheckContains( actual, "vblock {" );

    }
  }
  if (SECTION("参照しているTensorのRefがちゃんと生成されるか")) {SG g;
      BuildIR( builder, parser );

      auto &result = builder._binary->Result();
      auto& refs = result.GetTensorRefs();
      REQUIRE( refs.size() == 1 );
  }

  if (SECTION("バックエンドで実行して結果を確認")) {SG g;
    mfg_pal::Image32 inputTile;
    mfg_pal::Image32 resTile;

    SetupNegaPosi( inputTile, resTile );

    #ifdef MFG_BACKEND_ENABLED
    if (SECTION("Backendでネガポジ反転フィルタが動くか？")) {SG g;
      BuildIR( builder, parser );

      TargetBackend backend;

      RunBackend( backend, *builder._binary.get(), inputTile, resTile );

      auto guard_r = PixelLocker( &resTile );
      AssertNegaPosi( resTile );
    }
    if (SECTION("mfg_filter.cppのテスト")) {SG g;
      mfg::MFG mfg;
      auto bin = mfg.Compile( mfg_samples::g_mfg_script_negaposi_invert );
      RunFacade( mfg, bin, inputTile, resTile );

      auto guard_r = PixelLocker( &resTile );
      AssertNegaPosi( resTile );
    }
    #endif
    #ifdef MFG_BACKEND_METAL
    // ここでなくてもいいのだが、
    // 一番簡単なランダムを含んでないツリーで無いと認識するかのテストをしておく
    if (SECTION("RandomFinderが何も無い時に無いと言うか")) {SG g;
      BuildIR( builder, parser );

      RandomFinder finder;

      REQUIRE( false == finder.HasRandom( builder._binary->Result() ) );
    }
    #endif
  }
  #ifdef MFG_BACKEND_ENABLED
    // これはD3Dの方と部分的には統合できそう（assertは分かれる）
    if (SECTION("Randomやsortなどが不要な時に生成されないか")) {SG g;
      BuildIR( builder, parser );

      auto sources = GenShaderSources( *builder._binary.get() );
      for( auto src: sources)
      {
        #ifdef MFG_BACKEND_METAL
        CheckNotContains( src, "struct RandomGenerator" );
        CheckNotContains( src, "sort_int32_1d" );
        #elif defined(MFG_BACKEND_D3D)
        CheckNotContains( src, "uint RandomGenerator_Next" );
        #endif
      }
    }
  #endif
  if (SECTION("64bppのレイヤーに32bppのネガポジ反転を適用するテスト")) {SG g;
    #ifdef MFG_BACKEND_ENABLED
      mfg_pal::Image64 inputTile;
      mfg_pal::Image64 resTile;

      SetupNegaPosi64( inputTile, resTile );

      BuildIR( builder, parser );

      TargetBackend backend;

      RunBackend( backend, *builder._binary.get(), inputTile, resTile );

      auto guard_r = PixelLocker( &resTile );

      // AssertNegaPosi64を参考に、32bppからの拡張にする
      REQUIRE( 0xffff0000ffffffff == resTile.PixelGet( 0, 0 ).Value );
      REQUIRE( 0xffffffff0000ffff == resTile.PixelGet( 1, 0 ).Value );
      REQUIRE( 0xffffffffffff0000 == resTile.PixelGet( 2, 0 ).Value );
      REQUIRE( 0xffffededffffffff == resTile.PixelGet( 0, 1 ).Value );
      REQUIRE( 0xffffffffededffff == resTile.PixelGet( 1, 1 ).Value );
      REQUIRE( 0xffffffffffffeded == resTile.PixelGet( 2, 1 ).Value );
    #endif
  }

}},
{"uint32のグローバル変数のテスト", []{
  ResetUniqueName();
  std::string src = R"(
@title "青の所は0、それ以外は0xffff0000を入れるフィルタ"

# blue
let glob_col = input_u8(100, 60)

def result_u8 |x, y| {
  let col = input_u8(x, y)
  ifel(all( col == glob_col),
        u8[0, 0, 0, 0],
        u8[0, 0, 0xff, 0xff])
}
)";

  
  TreeBuilder builder;
  Parser parser( builder, src );
  BuildIR( builder, parser );

  mfg_pal::Image32 inputTile;
  mfg_pal::Image32 resTile;

  REQUIRE( LoadPngAsTile( "test_data/simple_small.png", inputTile ) );
  resTile.Resize( inputTile.Width(), inputTile.Height() );

  #ifdef MFG_BACKEND_ENABLED
  if (SECTION("Backendで動くか？")) {SG g;

    TargetBackend backend;
    // backend.EnableDebugLog( true );

    RunBackend( backend, *builder._binary.get(), inputTile, resTile );

    // std::cout << backend.DrainLogs() << std::endl;

   
    auto guard_r = PixelLocker( &resTile );
    /*
    printf( "%x\n", resTile.PixelGet( 100, 65 ).Value ); // 青の四角の場所
    printf( "%x\n", resTile.PixelGet( 80, 150 ).Value ); // 紫の四角の場所

    auto guard_i = PixelLocker( &inputTile );
    printf( "%x\n", inputTile.PixelGet( 100, 60 ).Value ); // 青の四角の場所1
    printf( "%x\n", inputTile.PixelGet( 100, 65 ).Value ); // 青の四角の場所2
    printf( "%x\n", inputTile.PixelGet( 80, 150 ).Value ); // 紫の四角の場所
    */

    REQUIRE( 0 == resTile.PixelGet( 100, 65 ).Value );  // 青の四角の場所
    REQUIRE( 0xffff0000 == resTile.PixelGet( 80, 150 ).Value ); // 紫の四角の場所
  }
  #endif
}},
// モザイク関連
{"@param_i32のパースのテスト", []{
  ResetUniqueName();
  std::string src = "@param_i32 MOSAIC_WIDTH(SLIDER, label=\"サイズ\", min=2, max=256, init=16)\n";  
  
  TestPBed pbed( src );
  pbed._parser.ParsePureAttribute();

  auto& params = pbed._builder._binary->_params;
  REQUIRE( params.size() == 1 );
  auto one = params[0];
  REQUIRE( one._paramName == "u0_MOSAIC_WIDTH" );
  REQUIRE( one.GetParamType() == ParamInfo::SLIDER_INT32 );
  REQUIRE( one._paramLabel == "サイズ" );
  REQUIRE( one.GetInit().Value<int>() == 16 );
  REQUIRE( one.GetMin().Value<int>() == 2 );
  REQUIRE( one.GetMax().Value<int>() == 256 );
}},
{"@param_f32のパースのテスト", []{
  ResetUniqueName();
  std::string src = "@param_f32 angle(SLIDER, label=\"角度\", min=0.0, max=3.2, init=1.0)\n";  
  
  TestPBed pbed( src );
  pbed._parser.ParsePureAttribute();

  auto& params = pbed._builder._binary->_params;
  REQUIRE( params.size() == 1 );
  auto one = params[0];
  REQUIRE( one._paramName == "u0_angle" );
  REQUIRE( one.GetParamType() == ParamInfo::SLIDER_FLOAT32 );
  REQUIRE( one._paramLabel == "角度" );
  REQUIRE( one.GetInit().Value<float>() == 1.0f );
  REQUIRE( one.GetMin().Value<float>() == 0.0f );
  REQUIRE( one.GetMax().Value<float>() == 3.2f );
}},
{"@param_f32でマイナスを使ったケース", []{
  ResetUniqueName();
  std::string src = "@param_f32 angle(SLIDER, label=\"角度\", min=-1.0, max=3.2, init=1.0)\n";  
  
  TestPBed pbed( src );
  pbed._parser.ParsePureAttribute();

  auto& params = pbed._builder._binary->_params;
  auto one = params[0];
  REQUIRE( one.GetMin().Value<float>() == -1.0f );
}},
{"@param_f32のDIRECTIONのパースのテスト", []{
  ResetUniqueName();
  std::string src = "@param_f32 angle(DIRECTION, init=1.0)\n";  
  
  TestPBed pbed( src );
  pbed._parser.ParsePureAttribute();

  auto& params = pbed._builder._binary->_params;
  REQUIRE( params.size() == 1 );
  auto one = params[0];
  REQUIRE( one._paramName == "u0_angle" );
  REQUIRE( one.GetParamType() == ParamInfo::DIRECTION_FLOAT32 );
  REQUIRE( one.GetInit().Value<float>() == 1.0f );
}},
{"@param_i32のラベルの国際化のテスト（面倒なので日本語だけテスト）", []{
  ResetUniqueName();
  std::string src = "@param_i32 MOSAIC_WIDTH(SLIDER, label=$LABEL_SIZE, min=2, max=256, init=16)\n";  

  auto guard = ScopeGuard([]{ mfg_resource::SetLanguage( NL_ENGLISH ); });
  mfg_resource::SetLanguage( NL_JAPANESE );
  
  TestPBed pbed( src );
  pbed._parser.ParsePureAttribute();

  auto& params = pbed._builder._binary->_params;
  REQUIRE( params.size() == 1 );
  auto one = params[0];
  REQUIRE( one._paramLabel == "サイズ" );
}},
{"@param_i32のCHECKBOXのパースのテスト", []{
  ResetUniqueName();
  std::string src = "@param_i32 debug(CHECKBOX, label=\"Debug\", init=1)\n";  
  
  TestPBed pbed( src );
  pbed._parser.ParsePureAttribute();

  auto& params = pbed._builder._binary->_params;
  REQUIRE( params.size() == 1 );
  auto one = params[0];
  REQUIRE( one._paramName == "u0_debug" );
  REQUIRE( one.GetParamType() == ParamInfo::CHECKBOX_INT32 );
  REQUIRE( one.GetLabel() == "Debug" );
  REQUIRE( one.GetInit().Value<int>() == 1 );
}},
{"@param_i32のDROPDOWNのパースのテスト", []{
  ResetUniqueName();
  std::string src = R"(@param_i32 debug(DROPDOWN, label="Debug", items=["abc", "def", "ghi"])
)";

  TestPBed pbed( src );
  pbed._parser.ParsePureAttribute();

  auto& params = pbed._builder._binary->_params;
  REQUIRE( params.size() == 1 );
  auto one = params[0];
  REQUIRE( one._paramName == "u0_debug" );
  REQUIRE( one.GetParamType() == ParamInfo::DROPDOWN_INT32 );
  REQUIRE( one.GetLabel() == "Debug" );
  REQUIRE( one.GetItems() == std::vector<std::string>({ "abc", "def", "ghi" }) );

  // 結果のassignでクラッシュしないか。
  one.Assign( 1 );

}},
{"@param_i32のDROPDOWNのラベルの国際化のテスト", []{
  ResetUniqueName();
  std::string src = R"(@param_i32 ddl(DROPDOWN, label="Debug", items=[$LABEL_SIZE, $LABEL_THRESHOLD]))
)";

  auto guard = ScopeGuard([]{ mfg_resource::SetLanguage( NL_ENGLISH ); });
  mfg_resource::SetLanguage( NL_JAPANESE );  
  
  TestPBed pbed( src );
  pbed._parser.ParsePureAttribute();

  auto& params = pbed._builder._binary->_params;
  REQUIRE( params.size() == 1 );
  auto one = params[0];
  REQUIRE( one.GetItems() == std::vector<std::string>({ "サイズ", "しきい値" }) );
}},
{"samplerのパースのテスト", []{
  ResetUniqueName();
  std::string src = "sampler<input_u8>(address=.ClampToEdge)\n";
  
  TestPBed pbed( src );
  auto& builder = pbed._builder;

  auto actual = pbed._parser.ParsePrimaryWOE();
  REQUIRE( actual.GetElemType() == IRElemType::SamplerElem );

  auto macro = actual.As<SamplerElem>();
  REQUIRE( macro->_addressType == SamplerElem::CLAMP_TO_EDGE );
  REQUIRE( macro->_target.Name() == "input_u8" );

}},
{"samplerのclamp_to_edgeのテスト", []{
  ResetUniqueName();

  std::string src = R"(
let clamped = sampler<input_u8>(address=.ClampToEdge)
def result_u8 |x, y| {
  clamped(x-1, y-1)+clamped(x, y)+clamped(x+1, y+1)
}
)";
  
  TreeBuilder builder;
  Parser parser( builder, src );

  BuildIR( builder, parser );

  if (SECTION("サンプラーの展開がちゃんとされるか")) {SG g;
    auto actual = PrintExprS( builder._binary->Result().GetBody()) ;
    // cout << actual << endl;
    CheckContains( actual, "clamp(u0_x + 1, 0, extent.input_u8.0 - 1)" );
    CheckContains( actual, "clamp(u1_y + 1, 0, extent.input_u8.1 - 1)" );
    if (SECTION("余計なclampが生成されていないか")) {SG g;
      // ClampToEdgeの時には既に引数はチェック済みなので全体のclampはいらないはず。
      CheckNotContains( actual, "clamp((clamp" );
    }
  }
  if (SECTION("バックエンドで実行して結果を確認")) {SG g;

    mfg_pal::Image32 inputTile;
    mfg_pal::Image32 resTile;

    inputTile.Resize( 2, 2 );
    resTile.Resize( 2, 2 );

    {
      auto guard_i = PixelLocker( &inputTile );
      PixelSetInt( inputTile, 0, 0, 1 );
      PixelSetInt( inputTile, 0, 1, 2 );
      PixelSetInt( inputTile, 1, 0, 3 );
      PixelSetInt( inputTile, 1, 1, 4 );
    }

    #ifdef MFG_BACKEND_ENABLED
    if (SECTION("バックエンドで実行")) {SG g;

      TargetBackend backend;
      RunBackend( backend, *builder._binary.get(), inputTile, resTile );

      {
        auto guard_r = PixelLocker( &resTile );
        REQUIRE( 6 == resTile.PixelGet( 0, 0 ).Value );
        REQUIRE( 8 == resTile.PixelGet( 1, 0 ).Value );
        REQUIRE( 7 == resTile.PixelGet( 0, 1 ).Value );
        REQUIRE( 9 == resTile.PixelGet( 1, 1 ).Value );
      }
    }
    #endif
  }

}},
{"ClampToBorderValueのスクリプトのテスト", []{
  ResetUniqueName();
  
  std::string src = R"(
let extended = sampler<input_u8>(address=.ClampToBorderValue, border_value=u8[0, 0, 0, 0])
def result_u8 |x, y| {
  extended(x-1, y)+extended(x, y)+extended(x, y+1)
}
)";
  
  TreeBuilder builder;
  Parser parser( builder, src );

  BuildIR( builder, parser );

  mfg_pal::Image32 inputTile;
  mfg_pal::Image32 resTile;

  inputTile.Resize( 3, 2 );
  resTile.Resize( 3, 2 );

  {
    auto guard_i = PixelLocker( &inputTile );

    PixelSetInt( inputTile, 0, 0, 10 );
    PixelSetInt( inputTile, 1, 0, 11 );
    PixelSetInt( inputTile, 2, 0, 12 );
    PixelSetInt( inputTile, 0, 1, 13 );
    PixelSetInt( inputTile, 1, 1, 14 );
    PixelSetInt( inputTile, 2, 1, 15 );
  }

  #ifdef MFG_BACKEND_ENABLED
  if (SECTION("バックエンドで実行")) {SG g;

    TargetBackend backend;
    RunBackend( backend, *builder._binary.get(), inputTile, resTile );

    {
      auto guard_r = PixelLocker( &resTile );

      REQUIRE( 23 == resTile.PixelGet( 0, 0 ).Value );
      REQUIRE( 35 == resTile.PixelGet( 1, 0 ).Value );
      REQUIRE( 38 == resTile.PixelGet( 2, 0 ).Value );
      REQUIRE( 13 == resTile.PixelGet( 0, 1 ).Value );
      REQUIRE( 27 == resTile.PixelGet( 1, 1 ).Value );
      REQUIRE( 29 == resTile.PixelGet( 2, 1 ).Value );
    }
  }
  #endif
}},
{"ClampToBorderValueの型間違いスクリプトエラーのテスト", []{
  ResetUniqueName();

  std::string src = R"(
let extended = sampler<input_u8>(address=.ClampToBordervalue, border_value=0)
def result_u8 |x, y| {
  extended(x-1, y)+extended(x, y)+extended(x, y+1)
}
)";

  VerifyParseError( src );
}},
{"NormalizedNearestのスクリプトのテスト", []{
  ResetUniqueName();

  // 0, 1, 2の3x3のインデックスに対して、
  // 0〜0.33まで0, 0.33〜0.66まで1, 0.66〜1までが2となるべき。
  std::string src = R"(
let finput = sampler<input_u8>(coord=.NormalizedNearest)
def result_u8 |x, y| {
  # とりあえずxは0.2, 0.4, 0.7の三つを試す。
  let fx = ifel(x == 0, 0.2, ...)
           elif(x == 1, 0.4, 0.7)
  # yは0.0, 0.66, 1.0の三つを試す。
  let fy = ifel(y == 0, 0.0, ...)
           elif(y == 1, 0.66, 1.0)
  finput(fx, fy)
}
)";

  TreeBuilder builder;
  Parser parser( builder, src );

  BuildIR( builder, parser );

  if (SECTION("nearest neighborの展開がちゃんとされるか")) {SG g;
    auto actual = PrintExprS( builder._binary->Result().GetBody()) ;
    // cout << actual << endl;

    // printの精度が雑なので0.00001や0.99999が丸められてしまう。テストとしてはまぁいいだろう。
    CheckContains( actual, "let t1 = ifel(abs(u3_fy - 1.00) < 0.00, 1.00, u3_fy)" );
    CheckContains( actual, "int32(t1 * float32(extent.input_u8.1))" );
  }

  #ifdef MFG_BACKEND_ENABLED
  if (SECTION("バックエンドで実行")) {SG g;
    mfg_pal::Image32 inputTile;
    mfg_pal::Image32 resTile;

    inputTile.Resize( 3, 3 );
    resTile.Resize( 3, 3 );

    {
      auto guard_i = PixelLocker( &inputTile );

      PixelSetInt( inputTile, 0, 0, 10 );
      PixelSetInt( inputTile, 1, 0, 11 );
      PixelSetInt( inputTile, 2, 0, 12 );
      PixelSetInt( inputTile, 0, 1, 13 );
      PixelSetInt( inputTile, 1, 1, 14 );
      PixelSetInt( inputTile, 2, 1, 15 );
      PixelSetInt( inputTile, 0, 2, 16 );
      PixelSetInt( inputTile, 1, 2, 17 );
      PixelSetInt( inputTile, 2, 2, 18 );
    }

    TargetBackend backend;
    RunBackend( backend, *builder._binary.get(), inputTile, resTile );

    {
      auto guard_r = PixelLocker( &resTile );

      REQUIRE( 10 == resTile.PixelGet( 0, 0 ).Value );
      REQUIRE( 11 == resTile.PixelGet( 1, 0 ).Value );
      REQUIRE( 12 == resTile.PixelGet( 2, 0 ).Value );
      REQUIRE( 13 == resTile.PixelGet( 0, 1 ).Value );
      REQUIRE( 14 == resTile.PixelGet( 1, 1 ).Value );
      REQUIRE( 15 == resTile.PixelGet( 2, 1 ).Value );
      REQUIRE( 16 == resTile.PixelGet( 0, 2 ).Value );
      REQUIRE( 17 == resTile.PixelGet( 1, 2 ).Value );
      REQUIRE( 18 == resTile.PixelGet( 2, 2 ).Value );
    }
  }
  #endif
}},
{"samplerのspreadが展開されるかのテスト", []{
  ResetUniqueName();

  // 上のテストと同じ内容
  std::string src = R"(
let finput = sampler<input_u8>(coord=.NormalizedNearest)
def result_u8 |x, y| {
  # とりあえずxは0.2, 0.4, 0.7の三つを試す。
  let fx = ifel(x == 0, 0.2, ...)
           elif(x == 1, 0.4, 0.7)
  # yは0.0, 0.66, 1.0の三つを試す。
  let fy = ifel(y == 0, 0.0, ...)
           elif(y == 1, 0.66, 1.0)
  let v = [fx, fy]
  finput(*v)
}
)";

  auto actual = ParseAndLowerAndReturnTreeDump( src );
  // cout << actual << endl;
  // 上のテストと同じツリーになっていればOK
  CheckContains( actual, "let t1 = ifel(abs(u4_v.1 - 1.00) < 0.00, 1.00, u4_v.1)" );
  CheckContains( actual, "int32(t1 * float32(extent.input_u8.1))" );
}},
{"テンソルのspreadが展開されるかのテスト", []{
  ResetUniqueName();

  std::string src = R"(
def result_u8 |x, y| {
  let v = [x, y]
  input_u8(*v)
}
)";

  auto actual = ParseAndLowerAndReturnTreeDump( src );
  // cout << actual << endl;
  CheckContains( actual, "load:b_input_u8.0[clamp((u2_v.1 * extent.input_u8.0) + u2_v.0, 0, (extent.input_u8.1 * extent.input_u8.0) - 1)]" );
}},
{"samplerの.NormalizedNearstと.ClampToEdgeの組み合わせのテスト", []{
  ResetUniqueName();

  std::string src = R"(
let fextended = sampler<input_u8>(address=.ClampToBorderValue, coord=.NormalizedNearest, border_value=u8[123, 0, 0, 0])

def result_u8 |x, y| {
  # とりあえずxは0.0, 0.4, 1.0の三つを試す。
  let fx = ifel(x == 0, 0.0, ...)
           elif(x == 1, 0.4, 0.7)
  # yは-0.34, 0.66, 1.1の三つを試す。
  let fy = ifel(y == 0, -0.34, ...)
           elif(y == 1, 0.66, 1.1)
  fextended(fx, fy)
}
)";
  TreeBuilder builder;
  Parser parser( builder, src );

  BuildIR( builder, parser );

  if (SECTION("nearest neighborとconstant exteriorの両方がちゃんと展開されるか")) {SG g;
    auto actual = PrintExprS( builder._binary->Result().GetBody()) ;
    // cout << actual << endl;

    CheckContains( actual, "ifel(abs(u2_fx - 1.00) < 0.00, 1.00, u2_fx)" );
    CheckContains( actual, "let t5 = ((t2 < 0) || (t2 > (extent.input_u8.0 - 1)" );
    CheckContains( actual, "ifel(t5," );
  }

  #ifdef MFG_BACKEND_ENABLED
  if (SECTION("バックエンドで実行")) {SG g;
    mfg_pal::Image32 inputTile;
    mfg_pal::Image32 resTile;

    inputTile.Resize( 3, 3 );
    resTile.Resize( 3, 3 );

    {
      auto guard_i = PixelLocker( &inputTile );

      PixelSetInt( inputTile, 0, 0, 10 );
      PixelSetInt( inputTile, 1, 0, 11 );
      PixelSetInt( inputTile, 2, 0, 12 );
      PixelSetInt( inputTile, 0, 1, 13 );
      PixelSetInt( inputTile, 1, 1, 14 );
      PixelSetInt( inputTile, 2, 1, 15 );
      PixelSetInt( inputTile, 0, 2, 16 );
      PixelSetInt( inputTile, 1, 2, 17 );
      PixelSetInt( inputTile, 2, 2, 18 );
    }

    TargetBackend backend;
    RunBackend( backend, *builder._binary.get(), inputTile, resTile );

    {
      auto guard_r = PixelLocker( &resTile );

      REQUIRE( 123 == resTile.PixelGet( 0, 0 ).Value );
      REQUIRE( 123 == resTile.PixelGet( 1, 0 ).Value );
      REQUIRE( 123 == resTile.PixelGet( 2, 0 ).Value );
      REQUIRE( 13 == resTile.PixelGet( 0, 1 ).Value );
      REQUIRE( 14 == resTile.PixelGet( 1, 1 ).Value );
      REQUIRE( 15 == resTile.PixelGet( 2, 1 ).Value );
      REQUIRE( 123 == resTile.PixelGet( 0, 2 ).Value );
      REQUIRE( 123 == resTile.PixelGet( 1, 2 ).Value );
      REQUIRE( 123 == resTile.PixelGet( 2, 2 ).Value );
    }
  }
  #endif
}},
{".NormalizedLinearのスクリプトのテスト", []{
  ResetUniqueName();

  // 0, 1, 2の3x3のインデックスに対して、
  // 0, 0.5, 1.0の三つの座標からの距離で割合が決まる。
  std::string src = R"(
let finput = sampler<input_u8>(coord=.NormalizedLinear)
def result_u8 |x, y| {
  # とりあえずxは0, 0.2, 1.0の三つを試す。
  let fx = ifel(x == 0, 0.0, ...)
           elif(x == 1, 0.2, 1.0)
  # yは0.0, 0.5, 0.8の三つを試す。
  let fy = ifel(y == 0, 0.0, ...)
           elif(y == 1, 0.5, 0.9)
  finput(fx, fy)
}
)";

  TreeBuilder builder;
  Parser parser( builder, src );

  BuildIR( builder, parser );

  if (SECTION("bilinearの展開がちゃんとされるか")) {SG g;
    auto actual = PrintExprS( builder._binary->Result().GetBody()) ;
    // cout << actual << endl;

    /*
      文字列でチェックするには生成されるコードが複雑過ぎるので、上記コメントを外して目視で確認。
      テストとしては、正しいコードの一部が出力されているかをチェックするに留める。
    */
    CheckContains( actual, "let t0 = u2_fx * float32(extent.input_u8.0 - 1)" );
  }

  #ifdef MFG_BACKEND_ENABLED
  if (SECTION("バックエンドで実行")) {SG g;
    mfg_pal::Image32 inputTile;
    mfg_pal::Image32 resTile;

    inputTile.Resize( 3, 3 );
    resTile.Resize( 3, 3 );

    {
      auto guard_i = PixelLocker( &inputTile );

      PixelSetInt( inputTile, 0, 0, 0x11111111 );
      PixelSetInt( inputTile, 1, 0, 0x22222222 );
      PixelSetInt( inputTile, 2, 0, 0x33333333 );
      PixelSetInt( inputTile, 0, 1, 0x44444444 );
      PixelSetInt( inputTile, 1, 1, 0x55555555 );
      PixelSetInt( inputTile, 2, 1, 0x66666666 );
      PixelSetInt( inputTile, 0, 2, 0x77777777 );
      PixelSetInt( inputTile, 1, 2, 0x88888888 );
      PixelSetInt( inputTile, 2, 2, 0x99999999 );
    }

    TargetBackend backend;
    RunBackend( backend, *builder._binary.get(), inputTile, resTile );

    {
      auto guard_r = PixelLocker( &resTile );
      /*
        xは0, 0.2, 1.0
        yは0.0, 0.5, 0.9
      */
      REQUIRE( 0x11111111 == resTile.PixelGet( 0, 0 ).Value );

      // ２つ目の点が0.5。
      // 0-0.2 : 0.2-0.5 = 2:3
      // 0.6*0x11+0.4*0x22 = 0x17
      REQUIRE( 0x17171717 == resTile.PixelGet( 1, 0 ).Value );
      REQUIRE( 0x33333333 == resTile.PixelGet( 2, 0 ).Value );

      REQUIRE( 0x44444444 == resTile.PixelGet( 0, 1 ).Value );
      // 0.6*0x44+0.4*0x55 = 0x4a 
      REQUIRE( 0x4a4a4a4a == resTile.PixelGet( 1, 1 ).Value );
      REQUIRE( 0x66666666 == resTile.PixelGet( 2, 1 ).Value );


      // 0.2*0x44+0.8*0x77 = 0x6c
      REQUIRE( 0x6c6c6c6c == resTile.PixelGet( 0, 2 ).Value );
      // 0.6*0x44+0.4*0x55 = 74.8
      // 0.6*0x77+0.4*0x88 = 125.8
      // 0.8*125.8+0.2*74.8 = 115.6 = 0x73
      REQUIRE( 0x73737373 == resTile.PixelGet( 1, 2 ).Value );
      REQUIRE( 0x8e8e8e8e == resTile.PixelGet( 2, 2 ).Value );
    }
  }
  #endif
}},
{"rsumのスクリプトのテスト", []{
 ResetUniqueName();
  std::string src = R"(
let clamped = sampler<input_u8>(address=.ClampToEdge)
def result_u8 |x, y| {
  rsum(0..<2, 0..<3) |rx, ry| {
    clamped( x+rx, y+ry )
  }
}
)";
   // 4, 5
  TreeBuilder builder;
  Parser parser( builder, src );

  BuildIR( builder, parser );

  mfg_pal::Image32 inputTile;
  mfg_pal::Image32 resTile;

  inputTile.Resize( 4, 5 );
  resTile.Resize( 4, 5 );

  /*
      3  4   5  6
      8  9  10 11
      13 14 15 16
      18 19 20 21
  */
  {
    auto guard_i = PixelLocker( &inputTile );
    for( auto j: NRange(5) )
    {
      for( auto i : NRange(4))
      {
        PixelSetInt( inputTile, i, j, 3+i+5*j );
      }
    }
  }

  #ifdef MFG_BACKEND_ENABLED
  if (SECTION("Backendで実行して結果が合うか？")) {SG g;
    TargetBackend backend;
    RunBackend( backend, *builder._binary.get(), inputTile, resTile );

    {
      auto guard_r = PixelLocker( &resTile );
      /*
      for( auto j: NRange(5) )
      {
        for( auto i : NRange(4))
        {
          cout << i << "," << j << ": " << resTile.PixelGet( i, j ).Value << endl;;
        }
      }
      */
      /*
          3  4   5  6
          8  9  10 11
          13 14 15 16
          18 19 20 21

          を2x3で足す
      */
      REQUIRE( 3+4+8+9+13+14 == resTile.PixelGet( 0, 0 ).Value );
      REQUIRE( 4+5+9+10+14+15 == resTile.PixelGet( 1, 0 ).Value );
      REQUIRE( 5+6+10+11+15+16 == resTile.PixelGet( 2, 0 ).Value );
      REQUIRE( 2*(6+11+16) == resTile.PixelGet( 3, 0 ).Value );
      REQUIRE( 8+9+13+14+18+19 == resTile.PixelGet( 0, 1 ).Value );
    }
  }
  #endif


}},
{"reduceのテスト", []{
  ResetUniqueName();
  std::string src = R"(
def result_u8 |x, y| {
  let res = reduce(init=1, 2..<4) |r, accm| {
    r*accm
  }
  u8[res, 0, 0, 0]
}
)";
  TreeBuilder builder;
  Parser parser( builder, src );

  if (SECTION("パースしてツリーが作れるか？")) {SG g;
    parser.ParseAll();

    auto actual = PrintExprS( builder._binary->Result().GetBody() );
    // cout << actual << endl;

    CheckContains( actual, "reduce r0 init=1, 2..<4" );
    CheckContains( actual, "u2_r * u3_accm" );

    if (SECTION("Lowerがただしく動いているか？")) {SG g;
      builder._binary->LowerAndAnalyze();

      auto lowertree = PrintExprS( builder._binary->Result().GetBody() );
      // cout << lowertree << endl;

      CheckContains( lowertree, "var u3_accm = 1" );
      // 一回間違って生成されていたのでテストしておく。
      CheckNotContains( lowertree, "for(u3_accm:" );
      CheckContains( lowertree, "for(u2_r:" );
      CheckContains( lowertree, "store u3_accm = u2_r * u3_accm" );
      size_t pos = 0;
      CheckContains( lowertree, "let rres.r0.0 = u3_accm", pos );
      CheckContains( lowertree, "rres.r0.0" , pos );
    }
  }
  #ifdef MFG_BACKEND_ENABLED
  if (SECTION("Backendで実行して結果が合うか？")) {SG g;
    BuildIR( builder, parser );

    mfg_pal::Image32 inputTile;
    mfg_pal::Image32 resTile;

    inputTile.Resize( 1, 1 );
    resTile.Resize( 1, 1 );

    TargetBackend backend;

    RunBackend( backend, *builder._binary.get(), inputTile, resTile );

    {
      auto guard_r = PixelLocker( &resTile );
      REQUIRE( 2*3 == resTile.PixelGet( 0, 0 ).Value );
    }
  }
  #endif

}},
{"reduceのaccmの重複のテスト(#1140)", []{
  ResetUniqueName();
  std::string src = R"(
def result_u8 |x, y| {
  let res = reduce(init=1, 2..<4) |r, accm| {
    r*accm
  }
  let res2 = reduce(init=1, 2..<4) |r, accm| {
    r*accm
  }
  u8[res, res2, 0, 0]
}
)";

  if (SECTION("ツリーではaccmに別の名前が振られているか？")) {SG g;
    auto actual = ParseAndReturnTreeDump( src );
    // cout << actual << endl;

    CheckContains( actual, "u3_accm" );
    CheckContains( actual, "u6_accm" );
  }
  #ifdef MFG_BACKEND_ENABLED
  if (SECTION("Backendで実行して結果が合うか？")) {SG g;
    ExecVerifyScript( src, 1, 1, [](mfg_pal::Image32& resTile) {
      REQUIRE( 0x00000606 == resTile.PixelGet( 0, 0 ).Value );
    });
  }
  #endif
}},
{"reduceのタプルのテスト", []{
  ResetUniqueName();
  auto src = R"(
def result_u8 |x, y| {
  let [_, vres] = reduce(init=[0, 1], 2..<7) |r, accm| {
    let [flag, value] = accm
    let nextValue = value*r
    ifel(flag == 1,
         accm, ...)
    elif(nextValue > 10,
          [1, r],
          [0, nextValue])
  }
  u8[vres, 0, 0, 0]
}
)";

  TreeBuilder builder;
  Parser parser( builder, src );

  if (SECTION("accmのtuple letのパースが出来るか？")) {SG g;
    parser.ParseAll();

    auto actual = PrintExprS( builder._binary->Result().GetBody() );
    // cout << actual << endl;

    // under scoreがちゃんとunder scoreのままかをついでにここでチェック。
    CheckContains( actual, "let [_, u7_vres] = " );

    CheckContains( actual, "let [u4_flag, u5_value] = u3_accm" );

    if (SECTION("accmのletがlowerで展開されているか")) {SG g;
      builder._binary->LowerAndAnalyze();

      auto lowertree = PrintExprS( builder._binary->Result().GetBody() );
      // cout << lowertree << endl;

      if (SECTION("accmの初期化の生成が正しく展開されるか")) {SG g;
        CheckContains( lowertree, "var u3_accm.0 = 0" );
        CheckContains( lowertree, "var u3_accm.1 = 1" );
      }

      if (SECTION("accm変数に対するdestructuringが正しく展開されるか")) {SG g;
        CheckContains( lowertree, "let u4_flag = u3_accm.0" );
        CheckContains( lowertree, "let u5_value = u3_accm.1" );
      }

      if (SECTION("結果のdestructuringが正しく展開されているか")) {SG g;
        CheckContains( lowertree, "let u7_vres = rres.r0.1" );
      }
    }
  }

  #ifdef MFG_BACKEND_ENABLED
  if (SECTION("Backendで実行して結果が合うか？")) {SG g;
    BuildIR( builder, parser );

    mfg_pal::Image32 inputTile;
    mfg_pal::Image32 resTile;

    inputTile.Resize( 1, 1 );
    resTile.Resize( 1, 1 );

    TargetBackend backend;

    RunBackend( backend, *builder._binary.get(), inputTile, resTile );

    {
      auto guard_r = PixelLocker( &resTile );
      // rは2から6まで。最初に10を超えるのはrが4の時。
      // 1*2 *3 *4
      REQUIRE( 4 == resTile.PixelGet( 0, 0 ).Value );
    }
  }
  #endif
}},
{"加減乗除のパースのテスト", []{
  ResetUniqueName();
  std::string src = R"(
def result_u8 |x, y| {
  u8[(x+2+3*y*5-6)/4, 0, 0, 0]
})";

  TestPBed pbed( src );
  pbed.ParseAll();

  auto actual = PrintExprS( pbed.GetReturnExpr() );
  // cout << actual << endl;
  CheckContains( actual, "(((u0_x + 2) + ((3 * u1_y) * 5)) - 6) / 4" );
}},
{"加減乗除のパースのテスト2", []{
  // モザイクで見つけたバグの再現
  ResetUniqueName();
  std::string ssrc = R"(
def result_u8 |x, y| {
  u8[x*2+4, 0, 0, 0]
})";

  TestPBed pbed( ssrc );
  pbed.ParseAll();

  auto actual = PrintExprS( pbed.GetReturnExpr() );
  // cout << actual << endl;
  CheckContains( actual, "(u0_x * 2) + 4" );
}},
{"累乗のパースのテスト", []{
  ResetUniqueName();
  std::string src = R"(
def result_u8 |x, y| {
  u8[(x+2)^2*3, 0, 0, 0]
})";

  TestPBed pbed( src );
  pbed.ParseAll();

  auto actual = PrintExprS( pbed.GetReturnExpr() );
  // cout << actual << endl;
  CheckContains( actual, "pow(u0_x + 2, 2) * 3" );
}},
{"複数カーネル対応のスクリプトのテスト", []{
  // Ver 0.4とほぼ同じ内容だが最後がresultになっている
  ResetUniqueName();
  std::string src = R"(
@bounds(5, 5)
def ts |x, y| { x+y }

def result_u8 |x, y| {
  u8[(ts(x, y) + ts(x, y+1) + ts(x+1, y) + ts(x+1, y+1))/4, 0, 0, 0]
})";

  TreeBuilder builder;
  Parser parser( builder, src );
  BuildIR( builder, parser );

  REQUIRE( builder._binary->ResultDefined() );
  REQUIRE( 1 == builder._binary->_tensors.size() );

  mfg_pal::Image32 dummyInput;
  mfg_pal::Image32 resTile;

  dummyInput.Resize( 4, 4 );
  resTile.Resize( 4, 4 );
  auto& binary = *builder._binary.get();

  #ifdef MFG_BACKEND_ENABLED
  if (SECTION("Backendで実行して結果が合うか？")) {SG g;
    TargetBackend backend;

    RunBackend( backend, binary, dummyInput, resTile );

    ImageTileAccessor acc( resTile );
    AssertVer04( acc );
  }
  #endif
}},
{"タプルのスクリプトのテスト", []{ // このテストはresult_u8になったのでhello worldとあまり変わらなくなってしまったが一応残しておく。
  ResetUniqueName();
  std::string src = R"(
def result_u8 |x, y| { u8[x, 2*y, 0, 0] }
)";

  TestPBed pbed( src );
  pbed.ParseAll();

  auto& ts = pbed.Result();
  auto tsinfo = ts.GetTensorInfo();
  REQUIRE( tsinfo.ElemNum() == 4 );

  auto ret = pbed.GetReturnExpr( ts );

  // U8V4へのキャストがあって、その下にタプル
  REQUIRE( ret->_elemType == IRElemType::Call );
  REQUIRE( ret->As<Call>()->GetArg(0)->_elemType == IRElemType::TupleElem );

  auto tree = PrintExprS( ret );
  // cout << tree << endl;
  CheckContains( tree, "tuple(u0_x, 2 * u1_y, 0, 0)" );
}},
{"タプルのベクトル演算のテスト、右から掛けるケース", []{
  ResetUniqueName();
  std::string src = R"(
def result_u8 |x, y| { u8([x, 2*y, 0, 0]*3) }
)";

  TestPBed pbed( src );
  pbed.BuildIR();

  auto& ts = pbed.Result();
  auto tsinfo = ts.GetTensorInfo();
  REQUIRE( tsinfo.ElemNum() == 4 );

  auto ret = pbed.GetReturnExpr( ts );

  auto tree = PrintExprS( ret );
  // cout << tree << endl;
  CheckContains( tree, "tuple(uint8(u0_x * 3), uint8((2 * u1_y) * 3), uint8(0 * 3), uint8(0 * 3))" );
}},
{"タプルのベクトル演算のテスト、左から掛けるケース", []{
  ResetUniqueName();
  std::string src = R"(
def result_u8 |x, y| { u8(3* [x, 2*y, 0, 0]) }
)";

  TestPBed pbed( src );
  pbed.BuildIR();

  auto& ts = pbed.Result();
  auto tsinfo = ts.GetTensorInfo();
  REQUIRE( tsinfo.ElemNum() == 4 );

  auto ret = pbed.GetReturnExpr( ts );

  auto tree = PrintExprS( ret );
  // cout << tree << endl;
  CheckContains( tree, "tuple(uint8(3 * u0_x), uint8(3 * (2 * u1_y)), uint8(3 * 0), uint8(3 * 0))" );
}},
{"+のベクトライズのテスト", []{
  auto src = R"(
def result_u8 |x, y| {
  let vec = [1, 2, 3]
  let v2 = vec+1
  let [a1, a2, a3] = v2
  u8[a1+a2+a3, 0, 0, 0]
}
)";

  auto actual = ParseAndLowerAndReturnTreeDump( src );
  // cout << actual << endl;
  CheckContains( actual, "let u3_v2.1 = u2_vec.1 + 1" );

}},
{"sinのベクトライズのテスト、5要素でも動くはず", []{
  auto src = R"(
def result_u8 |x, y| {
  let vec = sin([1.0, 2.0, 3.0, 4.0, 5.0])
  let a = vec.0+vec.1+vec.2+vec.3+vec.4
  u8[a, 0, 0, 0]
}
)";

  auto actual = ParseAndLowerAndReturnTreeDump( src );
  // cout << actual << endl;
  // 5番目の要素が展開されてれば多分OK。
  CheckContains( actual, "let u2_vec.4 = sin(5.00)" );
}},
{"Orのベクトライズのテスト", []{
  auto src = R"(
def result_u8 |x, y| {
  let vec = [1, 2, 3]
  let v2 = vec|0x80
  let [a1, a2, a3] = v2
  u8[a1+a2+a3, 0, 0, 0]
}
)";

  auto actual = ParseAndLowerAndReturnTreeDump( src );
  // // cout << actual << endl;
  CheckContains( actual, "let u3_v2.1 = u2_vec.1 | 128" );

}},
// 以前は0x80がunsignedだったころの名残のテスト。
// もう不要だが、0x80uに変えて残しておく
{"二項演算の型違いのパースエラーのテスト", []{
  auto src = R"(
def result_u8 |x, y| {
  let vec = [1, 2, 3]
  let v2 = vec|0x80u
  let [a1, a2, a3] = v2
  u8[a1+a2+a3, 0, 0, 0]
}
)";

  VerifyParseError( src );
}},
{"ifelと==のパースのテスト", []{
  ResetUniqueName();
  std::string src = R"(
def result_u8 |x, y| { u8[ifel(x == 0, x, x+y), 0, 0, 0] }
)";

  TestPBed pbed( src );
  pbed.ParseAll();

  auto& ts = pbed.Result();
  auto ret = pbed.GetReturnExpr( ts );

  // cast u8v4
  REQUIRE( ret->_elemType == IRElemType::Call );
  auto tup = ret->As<Call>()->GetArg(0);
  REQUIRE( tup->_elemType == IRElemType::TupleElem );
  auto ifel = tup->As<TupleElem>()->GetItemsSC()[0];
  REQUIRE( ifel->_elemType == IRElemType::IfEl );

  auto cond = ifel->As<IfEl>()->GetCondition();
  REQUIRE( cond->_elemType == IRElemType::BinOp && cond->As<BinOp>()->_opType == BinOp::Eq );
  auto tree = PrintExprS( ifel );
  // cout << tree << endl;
  CheckContains( tree, "ifel(u0_x == 0, u0_x, u0_x + u1_y)" );
}},
{"モザイクフィルタのスクリプトのテスト", []{
  ResetUniqueName();
  std::string src = R"(
@param_i32 MOSAIC_WIDTH(SLIDER, label="サイズ", min=2, max=256, init=16)

let clamped = sampler<input_u8>(address=.ClampToEdge)


@bounds( (input_u8.extent(0)-1)/MOSAIC_WIDTH+1, (input_u8.extent(1)-1)/MOSAIC_WIDTH+1 )
def avg |x, y|{
	rsum(0..<MOSAIC_WIDTH, 0..<MOSAIC_WIDTH) |rx, ry|{
		let [b, g, r, a] = i32(clamped( MOSAIC_WIDTH*x+rx, MOSAIC_WIDTH*y+ry ))
    [*[b, g, r]*a, a]
	}
}

def result_u8 |x2, y2| {
  let [b2, g2, r2, a2] = avg( x2/MOSAIC_WIDTH, y2/MOSAIC_WIDTH )

	ifel(a2==0, u8[0, 0, 0, 0],
		    u8[*[b2, g2, r2]/a2, a2/(MOSAIC_WIDTH*MOSAIC_WIDTH)] )
}
)";

  TreeBuilder builder;
  Parser parser( builder, src );
  BuildIR( builder, parser );

  if (SECTION("タプルのテンソルのdestructuringがちゃんと出来ているか")) {SG g;
    auto& result = builder._binary->Result();
    auto actual = PrintExprS( result.GetBody() );
    // cout << actual << endl;

    // ベクトルバッファ
    CheckContains( actual, "letv t3 = load:b_avg.0[clamp(((u10_y2 / u0_MOSAIC_WIDTH) * extent.avg.0) + (u9_x2 / u0_MOSAIC_WIDTH), 0, (extent.avg.1 * extent.avg.0) - 1)]" );
    CheckContains( actual, "let u11_b2 = t3.0" );
    CheckContains( actual, "let u14_a2 = t3.3" );
  }

  if (SECTION("バックエンドで実行して結果を確認")) {SG g;
    const int MOSAIC_WIDTH = 16;

    mfg_pal::Image32 inputTile;
    mfg_pal::Image32 resTile;

    REQUIRE( LoadPngAsTile( "test_data/simple_small.png", inputTile ) );
    resTile.Resize( inputTile.Width(), inputTile.Height() );

    RVal32 tmp;
    tmp.u._ival = MOSAIC_WIDTH;

    #ifdef MFG_BACKEND_ENABLED
    if (SECTION("Backendで実行")) {SG g;

      TargetBackend backend;

      backend._renv.Assign( "u0_MOSAIC_WIDTH", tmp );
      // backend._debug = true;

      RunBackend( backend, *builder._binary.get(), inputTile, resTile );

      {
        auto guard_r = PixelLocker( &resTile );

        // 255 10 10 44
        REQUIRE( 0x2cff0a0a == resTile.PixelGet( 150, 100 ).Value );
        // 181 10 255 33
        REQUIRE( 0x21b50aff == resTile.PixelGet( 38, 126 ).Value );
      }
    }
    #endif
  }
}},
// モーションブラー関連
{"トップレベルのブロックの簡単なパースのテスト", []{
  ResetUniqueName();
  std::string src = R"(
@param_f32 strength(SLIDER, label="強度", min=3.0, max=300.0, init=10.0)

let sample = 2    
let mw = i32( strength * sample )

let ar = 0.33 * mw

@bounds(mw+1)
def weights |m|{ 100 * exp( -m*m / (2*ar*ar) ) }
)";

  TestPBed pbed( src );
  pbed.ParseAll();
  // 本当はLowerしたあとに呼ぶの前提だが、inline関数とかなければ問題無い
  pbed._builder._binary->CollectFreeRef();

  auto& tblocks = pbed._builder._binary->_tblocks;
  auto& tensors = pbed.Tensors();

  REQUIRE( tblocks.size() == 1 );
  auto &tblock = *tblocks[0];

  auto& refs = tblock.GetParamRefs();
  REQUIRE( refs.size() == 1 );
  REQUIRE( refs[0]._name == "u0_strength" );

  auto tbdump = PrintExprS( tblock.GetIRBody() );
  // cout << tbdump << endl;

  size_t pos = 0;
  CheckContains( tbdump, "let u1_sample = 2", pos );
  CheckContains( tbdump, "let u2_mw = ", pos );
  CheckContains( tbdump, "let u3_ar = ", pos );

  REQUIRE( tensors.size() == 1 );
  auto& ts = *tensors[0];
  REQUIRE( ts.Name() == "weights" );
  auto& refs2 = ts.GetParamRefs();
  REQUIRE( refs2.size() == 1 );
  REQUIRE( refs2[0]._name == "u3_ar" );

  auto tsdump = PrintExprS( ts.GetBody() );
  // cout << tsdump << endl;
  // 空じゃない事だけ確認しておく。
  CheckContains( tsdump, "float32" );
}},
{"モーションブラーのスクリプトのテスト", []{
  ResetUniqueName();
  std::string src = R"(
@param_f32 strength(SLIDER, label="強度", min=3.0, max=300.0, init=10.0)
@param_f32 angle(SLIDER, label="角度", min=0.0, max=3.2, init=0.0)

let sample = 2

let DeltaU = i32( cos(angle) * 65536 / sample )
let DeltaV = i32( sin(angle) * 65536 / sample )

let mw = i32( strength * sample )

let ar = 0.33 * mw

@bounds(mw+1)
def weights |m|{ 100 * exp( -m*m / (2*ar*ar) ) }

let extended = sampler<input_u8>(address=.ClampToBorderValue, border_value=u8[0, 0, 0, 0] )

@bounds(input_u8.extent(0), input_u8.extent(1))
def mid_all |x, y| {
  rsum(-mw..<mw+1) |rx| {

	  let px = x + ((DeltaU * rx) >> 16)
	  let py = y + ((DeltaV * rx) >> 16)
	
	  let gauss = weights( abs(rx) )
	
	  let [b, g, r, a] = f32(extended( px, py ))
	  let ga = gauss * a
	
		[
      *[b, g, r]*ga,
	    ga,
	    gauss
	   ]
	}
}

def result_u8 |x, y| {
  let [mid_b, mid_g, mid_r, mid_a, count] = mid_all(x, y)

  ifel( mid_a == 0,
    u8[0, 0, 0, 0],
    u8[*[mid_b, mid_g, mid_r]/mid_a,  (mid_a / count)]
  )
}
)";

  TestPBed pbed( src );
  pbed.BuildIR();

  auto& binary = *pbed._builder._binary.get();

  if (SECTION("@boundsが正しく処理されているか")) {SG g;
    auto top1 = PrintExprS( binary._tblocks[0]->_block );
    // cout << top1 << endl;

    CheckContains( top1, "let t0 = u5_mw + 1" );

    auto iter = binary._binfo._tensorBounds.find( binary._tensors[0]->Name() );
    REQUIRE( iter != binary._binfo._tensorBounds.end() );

    REQUIRE( iter->second.size() == 1 );
    REQUIRE( iter->second[0].IsVariable() );
    REQUIRE( iter->second[0]._varName == "t0" );
  }
  if (SECTION("input_u8.extent(0)などが正しく処理されているか")) {SG g;
    // mid_allのboundsを調べる
    auto iter = binary._binfo._tensorBounds.find( binary._tensors[1]->Name() );
    REQUIRE( iter != binary._binfo._tensorBounds.end() );

    REQUIRE( iter->second.size() == 2 );
    REQUIRE( iter->second[0].IsVariable() );
    REQUIRE( iter->second[1].IsVariable() );
    REQUIRE( iter->second[0]._varName == "extent.input_u8.0" );
    REQUIRE( iter->second[1]._varName == "extent.input_u8.1" );
  }

  #ifdef MFG_BACKEND_METAL
  if (SECTION("LookupLibraryBoundaryがちゃんとTopLevelBlockを認識しているか？")) {SG g;
    MetalBackend backend;
    auto& binary = *pbed._builder._binary.get();
    auto& tles = binary._tles;
    
    auto actual = backend.LookupLibraryBoundary( tles, tles.begin() );
    REQUIRE( actual->_etype == TLEntityRef::TENSOR );
    REQUIRE( actual->u._tensor == tles[1].u._tensor );
  }
  #endif
  if (SECTION("inputTileとresTileを用いたテスト")) {SG g;
    mfg_pal::Image32 inputTile;
    mfg_pal::Image32 resTile;

    REQUIRE( LoadPngAsTile( "test_data/simple_small.png", inputTile ) );
    resTile.Resize( inputTile.Width(), inputTile.Height() );

    TileReference resRef( &resTile );
    InputTiles tiles;
    tiles.Insert( 0, &inputTile );

    #ifdef MFG_BACKEND_METAL
    if (SECTION("参照されてないバッファが解放されているか？")) {SG g;
      MetalBackend backend;
      backend._renv.Assign( "u0_strength", { 10.0f } );
      backend._renv.Assign( "u1_angle", { -0.488f } );

      auto& binary = *pbed._builder._binary.get();
      auto& tles = binary._tles;
      auto begIter = tles.begin();

      backend.SetupInputResult( binary, tiles, resRef );

      /*
      tles:
        0: toplevelblock
        1: weights
        2: midall
        3: result 
      */

      // toplevelを実行
      backend.RunLibrary( binary, begIter, begIter+1, 0 );
      // toplevelのバッファはここで解放されるが、
      // toplevelのバッファはGeneratorのカーネルの名前でしかlookupできなく、
      // これはRunLibraryの中で一時的に作られるだけなのでテストできない

      // midallまで実行
      // Lifetimeの確認のため、Libraryのboundaryを通常と違う所にしてある。
      backend.RunLibrary( binary, begIter+1, begIter+3, 1 );

      // input, weightsは解放されて、midallはまだ。
      REQUIRE( backend.IsBufferReleased( GetFirstInput( binary.Input() ) ) );
      REQUIRE( backend.IsBufferReleased( *tles[1].u._tensor ) );
      REQUIRE( !backend.IsBufferReleased( *tles[2].u._tensor ) );

      // resultまで実行
      backend.RunLibrary( binary, begIter+3, begIter+4, 3 );

      // midallは解放される、resultはまだ。
      REQUIRE( backend.IsBufferReleased( *tles[2].u._tensor ) );
      REQUIRE( !backend.IsBufferReleased( binary.Result() ) );

      if (SECTION("手動で実行してもフィルタ自体が正しく動いているか")) {SG g;
        backend.CopyBackResult( binary, resRef );
        {
          auto locker = PixelLocker( &resTile );
          REQUIRE( 0x19fe0909 == resTile.PixelGet( 172, 57 ).Value ||
                    0x19ff0a0a == resTile.PixelGet( 172, 57 ).Value );
        }
      }
    }
    #endif

    #ifdef MFG_BACKEND_D3D
    if (SECTION("参照されてないバッファが解放されているか？")) {SG g;
      D3DBackend backend;
      backend._renv.Assign( "u0_strength", { 10.0f } );
      backend._renv.Assign( "u1_angle", { -0.488f } );

      auto& binary = *pbed._builder._binary.get();
      auto& tles = binary._tles;

      backend.SetupInputResult( binary, tiles, resRef );

      /*
      tles:
        0: toplevelblock
        1: weights
        2: midall
        3: result 
      */

      // toplevelを実行
      backend.RunOneTLE( binary, 0 );
      // toplevelのバッファはここで解放されるが、
      // toplevelのバッファはGeneratorのカーネルの名前でしかlookupできなく、
      // これはRunLibraryの中で一時的に作られるだけなのでテストできない

      // midallまで実行
      backend.RunOneTLE( binary, 1 );
      backend.RunOneTLE( binary, 2 );

      // input, weightsは解放されて、midallはまだ。
      REQUIRE( backend.IsBufferReleased( GetFirstInput( binary.Input() ) ) );
      REQUIRE( backend.IsBufferReleased( *tles[1].u._tensor ) );
      REQUIRE( !backend.IsBufferReleased( *tles[2].u._tensor ) );

      // resultまで実行
      backend.RunOneTLE( binary, 3 );

      // midallは解放される、resultはまだ。
      REQUIRE( backend.IsBufferReleased( *tles[2].u._tensor ) );
      REQUIRE( !backend.IsBufferReleased( binary.Result() ) );

      // フィルタ自体が正しく動いているかを確認。
      backend.CopyBackResult( binary, resRef );
      {
        auto locker = PixelLocker( &resTile );

        // Radeon RX 7900はなぜかrとgとb成分が1多くなる。
        // 理由はまだわかってないが、floatでの計算をintにする時に
        // ほぼ正しい値になっているので、とりあえずそういうものとしてテストをff0a0aでもOKとしておく。
        REQUIRE( 0x19fe0909 == resTile.PixelGet( 172, 57 ).Value
                 || 0x19ff0a0a == resTile.PixelGet(172, 57).Value);
      }
    }
    #endif

    #ifdef MFG_BACKEND_ENABLED
    if (SECTION("GPGPUバックエンドで実行")) {SG g;

      TargetBackend backend;
      backend._renv.Assign( "u0_strength", { 10.0f } );
      backend._renv.Assign( "u1_angle", { -0.488f } );

      RunBackend( backend, binary, inputTile, resTile );

      {
        auto locker = PixelLocker( &resTile );
        // Radeon RX 7900はなぜかgとb成分が1多くなる。
        // 理由はまだわかってないが、floatでの計算をintにする時に
        // ほぼ正しい値になっているので、とりあえずそういうものとしてテストをff0a0aでもOKとしておく。
        REQUIRE(0x19fe0909 == resTile.PixelGet(172, 57).Value
          || 0x19ff0a0a == resTile.PixelGet(172, 57).Value);
      }
    }
    #endif
  }
}},
{"レンズぼかしのスクリプトのテスト", []{
  ResetUniqueName();
  std::string src = R"(
@param_i32 radius(SLIDER, label="半径", min=1, max=200, init=5)

let extended = sampler<input_u8>(address=.ClampToBorderValue, border_value=u8[0, 0, 0, 0] )

@bounds(2*radius+1)
def edge |dy1|{
 i32(sqrt( f32(radius^2 - (dy1 - radius)^2) ))
}

let area = edge.sum |i, val| { 2*val+1 }

@bounds(input_u8.extent(0), input_u8.extent(1))
def expbuf |x, y| {
  let [b, g, r, a] = f32(extended(x, y))

  let rcomp = r*r*a
  let gcomp = g*g*a
  let bcomp = b*b*a

  [bcomp, gcomp, rcomp, a]
}

let extendedExp = sampler<expbuf>(address=.ClampToBorderValue, border_value=[0.0, 0.0, 0.0, 0.0] )

@bounds(input_u8.extent(0), input_u8.extent(1))
def sumList |x, y| {
	rsum(0..<2*radius+1, 0..<2*radius+1) |r_edgex, r_edgey|
	{
	  let sx = x+r_edgex-radius
	  let sy = y+r_edgey-radius

	  ifel ( (r_edgex-radius)^2 + (r_edgey-radius)^2 <= radius^2,
		  extendedExp(sx, sy),
		  [0.0, 0.0, 0.0, 0.0]
	  )
	}
}

def result_u8 |x, y|{
  let [b3, g3, r3, a3] = sumList(x, y)/area

  u8[*sqrt([b3, g3, r3]/a3), a3]
}
)";


  // inputTileとresTileの準備
  mfg_pal::Image32 inputTile;
  mfg_pal::Image32 resTile;

  REQUIRE( LoadPngAsTile( "test_data/simple_small.png", inputTile ) );
  resTile.Resize( inputTile.Width(), inputTile.Height() );

  if (SECTION("普通の内部クラス使ったテスト")) {SG g;
    TestPBed pbed( src );
    pbed.BuildIR();

    if (SECTION("テンソルのlifetime解析のテスト")) {SG g;
      auto pbinary = pbed._builder._binary.get();

      /*
      tles
        0: block
        1: edge
        2: block
        3: expbuf
        4: sumList
        5: result
      */

      // input ... 3のexpbufまで
      REQUIRE( GetFirstInput( pbinary->Input() )._lastReferenced == 3 );

      // result, 5より大きい何か。
      REQUIRE( pbinary->Result()._lastReferenced > 5 );

      // edge ... 2のblockまで
      REQUIRE( pbinary->_tles[1].u._tensor->_lastReferenced == 2 );

      // expbuf ... 4のsumListまで
      REQUIRE( pbinary->_tles[3].u._tensor->_lastReferenced == 4 );

      // sumList ... 5のresultまで
      REQUIRE( pbinary->_tles[4].u._tensor->_lastReferenced == 5 );
    }

    #ifdef MFG_BACKEND_ENABLED
    if (SECTION("backendで実行して結果が一致するか")) {SG g;
      TargetBackend backend;
      backend._renv.Assign( "u0_radius", { 10 } );

      RunBackend( backend, pbed.IRBinary(), inputTile, resTile );

      {
        auto guard_r = PixelLocker( &resTile );
        REQUIRE( 0xff6ae017 == resTile.PixelGet( 196, 92 ).Value );
      }
    }
    #endif
  }
  #ifdef MFG_BACKEND_ENABLED
  if (SECTION("facadeを使ったテスト")) {SG g;
    mfg::MFG mfg;
    auto bin = mfg.Compile( src );
    auto& pinfo = bin.GetParamInfo();
    pinfo[0].Assign( 10 );

    RunFacade( mfg, bin, inputTile, resTile );
    
    {
      auto guard_r = PixelLocker( &resTile );
      REQUIRE( 0xff6ae017 == resTile.PixelGet( 196, 92 ).Value );
    }

  }
  #endif

}},
{"LocationMap::Ofのテスト", []{
  if (SECTION("空の文字列でもちゃんと動く")) {SG g;
    auto lm = LocationMap::Of( "" );
    REQUIRE( lm._lineStart.size() == 2 );
  }
  if (SECTION("最初が改行のみ")) {SG g;
    auto lm = LocationMap::Of( "\n" );
    REQUIRE( lm._lineStart.size() == 3 );
    REQUIRE( lm._lineStart[2] == 1 );
  }
  if (SECTION("普通のケース")) {SG g;
    auto lm = LocationMap::Of( R"(
123
45
)" );
    REQUIRE( lm._lineStart.size() == 5 );
    REQUIRE( lm._lineStart[3] == 5 );
    REQUIRE( lm._lineStart[4] == 8 );
  }
}},
{"LocationMap::ToLocationのテスト", []{
  auto lm = LocationMap::Of( R"(
123
45
6)" );
  /* 0, 0, 1, 5, 8 */
  if (SECTION("GetLineAtのテスト")) {SG g;
    REQUIRE( 1 == lm.GetLineAt( 0 ) );
    REQUIRE( 2 == lm.GetLineAt( 1 ) );
    REQUIRE( 2 == lm.GetLineAt( 2 ) );
    REQUIRE( 3 == lm.GetLineAt( 5 ) );
    REQUIRE( 3 == lm.GetLineAt( 6 ) );
    REQUIRE( 4 == lm.GetLineAt( 8 ) );
    REQUIRE( 4 == lm.GetLineAt( 9 ) );
  }
  if (SECTION("サイズ外のexceptionが正しく投げられるか")) {SG g;
    bool thrown = false;
    try
    {
      lm.ToLocation( 10 );
    }
    catch( const InternalError )
    {
      thrown = true;
    }
    REQUIRE( thrown );
  }
  if (SECTION("境界条件、offset=0")) {SG g;
    auto loc = lm.ToLocation( 0 );
    REQUIRE( loc._col == 1 );
    REQUIRE( loc._line == 1 );
  }
  if (SECTION("境界条件、行頭")) {SG g;
    auto loc = lm.ToLocation( 1 );
    REQUIRE( loc._col == 1 );
    REQUIRE( loc._line == 2 );
  }
  if (SECTION("境界条件、行頭の次")) {SG g;
    auto loc = lm.ToLocation( 2 );
    REQUIRE( loc._col == 2 );
    REQUIRE( loc._line == 2 );
  }

}},
{"パースエラーのテスト", []{
  std::string src = R"(

def result hoge |x, y| {
}
)";
  
  TestPBed pbed( src );
  bool thrown = false;
  try
  {
    pbed.ParseAll();
  }
  catch( const ParseError& e )
  {
    thrown = true;
    REQUIRE( e.GetLineNum() == 3 );
    REQUIRE( e.GetColumn() == 12 );
  }
  REQUIRE( thrown );

}},
{"乱数のテスト", []{
  ResetUniqueName();
  std::string src = R"(
@title "砂あらし"

def result_u8 |x, y| {
  ifel( rand() > 0.5, u8[0, 0, 0, 0xff], u8[0xff, 0xff, 0xff, 0xff] )
}
)";
  
  TreeBuilder builder;
  Parser parser( builder, src );
  BuildIR( builder, parser );

  if (SECTION("RandomFinderがrandを見つけるか")) {SG g;
    RandomFinder finder;
    REQUIRE( true == finder.HasRandom( builder._binary->Result() ) );
  }
  #ifdef MFG_BACKEND_METAL
  if (SECTION("rgenの初期化が生成されているか")) {SG g;
    mfg_msl_converter::MSLTensorGenerator tgen( builder._binary->Result() );
    SeedGenerator sgen( 123 );
    tgen.GenFuncPrologue( sgen );
    auto actual = tgen.ToString();
    // cout << actual << endl << endl;

    CheckContains( actual , "RandomGenerator rgen(");
    CheckContains( actual , "rgen.AddNextSeed(grid_idx.x);");
    CheckContains( actual , "rgen.AddNextSeed(grid_idx.y);");
  }
  if (SECTION("Randomのコードが必要な時に生成されているか")) {SG g;
    auto sources = GenShaderSources( *builder._binary.get() );
    for( auto src: sources)
    {
      CheckContains( src, "struct RandomGenerator" );
      CheckNotContains( src, "sort_int32_1d" );
    }
  }
  #endif
  #ifdef MFG_BACKEND_D3D
  if (SECTION("RandomがあるケースでのHLSLの生成")) {SG g;
    mfg_hlsl_converter::HLSLTensorGenerator tgen( builder._binary->Result() );
    tgen.GenAll( builder._binary->GetSeedGen() );
    auto actual = tgen.ToString();
    // cout << actual << endl << endl;

    if (SECTION("AddNextSeedが正しく初期化されているか")){ SG g;
      CheckContains( actual , "RandomGenerator_AddNextSeed( DTid.x );");
      CheckContains( actual , "RandomGenerator_AddNextSeed( DTid.y );");
    }
    if (SECTION("RandomGenerator_FNextが生成されているか")){ SG g;
      CheckContains( actual , "RandomGenerator_FNext()>0.5" );
    }
  }
  if (SECTION("Randomのコードが必要な時に生成されているか")) {SG g;
    auto sources = GenShaderSources( *builder._binary.get() );
    for( auto src: sources)
    {
      CheckContains( src, "uint RandomGenerator_Next" );
    }
  }
  #endif

  if (SECTION("バックエンドで実行して結果を確認")) {SG g;
    mfg_pal::Image32 inputTile;
    mfg_pal::Image32 resTile;

    inputTile.Resize( 100, 100 );
    resTile.Resize( 100, 100 );

    {
      auto guard_i = PixelLocker( &inputTile );
      TBpp32 color;
      color.Value = 0x12345678;
      inputTile.Fill( color );
    }

    auto verifyFunc = [&]() {
      size_t whiteCnt, blackCnt, otherCnt;
      CountSaltPepper( resTile, whiteCnt, blackCnt, otherCnt );
      // cout << blackCnt << "," << whiteCnt << std::endl;
      
      // むちゃくちゃ運が悪いとfailするが
      REQUIRE( blackCnt >= (100*30) );
      REQUIRE( whiteCnt >= (100*30) );
      REQUIRE( otherCnt == 0 );
    };

    #ifdef MFG_BACKEND_ENABLED
    if (SECTION("Backendで実行")) {SG g;
      TargetBackend backend;
      RunBackend( backend, *builder._binary.get(), inputTile, resTile );

      verifyFunc();
    }
    #endif
  }
}},
{"...のパース", []{
  ResetUniqueName();
  std::string src = R"(
def result_u8 |x, y| {
	ifel( x < 2, u8[4, 0, 0, 0], ...)
	elif( y < 3, u8[5, 0, 0, 0], input_u8(x, y))
}
)";

  TreeBuilder builder;
  Parser parser( builder, src );
  parser.ParseAll();

  auto actual = PrintExprS( builder._binary->Result().GetBody() );
  // cout << actual << endl;
  CheckContains( actual, "ifel(u0_x < 2, (u8v4)(tuple(4, 0, 0, 0)), ifel(u1_y < 3, (u8v4)(tuple(5, 0, 0, 0)), input_u8(u0_x, u1_y)))" );
}},
{"ローカルテンソルのテスト", []{
  ResetUniqueName();
  auto src = R"(

@title "ネガポジ反転ローカルバッファ版"
def result_u8 |x, y| {

  @bounds(256)
  def loc |v| {
    u8(255-v)
  }

  let [b, g, r, a] = input_u8(x, y)
  u8[loc(b), loc(g), loc(r), a]
}
)";

  TreeBuilder builder;
  Parser parser( builder, src );

  parser.ParseAll();

  if (SECTION("ExecTensorInitializeとそのloadが生成されているか")) {SG g;
    auto tree = PrintExprS( builder._binary->Result().GetBody()) ;
    // cout << tree << endl;

    CheckContains( tree, "realize_tensor r0: (256)" );
    CheckContains( tree, "r0(u4_g)" );
  }

  if (SECTION("Lower")) {SG g;
    builder._binary->Lower();

    if (SECTION("LowerでExecTensorInitializeがちゃんと展開されているか")) {SG g;
      auto& tltensor = builder._binary->Result();
      auto tree = PrintExprS( tltensor.GetBody() );
      // cout << tree << endl;

      CheckContains( tree, "let extent.r0.0 = 256" );
      CheckContains( tree, "allocate b_r0.0:uint8[256]" );
      CheckContains( tree, "for(u2_v: begin=0, end=extent.r0.0)" );
      CheckContains( tree, "store b_r0.0[u2_v] = uint8(255 - u2_v)" );
    }

    #ifdef MFG_BACKEND_METAL
    if (SECTION("Metalで展開したローカルテンソルが正しく生成されるか")) {SG g;
      mfg_msl_converter::MSLTensorGenerator tgen( builder._binary->Result() );
      tgen.GenKernel( builder._binary->GetSeedGen() );
      auto actual = tgen.ToString();
      // cout << actual << endl << endl;

      size_t pos = 0;
      CheckContains( actual, "uint8_t _b_r0_0[256];", pos );
      CheckContains( actual, "for(int32_t _u2_v = 0; _u2_v < _extent_r0_0; _u2_v++ )", pos );
      CheckContains( actual, "{", pos );
      CheckContains( actual, "_b_r0_0[_u2_v] = ((uint8_t)(255-_u2_v));", pos );
      CheckContains( actual, "}", pos );
    }
    #endif
    #ifdef MFG_BACKEND_D3D
    if (SECTION("D3Dでローカルテンソルの生成が正しいか")) {SG g;
      mfg_hlsl_converter::HLSLTensorGenerator tgen( builder._binary->Result() );
      tgen.GenAll( builder._binary->GetSeedGen() );
      auto actual = tgen.ToString();
      // cout << actual << endl << endl;

      size_t pos = 0;
      CheckContains( actual, "uint _b_r0_0[256];", pos );
      CheckContains( actual, "for(int _u2_v = 0; _u2_v < _extent_r0_0; _u2_v++ )", pos );
      CheckContains( actual, "{", pos );
      CheckContains( actual, "_b_r0_0[_u2_v] = ((uint)(255-_u2_v));", pos );
      CheckContains( actual, "}", pos );
    }
    #endif

    if (SECTION("バックエンドで実行して結果を確認")) {SG g;
      mfg_pal::Image32 inputTile;
      mfg_pal::Image32 resTile;

      SetupNegaPosi( inputTile, resTile );

      #ifdef MFG_BACKEND_ENABLED
      if (SECTION("Backendで動くか？")) {SG g;
        TargetBackend backend;
        builder._binary->LowerAndAnalyze();

        RunBackend( backend, *builder._binary.get(), inputTile, resTile );

        auto guard_r = PixelLocker( &resTile );
        AssertNegaPosi( resTile );
      }
      #endif
    }
  }


}},
{"2次元のローカルテンソルでMetalのテスト", []{
  ResetUniqueName();
  auto src = R"(

@title "ネガポジ反転ローカルバッファ版、RGB"
def result_u8 |x, y| {

  @bounds(256, 3)
  def loc |v, rgb| {
    u8(255-v)
  }

  let [b, g, r, a] = input_u8(x, y)
  [loc(b, 1), loc(g, 1), loc(r, 0), a]
}
)";

  TreeBuilder builder;
  Parser parser( builder, src );

  BuildIR( builder, parser );

  #ifdef MFG_BACKEND_METAL
  if(SECTION("Metalでローカルテンソルのアクセスが一次元化されているか")) {SG g;
    mfg_msl_converter::MSLTensorGenerator tgen( builder._binary->Result() );

    tgen.GenKernel( builder._binary->GetSeedGen() );

    auto actual = tgen.ToString();
    // cout << actual << endl << endl;

    if (SECTION("定義が一次元化されているか？")) {SG g;
      CheckContains( actual, "_b_r0_0[768];" );
    }
    if (SECTION("代入が一次元化されているか？")) {SG g;
      CheckContains( actual, "_b_r0_0[((_u3_rgb*_extent_r0_0)+_u2_v)] =" );
    }
    if (SECTION("extentが生成されているか？")) {SG g;
      CheckContains( actual, "const int32_t _extent_r0_0 = 256;" );
      CheckContains( actual, "const int32_t _extent_r0_1 = 3;" );
    }
    if(SECTION("LoadExprも正しく一次元化されているか")) {SG g;
      CheckContains( actual, "_b_r0_0[clamp(((1*_extent_r0_0)+((int32_t)_u4_b)), 0, ((_extent_r0_1*_extent_r0_0)-1))]" );
    }
  }
  #endif
  #ifdef MFG_BACKEND_ENABLED
  if (SECTION("Backendで動くか？")) {SG g;
    mfg_pal::Image32 inputTile;
    mfg_pal::Image32 resTile;

    SetupNegaPosi( inputTile, resTile );
    TargetBackend backend;

    RunBackend( backend, *builder._binary.get(), inputTile, resTile );

    auto guard_r = PixelLocker( &resTile );
    AssertNegaPosi( resTile );
  }
  #endif  
}},
{"+=のテスト", []{
  ResetUniqueName();
  auto src = R"(

@title "ネガポジの0だけ+=で差し替えた版"
def result_u8 |x, y| {

  @bounds(256)
  def loc |v| {
    u8(255-v)
  }
  mut! loc(255) += 111

  let [b, g, r, a] = input_u8(x, y)
  [loc(b), loc(g), loc(r), a]
}
)";

  TreeBuilder builder;
  Parser parser( builder, src );

  BuildIR( builder, parser );

  if (SECTION("CompoundAssignmentが生成されているか")) {SG g;
    auto tree = PrintExprS( builder._binary->Result().GetBody()) ;
    // cout << tree << endl;

    CheckContains( tree, "mut b_r0.0[clamp(255, 0, extent.r0.0 - 1)] += 111" );
  }
  #ifdef MFG_BACKEND_METAL
  if (SECTION("MetalでのtgenでのMSL生成")) {SG g;
    mfg_msl_converter::MSLTensorGenerator tgen( builder._binary->Result() );
    tgen.GenKernel( builder._binary->GetSeedGen() );
    auto actual = tgen.ToString();
    // cout << actual << endl << endl;

    CheckContains( actual, "_b_r0_0[clamp(255, 0, (_extent_r0_0-1))] += 111;" );
  }
  #endif

  if (SECTION("バックエンドで実行して結果を確認")) {SG g;
    mfg_pal::Image32 inputTile;
    mfg_pal::Image32 resTile;

    SetupNegaPosi( inputTile, resTile );

    auto AssertResult = [&]() {
      auto guard_r = PixelLocker( &resTile );
      // 111 = 0x6f。ネガポジのassertの0を0x6fに置き換えたもの。
      REQUIRE( 0xff6fffff == resTile.PixelGet( 0, 0 ).Value );
      REQUIRE( 0xffff6fff == resTile.PixelGet( 1, 0 ).Value );
      REQUIRE( 0xffffff6f == resTile.PixelGet( 2, 0 ).Value );
      REQUIRE( 0xff11ffff == resTile.PixelGet( 0, 1 ).Value );
      REQUIRE( 0xffff11ff == resTile.PixelGet( 1, 1 ).Value );
      REQUIRE( 0xffffff11 == resTile.PixelGet( 2, 1 ).Value );

    };

    #ifdef MFG_BACKEND_ENABLED
    if (SECTION("Backendで動くか？")) {SG g;
      TargetBackend backend;

      RunBackend( backend, *builder._binary.get(), inputTile, resTile );

      /*
      auto guard_r = PixelLocker( &resTile );
      printf("%x, %x, %x\n", resTile.PixelGet( 0, 0 ).Value, resTile.PixelGet( 1, 0 ).Value, resTile.PixelGet( 2, 0 ).Value );
      printf("%x, %x, %x\n", resTile.PixelGet( 0, 1 ).Value, resTile.PixelGet( 1, 1 ).Value, resTile.PixelGet( 2, 1 ).Value );
      */
      AssertResult();
    }
    #endif
  }

}},
{"ts.for_eachのテスト", []{
  ResetUniqueName();
  auto src = R"(

@title "Rだけヒストグラム"
def result_u8 |x, y| {

  @bounds(256)
  def hist |v| { u8(0) }

  input_u8.for_each |x2, y2, val| {
    let [_, _, r, _] = val
    mut! hist(r) += 1
  }

  let [b, g, _, a] = input_u8(x, y)
  [b, g, hist(x+input_u8.extent(0)*y), a]
}
)";

  TreeBuilder builder;
  Parser parser( builder, src );

  parser.ParseAll();

  if (SECTION("for_eachのツリーが正しく生成されているか")) {SG g;
    auto tree = PrintExprS( builder._binary->Result().GetBody()) ;
    // cout << tree << endl;
    size_t pos = 0;
    CheckContains( tree, "input_u8.for_each", pos );
    CheckContains( tree, "func(u3_x2:int32, u4_y2:int32, u5_val:(u8v4))", pos );
    CheckContains( tree, "{", pos );
    CheckContains( tree, "mut r0(u6_r) += 1", pos );
  }

  if (SECTION("Lower")) {SG g;
    builder._binary->LowerAndAnalyze();

    if (SECTION("for_eachが正しくLowerされているか")) {SG g;
      auto tree = PrintExprS( builder._binary->Result().GetBody()) ;
      // cout << tree << endl;

      size_t pos = 0;
      CheckContains( tree, "for(u3_x2: begin=0, end=extent.input_u8.0)", pos );
      CheckContains( tree, "for(u4_y2: begin=0, end=extent.input_u8.1)", pos );
      CheckContains( tree, "letv u5_val = load:b_input_u8.0[clamp((u4_y2 * extent.input_u8.0) + u3_x2, 0, (extent.input_u8.1 * extent.input_u8.0) - 1)]", pos );
      CheckContains( tree, "let u6_r = u5_val.2", pos );
      CheckContains( tree, "mut b_r0.0[clamp(int32(u6_r), 0, extent.r0.0 - 1)] += 1", pos );
    }

    #ifdef MFG_BACKEND_METAL
    if (SECTION("MetalでのtgenでのMSL生成")) {SG g;
      mfg_msl_converter::MSLTensorGenerator tgen( builder._binary->Result() );
      SeedGenerator sgen( 123 );
      tgen.GenKernel( sgen );
      auto actual = tgen.ToString();
      // cout << actual << endl << endl;

      CheckContains( actual, "for(int32_t _u3_x2 = 0; _u3_x2 < _extent_input_u8_0; _u3_x2++ )" );
      CheckContains( actual, "for(int32_t _u4_y2 = 0; _u4_y2 < _extent_input_u8_1; _u4_y2++ )" );
      CheckContains( actual, "uint4 _u5_val = uint4(_b_input_u8_0[(clamp(((_u4_y2*_extent_input_u8_0)+_u3_x2), 0, ((_extent_input_u8_1*_extent_input_u8_0)-1))*4)], _b_input_u8_0[(clamp(((_u4_y2*_extent_input_u8_0)+_u3_x2), 0, ((_extent_input_u8_1*_extent_input_u8_0)-1))*4)+1], _b_input_u8_0[(clamp(((_u4_y2*_extent_input_u8_0)+_u3_x2), 0, ((_extent_input_u8_1*_extent_input_u8_0)-1))*4)+2], _b_input_u8_0[(clamp(((_u4_y2*_extent_input_u8_0)+_u3_x2), 0, ((_extent_input_u8_1*_extent_input_u8_0)-1))*4)+3]);" );
      CheckContains( actual, "const uint8_t _u5_val_1 = _u5_val.y;" );
      CheckContains( actual, "_b_r0_0[clamp(((int32_t)_u6_r), 0, (_extent_r0_0-1))] += 1;" );


      if (SECTION("extentとtsのリファレンスがある時に重複を除去出来ているか")) {SG g;
        REQUIRE( 1 == CountContains( actual, "int32_t _extent_input_u8_0 =" ) );
      }
    }
    #endif

    if (SECTION("バックエンドで実行")) {SG g;
      mfg_pal::Image32 inputTile;
      mfg_pal::Image32 resTile;

      SetupHist( inputTile, resTile );

      auto AssertResult = [&]() {
        auto guard_r = PixelLocker( &resTile );

        // Rの位置にヒストグラムが入っている。
        // hist = [0, 3, 1, 1, 0, 1];
        REQUIRE( 0xff000000 == resTile.PixelGet( 0, 0 ).Value );
        REQUIRE( 0xff030000 == resTile.PixelGet( 1, 0 ).Value );
        REQUIRE( 0xff010000 == resTile.PixelGet( 2, 0 ).Value );
        REQUIRE( 0xff010000 == resTile.PixelGet( 0, 1 ).Value );
        REQUIRE( 0xff000000 == resTile.PixelGet( 1, 1 ).Value );
        REQUIRE( 0xff010000 == resTile.PixelGet( 2, 1 ).Value );
      };

      #ifdef MFG_BACKEND_ENABLED
      if (SECTION("Backendで動くか？")) {SG g;
        TargetBackend backend;

        RunBackend( backend, *builder._binary.get(), inputTile, resTile );

        AssertResult();
      }
      #endif
    }
  }


}},
{"trans_accmのテスト", []{
  ResetUniqueName();
  auto src = R"(

@title "Rだけヒストグラム"
def result_u8 |x, y| {

  @bounds(256)
  def hist |v| { u8(0) }

  input_u8.for_each |x2, y2, val| {
    let [_, _, r, _] = val
    mut! hist(r) += 1
  }
  mut! trans<hist>.accumulate!(0) |i, h, accm| { h+accm }

  let [b, g, _, a] = input_u8(x, y)
  [b, g, hist(x+input_u8.extent(0)*y), a]
}
)";

  TreeBuilder builder;
  Parser parser( builder, src );

  parser.ParseAll();

  if (SECTION("trans_accmのツリーが正しく生成されているか")) {SG g;
    auto tree = PrintExprS( builder._binary->Result().GetBody()) ;
    // cout << tree << endl;
    size_t pos = 0;
    CheckContains( tree, "r0.trans_accm!(0)", pos );
    CheckContains( tree, "func(u7_i:int32, u8_h:uint8, u9_accm:uint8)", pos );
    CheckContains( tree, "{", pos );
    CheckContains( tree, "u8_h + u9_accm", pos );
  }

  if (SECTION("Lower")) {SG g;
    builder._binary->Lower();

    if (SECTION("Lowerでtrans_accmが正しく展開されているか")) {SG g;
      auto tree = PrintExprS( builder._binary->Result().GetBody()) ;
      // cout << tree << endl;

      size_t pos = 0;
      CheckContains( tree, "var u9_accm = load_loc:b_r0.0[0]", pos );
      CheckContains( tree, "for(u7_i: begin=1, end=extent.r0.0)", pos );
      CheckContains( tree, "let u8_h = load_loc:b_r0.0[clamp(u7_i, 0, extent.r0.0 - 1)]", pos );
      CheckContains( tree, "store u9_accm = u8_h + u9_accm", pos );
      CheckContains( tree, "store b_r0.0[u7_i] = u9_accm", pos );
    }

    #ifdef MFG_BACKEND_METAL
    if (SECTION("MetalでのtgenでのMSL生成")) {SG g;
      mfg_msl_converter::MSLTensorGenerator tgen( builder._binary->Result() );
      SeedGenerator sgen( 123 );
      tgen.GenKernel( sgen );
      auto actual = tgen.ToString();
      // cout << actual << endl << endl;

      size_t pos = 0;
      CheckContains( actual, "uint8_t _u9_accm = _b_r0_0[0];", pos );
      CheckContains( actual, "for(int32_t _u7_i = 1; _u7_i < _extent_r0_0; _u7_i++ )", pos );
      CheckContains( actual, "const uint8_t _u8_h = _b_r0_0[clamp(_u7_i, 0, (_extent_r0_0-1))];" ,pos );
      CheckContains( actual, "_u9_accm = (_u8_h+_u9_accm);", pos );
      CheckContains( actual, "_b_r0_0[_u7_i] = _u9_accm;", pos );
    }
    #endif
    #ifdef MFG_BACKEND_ENABLED
    if (SECTION("バックエンドで実行")) {SG g;
      mfg_pal::Image32 inputTile;
      mfg_pal::Image32 resTile;

      SetupHist( inputTile, resTile );

      TargetBackend backend;
      builder._binary->LowerAndAnalyze();

      RunBackend( backend, *builder._binary.get(), inputTile, resTile );

      // Rの位置に以下のヒストグラムのaccmが入っている。
      // hist = [0, 3, 1, 1, 0, 1];
      // つまり
      // hist_accm = [0, 3, 4, 5, 5, 6];
      auto guard_r = PixelLocker( &resTile );

      REQUIRE( 0xff000000 == resTile.PixelGet( 0, 0 ).Value );
      REQUIRE( 0xff030000 == resTile.PixelGet( 1, 0 ).Value );
      REQUIRE( 0xff040000 == resTile.PixelGet( 2, 0 ).Value );
      REQUIRE( 0xff050000 == resTile.PixelGet( 0, 1 ).Value );
      REQUIRE( 0xff050000 == resTile.PixelGet( 1, 1 ).Value );
      REQUIRE( 0xff060000 == resTile.PixelGet( 2, 1 ).Value );
    }
    #endif
  }

}},
{"cumsumのテスト", []{
  // 一つ前のtrans_accmのテストと同じ内容になるはずのcumsum。
  ResetUniqueName();
  auto src = R"(

@title "Rだけヒストグラム"
def result_u8 |x, y| {

  @bounds(256)
  def hist |_| { u8(0) }

  input_u8.for_each |x2, y2, val| {
    let [_, _, r, _] = val
    mut! hist(r) += 1
  }
  mut! trans<hist>.cumsum!(dim=0)

  let [b, g, _, a] = input_u8(x, y)
  [b, g, hist(x+input_u8.extent(0)*y), a]
}
)";

  TreeBuilder builder;
  Parser parser( builder, src );

  parser.ParseAll();

  if (SECTION("ツリーとしてはtrans_accmが生成される")) {SG g;
    auto tree = PrintExprS( builder._binary->Result().GetBody()) ;
    // cout << tree << endl;
    size_t pos = 0;
    CheckContains( tree, "r0.trans_accm!(0)", pos );
    CheckContains( tree, "func(t1:int32, t2:uint8, t3:uint8)", pos );
    CheckContains( tree, "{", pos );
    CheckContains( tree, "t2 + t3", pos );
  }
  #ifdef MFG_BACKEND_ENABLED
  if (SECTION("バックエンドで実行")) {SG g;
    builder._binary->LowerAndAnalyze();

    // 以下、一つ前のtrans_accmのテストからコピペ。
    mfg_pal::Image32 inputTile;
    mfg_pal::Image32 resTile;

    SetupHist( inputTile, resTile );

    TargetBackend backend;

    RunBackend( backend, *builder._binary.get(), inputTile, resTile );

    auto guard_r = PixelLocker( &resTile );

    REQUIRE( 0xff000000 == resTile.PixelGet( 0, 0 ).Value );
    REQUIRE( 0xff030000 == resTile.PixelGet( 1, 0 ).Value );
    REQUIRE( 0xff040000 == resTile.PixelGet( 2, 0 ).Value );
    REQUIRE( 0xff050000 == resTile.PixelGet( 0, 1 ).Value );
    REQUIRE( 0xff050000 == resTile.PixelGet( 1, 1 ).Value );
    REQUIRE( 0xff060000 == resTile.PixelGet( 2, 1 ).Value );
  }
  #endif
}},
{"def by reduceのテスト", []{
  ResetUniqueName();
  auto src = R"(

def result_u8 |x, y| {

  @bounds(4, 3)
  def tmp |v, rgb| { (v+1)*(rgb+1) }

  # デバッグのためにinitは1から。
  def tmp2 by reduce<tmp>.accumulate(0, 1) |i, rgb, val, accm| {
    accm+val
  }

  ifel( x == 0,
     u8[tmp2(2), tmp2(1), tmp2(0), 0xff],
     u8[0, 0, 0, 0]
  )
}
)";

  TreeBuilder builder;
  Parser parser( builder, src );

  parser.ParseAll();
  if (SECTION("reduceのツリーが正しく生成されているか")) {SG g;
    auto tree = PrintExprS( builder._binary->Result().GetBody()) ;
    // cout << tree << endl;

    size_t pos = 0;
    CheckContains( tree, "def r1 by reduce<r0>.accumulate(dim=0, init=1)", pos );
    CheckContains( tree, "func(u4_i:int32, u5_rgb:int32, u6_val:int32, u7_accm:int32)", pos );
    CheckContains( tree, "{", pos );
    CheckContains( tree, "u7_accm + u6_val", pos );
  }

  if (SECTION("Lower")) {SG g;
    builder._binary->LowerAndAnalyze();

    if (SECTION("Lowerでreduceがちゃんと展開されているか")) {SG g;
      auto& tltensor = builder._binary->Result();
      auto tree = PrintExprS( tltensor.GetBody() );
      // cout << tree << endl;

      size_t pos = 0;
      CheckContains( tree, "let extent.r1.0 = extent.r0.1", pos );
      CheckContains( tree, "allocate b_r1.0:int32[extent.r1.0]", pos );
      CheckContains( tree, "for(u5_rgb: begin=0, end=extent.r0.1)", pos );
      CheckContains( tree, "var u7_accm = 1", pos );
      CheckContains( tree, "let u6_val = load_loc:b_r0.0[clamp((u5_rgb * extent.r0.0) + u4_i, 0, (extent.r0.1 * extent.r0.0) - 1)]", pos );
      CheckContains( tree, "store u7_accm = u7_accm + u6_val", pos );
      CheckContains( tree, "}", pos ); // vblockの閉じカッコ
      CheckContains( tree, "}", pos );
      CheckContains( tree, "store b_r1.0[u5_rgb] = u7_accm", pos );
    }

    #ifdef MFG_BACKEND_METAL
    if (SECTION("MetalでのtgenでのMSL生成")) {SG g;
      mfg_msl_converter::MSLTensorGenerator tgen( builder._binary->Result() );
      SeedGenerator sgen( 123 );
      tgen.GenKernel( sgen );
      auto actual = tgen.ToString();
      // cout << actual << endl << endl;

      size_t pos = 0;
      CheckContains( actual, "const int32_t _extent_r1_0 = _extent_r0_1;", pos );
      CheckContains( actual, "int32_t _b_r1_0[_extent_r1_0];", pos );
      CheckContains( actual, "for(int32_t _u5_rgb = 0; _u5_rgb < _extent_r0_1; _u5_rgb++ )", pos );
      CheckContains( actual, "int32_t _u7_accm = 1;", pos );
      CheckContains( actual, "for(int32_t _u4_i = 0; _u4_i < _extent_r0_0; _u4_i++ )", pos );
      CheckContains( actual, "const int32_t _u6_val = _b_r0_0[clamp(((_u5_rgb*_extent_r0_0)+_u4_i), 0, ((_extent_r0_1*_extent_r0_0)-1))];" ,pos );
      CheckContains( actual, "_u7_accm = (_u7_accm+_u6_val);", pos );
      CheckContains( actual, "_b_r1_0[_u5_rgb] = _u7_accm;", pos );
    }
    #endif
    #ifdef MFG_BACKEND_ENABLED
    if (SECTION("バックエンドで実行")) {SG g;
      mfg_pal::Image32 inputTile;
      mfg_pal::Image32 resTile;

      inputTile.Resize( 3, 2 );
      resTile.Resize( 3, 2 );

      TargetBackend backend;

      RunBackend( backend, *builder._binary.get(), inputTile, resTile );

      /*
        tmp[4][3] = [
          [1, 2, 3],
          [2, 4, 6],
          [3, 6, 9],
          [4, 8, 12]        
        ];

        tmp2[3] = [
          11, = 0xb
          21, = 0x15
          31  = 0x1f
        ]
      */

      auto guard_r = PixelLocker( &resTile );

      // printf("%x\n", resTile.PixelGet( 0, 0 ).Value);
      REQUIRE( 0xff0b151f == resTile.PixelGet( 0, 0 ).Value );
    }
    #endif
  }
}},
{"ローカルテンソルの256x5のテスト", []{
  // メディアンフィルタが動かない時の切り分け用に同じサイズのバッファを使ったテスト。
  ResetUniqueName();
  auto src = R"(

@title "大きめのローカルテンソル"
def result_u8 |x, y| {

  @bounds(256,5)
  def hist |i, col| { i+256*col }

  def accm by reduce<hist>.accumulate(0, 1) | i, rgb, val, accm| { accm+val }

  # accm(0) == 7f81
  # accm(1) == 17f81
  # accm(2) == 27f81
  # accm(3) == 37f81
  # accm(4) == 47f81

  # 0x7f17277e
  u8[(accm(3) >>12)+(accm(4)>>12), accm(2) >>12, accm(1) >> 12, accm(0) >> 8]
}
)";

  TreeBuilder builder;
  Parser parser( builder, src );

  BuildIR( builder, parser );

  #ifdef MFG_BACKEND_ENABLED

  mfg_pal::Image32 inputTile;
  mfg_pal::Image32 resTile;

  inputTile.Resize( 3, 2 );
  resTile.Resize( 3, 2 );

  TargetBackend backend;
  // backend._debug = true;

  RunBackend( backend, *builder._binary.get(), inputTile, resTile );
  auto guard_r = PixelLocker( &resTile );

  // printf("%x\n", resTile.PixelGet( 0, 0 ).Value);
  REQUIRE( 0x7f17277e == resTile.PixelGet( 0, 0 ).Value );
  #endif
}},
{"メディアンフィルタ", []{
  ResetUniqueName();
  auto src = R"(
# 1, 2, 1
# 2, 3, 2
# 1, 2, 1
@bounds(3, 3)
def weight |x, y| {
  3 - abs(x-1) - abs(y-1)
}

# xとyがmedianを求められるように、
# x: 1からw-1
# y: 1からh-1
# の範囲で計算する。
@bounds(input_u8.extent(0)-2, input_u8.extent(1)-2)
def median |x, y| {
  @bounds(256, 4)
  def _hist |i, col| { 0 }

  weight.for_each |ix, iy, wval| {
     let [b, g, r, a] = input_u8(ix+x, iy+y)
     mut! _hist(b, 0) += wval
     mut! _hist(g, 1) += wval
     mut! _hist(r, 2) += wval
     mut! _hist(a, 3) += wval
  }

  # cumsumを求めるが、配列としては_histを上書きしていく（サイズ制限がきついので）
  mut! trans<_hist>.cumsum!(dim=0)

  # hist(255, rgb)は総登場回数（weightはその分重複して登場したと解釈）。その半分で
	def _med by reduce<_hist>.accumulate(dim=0, init=-1) |i, col, val, accm| {
    ifel(accm != -1, accm, ...)
    elif(val < _hist(255, col)/2, -1, i)
  }

  u8[_med(0), _med(1), _med(2), _med(3)]
}

def result_u8 |x, y| {
  ifel( x == 0 || y == 0 || x == input_u8.extent(0)-1 || y == input_u8.extent(1) -1,
      input_u8(x, y),
      median(x-1, y-1))
}
)";

  TreeBuilder builder;
  Parser parser( builder, src );

  BuildIR( builder, parser );

  if (SECTION("!=が正しく生成されているか")) {SG g;
    REQUIRE( builder._binary->ResultDefined() );
    REQUIRE( builder._binary->_tensors.size() == 2 );
    // cout << builder._binary->_tensors[1]->Name() << std::endl;

    auto medianTree = PrintExprS( builder._binary->_tensors[1]->GetBody()) ;
    // cout << medianTree << endl;
    CheckContains( medianTree, "accm != -1" );

  }
  if (SECTION("||の優先順位が正しく処理されているか")) {SG g;
    auto resultTree = PrintExprS( builder._binary->Result().GetBody()) ;
    // cout << resultTree << endl;
    CheckContains( resultTree, "(u17_x == 0) || (u18_y == 0)" );
  }
  #ifdef MFG_BACKEND_METAL
  if (SECTION("medianのMSL生成")) {SG g;
    mfg_msl_converter::MSLTensorGenerator tgen( *builder._binary->_tensors[1] );
    SeedGenerator sgen( 123 );
    tgen.GenKernel( sgen );
    auto actual = tgen.ToString();
    // cout << actual << endl << endl;

    // compound assignmentの2次元はここが初なので一行だけチェックする。
    CheckContains( actual, "_b_r0_0[clamp(((1*_extent_r0_0)+((int32_t)_u10_g)), 0, ((_extent_r0_1*_extent_r0_0)-1))] += _u8_wval;" );

  }
  #endif  
  #ifdef MFG_BACKEND_D3D
  if (SECTION("D3Dの手動実行")) {SG g;
    ReleaseD3DAllDll();

    mfg_pal::Image32 inputTile;
    mfg_pal::Image32 resTile;

    REQUIRE( LoadPngAsTile( "test_data/photo_sp_noise.png", inputTile ) );
    resTile.Resize( inputTile.Width(), inputTile.Height() );

    auto& binary = *builder._binary.get();
    binary._tles[0].u._tensor;

    D3DAllDllLoader d3druntime;
    D3DContext d3dctx;
 
    D3DBuffer weightBuf( d3dctx, sizeof(int32_t), 3*3 );


    {
      HLSLTensorGenerator wgen( *binary._tles[0].u._tensor );
      wgen.GenAll( binary.GetSeedGen() );

      D3DShader wshader( d3dctx, wgen.ToString() );
      std::vector<int32_t> iparam1 { 3, 3 };
      D3DBuffer wParamBuf( d3dctx, sizeof(int32_t), iparam1.size(), iparam1.data() );

      d3dctx.Dispatch( wshader, { &wParamBuf }, {&weightBuf }, 3, 3);

      if (SECTION("weightsの値が正しいか？")) {SG g;
        D3DCpuBuffer debWeights( d3dctx, weightBuf, { 3, 3 } );

        REQUIRE( 1 == debWeights.Int32( 0, 0 ) );
        REQUIRE( 2 == debWeights.Int32( 1, 0 ) );
        REQUIRE( 1 == debWeights.Int32( 2, 0 ) );

        REQUIRE( 2 == debWeights.Int32( 0, 1 ) );
        REQUIRE( 3 == debWeights.Int32( 1, 1 ) );
        REQUIRE( 2 == debWeights.Int32( 2, 1 ) );

        REQUIRE( 1 == debWeights.Int32( 0, 2 ) );
        REQUIRE( 2 == debWeights.Int32( 1, 2 ) );
        REQUIRE( 1 == debWeights.Int32( 2, 2 ) );

        /*
        for( auto i : NRange(3) )
        {
          for( auto j : NRange(3) )
          {
            fprintf( stderr, "%d, %d: %d\n", i, j, debWeights.Int32( i, j ) );
          }
        }
        */
      }      
    }

    #if 0
    // 途中まで調査したコードをしばらく残しておく。
    // 今の所なにが悪いのかは良く分かっていない。
    // 最後の_r3の配列のうち、0, 1, 2, 3のどれかにアクセスする分には正しい値が得られているが、
    // ２つ以上同時にアクセスすると（_r3[0]+_r3[1]など）、その式はいつでも0になってしまう。
    // NaNのような挙動をしているので範囲外アクセスとみなされていると思うのだが、アセンブリを見てもどうしてそうなるのか不明。
    // 生成されるコードとしては正しそうに思えるので一旦調査を打ち切るが、あとで再開するかもしれないのでifdefで調査コードは残しておく。

    D3DBuffer inputBuf = D3DBuffer::FromTile( d3dctx._device, inputTile );

    // input: 640x480
    // median: 638x478
    int32_t medW = inputTile.Width() - 2;
    int32_t medH = inputTile.Height() - 2;
    D3DBuffer medBuf( d3dctx._device, sizeof(uint32_t), medW*medH );


    {
      // 1はTopLevel、手動で設定するので不要なのでスキップして2へ。
      HLSLTensorGenerator medGen( *binary._tles[2].u._tensor );
      medGen.GenAll();

      // cout << medGen.ToString() << endl;

      // D3DShader medShader( d3dctx, medGen.ToString() );


      std::string medSource = R"(
uint argb(uint a, uint r, uint g, uint b) {
  return (a << 24) | (r << 16) | (g << 8) | b;  
}

ByteAddressBuffer paramBuf : register(t0);
ByteAddressBuffer _input_u8: register(t1);
ByteAddressBuffer _weight: register(t2);
RWByteAddressBuffer resultBuf0 : register(u0);

#define _median_extent_0 638
#define _median_extent_1 478
#define _extent_input_u8_0 640
#define _extent_input_u8_1 480


[numthreads(1, 1, 1)]
void CSMain( uint3 DTid : SV_DispatchTHreadID ){
  const int _x = (int)DTid.x;
  const int _y = (int)DTid.y;

    // input: 640x480
    // median: 638x478
  /*
  const int _median_extent_0 = asint(paramBuf.Load(4*0));
  const int _median_extent_1 = asint(paramBuf.Load(4*1));
  const int _extent_input_u8_0 = asint(paramBuf.Load(4*2));
  const int _extent_input_u8_1 = asint(paramBuf.Load(4*3));
  const int _weight_extent_0 = asint(paramBuf.Load(4*4));
  const int _weight_extent_1 = asint(paramBuf.Load(4*5));
  */
  // const int _median_extent_0 = 638;
  // const int _median_extent_1 = 478;
  // const int _extent_input_u8_0 = 640;
  // const int _extent_input_u8_1 = 480;
  const int _weight_extent_0 = 3;
  const int _weight_extent_1 = 3;
  const int _extent_r0_0 = 256;
  const int _extent_r0_1 = 4;
  int _r0[1024];
  for(int _i = 0; _i < 0 + _extent_r0_0; _i++ )
  {
    for(int _col = 0; _col < 0 + _extent_r0_1; _col++ )
    {
      _r0[((_col*_extent_r0_0)+_i)] = 0;
    }
  }
  for(int _ix = 0; _ix < 0 + _weight_extent_0; _ix++ )
  {
    for(int _iy = 0; _iy < 0 + _weight_extent_1; _iy++ )
    {
const int _wval = asint(_weight.Load(4*((_iy*_weight_extent_0)+_ix)));
const uint _t2 = asuint(_input_u8.Load(4*(((_iy+_y)*_extent_input_u8_0)+(_ix+_x))));
const int _a = ((int)(255&(_t2>>24)));
const int _r = ((int)(255&(_t2>>16)));
const int _g = ((int)(255&(_t2>>8)));
const int _b = ((int)(255&_t2));
_r0[((0*_extent_r0_0)+_a)] += _wval;
_r0[((1*_extent_r0_0)+_r)] += _wval;
_r0[((2*_extent_r0_0)+_g)] += _wval;
_r0[((3*_extent_r0_0)+_b)] += _wval;


    }
  }
  for(int _col = 0; _col < 0 + _extent_r0_1; _col++ )
  {
    int _accm = _r0[((_col*_extent_r0_0)+0)];
    for(int _i = 1; _i < 1 + (_extent_r0_0-1); _i++ )
    {
      const int _val = _r0[((_col*_extent_r0_0)+_i)];
      _accm = (_val+_accm);
      _r0[((_col*_extent_r0_0)+_i)] = _accm;
    }
  }
  const int _extent_r3_0 = _extent_r0_1;
  // int _r3[_extent_r3_0];
  // int _r3[4];
  // uint _r3[4];
  // uint4 _r3;
  // min16uint4 _r3;
  uint _r3[4];
  for(int _col2 = 0; _col2 < 0 + _extent_r0_1; _col2++ )
  {
    int _accm = (0-1);
    for(int _i = 0; _i < 0 + _extent_r0_0; _i++ )
    {
      const int _val = _r0[((_col2*_extent_r0_0)+_i)];
      _accm = ((_accm!=(0-1)) ? _accm : ((_val<(_r0[((_col2*_extent_r0_0)+255)]/2)) ? (0-1) : _i));
    }
    _r3[_col2] = _accm;
  }
  // r3 = 0 2f 32 32
  uint res = argb( _r3[0], _r3[1], _r3[2], _r3[3] );
  // OK uint res = argb( _r3[1], _r3[1], _r3[1], _r3[1] );
  // OK uint res = argb( _r3[0], _r3[0], _r3[0], _r3[0] );
  // NG uint res = argb( _r3[0], _r3[0], _r3[0], _r3[1] );
  // uint res = _r3[0]+_r3[1];
  resultBuf0.Store(((_y*_median_extent_0)+_x)*4, res );
}
)";

      D3DShader medShader( d3dctx, medSource, true );
      std::vector<int32_t> iparam { medW, medH, inputTile.Width(), inputTile.Height(), 3, 3 };
      D3DBuffer paramBuf( d3dctx._device, sizeof(int32_t), iparam.size(), iparam.data() );

      d3dctx.Dispatch( medShader, { &paramBuf, &inputBuf, &weightBuf }, {&medBuf }, medW, medH );

      TestCPUBuffer debMed( d3dctx, medBuf, { (size_t)medW, (size_t)medH } );

      fprintf( stderr, "%x\n", debMed.Value<uint32_t>( 386, 219 ) );
      fprintf( stderr, "%x\n", debMed.Value<uint32_t>( 386+1, 219+1 ) );
    }
    #endif


  }
  #endif
  // D3Dでは動いていない。原因究明中
  // #ifdef MFG_BACKEND_ENABLED
  #ifdef MFG_BACKEND_METAL

    // 387x220、0オリジンで386x219のソルトが消えているか。
    auto assertInput = [&]( mfg_pal::Image32& inputTile ) {
      // 2a2c2b
      auto guard_s = PixelLocker( &inputTile );

      /*
      printf("%x\n", inputTile.PixelGet( 386, 219 ).Value);
      */
      REQUIRE( 0xffffffff == inputTile.PixelGet( 386, 219 ).Value );
    };
    auto assertResult = [&]( mfg_pal::Image32& resTile ) {
      auto guard_r = PixelLocker( &resTile );

      /*
        printf("%x\n", resTile.PixelGet( 386, 219 ).Value);
      */
      REQUIRE( 0xff2a2c2b == resTile.PixelGet( 386, 219 ).Value );
    };

  if (SECTION("バックエンドで実行")) {SG g;
    mfg_pal::Image32 inputTile;
    mfg_pal::Image32 resTile;

    REQUIRE( LoadPngAsTile( "test_data/photo_sp_noise.png", inputTile ) );
    resTile.Resize( inputTile.Width(), inputTile.Height() );


    TargetBackend backend;
    RunBackend( backend, *builder._binary.get(), inputTile, resTile );
    // REQUIRE( SaveTileAsPng( "tmp_test_median.png", resTile ) );
    assertInput( inputTile );
    assertResult( resTile );

    if (SECTION("PerfInfoがちゃんと測れているか")){SG g;
      auto info = backend.GetPerfInfo();
      REQUIRE( info._inputCopy > 0 );
      REQUIRE( info._kernelRun > 0 );
      REQUIRE( info._resultCopy > 0 );
    }
  }
  #endif
}},
{"名前無し引数の解決のテスト", []{
  // 名前つき引数の導入にあたり、名前無しの場合が壊れていない事を確認するテスト。
  ResetUniqueName();

  auto src = R"(
def result_u8 |x, y| {

  @bounds(256, 3)
  def hist |i, col| { 0 }

  # 引数の解決をテストしたいだけなので、コードに意味は無い。
  # テスト目的に0でない数字を指定しておく。
  mut! trans<hist>.accumulate!(1) |i, col, h, accm| { h+accm }

  def tmp2 by reduce<hist>.accumulate(0, 1) |i, col, val, accm | { accm + val }
  u8[0xff, 0xff, 0xff, 0xff]
}
  )";

  auto tree = ParseAndReturnTreeDump( src );
  // cout << tree << endl;

  if (SECTION("trans<>.accumulateの引数が正しいか")) {SG g;
    CheckContains(tree, "accm!(1)" );
  }

  if (SECTION("reduce<>.accumulateの引数が正しいか")) {SG g;
    CheckContains(tree, "reduce<r0>.accumulate(dim=0, init=1)" );
  }
}},
{"名前つき引数の解決のテスト", []{
  ResetUniqueName();

  // 名前無し引数版とほとんど同じ
  auto src = R"(
def result_u8 |x, y| {

  @bounds(256, 3)
  def hist |i, col| { 0 }

  mut! trans<hist>.accumulate!(dim=1) |i, col, h, accm| { h+accm }

  def tmp2 by reduce<hist>.accumulate(init=1, dim=0) |i, col, val, accm | { accm + val }
  u8[0xff, 0xff, 0xff, 0xff]
}
  )";

  auto tree = ParseAndReturnTreeDump( src );
  // cout << tree << endl;

  if (SECTION("trans<>.accumulateの引数が正しいか")) {SG g;
    CheckContains(tree, "accm!(1)" );
  }

  if (SECTION("reduce<>.accumulateの引数が（順番も）正しいか")) {SG g;
    CheckContains(tree, "reduce<r0>.accumulate(dim=0, init=1)" );
  }
}},
{"名前つき引数と名前無し引数が両方あるケースのテスト", []{
  ResetUniqueName();

  // 名前無し引数版とほとんど同じ
  auto src = R"(
def result_u8 |x, y| {

  @bounds(256, 3)
  def hist |i, col| { 0 }

  def tmp2 by reduce<hist>.accumulate(init=1, 0) |i, col, val, accm | { accm + val }
  u8[0xff, 0xff, 0xff, 0xff]
}
  )";

  auto tree = ParseAndReturnTreeDump( src );
  // cout << tree << endl;

  if (SECTION("reduce<>.accumulateの引数が（順番も）正しいか")) {SG g;
    CheckContains(tree, "reduce<r0>.accumulate(dim=0, init=1)" );
  }
}},
{"BlankId、Tensorのケース", []{
  ResetUniqueName();

  auto parseAndReturnArgs = []( const std::string& src ) {
    TestPBed pbed( src );
    pbed.ParseAll();


    auto& result = pbed.Result();
    return result.GetTensorElem()->_args;
  };

  if (SECTION("両方_のケース")) {SG g;
    std::string ssrc = R"(
      def result_u8 |_, _| {
        u8[0, 0, 0, 0]
    })";
    auto args = parseAndReturnArgs( ssrc );

    // UniqueNameの名前づけに依存したテスト、以下同じ
    REQUIRE( args[0].first == "t0" );
    REQUIRE( args[1].first == "t1" );
  }  

  if (SECTION("1つ目が_のケース")) {SG g;
    std::string ssrc = R"(
      def result_u8 |_, y| {
        u8[0, 0, 0, 0]
    })";
    auto args = parseAndReturnArgs( ssrc );

    REQUIRE( args[0].first == "t0" );
    REQUIRE( args[1].first == "u0_y" );
  }  

  if (SECTION("2つ目が_のケース")) {SG g;
    std::string ssrc = R"(
      def result_u8 |x, _| {
        u8[0, 0, 0, 0]
    })";
    auto args = parseAndReturnArgs( ssrc );

    REQUIRE( args[0].first == "u0_x" );
    REQUIRE( args[1].first == "t0" );
  }  
}},
{"BlankId、LocalTensorのケース", []{
  ResetUniqueName();

  // 名前無し引数版とほとんど同じ
  auto src = R"(
def result_u8 |x, y| {

  @bounds(256, 3)
  def hist |_, _| { 0 }

  u8[0xff, 0xff, 0xff, 0xff]
}
  )";

  auto tree = ParseAndReturnTreeDump( src );
  // cout << tree << endl;

  // ローカルテンソルは置き換え忘れさえしてなければTensorの方と同じ処理が走るので、
  // テストとしては一つのケースが動いていれば十分。
  CheckContains( tree, "func(t0:int32, t1:int32)" );
}},
// blank idのテストの前に通常のTupleLetのテストから
{"Letでinput_u8をタプルにバラす時のテスト", []{
  ResetUniqueName();
  auto src = R"(
def result_u8 |x, y| {
  let [b, g, r, a] = input_u8(x, y)
  u8[b, g, r, a]
}
  )";

  auto tree = ParseAndLowerAndReturnTreeDump( src );
  // cout << tree << endl;

  CheckContains( tree, "letv t0 = load:b_input_u8.0[clamp((u1_y * extent.input_u8.0) + u0_x, 0, (extent.input_u8.1 * extent.input_u8.0) - 1)]");
  CheckContains( tree, "let u2_b = t0.0");
  CheckContains( tree, "let u3_g = t0.1");
  CheckContains( tree, "let u4_r = t0.2");
  CheckContains( tree, "let u5_a = t0.3");
}},
{"Blank Id, Letでinput_u8をタプルにバラす時のテスト", []{
  ResetUniqueName();
  auto src = R"(
def result_u8 |x, y| {
  let [_, g, _, a] = input_u8(x, y)
  u8[a, g, a, g]
}
  )";
  auto tree = ParseAndLowerAndReturnTreeDump( src );
  // cout << tree << endl;

  CheckContains( tree, "letv t0 = load:b_input_u8.0[clamp((u1_y * extent.input_u8.0) + u0_x, 0, (extent.input_u8.1 * extent.input_u8.0) - 1)]");
  CheckNotContains( tree, "_b = t0.0");
  CheckContains( tree, "let u2_g = t0.1");
  CheckNotContains( tree, "_r = t0.2");
  CheckContains( tree, "let u3_a = t0.3");
}},
{"Blank Id, Letでタプルをバラす時のテスト", []{
  ResetUniqueName();
  auto src = R"(
def result_u8 |x, y| {
  let [_, r, _, b] = [1, 2, 3, 4]  
  u8[r, b, r, b]
}
  )";
  auto tree = ParseAndLowerAndReturnTreeDump( src );
  // cout << tree << endl;

  CheckContains( tree, "let u2_r = 2");
  CheckContains( tree, "let u3_b = 4");
}},
{"Blank Id, rsumのテスト", []{
  ResetUniqueName();
  auto src = R"(
    def result_u8 |x, y| {
      rsum(0..<3, 0..<5) |_, _| { u8[1, 2, 3, 4] }
    } 
  )";
  

  auto tree = ParseAndReturnTreeDump( src );
  // cout << tree << endl;

  CheckContains( tree, "func(t0:int32, t1:int32)" );
}},
{"rsumの引数がprefixされるかのテスト", []{
  ResetUniqueName();
  auto src = R"(
    def result_u8 |x, y| {
      rsum(0..<3, 0..<5) |rx, ry| { u8[rx, ry, 3, 4] }
    } 
  )";
  

  auto tree = ParseAndReturnTreeDump( src );
  // cout << tree << endl;

  CheckContains( tree, "func(u2_rx:int32, u3_ry:int32)" );
  CheckContains( tree, "tuple(u2_rx, u3_ry" );
}},
{"blank id、ts.for_eachのテスト", []{
  ResetUniqueName();

  if (SECTION("blank id無しのケース")) {SG g;
    // 本当はいらないケースだが、
    // BITWISE_ANDが未実装だったのでそのテストケースとして残しておく。

    auto src = R"(

  def result_u8 |x, y| {

    @bounds(256)
    def hist |v| { 0 }

    input_u8.for_each |x2, y2, val| {
      let [b, _, _, _] = val
      mut! hist( b&255 ) += 1
    }

    u8[3, 3, 3, 3]
  }
  )";

    auto tree = ParseAndReturnTreeDump( src );
    // cout << tree << endl;
    CheckContains( tree, "int32(u6_b) & 255" );
  }

  if (SECTION("ループ変数がblank idのケース")) {SG g;
    auto src = R"(

  def result_u8 |x, y| {

    @bounds(256)
    def hist |v| { 0 }

    input_u8.for_each |_, _, val| {
      let [b, _, _, _] = val
      mut! hist( b ) += 1
    }

    u8[3, 3, 3, 3]
  }
  )";

    auto tree = ParseAndReturnTreeDump( src );
    // cout << tree << endl;

    // ループ変数はパース時に置き換える
    CheckContains( tree, "func(t0:int32, t1:int32" );
  }

  if (SECTION("値の変数がblank idのケース")) {SG g;
    auto src = R"(

  def result_u8 |x, y| {

    @bounds(256)
    def hist |v| { 0 }

    input_u8.for_each |ix, iy, _| {
      mut! hist( 255 ) += ix
    }

    u8[3, 3, 3, 3]
  }
  )";

    auto tree = ParseAndReturnTreeDump( src );

    // cout << tree << endl;

    // 値の変数はパース時にはそのままでLowerで処理
    CheckContains( tree, "func(u3_ix:int32, u4_iy:int32, _:(u8v4))" );

    if (SECTION("値の_はLowerで消す")) {SG g;
      auto tree = ParseAndLowerAndReturnTreeDump( src );
      // cout << tree << endl;

      // 以下が誤って生成されてないかをチェック
      // let _ = load:b_i1[((iy * i1.extent.0) + ix)]
      CheckNotContains( tree, "load:b_i1" );
    }
  }

}},
{"blank id、ts.sumのテスト", []{
  ResetUniqueName();

  if (SECTION("ループ変数はパース時にtmp変数に置き換え")) {SG g;
    auto src = R"(
  def result_u8 |x, y| {
    input_u8.sum |_, _, val| { 2*val }
  }
  )";

    auto tree = ParseAndReturnTreeDump( src );
    // cout << tree << endl;

    CheckContains( tree, "func(t0:int32, t1:int32, u2_val" );
  }

  if (SECTION("値変数はパース時にはそのままで、Lower時に置き換え")) {SG g;
    auto src = R"(
  def result_u8 |x, y| {
    input_u8.sum |ix, iy, _| { u8[2+ix, 0, 0, 0] }
  }
  )";

    auto tree = ParseAndReturnTreeDump( src );
    // cout << tree << endl;

    // 値の変数はパース時にはそのままでLowerで処理
    CheckContains( tree, "func(u2_ix:int32, u3_iy:int32, _:(u8v4))" );

    if (SECTION("値変数をLowerで処理しているか")) {SG g;
      auto tree = ParseAndLowerAndReturnTreeDump( src );
      // cout << tree << endl;

      // 以下が誤って生成されてないかをチェック
      // let _ = load:b_input_u8[((iy * extent.input_u8.0) + ix)]
      CheckNotContains( tree, "load:b_input_u8" );
    }
  }
}},
#ifdef MFG_BACKEND_ENABLED
{"blank id, input_u8.sumをバックエンドでも実行するテスト", []{
  ResetUniqueName();
  auto src = R"(
  def result_u8 |x, y| {
    input_u8.sum |_, _, val| { 2*val }
  }
  )";

  TestPBed pbed( src );
  pbed.BuildIR();

  mfg_pal::Image32 inputTile;
  mfg_pal::Image32 resTile;

  inputTile.Resize( 2, 1 );
  resTile.Resize( 2, 1 );

  {
    auto guard = PixelLocker( &inputTile );
    TBpp32 color;
    color.Value = 1;
    inputTile.PixelSet( 0, 0, color );
    color.Value = 2;
    inputTile.PixelSet( 1, 0, color );
  }

  TargetBackend backend;
  // backend._debug = true;

  RunBackend( backend, pbed.IRBinary(), inputTile, resTile );

  auto guard_r = PixelLocker( &resTile );
  REQUIRE( resTile.PixelGet( 0, 0 ).Value == 2+4 );
}},
#endif
{"blank id、trans<ts>.accumulateのテスト", []{
  ResetUniqueName();

    if (SECTION("ループ変数はパース時に一時変数へ")) {SG g;
      auto src = R"(

    def result_u8 |x, y| {

      @bounds(256)
      def hist |v| { 0 }

      mut! trans<hist>.accumulate!(dim=0) |_, count, accm| { count+accm }

      u8[3, 3, 3, 3]
    }
    )";

      auto tree = ParseAndReturnTreeDump( src );
      CheckContains( tree, "func(t0:int32, u3_count" );
    }

    if (SECTION("value変数はパース時はそのまま残す")) {SG g;
      auto src = R"(

    def result_u8 |x, y| {

      @bounds(256)
      def hist |v| { 0 }

      mut! trans<hist>.accumulate!(dim=0) |i, _, accm| { i+accm }

      u8[3, 3, 3, 3]
    }
    )";

      auto tree = ParseAndReturnTreeDump( src );
      CheckContains( tree, "func(u3_i:int32, _:int32" );

      if (SECTION("Lowerでvalueの生成をスキップしているか")) {SG g;
        auto tree = ParseAndLowerAndReturnTreeDump( src );
        // cout << tree << endl;

        CheckNotContains( tree, "let _ =" );
      }
    }
}},
{"linear2gammaの簡単なテスト", []{
  ResetUniqueName();
  auto src = R"(

def result_u8 |x, y| {

  let bgra = to_ncolor(input_u8(x, y))

  let res = linear2gamma(bgra.xyz)
  to_u8color([*res, bgra.w])
}
)";
  auto tree = ParseAndLowerAndReturnTreeDump( src );
  // cout << tree << endl;

  // 0.05のベクトライズが効いているかを確認するため、t4.2をチェックする
  CheckContains( tree, " = ifel(t4.2, (1.05 * pow(i1_v0.2, 1.00 / 2.40)) - 0.05" );
}},
{"blank id、def by reduceのテスト", []{
  if (SECTION("ループ変数はパース時に一時変数へ")) {SG g;
    auto src = R"(

  def result_u8 |x, y| {

    @bounds(4, 3)
    def tmp |a, b| { 1 }

    def tmp2 by reduce<tmp>.accumulate(dim=0, init=1) |_, _, val, accm| {
      accm+val
    }

    u8[0, 0, 0, 0]
  }
  )";
    auto tree = ParseAndReturnTreeDump( src );

    CheckContains( tree, "func(t0:int32, t1:int32, u4_val" );
  }
  if (SECTION("value変数はパース時はそのまま残す")) {SG g;
    auto src = R"(

  def result_u8 |x, y| {

    @bounds(4, 3)
    def tmp |a, b| { 1 }

    def tmp2 by reduce<tmp>.accumulate(dim=0, init=1) |a, b, _, accm| {
      a+accm
    }

    u8[0, 0, 0, 0]
  }
  )";

    auto tree = ParseAndReturnTreeDump( src );
    CheckContains( tree, "func(u4_a:int32, u5_b:int32, _:int32" );

    if (SECTION("Lowerでvalueの生成をスキップしているか")) {SG g;
      auto tree = ParseAndLowerAndReturnTreeDump( src );
      // cout << tree << endl;

      CheckNotContains( tree, "let _ =" );
    }
  }
}},
{"def by reduceで0次元になるケース", []{
  ResetUniqueName();
  auto src = R"(

def result_u8 |x, y| {

  @bounds(10)
  def tmp |v| { 
    sin(f32(v))
  }

  # 最初に負になる整数。pi以上なので4。
  def tmp2 by reduce<tmp>.accumulate(dim=0, init=-1) |i, val, accm| {
    ifel(accm != -1, accm, ...)
    elif(val < 0, i, -1)
  }

  u8[tmp2, 0, 0, 0]
}
)";

  TreeBuilder builder;
  Parser parser( builder, src );

  BuildIR( builder, parser );
  if (SECTION("Lowerの結果が期待通りか")) {SG g;
    auto tree = PrintExprS( builder._binary->Result().GetBody()) ;
    // cout << tree << endl;

    size_t pos = 0;
    CheckContains( tree, "var u5_accm = -1", pos );
    CheckContains( tree, "for(u3_i: begin=0, end=extent.r0.0)", pos );
    CheckContains( tree, "let rres.r1.0 = u5_accm", pos );
  }
  #ifdef MFG_BACKEND_ENABLED
  if (SECTION("バックエンドで実行")) {SG g;
    mfg_pal::Image32 inputTile;
    mfg_pal::Image32 resTile;

    inputTile.Resize( 2, 2 );
    resTile.Resize( 2, 2 );

    TargetBackend backend;
    // backend._debug = true;

    RunBackend( backend, *builder._binary.get(), inputTile, resTile );

    auto guard_r = PixelLocker( &resTile );
    // printf("%x\n", resTile.PixelGet( 0, 0 ).Value);
    REQUIRE( 4 == resTile.PixelGet( 0, 0 ).Value );
  }
  #endif
}},
{"trans<TS>.sort!のテスト", []{
  ResetUniqueName();
  auto src = R"(

def result_u8 |x, y| {

  @bounds(10)
  def tmp |v| { 
    i32(10*sin(f32(v)))
  }

  mut! trans<tmp>.sort!(dim=0)

  u8[tmp(x%10), 0, 0, 0]
}
)";

  TreeBuilder builder;
  Parser parser( builder, src );

  BuildIR( builder, parser );

  if (SECTION("sortのcallstmtが正しく生成されているか")) {SG g;
    auto tree = PrintExprS( builder._binary->Result().GetBody()) ;
    // cout << tree << endl;

    CheckContains( tree, "callstmt: sort_int32_1d(b_r0.0, 1, extent.r0.0)" );
  }

  #ifdef MFG_BACKEND_METAL
  if (SECTION("sort_int32_1dのコール文が生成されているか")) {SG g;
    mfg_msl_converter::MSLTensorGenerator tgen( builder._binary->Result() );
    tgen.GenKernel( builder._binary->GetSeedGen() );
    auto actual = tgen.ToString();
    // cout << actual << endl << endl;

    CheckContains( actual , "sort_int32_1d(_b_r0_0, 1, _extent_r0_0);");
  }
  if (SECTION("sort_int32_1dのコードが必要な時に生成されているか")) {SG g;
    auto sources = GenShaderSources( *builder._binary.get() );
    for( auto src: sources)
    {
      CheckNotContains( src, "struct RandomGenerator" );
      CheckContains( src, "sort_int32_1d" );
    }
  }
  #endif
  #ifdef MFG_BACKEND_ENABLED
  if (SECTION("Backendで実行")) {SG g;
    mfg_pal::Image32 inputTile;
    mfg_pal::Image32 resTile;

    inputTile.Resize( 10, 1 );
    resTile.Resize( 10, 1 );

    TargetBackend backend;
    // backend._debug = true;

    RunBackend( backend, *builder._binary.get(), inputTile, resTile );

    auto guard_r = PixelLocker( &resTile );

    /*
    Pythonで試すと以下。
    >>> vals = [int(10*math.sin(i)) for i in range(10)]
    >>> vals.sort()
    >>> vals
    [-9, -7, -2, 0, 1, 4, 6, 8, 9, 9]
    */

    /*
    for (auto i : NRange(10))
      printf("i=%d: %d\n", i, (int)resTile.PixelGet( i, 0 ).Value);
    */

    REQUIRE( 256-9 == (int)resTile.PixelGet(0, 0).Value );
    REQUIRE( 256-7 == (int)resTile.PixelGet(1, 0).Value );
    REQUIRE( 0 == (int)resTile.PixelGet(3, 0).Value );
    REQUIRE( 9 == (int)resTile.PixelGet(9, 0).Value );
  }
  #endif
}},
{"メディアンフィルタ、sort版", []{
  ResetUniqueName();
  auto src = R"(
# 1, 2, 1
# 2, 3, 2
# 1, 2, 1
@bounds(3, 3)
def weight |x, y| {
  3 - abs(x-1) - abs(y-1)
}

# let wsum = weight.sum |x, y, val| { val }

# xとyがmedianを求められるように、
# x: 1からw-1
# y: 1からh-1
# の範囲で計算する。
@bounds(input_u8.extent(0)-2, input_u8.extent(1)-2)
def median |x, y| {

	@bounds(9)
  def wcumsum |i| {
     let [ix, iy] = [i%3, i/3]
     weight(ix, iy)
  }

  mut! trans<wcumsum>.cumsum!(dim=0)

  # @bounds(wsum) まだIMMしかサポートしてない。ここは見直しても良い気がする。
  @bounds(15)
  def wmat |i| {
     let i3 = reduce<wcumsum>.find_first_index(dim=0) |_, val| { i < val }
     let [ix, iy] = [i3%weight.extent(0), i3/weight.extent(0)]
     i32(input_u8(ix+x, iy+y))
  }
  
  mut! trans<wmat>.sort!(dim=0)

  u8(wmat(wmat.extent(0)/2))
}

def result_u8 |x, y| {
  ifel( x == 0 || y == 0 || x == input_u8.extent(0)-1 || y == input_u8.extent(1) -1,
			input_u8(x, y),
			median(x-1, y-1))
}  
)";

  TreeBuilder builder;
  Parser parser( builder, src );

  BuildIR( builder, parser );

  #ifdef MFG_BACKEND_METAL
  if (SECTION("medianのMSL生成のテスト")) {SG g;
    // 0: result, 1: weight, 2: median
    mfg_msl_converter::MSLTensorGenerator tgen( *builder._binary->_tensors[1].get() );
    tgen.GenKernel( builder._binary->GetSeedGen() );
    auto actual = tgen.ToString();
    // cout << actual << endl << endl;

    if (SECTION("タプルのallocateが期待通り生成されているか")) {SG g;
      // ベクトルバッファ
      CheckContains( actual , "int32_t _b_r3_0[(15*4)];");
    }
    if (SECTION("タプルのstoreが期待通り生成されているか")) {SG g;
      CheckContains( actual , "_b_r3_0[(_u7_i*4)] = ((int32_t)_t8_0);");
      CheckContains( actual , "_b_r3_0[(_u7_i*4)+3] = ((int32_t)_t8_3);");
    }
    if (SECTION("タプルのsortが期待通り生成されているか")) {SG g;
      CheckContains( actual , "sort_int32_1d(_b_r3_0, 4, _extent_r3_0);");
    }
  }
  #endif
  #ifdef MFG_BACKEND_D3D
  if (SECTION("medianのHLSLにsortのコードが正しく生成されているか")) {SG g;
    // 0: result, 1: weight, 2: median
    mfg_hlsl_converter::HLSLTensorGenerator tgen( *builder._binary->_tensors[1].get() );
    tgen.GenAll( builder._binary->GetSeedGen() );
    auto actual = tgen.ToString();
    // cout << actual << endl << endl;

    // ほとんどハードコードなので目視で良さそうかしか確認出来ない。
    // とりあえず一部が正しく出ている事を一応チェックしておく。
    CheckContains( actual, "for( int j = 1; j < _extent_r3_0;" );
    // ベクトル
    CheckContains( actual, "for( int v = 0; v < 4; v++ ) {" );
  }
  #endif


  #ifdef MFG_BACKEND_ENABLED
  // 以下は以前のメディアンフィルタと同じverify。
  if (SECTION("バックエンドで実行して結果があうか")) {SG g;
    mfg_pal::Image32 inputTile;
    mfg_pal::Image32 resTile;

    REQUIRE( LoadPngAsTile( "test_data/photo_sp_noise.png", inputTile ) );
    resTile.Resize( inputTile.Width(), inputTile.Height() );


    TargetBackend backend;
    // backend._debug = true;

    RunBackend( backend, *builder._binary.get(), inputTile, resTile );
    // REQUIRE( SaveTileAsPng( "tmp_test_median.png", resTile ) );
    auto guard_r = PixelLocker( &resTile );

    REQUIRE( 0xff2a2c2b == resTile.PixelGet( 386, 219 ).Value );
  }
  #endif
}},
{"find_first_indexのテスト", []{
  ResetUniqueName();

  // def by reduceの0次元のテストから持ってきた
  auto src = R"(

def result_u8 |x, y| {

  @bounds(10)
  def tmp |v| { 
    sin(f32(v))
  }

  # 最初に負になる整数。pi以上なので4。
  def tmp2 by reduce<tmp>.find_first_index(dim=0)  |_, val| { val < 0 }

  u8[tmp2, 0, 0, 0]
}
)";

  TreeBuilder builder;
  Parser parser( builder, src );


  if (SECTION("パースの結果適切なreduce accmに展開されているか")) {SG g;
    parser.ParseAll();

    auto tree = PrintExprS( builder._binary->Result().GetBody()) ;
    // cout << tree << endl;

    size_t pos = 0;
    CheckContains( tree, "let u4_tmp2 = reduce_to_scalar<r0>.accumulate(dim=0, init=-1, block=", pos );
    CheckContains( tree, "func(t0:int32, u3_val:float32, t1:int32)", pos );
    CheckContains( tree, "ifel(t1 != -1, t1, ifel(u3_val < float32(0), t0, -1))", pos );
  }

  // def by reduceの0次元のテストから持ってきた
  #ifdef MFG_BACKEND_ENABLED
  if (SECTION("バックエンドで実行")) {SG g;
    BuildIR( builder, parser );

    mfg_pal::Image32 inputTile;
    mfg_pal::Image32 resTile;

    inputTile.Resize( 2, 2 );
    resTile.Resize( 2, 2 );

    TargetBackend backend;
    // backend._debug = true;

    RunBackend( backend, *builder._binary.get(), inputTile, resTile );

    auto guard_r = PixelLocker( &resTile );
    // printf("%x\n", resTile.PixelGet( 0, 0 ).Value);
    REQUIRE( 4 == resTile.PixelGet( 0, 0 ).Value );
  }
  #endif
}},
{"find_first_indexのexprでのテスト", []{
  ResetUniqueName();

  // def by reduceのfind_first_indexのテストとほとんど同じ。
  // シンタックスだけ違う

  auto src = R"(

def result_u8 |x, y| {

  @bounds(10)
  def tmp |v| { 
    sin(f32(v))
  }

  # 最初に負になる整数。pi以上なので4。
  let tmp2 = reduce<tmp>.find_first_index(dim=0)  |_, val| { val < 0 }

  u8[tmp2, 0, 0, 0]
}
)";

  TreeBuilder builder;
  Parser parser( builder, src );


  if (SECTION("パースの結果適切なreduce accmに展開されているか")) {SG g;
    parser.ParseAll();

    auto tree = PrintExprS( builder._binary->Result().GetBody()) ;
    // cout << tree << endl;

    // def by reduceの方のテストで詳細はチェックしてあるので、ある程度一致していれば問題無いだろうというチェック。
    CheckContains( tree, "let u4_tmp2 = reduce_to_scalar<r0>.accumulate(dim=0, init=-1, block=" );
  }

  #ifdef MFG_BACKEND_ENABLED
  if (SECTION("バックエンドで実行")) {SG g;
    BuildIR( builder, parser );

    mfg_pal::Image32 inputTile;
    mfg_pal::Image32 resTile;

    inputTile.Resize( 2, 2 );
    resTile.Resize( 2, 2 );

    TargetBackend backend;
    // backend._debug = true;

    RunBackend( backend, *builder._binary.get(), inputTile, resTile );

    auto guard_r = PixelLocker( &resTile );
    // printf("%x\n", resTile.PixelGet( 0, 0 ).Value);
    REQUIRE( 4 == resTile.PixelGet( 0, 0 ).Value );
  }
  #endif
}},
{"print_exprのテスト", []{
  ResetUniqueName();
  auto src = R"(

@print_expr(sin(3.14/4))

def result_u8 |x, y| {
  u8[x+y, 0, 0, 0]
}
)";

  TestPBed pbed( src );
  IRBinary& binary = pbed.IRBinary();

  pbed.BuildIR();

  if (SECTION("トップレベルブロックのテスト")) {SG g;
    REQUIRE( binary._tblocks.size() == 1 );
    auto &tblock = *binary._tblocks[0];

    if (SECTION("letが生成されるか")) {SG g;
      auto tldump = PrintExprS( tblock.GetIRBody() );
      // cout << tldump << endl;

      CheckContains( tldump, "let t0 = sin" );
    }

    if (SECTION("ログのエントリがTopLevelBlockに生成されているか")) {SG g;
      REQUIRE( tblock._logEntries.size() == 1 );
      REQUIRE( tblock._logEntries[0]._varName == "t0" );
    }
  }
  #ifdef MFG_BACKEND_ENABLED
  if (SECTION("バックエンドで実行してログが出力されているか")) {SG g;
    mfg_pal::Image32 inputTile;
    mfg_pal::Image32 resTile;

    inputTile.Resize( 1, 1 );
    resTile.Resize( inputTile.Width(), inputTile.Height() );

    TargetBackend backend;
    RunBackend( backend, binary, inputTile, resTile );

    auto logs = backend.DrainLogs();
    // cout << logs << endl;
    CheckContains( logs, "0.7068" );
  }
  #endif

}},
{"@param_i32の位置が途中に来ても平気かどうかのテスト", []{
  ResetUniqueName();
  auto src = R"(

let gvar = sin(3.14/4)
@param_i32 WIDTH(SLIDER, label="サイズ", min=2, max=256, init=16)

def result_u8 |x, y| {
  u8[i32(gvar), 0, 0, 0]
}
)";

  TestPBed pbed( src );
  IRBinary& binary = pbed.IRBinary();

  pbed.BuildIR();

  REQUIRE( pbed._builder._binary->_params.size() == 1 );
  REQUIRE( pbed._builder._binary->_tblocks.size() == 1 );
}},
{"@param_posのテスト(deprecatedだがしばらくサポート)", []{
  ResetUniqueName();
  auto src = R"(

@param_pos POS(POINTER, label="中心の指定")

def result_u8 |x, y| {
  let [x0, y0] = POS
  let x1 = i32( x0*100 )
  let y1 = i32( y0*100 )
  u8[x+x1+y+y1, 0, 0, 0]
}
)";

  if (SECTION("パースの結果が正しいか")) {SG g;
    TestPBed pbed( src );
    IRBinary& binary = pbed.IRBinary();

    pbed.BuildIR();

    REQUIRE( pbed._builder._binary->_params.size() == 1 );
    auto& param = pbed._builder._binary->_params[0];
    REQUIRE( param.GetParamType() == ParamInfo::POINTER_FLOAT32_XY );
    REQUIRE( param.GetLabel() == "中心の指定" );
    REQUIRE_FEQUALS( param.GetActual().Value<float>(), 0.5 );
    REQUIRE_FEQUALS( param.GetY().Value<float>(), 0.5 );
  }
}},
{"@param_f32v2のテスト", []{
  ResetUniqueName();
  auto src = R"(

@param_f32v2 POS(POINTER, label="中心の指定")

def result_u8 |x, y| {
  let [x0, y0] = POS
  let x1 = i32( x0*100 )
  let y1 = i32( y0*100 )
  u8[x+x1+y+y1, 0, 0, 0]
}
)";

  if (SECTION("パースの結果が正しいか")) {SG g;
    TestPBed pbed( src );
    IRBinary& binary = pbed.IRBinary();

    pbed.BuildIR();

    REQUIRE( pbed._builder._binary->_params.size() == 1 );
    auto& param = pbed._builder._binary->_params[0];
    REQUIRE( param.GetParamType() == ParamInfo::POINTER_FLOAT32_XY );
    REQUIRE( param.GetLabel() == "中心の指定" );
    REQUIRE_FEQUALS( param.GetActual().Value<float>(), 0.5 );
    REQUIRE_FEQUALS( param.GetY().Value<float>(), 0.5 );
  }

  #ifdef MFG_BACKEND_ENABLED
  if (SECTION("Facadeで実行して結果が合うか")) {SG g;
    mfg_pal::Image32 inputTile;
    mfg_pal::Image32 resTile;

    inputTile.Resize( 1, 1 );
    resTile.Resize( 1, 1 );

    mfg::MFGFacade<TargetBackend> mfg;
    // mfg._debug = true;
    auto bin = mfg.Compile( src );
    auto& pinfo = bin.GetParamInfo();
    pinfo[0].AssignXY( 0.3f, 0.7f );

    RunFacade( mfg, bin, inputTile, resTile );

    auto guard_r = PixelLocker( &resTile );
    REQUIRE( 30+70 == resTile.PixelGet( 0, 0 ).Value );
  }
  #endif

}},
{"@param_f32 DIRECTIONのテスト", []{
  ResetUniqueName();
  auto src = R"(

@param_f32 dir(DIRECTION, init=10.0)

def result_u8 |x, y| {
  let x1 = i32(10.0*cos(dir))
  let y1 = i32(10.0*sin(dir))
  u8[x1, y1, 0, 0]
}
)";

  if (SECTION("パースの結果が正しいか")) {SG g;
    TestPBed pbed( src );
    IRBinary& binary = pbed.IRBinary();

    pbed.BuildIR();

    REQUIRE( pbed._builder._binary->_params.size() == 1 );
    auto& param = pbed._builder._binary->_params[0];
    REQUIRE( param.GetParamType() == ParamInfo::DIRECTION_FLOAT32 );
    REQUIRE_FEQUALS( param.GetActual().Value<float>(), 10.0 );
  }

  #ifdef MFG_BACKEND_ENABLED
  if (SECTION("Facadeで実行して結果が合うか")) {SG g;
    mfg_pal::Image32 inputTile;
    mfg_pal::Image32 resTile;

    inputTile.Resize( 1, 1 );
    resTile.Resize( 1, 1 );

    mfg::MFGFacade<TargetBackend> mfg;
    // mfg._debug = true;
    auto bin = mfg.Compile( src );
    auto& pinfo = bin.GetParamInfo();
    pinfo[0].Assign( 3.14f/3.0f );

    RunFacade( mfg, bin, inputTile, resTile );

    auto guard_r = PixelLocker( &resTile );
    // printf("%x\n", resTile.PixelGet( 0, 0 ).Value);
    // 10*(1.732)/2 = 8, 10*1/2 = 5
    REQUIRE( 0x805 == resTile.PixelGet( 0, 0 ).Value );
  }
  #endif

}},
{"@param_f32v4のカラーピッカーのテスト", []{
  ResetUniqueName();
  auto src = R"(

@param_f32v4 fcol(COLOR_PICKER, label="表の色")

def result_u8 |x, y| {
  let [fb, fg, fr, fa] = fcol
  u8[fb*100, fg*100, fr*100, fa*100]
}
)";

  if (SECTION("パースの結果が正しいか")) {SG g;
    TestPBed pbed( src );
    IRBinary& binary = pbed.IRBinary();

    pbed.BuildIR();

    REQUIRE( pbed._builder._binary->_params.size() == 1 );
    auto& param = pbed._builder._binary->_params[0];
    REQUIRE( param.GetParamType() == ParamInfo::COLOR_PICKER_FLOAT32_V4);
    REQUIRE( param.GetLabel() == "表の色" );
  }

  #ifdef MFG_BACKEND_ENABLED
  if (SECTION("Facadeで実行して結果が合うか")) {SG g;
    mfg_pal::Image32 inputTile;
    mfg_pal::Image32 resTile;

    inputTile.Resize( 1, 1 );
    resTile.Resize( 1, 1 );

    mfg::MFGFacade<TargetBackend> mfg;
    // mfg._debug = true;
    auto bin = mfg.Compile( src );
    auto& pinfo = bin.GetParamInfo();
    pinfo[0].AssignBGRA( 0.1f, 0.2f, 0.3f, 0.4f );

    RunFacade( mfg, bin, inputTile, resTile );

    auto guard_r = PixelLocker( &resTile );
    // printf("%x\n", resTile.PixelGet( 0, 0 ).Value );
    REQUIRE( 0x281e140a == resTile.PixelGet( 0, 0 ).Value );
  }
  #endif

}},
{"@param_f32v4のカラーピッカーのinitのテスト", []{
  ResetUniqueName();
  auto src = R"(

@param_f32v4 fcol(COLOR_PICKER, label="表の色", init=[0.1, 0.2, 0.3, 0.4])

def result_u8 |x, y| {
  let [fb, fg, fr, fa] = fcol
  u8[fb*100, fg*100, fr*100, fa*100]
}
)";

  TestPBed pbed( src );
  IRBinary& binary = pbed.IRBinary();

  pbed.BuildIR();

  REQUIRE( pbed._builder._binary->_params.size() == 1 );
  auto& param = pbed._builder._binary->_params[0];
  REQUIRE( param.GetParamType() == ParamInfo::COLOR_PICKER_FLOAT32_V4);
  auto bgra = param.GetBGRA();
  REQUIRE_FEQUALS( 0.1, bgra[0] );
  REQUIRE_FEQUALS( 0.2, bgra[1] );
  REQUIRE_FEQUALS( 0.3, bgra[2] );
  REQUIRE_FEQUALS( 0.4, bgra[3] );
}},
{"fore_colorのテスト", []{
  // 上のCOLOR_PICKERのテストを流用。
  ResetUniqueName();
  auto src = R"(

def result_u8 |x, y| {
  let [fb, fg, fr, fa] = fore_color()
  u8[fb*100, fg*100, fr*100, fa*100]
}
)";

  if (SECTION("パースの結果が正しいか")) {SG g;
    TestPBed pbed( src );
    IRBinary& binary = pbed.IRBinary();

    pbed.BuildIR();
    auto tree = pbed.PrintResultTree();

    CheckContains( tree, "fr = fore_color_val.2" );

    REQUIRE( pbed._builder._binary->_sparams.size() == 1 );
    auto& sparam = pbed._builder._binary->_sparams[0];
    REQUIRE( sparam.GetParamType() == SParamInfo::FORE_COLOR);
  }
  // 以下はCOLOR_PIKCERと同じテスト。
  #ifdef MFG_BACKEND_ENABLED
  if (SECTION("Facadeで実行して結果が合うか")) {SG g;
    mfg_pal::Image32 inputTile;
    mfg_pal::Image32 resTile;

    inputTile.Resize( 1, 1 );
    resTile.Resize( 1, 1 );

    mfg::MFGFacade<TargetBackend> mfg;
    // mfg._debug = true;
    auto bin = mfg.Compile( src );
    auto& sinfo = bin.GetSParamInfo();
    sinfo[0].AssignBGRA( 0.1f, 0.2f, 0.3f, 0.4f );

    RunFacade( mfg, bin, inputTile, resTile );

    auto guard_r = PixelLocker( &resTile );
    // printf("%x\n", resTile.PixelGet( 0, 0 ).Value );
    REQUIRE( 0x281e140a == resTile.PixelGet( 0, 0 ).Value );
  }
  #endif
}},
{"back_colorのテスト", []{
  // fore_colorとほとんど同じなので、こちらはパース結果のみチェック。
  // headless_appでエンジンとの統合も合わせてそちらでテストする。
  ResetUniqueName();
  auto src = R"(

def result_u8 |x, y| {
  let [fb, fg, fr, fa] = back_color()
  u8[fb*100, fg*100, fr*100, fa*100]
}
)";

  if (SECTION("パースの結果が正しいか")) {SG g;
    TestPBed pbed( src );
    IRBinary& binary = pbed.IRBinary();

    pbed.BuildIR();
    auto tree = pbed.PrintResultTree();

    CheckContains( tree, "fr = back_color_val.2" );

    REQUIRE( pbed._builder._binary->_sparams.size() == 1 );
    auto& sparam = pbed._builder._binary->_sparams[0];
    REQUIRE( sparam.GetParamType() == SParamInfo::BACK_COLOR);
  }
}},
{"タプルのグローバル変数のテスト", []{
  ResetUniqueName();
  auto src = R"(

let a = [2, 10]
def result_u8 |x, y| {
  let [b, c] = a
  u8[x+b+y+c, 0, 0, 0]
}
)";

  TestPBed pbed( src );
  IRBinary& binary = pbed.IRBinary();

  pbed.BuildIR();

  if (SECTION("トップレベルブロックのletのタプルがちゃんと展開されているか")) {SG g;
    auto toptree = PrintExprS( binary._tblocks[0]->GetRootElem() );
    // cout << toptree << endl;
    CheckContains( toptree, "let u0_a.0 = 2");
    CheckContains( toptree, "let u0_a.1 = 10");
  }
  if (SECTION("トップレベルブロックのグローバル変数としては展開結果と認識されているか")) {SG g;
    auto gvars = binary._tblocks[0]->CollectGlobalVarsInfo();
    REQUIRE( gvars.size() == 2 );
    REQUIRE( gvars[0].second == "u0_a.0" );
    REQUIRE( gvars[1].second == "u0_a.1" );
  }
  if (SECTION("テンソルの中での参照は変数に展開されているか")) {SG g;
    auto resulttree = PrintExprS( binary.Result().GetBody() );
    // cout << resulttree << endl;
    CheckContains( resulttree, "let u3_b = u0_a.0");
    CheckContains( resulttree, "let u4_c = u0_a.1");
  }
  if (SECTION("テンソルのReferenceは展開結果の変数になっているか")) {SG g;
    auto refvec = binary.Result().GetParamRefs();
    REQUIRE( 2 == refvec.size() );
    REQUIRE( refvec[0]._name == "u0_a.0" );
    REQUIRE( refvec[1]._name == "u0_a.1" );
  }

  #ifdef MFG_BACKEND_ENABLED
  if (SECTION("バックエンドで実行して結果が合うか")) {SG g;
    mfg_pal::Image32 inputTile;
    mfg_pal::Image32 resTile;

    inputTile.Resize( 1, 1 );
    resTile.Resize( 1, 1 );

    TargetBackend backend;
    RunBackend( backend, binary, inputTile, resTile );

    auto guard_r = PixelLocker( &resTile );
    REQUIRE( 2+10 == resTile.PixelGet( 0, 0 ).Value );
  }
  #endif
}},
{"normalizeのテスト（ベクトルを返す関数）", []{
  ResetUniqueName();
  auto src = R"(
def result_u8 |x, y| {
  let [x0, y0] = normalize([3.0, 5.0])
  u8[i32(10.0*x0+10.0*y0), 0, 0, 0]
}
)";

  if (SECTION("ベクトル関数のhoistが期待通りか")) {SG g;
    auto tree = ParseAndLowerAndReturnTreeDump( src );
    // cout << tree << endl;

    CheckContains( tree, "letv t0 = normalize" );
    CheckContains( tree, "let u2_x0 = t0.0" );
    CheckContains( tree, "let u3_y0 = t0.1" );
  }

  // これはcommonで行われるのでMetalでだけテストしておけば十分
  #ifdef MFG_BACKEND_METAL
  if (SECTION("Metalでベクトル関数をタプルにするコードが生成されているか")) {SG g;
    auto actual = GenMSLResultDump( src );
    // cout << actual << endl;

    CheckContains( actual, "float2 _t0 = normalize(float2(" );
    CheckContains( actual, "const float _t0_1 = _t0.y" );
    CheckContains( actual, "const float _u3_y0 = _t0_1" );
  }
  #endif

  #ifdef MFG_BACKEND_ENABLED
  if (SECTION("Facadeで実行して結果が合うか")) {SG g;
    mfg_pal::Image32 inputTile;
    mfg_pal::Image32 resTile;

    inputTile.Resize( 1, 1 );
    resTile.Resize( 1, 1 );

    mfg::MFGFacade<TargetBackend> mfg;
    // mfg._debug = true;
    auto bin = mfg.Compile( src );

    RunFacade( mfg, bin, inputTile, resTile );

    auto guard_r = PixelLocker( &resTile );
    // [0.5145, 0.8575] = normalize([3.0, 5.0])
    REQUIRE( 5+8 == resTile.PixelGet( 0, 0 ).Value );
  }
  #endif
}},
{"maxの4項のテスト", []{
  auto src = R"(
def result_u8 |x, y| {  
  u8[max(2, 3, 5, 4), 0, 0, 0]
}
)";
  if (SECTION("Lowerでmaxが二項に展開されているか")) {SG g;
    auto tree = ParseAndLowerAndReturnTreeDump( src );
    // cout << tree << endl;
    CheckContains( tree, "max(max(2, 3), 5)" );
  }
  #ifdef MFG_BACKEND_ENABLED
  if (SECTION("Backendで実行して結果が合うか")) {SG g;
    ExecVerifyScript( src, 1, 1, [](mfg_pal::Image32& resTile) {
      REQUIRE( 5 == resTile.PixelGet( 0, 0 ).Value );
    });
  }
  #endif
}},
{"swizzleのxyz", []{
  auto src = R"(
  def result_u8 |x, y| {
    let tup = [1, 2, 3]
    let tup2 = tup.yxyz
    u8(tup2)
  }
)";
  if (SECTION("Parseが正しく行われているか")) {SG g;
    auto tree = ParseAndReturnTreeDump( src );
    // cout << tree << endl;
    CheckContains( tree, "tup.swizzle(1, 0, 1, 2)" );
  }

  if (SECTION("Lowerで正しく展開されるか")) {SG g;
    auto tree = ParseAndLowerAndReturnTreeDump( src );
    // cout << tree << endl;
    CheckContains( tree, "let u3_tup2.0 = u2_tup.1" );
    CheckContains( tree, "let u3_tup2.1 = u2_tup.0" );
    CheckContains( tree, "let u3_tup2.2 = u2_tup.1" );
    CheckContains( tree, "let u3_tup2.3 = u2_tup.2" );
  }

  #ifdef MFG_BACKEND_ENABLED
  if (SECTION("Backendで実行して結果が合うか")) {SG g;
    ExecVerifyScript( src, 1, 1, [](mfg_pal::Image32& resTile) {
      REQUIRE( 0x03020102 == resTile.PixelGet( 0, 0 ).Value );
    });
  }
  #endif
}},
{"64bppのネガポジ反転", []{
  ResetUniqueName();
  std::string src = R"(
@title "ネガポジ反転64bpp"
def result_u16 |x, y| {
  let [b, g, r, a] = input_u16(x, y)
  u16[0xffff-b, 0xffff-g, 0xffff-r, a]
}
)";

  TestPBed pbed( src );

  if (SECTION("パース結果が(input|result)_u16になっているか")) {SG g;
    pbed.ParseAll();
    auto& bin = pbed.IRBinary();
    REQUIRE( GetFirstInput( bin.Input() )._tensorType == IOTensorType::TENSOR_U16 );
    REQUIRE( bin._resultHolder._tensorType == IOTensorType::TENSOR_U16 );
  }

  if (SECTION("バックエンドで実行")) {SG g;
    pbed.BuildIR();
#ifdef MFG_BACKEND_ENABLED
    TargetBackend backend;

    if (SECTION("64bppに対して実行して結果を確認")) {SG g;
      mfg_pal::Image64 inputTile;
      mfg_pal::Image64 resTile;

      SetupNegaPosi64( inputTile, resTile );
      // backend._debug = true;

      RunBackend( backend, pbed.IRBinary(), inputTile, resTile );

      auto guard_r = PixelLocker( &resTile );
      AssertNegaPosi64( resTile );
    }
    if (SECTION("32bppに対して実行して結果を確認")) {SG g;
      mfg_pal::Image32 inputTile;
      mfg_pal::Image32 resTile;

      SetupNegaPosi( inputTile, resTile );
      RunBackend( backend, pbed.IRBinary(), inputTile, resTile );

      auto guard_r = PixelLocker( &resTile );
      AssertNegaPosi( resTile );
    }
#endif
  }
}},
{"下のレイヤーの参照のテスト", []{
  ResetUniqueName();
  std::string src = R"(
@title "Add Filter"
def result_u8 |x, y| {
  let [b, g, r, a] = input_u8(x, y)
  let [b2, g2, r2, a2] = input_u8[-1](x, y)
  u8[b+b2, g+g2, r+r2, a]
}
)";

  TreeBuilder builder;
  Parser parser( builder, src );
  BuildIR( builder, parser );

  if (SECTION("InputsInfoが正しく集まっているか？")) {SG g;
    auto& iinfo = builder._binary->Input();
    auto& imap = iinfo._inputMap;
    REQUIRE( imap.end() != imap.find( InputIndex( 0, IOTensorType::TENSOR_U8)));
    REQUIRE( imap.end() != imap.find( InputIndex( -1, IOTensorType::TENSOR_U8)));
    // 参照してないものが入ってないかも簡易チェック。
    REQUIRE( imap.end() == imap.find( InputIndex( 1, IOTensorType::TENSOR_U8)));
  }
  if (SECTION("バックエンドで実行して結果を確認")) {SG g;
    mfg_pal::Image32 inputTile;
    mfg_pal::Image32 inputTileM1;
    mfg_pal::Image32 resTile;    

    SetupAddFilter0( inputTile, resTile );
    SetupAddFilter1( inputTileM1 );

    InputTiles its;
    its.Insert( 0, &inputTile );
    its.Insert( -1, &inputTileM1 );
    TileReference resRef( &resTile );


    #ifdef MFG_BACKEND_ENABLED
    TargetBackend backend;

    backend.Run( *builder._binary.get(), its, resRef );

    AssertAddFilter( resTile );
    #endif
  }
  if (SECTION("レイヤーが無い時に0となるか？")) {SG g;
    mfg_pal::Image32 inputTile;
    mfg_pal::Image32 resTile;    

    SetupAddFilter0( inputTile, resTile );

    InputTiles its;
    its.Insert( 0, &inputTile );
    TileReference resRef( &resTile );


    #ifdef MFG_BACKEND_ENABLED
    TargetBackend backend;

    backend.Run( *builder._binary.get(), its, resRef );

    AssertAddFilterOneLayer( resTile );
    #endif
  }
}},
{"elseのテスト", []{
  // パースの段階で展開されるので展開されてるかだけ確認しておく。

  if (SECTION("ifelの直下のケース")) {SG g;
    auto src = R"(
      def result_u8 |x, y| {
        let a = ifel(x == 0, 1, ...)
                else(2)
        u8[0, 0, 0, a]
      }
    )";
    auto tree = ParseAndReturnTreeDump( src );
    // cout << tree << endl;
    CheckContains( tree, "ifel(u0_x == 0, 1, 2)" );
  }
  if (SECTION("elifの直下のケース")) {SG g;
    auto src = R"(
      def result_u8 |x, y| {
        let a = ifel(x == 0, 1, ...)
                elif(x ==1, 2, ...)
                else(3)
        u8[0, 0, 0, a]
      }
    )";
    auto tree = ParseAndReturnTreeDump( src );
    // cout << tree << endl;
    CheckContains( tree, "ifel(u0_x == 0, 1, ifel(u0_x == 1, 2, 3))" );
  }
  if (SECTION("elseがタプルのケース")) {SG g;
    auto src = R"(
      def result_u8 |x, y| {
        let a = ifel(x == 0, [*vec4(1), *vec4(1)], ...)
                else([*vec4(2), *vec4(2)])
        u8[0, 0, 0, a.7]
      }
    )";
    auto tree = ParseAndReturnTreeDump( src );
    // cout << tree << endl;
    // パースエラーにならなければOK。一応elseの引数が消えてないかくらいはチェック。
    CheckContains( tree, "tuple(spread(vec4(2)), spread(vec4(2)))" );
  }
  if (SECTION("elseがf32のタプルのケース")) {SG g;
    auto src = R"(
      def result_u8 |x, y| {
        let a = ifel(x == 0, [1.0, 2.0], ...)
                else([3.0, 4.0])
        u8[0, 0, 0, a.0]
      }
    )";
    auto tree = ParseAndReturnTreeDump( src );
    // cout << tree << endl;
    // パースエラーにならなければOK。一応elseの引数が消えてないかくらいはチェック。
    CheckContains( tree, "tuple(3.00, 4.00)" );
  }
  if (SECTION("型違いがパースエラーになるか")) {SG g;
    auto src = R"(
      def result_u8 |x, y| {
        let a = ifel(x == 0, 1, ...)
                else(2.0)
        u8[0, 0, 0, a]
      }
    )";

    VerifyParseError( src );
  }
}},
#ifdef MFG_BACKEND_ENABLED
{"zipのパッケージのテスト", []{
    /*
      test_data/test_mfg.zipは「パッケージからのテクスチャロード」のテストと同じ内容のzip版
    */
    mfg_pal::Image32 inputTile;
    mfg_pal::Image32 resTile;

    inputTile.Resize( 2, 2 );
    resTile.Resize( 2, 2 );

    mfg::MFGFacade<TargetBackend> mfg;
    std::ifstream zipStream( "test_data/test_mfg.zip", std::ios::binary );
    auto bin = mfg.Compile(zipStream);
    RunFacade( mfg, bin, inputTile, resTile );

    auto guard_r = PixelLocker( &resTile );
    REQUIRE( 0xFFFF0000 == resTile.PixelGet( 0, 0 ).Value );
    REQUIRE( 0xFF00FFFF == resTile.PixelGet( 1, 0 ).Value );
    REQUIRE( 0xFF00FF00 == resTile.PixelGet( 0, 1 ).Value );
    REQUIRE( 0xFF0000FF == resTile.PixelGet( 1, 1 ).Value );

}},
{"marのリソースのテスト", []{
    auto guard = ScopeGuard([]{ mfg_resource::SetLanguage( NL_ENGLISH ); });

    std::ifstream zipStream( "test_data/test_res.mar", std::ios::binary );
    mfg::MFGFacade<TargetBackend> mfg;

    if (SECTION("日本語")) {SG g;
      mfg_resource::SetLanguage( NL_JAPANESE );

      auto bin = mfg.Compile(zipStream);
      REQUIRE( bin.GetTitle() == "日本語タイトル" );
    }

    if (SECTION("英語")){ SG g;
      mfg_resource::SetLanguage( NL_ENGLISH );

      auto bin = mfg.Compile(zipStream);
      REQUIRE( bin.GetTitle() == "English or Default Title" );
    }

    if (SECTION("存在しない場合はデフォルトの英語")){SG g;
      mfg_resource::SetLanguage( NL_FRENCH );

      auto bin = mfg.Compile(zipStream);
      REQUIRE( bin.GetTitle() == "English or Default Title" );
    }
}},
#endif
{"mfgでリークが無いかの確認。", []{
  #ifdef MFG_BACKEND_D3D
  ReleaseD3DAllDll();
  #endif

  REQUIRE( g_node_alloc_count == 0 );
  REQUIRE( g_irelem_alloc_count == 0 );
}}
};

/*
  ExecVerifyScriptを使ったテストだけを集めたテストケース。
*/
static std::vector<TestPair> test_cases_verify_script = {
#ifdef MFG_BACKEND_ENABLED
{"モジュロのテスト", []{
  auto src = R"(
def result_u8 |x, y| {
  u8[x % 3, 0, 0, 0]
}
)";
  ExecVerifyScript( src, 5, 1, [](mfg_pal::Image32& resTile) {
    REQUIRE( 2 == resTile.PixelGet( 2, 0 ).Value );
    REQUIRE( 0 == resTile.PixelGet( 3, 0 ).Value );
    REQUIRE( 1 == resTile.PixelGet( 4, 0 ).Value );
  });  
}},
{"f32のモジュロのテスト", []{
  auto src = R"(
def result_u8 |x, y| {
  let res = 8.3 % 4.0
  u8[res*10, 0, 0, 0]
}
)";
  ExecVerifyScript( src, 1, 1, [](mfg_pal::Image32& resTile) {
    // printf("%x\n", resTile.PixelGet( 0, 0 ).Value);
    REQUIRE( 3 == resTile.PixelGet( 0, 0 ).Value );
  });  
}},
{"最初に空白がある時のテスト", []{
  auto src = R"(  def result_u8 |x, y| {
    u8[1, 2, 3, 4]
  }
)";

  ExecVerifyScript( src, 1, 1, [](mfg_pal::Image32& resTile) {
    REQUIRE( 0x04030201 == resTile.PixelGet( 0, 0 ).Value );
  });
}},
{"累乗のテスト", []{
  ResetUniqueName();
  auto src = R"(
def result_u8 |x, y| {
  u8[(x+1) ^ 3, 0, 0, 0]
}
)";

  ExecVerifyScript( src, 3, 1, [](mfg_pal::Image32& resTile) {
    REQUIRE( 1 == resTile.PixelGet( 0, 0 ).Value );
    REQUIRE( 8 == resTile.PixelGet( 1, 0 ).Value );
    REQUIRE( 27 == resTile.PixelGet( 2, 0 ).Value );
  });
}},
{"xorのテスト", []{
  auto src = R"(
def result_u8 |x, y| {
  let res = xor(5u, 12u)
  u8[res, 0u, 0u, 0u]
}
)";

  ExecVerifyScript( src, 1, 1, [](mfg_pal::Image32& resTile) {
    // xor(0101, 1100) = 1001
    REQUIRE( 0x00000009 == resTile.PixelGet( 0, 0 ).Value );
  });
}},
{"16進リテラルのunsignedのテスト", []{
  ResetUniqueName();
  auto src = R"(
def result_u8 |x, y| {
  let a = 0xAAu
  let b = 1u + a
  u8[a, b, 0, 0]
}
)";

  ExecVerifyScript( src, 1, 1, [](mfg_pal::Image32& resTile) {
    REQUIRE( 0x0000ABAA == resTile.PixelGet( 0, 0 ).Value );
  });
}},
{"&&のテスト", []{
  auto src = R"(
def result_u8 |x, y| {
  ifel(x ==1 && y==1,
    u8[1, 0, 0, 0],
    u8[2, 0, 0, 0])
}
)";
  ExecVerifyScript( src, 2, 2, [](mfg_pal::Image32& resTile) {
    REQUIRE( 2 == resTile.PixelGet( 0, 0 ).Value );
    REQUIRE( 2 == resTile.PixelGet( 1, 0 ).Value );
    REQUIRE( 1 == resTile.PixelGet( 1, 1 ).Value );
  });
}},
{"fvecとスカラーの>=がintのベクトルになるかのテスト", []{
  auto src = R"(
def result_u8 |x, y| {
  let fvec = f32[x, y]
  let [a, b] = ifel(fvec >= 0.5, u8[3, 4], u8[1, 2])
  u8[a, b, 0, 0]
}
)";
  ExecVerifyScript( src, 2, 2, [](mfg_pal::Image32& resTile) {
    REQUIRE( 0x00000201 == resTile.PixelGet( 0, 0 ).Value );
    REQUIRE( 0x00000203 == resTile.PixelGet( 1, 0 ).Value );
    REQUIRE( 0x00000401 == resTile.PixelGet( 0, 1 ).Value );
    REQUIRE( 0x00000403 == resTile.PixelGet( 1, 1 ).Value );
  });
}},
{"fvecとfvecの>=がintのベクトルになるかのテスト", []{
  auto src = R"(
def result_u8 |x, y| {
  let fvec = f32[x, y]
  let [a, b] = ifel(fvec >= [0.5, 0.5], u8[3, 4], u8[1, 2])
  u8[a, b, 0, 0]
}
)";
  ExecVerifyScript( src, 2, 2, [](mfg_pal::Image32& resTile) {
    REQUIRE( 0x00000201 == resTile.PixelGet( 0, 0 ).Value );
    REQUIRE( 0x00000203 == resTile.PixelGet( 1, 0 ).Value );
    REQUIRE( 0x00000401 == resTile.PixelGet( 0, 1 ).Value );
    REQUIRE( 0x00000403 == resTile.PixelGet( 1, 1 ).Value );
  });
}},
{"vec2のテスト", []{
  auto src = R"(
def result_u8 |x, y| {
  let v = vec2(2)
  u8[v.x, v.y, 0, 0]
}
)";

  if(SECTION("展開結果が2要素で3要素になってないか")) {SG g;
    ResetUniqueName();
    
    auto actual = ParseAndLowerAndReturnTreeDump( src );
    // cout << actual << endl;
    CheckContains( actual, "let u2_v.1 = 2" );
    CheckNotContains( actual, "let u2_v.2 = 2" );
  }

  if (SECTION("バックエンドで実行して正しいか")) {SG g;
    ExecVerifyScript( src, 1, 1, [](mfg_pal::Image32& resTile) {
      REQUIRE( 0x00000202 == resTile.PixelGet( 0, 0 ).Value );
    });
  }
}},
{"vec3のテスト", []{
  auto src = R"(
def result_u8 |x, y| {
  let v = vec3(2)
  u8[v.x,v.y, v.z, 0]
}
)";
  ExecVerifyScript( src, 1, 1, [](mfg_pal::Image32& resTile) {
    REQUIRE( 0x00020202 == resTile.PixelGet( 0, 0 ).Value );
  });
}},
{"vec4のテスト", []{
  auto src = R"(
def result_u8 |x, y| {
  let v = vec4(3)
  u8[v.x,v.y, v.z, v.w]
}
)";
  ExecVerifyScript( src, 1, 1, [](mfg_pal::Image32& resTile) {
    REQUIRE( 0x03030303 == resTile.PixelGet( 0, 0 ).Value );
  });
}},
{"<<のテスト", []{
  auto src = R"(
def result_u8 |x, y| {
  u8[0xf << 4, 0u, 0u, 0u]
}
)";

  ExecVerifyScript( src, 1, 1, [](mfg_pal::Image32& resTile) {
    REQUIRE( 0x000000f0 == resTile.PixelGet( 0, 0 ).Value );
  });
}},
{"|のテスト", []{
  auto src = R"(
def result_u8 |x, y| {
  u8[0xf0 | 0x0f, 0u, 0u, 0u]
}
)";

  ExecVerifyScript( src, 1, 1, [](mfg_pal::Image32& resTile) {
    REQUIRE( 0xff == resTile.PixelGet( 0, 0 ).Value );
  });
}},
{"atan2のテスト", []{
  // ついでにspreadとcastのvectorizeのテストもする
  auto src = R"(
def result_u8 |x, y| {
  let [fx, fy] = f32([x, y])
  u8[u32( 10*atan2( *[fy, fx] ) ), 0u, 0u, 0u]
}
)";
  ExecVerifyScript( src, 2, 2, [](mfg_pal::Image32& resTile) {
    REQUIRE( 0 == resTile.PixelGet( 0, 0 ).Value );
    REQUIRE( 15 == resTile.PixelGet( 0, 1 ).Value );
    REQUIRE( 0 == resTile.PixelGet( 1, 0 ).Value );
    REQUIRE( 7 == resTile.PixelGet( 1, 1 ).Value );
  });  
}},
{"タプルのspreadのテスト", []{
  auto src = R"(
def result_u8 |x, y| {
  let [a, b, c] = [*[x, y]*10, 3]
  u8[a+b+c, 0, 0, 0]
}
)";
  ExecVerifyScript( src, 2, 2, [](mfg_pal::Image32& resTile) {
    REQUIRE( 3 == resTile.PixelGet( 0, 0 ).Value );
    REQUIRE( 13 == resTile.PixelGet( 0, 1 ).Value );
    REQUIRE( 13 == resTile.PixelGet( 1, 0 ).Value );
    REQUIRE( 23 == resTile.PixelGet( 1, 1 ).Value );
  });  
}},
{"to_ncoordのspreadのテスト", []{
  auto src = R"(
def result_u8 |x, y| {
  let a = [*to_ncoord([x, y]), 3.0]
  u8[a.x, a.y, a.z, 0]
}
)";
  ExecVerifyScript( src, 2, 2, [](mfg_pal::Image32& resTile) {
    REQUIRE( 0x00030101 == resTile.PixelGet( 1, 1 ).Value );
  });  

}},
{"タプルspreadとifelを混ぜたテスト", []{
  auto src = R"(
def result_u8 |x, y| {
  let [a, b, c] = ifel( x+y == 0, [1, 1, 3], [*[x, y]*10, 3])
  u8[a+b+c, 0, 0, 0]
}
)";
  ExecVerifyScript( src, 2, 2, [](mfg_pal::Image32& resTile) {
    REQUIRE( 5 == resTile.PixelGet( 0, 0 ).Value );
    REQUIRE( 13 == resTile.PixelGet( 0, 1 ).Value );
    REQUIRE( 13 == resTile.PixelGet( 1, 0 ).Value );
    REQUIRE( 23 == resTile.PixelGet( 1, 1 ).Value );
  });  
}},
{"rsumタプルのletのテスト", []{
  auto src = R"(
def result_u8 |x, y| {
  let [XPY, XMY] = rsum(1..<3, 1..<3) |rx, ry| { [rx+ry, rx*ry] }
  u8[ifel( x ==0, XPY, XMY ), 0, 0, 0]
}
)";
  ExecVerifyScript( src, 2, 1, [](mfg_pal::Image32& resTile) {   
    REQUIRE( 12 == resTile.PixelGet( 0, 0 ).Value ); // 2+3+3+4
    REQUIRE( 9 == resTile.PixelGet( 1, 0 ).Value ); // 1+2+2+4
  });  
}},
{"minのテスト", []{
  auto src = R"(
def result_u8 |x, y| {
  u8[i32(3.0+min(-1.0, 3.2)), 0, 0, 0]
}
)";
  ExecVerifyScript( src, 1, 1, [](mfg_pal::Image32& resTile) {
    REQUIRE( 2 == resTile.PixelGet( 0, 0 ).Value );
  });  
}},
{"i32のminのテスト", []{
  auto src = R"(
def result_u8 |x, y| {
  u8[3+min(-1, 3), 0, 0, 0]
}
)";
  ExecVerifyScript( src, 1, 1, [](mfg_pal::Image32& resTile) {
    REQUIRE( 2 == resTile.PixelGet( 0, 0 ).Value );
  });  
}},
{"maxのテスト", []{
  auto src = R"(
def result_u8 |x, y| {
  u8[i32(3.0+max(-1.0, 3.2)), 0, 0, 0]
}
)";
  ExecVerifyScript( src, 1, 1, [](mfg_pal::Image32& resTile) {
    REQUIRE( 6 == resTile.PixelGet( 0, 0 ).Value );
  });  
}},
{"clampのテスト", []{
  auto src = R"(
def result_u8 |x, y| {
  u8[i32(clamp(f32(2*x), 1.0, 3.0)), 0, 0, 0]
}
)";
  ExecVerifyScript( src, 3, 1, [](mfg_pal::Image32& resTile) {
    REQUIRE( 1 == resTile.PixelGet( 0, 0 ).Value );
    REQUIRE( 2 == resTile.PixelGet( 1, 0 ).Value );
    REQUIRE( 3 == resTile.PixelGet( 2, 0 ).Value );
  });  
}},
{"clampのテスト、ベクトルのケース", []{
  auto src = R"(
def result_u8 |x, y| {
  let [a, b] = clamp( [2.3, 7.2], 3.0, 6.0 )
  u8[i32( a+b), 0, 0, 0]
}
)";
  ExecVerifyScript( src, 1, 1, [](mfg_pal::Image32& resTile) {
    REQUIRE( 9 == resTile.PixelGet( 0, 0 ).Value );
  });  
}},
{"clampのテスト、intのベクトルのケース", []{
  auto src = R"(
def result_u8 |x, y| {
  let [a, b, c] = clamp( [2, 5, 7], 3, 6 )
  u8[a+b+c, 0, 0, 0]
}
)";
  ExecVerifyScript( src, 1, 1, [](mfg_pal::Image32& resTile) {
    REQUIRE( 14 == resTile.PixelGet( 0, 0 ).Value );
  });  
}},
{"インライン関数のテスト", []{
  ResetUniqueName();
  auto src = R"(

def result_u8 |x, y| {

  let bgra = to_ncolor(input_u8(x, y))

  let res = gamma2linear(bgra.xyz)
  let res2 = gamma2linear(bgra.x)
  to_u8color([*res, res2])
}
)";

  TreeBuilder builder;
  Parser parser( builder, src );

  if (SECTION("インライン関数がちゃんとlookup出来ているか")) {SG g;
    parser.ParseAll();
    auto tree = PrintExprS( builder._binary->Result().GetBody()) ;
    // cout << tree << endl;

    CheckContains( tree, "inline<2>(u2_bgra.swizzle" );
  }
  if (SECTION("インライン関数がちゃんとLower出来ているか")) {SG g;
    BuildIR( builder, parser );

    auto actual = PrintExprS( builder._binary->Result().GetBody()) ;
    // cout << actual << endl;

    CheckContains( actual, "= ifel(t5.0, pow((i1_v0.0 + 0.05) / 1.05, 2.40), i1_v0.0 / 12.92)" );

    if (SECTION("インライン関数の二回目の呼び出しで違うprefixがつくか")) {SG g;
      CheckContains( actual, "ifel(i2_v1 >= 0.04, pow((i2_v1 + 0.05) / 1.05, 2.40), i2_v1 / 12.92)" );
    }
  }
 }},
{"インライン関数からインライン関数を呼び出すテスト", []{
  ResetUniqueName();
  auto src = R"(

def result_u8 |x, y| {
  let res = gamma2linearA([0.1, 0.2, 0.3, 0.4])

  u8[*(res.xyz*1000.0), res.w*10.0]
}
)";

  // ツリーのテストは一時変数依存が多くなりすぎるので、結果だけ確認。
  ExecVerifyScript( src, 1, 1, [](mfg_pal::Image32& resTile) {
    auto b = (uint8_t)(1000.0F*std::pow((0.1F+0.055F)/1.055F, 2.40F));
    auto g = (uint8_t)(1000.0F*std::pow((0.2F+0.055F)/1.055F, 2.40F));
    auto r = (uint8_t)(1000.0F*std::pow((0.3F+0.055F)/1.055F, 2.40F));
    auto a = 4;
    uint32_t v = a << 24 | r << 16 | g << 8 | b;

    REQUIRE( v == resTile.PixelGet( 0, 0 ).Value );
  } );
}},
{"lbgr_to_xyzのテスト", []{
  ResetUniqueName();
  auto src = R"(

def result_u8 |x, y| {
  let res = lbgr_to_xyz([0.1, 0.2, 0.3])

  u8[*(res.xyz*1000.0), 0]
}
)";

  ExecVerifyScript( src, 1, 1, [](mfg_pal::Image32& resTile) {
    const float bgr[] = { 0.1F, 0.2F, 0.3F };
    const float xMat [] = {0.1804F, 0.3576F, 0.4125F};
    const float yMat [] = {0.0722F, 0.7152F, 0.2127F};
    const float zMat [] = {0.9502F, 0.1192F, 0.0193F};
    auto dot = [](const float* a, const float* b) {
      return a[0]*b[0]+ a[1]*b[1] + a[2]*b[2];
    };

    auto x = dot(bgr, xMat);
    auto y = dot(bgr, yMat);
    auto z = dot(bgr, zMat);

    auto xi = (uint8_t)(1000.0*x);
    auto yi = (uint8_t)(1000.0*y);
    auto zi = (uint8_t)(1000.0*z);

    uint32_t v = zi << 16 | yi << 8 | xi;
    /*
    printf("%f, %f, %f\n", x, y, z);
    printf("%d, %d, %d, %x, %x, %x\n", xi, yi, zi, xi, yi, zi);
    printf("%x, %x\n", v, resTile.PixelGet( 0, 0 ).Value);
    */
    REQUIRE( v == resTile.PixelGet( 0, 0 ).Value );    
  } );
}},
{"to_xyzaのテスト", []{
  ResetUniqueName();
  auto src = R"(

def result_u8 |x, y| {
  let res = to_xyza(u8[11, 22, 33, 0x44])
  # let ncol = gamma2linearA(to_ncolor(u8[11, 22, 33, 0x44]))
  # let xyz = lbgr_to_xyz(ncol.xyz)
  # let res = [*xyz, ncol.w]

  u8[*(res.xyz*2550.0), res.w*255.0]
}
)";

  ExecVerifyScript( src, 1, 1, [](mfg_pal::Image32& resTile) {
    // アルファだけは0x44なのは確か。他は上のコメントのコードの結果と同じならよしとする。
    // 440b1718
    // printf("%x\n", resTile.PixelGet( 0, 0 ).Value);
    REQUIRE( 0x440b1718 == resTile.PixelGet( 0, 0 ).Value );
  });
}},
{"to_lbgraのテスト", []{
  ResetUniqueName();
  auto src = R"(

def result_u8 |x, y| {
  let res = to_lbgra(u8[11, 22, 33, 0x44])
  # let res = gamma2linearA(to_ncolor(u8[11, 22, 33, 0x44]))

  u8[*(res.xyz*2550.0), res.w*255.0]
}
)";

  ExecVerifyScript( src, 1, 1, [](mfg_pal::Image32& resTile) {
    // アルファだけは0x44なのは確か。他は上のコメントのコードの結果と同じならよしとする。
    // 44261408
    // printf("%x\n", resTile.PixelGet( 0, 0 ).Value);
    REQUIRE( 0x44261408 == resTile.PixelGet( 0, 0 ).Value );
  });
}},
{"パイプ演算子のテスト", []{
  ResetUniqueName();
  auto src = R"(

def result_u8 |x, y| {
  let res = u8[11, 22, 33, 0x44] |>
            to_ncolor(...) |>
            gamma2linearA(...)

  u8[*(res.xyz*2550.0), res.w*255.0]
}
)";

  ExecVerifyScript( src, 1, 1, [](mfg_pal::Image32& resTile) {
    // to_lbgraと同じ
    REQUIRE( 0x44261408 == resTile.PixelGet( 0, 0 ).Value );
  });
}},
{"パイプ演算子の継続行テスト", []{
  ResetUniqueName();
  auto src = R"(

def result_u8 |x, y| {
  let res = u8[11, 22, 33, 0x44]
            |> to_ncolor(...)

  u8[*(res.xyz*2550.0), res.w*255.0]
}
)";
  auto tree = ParseAndReturnTreeDump( src );
  // cout << tree << endl;
  CheckContains( tree, "inline<0>((u8v4)(tuple(11" );

}},
{"xyza_to_u8colorのテスト", []{
  ResetUniqueName();
  auto src = R"(

def result_u8 |x, y| {
  let res = to_xyza(u8[11, 22, 33, 0x44])
  xyza_to_u8color(res)
}
)";

  ExecVerifyScript( src, 1, 1, [](mfg_pal::Image32& resTile) {
    // printf("%x\n", resTile.PixelGet( 0, 0 ).Value);
    REQUIRE( 0x4421160a == resTile.PixelGet( 0, 0 ).Value );
  });
}},
{"lbgra_to_u8colorのテスト", []{
  ResetUniqueName();
  auto src = R"(

def result_u8 |x, y| {
  let res = to_lbgra(u8[11, 22, 33, 0x44])
  lbgra_to_u8color(res)
}
)";

  ExecVerifyScript( src, 1, 1, [](mfg_pal::Image32& resTile) {
    // printf("%x\n", resTile.PixelGet( 0, 0 ).Value);
    // 33 to 32.9970 to 32 (0x20)
    // テストとしては端数は重要ではないので多少ずれても問題ない。
    // metal 0x4420160a
    // HLSL 0x4420160b
    // MBA metal: 0x4421150a
    REQUIRE( 0x4421150a == resTile.PixelGet( 0, 0 ).Value || 0x4420160a == resTile.PixelGet( 0, 0 ).Value || 0x4420160b == resTile.PixelGet( 0, 0 ).Value );
  });
}},
{"to_ncoordのテスト", []{
  auto src = R"(
def result_u8 |x, y| {
  let [fx, fy] = to_ncoord([x, y])
  let v = i32(10*fx+1000*fy)
  u8[v&0xff, (v>>8)&0xff, 0, 0]
}
)";
  // 0, 1, 2 -> 0, 0.5, 1.0
  // 0, 1 -> 0, 1.0
  ExecVerifyScript( src, 3, 2, [](mfg_pal::Image32& resTile) {   
    REQUIRE( 0 == resTile.PixelGet( 0, 0 ).Value );
    REQUIRE( 5 == resTile.PixelGet( 1, 0 ).Value );
    REQUIRE( 10 == resTile.PixelGet( 2, 0 ).Value );
    REQUIRE( 0 == resTile.PixelGet( 0, 0 ).Value );
    REQUIRE( 1000 == resTile.PixelGet( 0, 1 ).Value );
    REQUIRE( 1005 == resTile.PixelGet( 1, 1 ).Value );
  });  
}},
{"to_ncolorのテスト", []{
  auto src = R"(
def result_u8 |x, y| {
  let ncolor = to_ncolor(u8[100, 125, 255, 0])

  u8(ncolor*100.0)
}
)";
  // [100, 125, 255, 0]/255.0 = [0.39, 0.49, 1.0, 0.0]*100.0
  // = [0x27, 0x31, 0x64, 0x00]
  ExecVerifyScript( src, 1, 1, [](mfg_pal::Image32& resTile) {
    REQUIRE( 0x00643127 == resTile.PixelGet( 0, 0 ).Value );
  });  
}},
{"to_u8colorのテスト", []{
  auto src = R"(
def result_u8 |x, y| {
  to_u8color([0.5, 1.0, 1.3, -0.2])
}
)";
  // [0.5, 1.0, 1.3, -0.2]*255 with clamp = [0.5, 1.0, 1.0, 0.0]*255 = [0x7f, 0xff, 0xff, 0]
  ExecVerifyScript( src, 1, 1, [](mfg_pal::Image32& resTile) {   
    REQUIRE( 0x00ffff7f == resTile.PixelGet( 0, 0 ).Value );
  });  
}},
{"is_insideのテスト", []{
  auto src = R"(
def result_u8 |x, y| {
  let res = input_u8.is_inside(x+1, y+1)
  u8[res, 0u, 0u, 0u]
}
)";

  ExecVerifyScript( src, 2, 2, [](mfg_pal::Image32& resTile) {
    REQUIRE( 0x00000001 == resTile.PixelGet( 0, 0 ).Value );
    REQUIRE( 0x00000000 == resTile.PixelGet( 1, 0 ).Value );
    REQUIRE( 0x00000000 == resTile.PixelGet( 0, 1 ).Value );
    REQUIRE( 0x00000000 == resTile.PixelGet( 1, 1 ).Value );
  });
}},
{"logical not(!)のテスト", []{
  auto src = R"(
def result_u8 |x, y| {
  let res = x>0 && !(y > 0)
  u8[res, 0u, 0u, 0u]
}
)";

  ExecVerifyScript( src, 2, 2, [](mfg_pal::Image32& resTile) {
    REQUIRE( 0x00000000 == resTile.PixelGet( 0, 0 ).Value );
    REQUIRE( 0x00000001 == resTile.PixelGet( 1, 0 ).Value );
    REQUIRE( 0x00000000 == resTile.PixelGet( 0, 1 ).Value );
    REQUIRE( 0x00000000 == resTile.PixelGet( 1, 1 ).Value );
  });
}},
{"extentの値が違う時にちゃんとそれぞれ違う値がくるか？", []{ // 正方形でテストしている事が多いので念の為
  auto src = R"(
def result_u8 |x, y| {
  u8[0, 0, input_u8.extent(1), input_u8.extent(0)]
}
)";
  ExecVerifyScript( src, 4, 3, [](mfg_pal::Image32& resTile) {   
    REQUIRE( 0x04030000 == resTile.PixelGet( 0, 0 ).Value );
  });  
}},
{".NormlaizedLinearの1次元のテスト", []{
  std::string src = R"(
@bounds(3)
def oneD |x| {
  f32(x)
}

let foneD = sampler<oneD>(coord=.NormalizedLinear)
def result_u8 |x, y| {
  # とりあえず0, 0.2, 1.0の三つを試す。
  let fx = ifel(x == 0, 0.0, ...)
           elif(x == 1, 0.2, 1.0)
  u8[10.0*foneD(fx), 0, 0, 0]
}
)";

  ExecVerifyScript( src, 3, 1, [](mfg_pal::Image32& resTile) {
    // oneDは 0.0, 1.0, 2.0
    // fxは0.0, 0.2, 1.0

    REQUIRE( 0 == resTile.PixelGet( 0, 0 ).Value );
    // 0.6*0+0.4*1.0 = 0.4
    REQUIRE( 0x00000004 == resTile.PixelGet( 1, 0 ).Value );
    // 20 = 0x14
    REQUIRE( 0x00000014 == resTile.PixelGet( 2, 0 ).Value );
  });  

}},
{".NormalizedLinearの2次元のテスト", []{
  std::string src = R"(
@bounds(3, 3)
def twoD |x, y| {
  f32(x+10*y)
}

let fTwoD = sampler<twoD>(coord=.NormalizedLinear)
def result_u8 |x, y| {
  # とりあえずxは0, 0.2, 1.0の三つを試す。
  let fx = ifel(x == 0, 0.0, ...)
           elif(x == 1, 0.2, 1.0)
  # yは0.0, 0.5, 0.8の三つを試す。
  let fy = ifel(y == 0, 0.0, ...)
           elif(y == 1, 0.5, 0.9)
  u8[10.0*fTwoD(fx, fy), 0, 0, 0]
}
)";

  ExecVerifyScript( src, 3, 3, [](mfg_pal::Image32& resTile) {
    /*
      twoDは
      0.0 1.0 2.0
      10.0 11.0 12.0
      20.0 21.0 22.0

      xは0, 0.2, 1.0
      yは0.0, 0.5, 0.9
    */

    REQUIRE( 0 == resTile.PixelGet( 0, 0 ).Value );

    // 0.6*0.0+0.4*1.0 = 0.4
    REQUIRE( 0x00000004 == resTile.PixelGet( 1, 0 ).Value );
    // 20 = 0x14
    REQUIRE( 0x00000014 == resTile.PixelGet( 2, 0 ).Value );

    // 10.0*10 = 100 = 0x64
    REQUIRE( 0x00000064 == resTile.PixelGet( 0, 1 ).Value );
    // 0.6*10.0+0.4*11.0 = 6+4.4 = 10.4
    REQUIRE( 0x00000068 == resTile.PixelGet( 1, 1 ).Value );
    // 120 = 0x78
    REQUIRE( 0x00000078 == resTile.PixelGet( 2, 1 ).Value );

    // 0.2*10+ 0.8*20.0= 18,  180 = 0xc8
    REQUIRE( 0x000000b4 == resTile.PixelGet( 0, 2 ).Value );

    // (1, 2), (2, 2)は計算ば面倒なのでさぼり。
  });  

}},
{".NormalizedLinearの2次元のタプルのテスト", []{
  std::string src = R"(
@bounds(3, 3)
def twoD |x, y| {
  [f32(x+10*y), x+10*y]
}

let fTwoD = sampler<twoD>(coord=.NormalizedLinear)
def result_u8 |x, y| {
  # とりあえずxは0, 0.2, 1.0の三つを試す。
  let fx = ifel(x == 0, 0.0, ...)
           elif(x == 1, 0.2, 1.0)
  # yは0.0, 0.5, 0.8の三つを試す。
  let fy = ifel(y == 0, 0.0, ...)
           elif(y == 1, 0.5, 0.9)
  let [fv, iv] = fTwoD(fx, fy)
  u8[10.0*fv, 10*iv, 0, 0]
}
)";

  ExecVerifyScript( src, 3, 3, [](mfg_pal::Image32& resTile) {
    /*
      twoDは
      0.0 1.0 2.0
      10.0 11.0 12.0
      20.0 21.0 22.0

      xは0, 0.2, 1.0
      yは0.0, 0.5, 0.9
    */

    REQUIRE( 0 == resTile.PixelGet( 0, 0 ).Value );

    // 0.6*0.0+0.4*1.0 = 0.4
    // i32にすると0
    REQUIRE( 0x00000004 == resTile.PixelGet( 1, 0 ).Value );
    // 20 = 0x14
    REQUIRE( 0x00001414 == resTile.PixelGet( 2, 0 ).Value );

    // 10.0*10 = 100 = 0x64
    REQUIRE( 0x00006464 == resTile.PixelGet( 0, 1 ).Value );
    // 0.6*10.0+0.4*11.0 = 6+4.4 = 10.4
    // i32にすると10
    REQUIRE( 0x00006468 == resTile.PixelGet( 1, 1 ).Value );
    // 120 = 0x78
    REQUIRE( 0x00007878 == resTile.PixelGet( 2, 1 ).Value );

    // 0.2*10+ 0.8*20.0= 18,  180 = 0xc8
    REQUIRE( 0x0000b4b4 == resTile.PixelGet( 0, 2 ).Value );

    // (1, 2), (2, 2)は計算ば面倒なのでさぼり。
  });  

}},
{"EOLでexprが切れるかのテスト", []{
  auto src = R"(
@bounds(1)
def tmp |x| {
  let a = -3
  - a
}
  
def result_u8 |x, y| {
  u8[tmp(0), 0, 0, 0]
}
)";
  ExecVerifyScript( src, 1, 1, [](mfg_pal::Image32& resTile) {
    REQUIRE( 0x00000003 == resTile.PixelGet( 0, 0 ).Value );
  });
}},
{"EOLでもbinopのあとは続きとみなされるテスト", []{
  auto src = R"(
@bounds(1)
def tmp |x| {
  let a = -3 -
  2
  -a
}
  
def result_u8 |x, y| {
  u8[tmp(0), 0, 0, 0]
}
)";
  ExecVerifyScript( src, 1, 1, [](mfg_pal::Image32& resTile) {
    REQUIRE( 0x00000005 == resTile.PixelGet( 0, 0 ).Value );
  });
}},
{"EOLのエスケープのテスト", []{
  auto src = R"(
@bounds(1)
def tmp |x| {
  let a = -3 \
  -2
  -a
}
  
def result_u8 |x, y| {
  u8[tmp(0), 0, 0, 0]
}
)";
ExecVerifyScript( src, 1, 1, [](mfg_pal::Image32& resTile) {
  REQUIRE( 0x00000005 == resTile.PixelGet( 0, 0 ).Value );
});
}},
{"予期せぬTokenの時のエラーメッセージのテスト", []{
  auto src = R"(
let a - 3
def result_u8 |x, y| {
  u8[0, 0, 0, 0]
}
)";
  bool thrown = false;
  try {
    ExecVerifyScript( src, 1, 1, [](mfg_pal::Image32& resTile) {
      REQUIRE(false); // ここには来ない
    });
  }
  catch( const ParseError& err )
  {
    REQUIRE( std::string("Expect token type: EQUAL, but was: MINUS") == err.what() );
    thrown = true;
  }
  REQUIRE( thrown );
}},
{"InlineFuncがvector内で動くケースのテスト", []{
  // f3がf4, f5, f6を足すと動く。
  // iterの参照を誤って保持しているとクラシュする。
auto src = R"(
fn f1 |v: i32| { v+1 }
fn f2 |v: i32| { v+2 }
fn f3 |v: i32| { v+3 }
fn f4 |v: i32| { v+4 }
fn f5 |v: i32| { v+5 }
fn f6 |v: i32| { v+6 }

def result_u8 |x, y| {
  let s = f3(1)

  u8[s, 0, 0, 0]
}
)";
  auto tree = ParseAndLowerAndReturnTreeDump( src );
  // cout << tree << endl;
  // パースエラーにならなければOK。一応展開した結果に+ 3が残っているかチェック。
  CheckContains( tree, "+ 3" );
}},
{"引数違いがちゃんとexceptionになるかのテスト", []{ // 以前assertで落としていたのをExceptionに変えたのでテスト
  auto src = R"(
def result_u8 |x, y| {
  u8[i32(sin(1.0, 2.0)), 0, 0, 0]
}
)";
  VerifyParseError( src );
}},
{"tupleのネストは未サポートなのでパースエラーにできているか", []{   // 以前assertで落としていたのをExceptionに変えたのでテスト
  auto src = R"(
def result_u8 |x, y| {
  let a = [1, 2]
  u8[1, a, 4]
}
)";
  VerifyParseError( src );
}},
{"rsumを変数名に使うのをちゃんと禁止にできているかのテスト", []{
  auto src = R"(
def result_u8 |x, y| {
  let rsum = 3
  u8[rsum, 0, 0, 0]
}
)";
  auto perr = VerifyParseError( src );
  // cout << perr.what() << std::endl;
  REQUIRE( perr.GetLineNum() == 3 );
}},
{"rsumを変数名に使うのをちゃんと禁止にできているか、destructuringバージョン", []{
  auto src = R"(
def result_u8 |x, y| {
  let [a, rsum, b, c] = input_u8(x, y)
  u8[rsum, 0, 1, 2]
}
)";
  auto perr = VerifyParseError( src );
  // cout << perr.what() << std::endl;
  REQUIRE( perr.GetLineNum() == 3 );
}},
{"@boundsが無い時にexceptionになるか", []{
  auto src = R"(
def mid |x, y| {
  x+y
}

def result_u8 |x, y| {
  u8[mid(x, y), 0, 0, 0]
}
)";
  auto perr = VerifyParseError( src );
  // cout << perr.what() << std::endl;
  REQUIRE( perr.GetLineNum() == 2 );
}},
{"@boundsと引数の個数がずれている場合にexceptionになるか", []{
  std::string src = R"(
@bounds(5, 5)
def ts |x| { x }

def result_u8 |x, y| {
  u8[(ts(x, y) + ts(x, y+1) + ts(x+1, y) + ts(x+1, y+1))/4, 0, 0, 0]
})";

  VerifyParseError( src );
}},
{"テンソルのint引数にfloatを入れたらパースエラーになるか", []{
  std::string src = R"(
def result_u8 |x, y| {
  input_u8(0.0, 1.0)
})";

  VerifyParseError( src );
}},
{"ifelの引数の次元違いがパースエラーになるか", []{
  std::string src = R"(
def result_u8 |x, y| {
  ifel([0, 1], u8[0, 0, 0, 0], u8[0, 0, 0, 0xff])
})";

  VerifyParseError( src );
}},
{"変数に入れたタプルのベクトライズのテスト", []{
  auto src = R"(
def result_u8 |x, y| {
  let vec = [1, 2, 3]
  let v2 = vec*2
  let [a1, a2, a3] = v2
  u8[a1+a2+a3, 0, 0, 0]
}
)";
  ExecVerifyScript( src, 1, 1, [](mfg_pal::Image32& resTile) {   
    REQUIRE( 12 == resTile.PixelGet( 0, 0 ).Value );
  });  
}},
{"引数無しのextentがタプルになるかのテスト", []{
  auto src = R"(
def result_u8 |x, y| {
  let [W, H] = input_u8.extent()
  u8[W+H, 0, 0, 0]
}
)";
  ExecVerifyScript( src, 3, 2, [](mfg_pal::Image32& resTile) {   
    REQUIRE( 5 == resTile.PixelGet( 0, 0 ).Value );
  });  
}},
{"extentf, 引数無しのケース", []{
  auto src = R"(
def result_u8 |x, y| {
  let [W, H] = input_u8.extentf()
  u8[(W+H)*1.5, 0, 0, 0]
}
)";
  ExecVerifyScript( src, 3, 2, [](mfg_pal::Image32& resTile) {
    REQUIRE( 7 == resTile.PixelGet( 0, 0 ).Value );
  });
}},
{"extentf, 引数ありのケース", []{
  auto src = R"(
def result_u8 |x, y| {
  let H = input_u8.extentf(1)
  u8[H*1.5, 0, 0, 0]
}
)";
  ExecVerifyScript( src, 3, 2, [](mfg_pal::Image32& resTile) {
    REQUIRE( 3 == resTile.PixelGet( 0, 0 ).Value );
  });
}},
{"TLBlockでのrandのテスト", []{
  auto src = R"(
let r1 = rand()
let r2 = rand()
def result_u8 |x, y| {
  let col1 = i32([r1*256, (r1*0xffff), (r1*0xffffff), (r1*0xffffffff)])%256
  let col2 = i32([r2*256, (r2*0xffff), (r2*0xffffff), (r2*0xffffffff)])%256
  ifel(x == 0, u8(col1), u8(col2))
}
)";

  ExecVerifyScript( src, 2, 1, [](mfg_pal::Image32& resTile) {
    /*
    printf( "%x\n", resTile.PixelGet( 0, 0 ).Value );
    printf( "%x\n", resTile.PixelGet( 1, 0 ).Value );
    */

    // 偶然0になる事は多分ない。二つの値が違っていたらまぁ乱数と思っておく。
    REQUIRE( 0 != resTile.PixelGet( 0, 0 ).Value );
    REQUIRE( 0 != resTile.PixelGet( 1, 0 ).Value );
    REQUIRE( resTile.PixelGet( 0, 0 ).Value != resTile.PixelGet( 1, 0 ).Value );
  });  
}},
{"トップレベルでのrsumのテスト", []{
  auto src = R"(
let r1 = rsum(1..<4) |r| { r }
def result_u8 |x, y| {
  u8[r1, 0, 0, 0]
}
)";

  ExecVerifyScript( src, 1, 1, [](mfg_pal::Image32& resTile) {
    REQUIRE( 0x00000006 == resTile.PixelGet( 0, 0 ).Value );
  });  
}},
{"トップレベルでのreduceのテスト", []{
  auto src = R"(
let r1 = reduce(init=1, 2..<4) |r, accm| { r*accm }
def result_u8 |x, y| {
  u8[r1, 0, 0, 0]
}
)";

  ExecVerifyScript( src, 1, 1, [](mfg_pal::Image32& resTile) {
    REQUIRE( 0x00000006 == resTile.PixelGet( 0, 0 ).Value );
  });  
}},
{"2次元のreduceのテスト", []{
  auto src = R"(
def result_u8 |x, y| {
  let res = reduce(init=1, 1..<3, 2..<4) |rx, ry, accm| {
    rx*ry*accm
  }
  u8[res, 0, 0, 0]
}
)";

  // 1*(1*2), 2*(2*2),8*(1*3), 24*(2*3)
  // 144 = 0x90
  ExecVerifyScript( src, 1, 1, [](mfg_pal::Image32& resTile) {
    // printf("deb: %x\n", resTile.PixelGet( 0, 0 ).Value);
    REQUIRE( 0x00000090 == resTile.PixelGet( 0, 0 ).Value );
  });  
}},
{"tanのテスト", []{
  auto src = R"(
def result_u8 |x, y| {
  u8[i32(100*tan(0.5)), 0, 0, 0]
}
)";
  ExecVerifyScript( src, 1, 1, [](mfg_pal::Image32& resTile) {
    // tan(0.5)は0.5463024898437905
    REQUIRE( 54 == resTile.PixelGet( 0, 0 ).Value );
  });  
}},
{"ceilのテスト", []{
  auto src = R"(
def result_u8 |x, y| {
  u8[i32(ceil(0.1)), 0, 0, 0]
}
)";
  ExecVerifyScript( src, 1, 1, [](mfg_pal::Image32& resTile) {
    REQUIRE( 1 == resTile.PixelGet( 0, 0 ).Value );
  });
}},
{"floorのテスト", []{
  auto src = R"(
def result_u8 |x, y| {
  u8[i32(floor(2.9)), 0, 0, 0]
}
)";
  ExecVerifyScript( src, 1, 1, [](mfg_pal::Image32& resTile) {
    REQUIRE( 2 == resTile.PixelGet( 0, 0 ).Value );
  });
}},
{"fractのテスト", []{
  auto src = R"(
def result_u8 |x, y| {
  u8[i32(10.0*fract(2.7)), 0, 0, 0]
}
)";
  ExecVerifyScript( src, 1, 1, [](mfg_pal::Image32& resTile) {
    REQUIRE( 7 == resTile.PixelGet( 0, 0 ).Value );
  });
}},
{"roundのテスト", []{
  auto src = R"(
def result_u8 |x, y| {
  u8[i32(ifel(x == 0, round(2.6), round(2.4))), 0, 0, 0]
}
)";
  ExecVerifyScript( src, 2, 1, [](mfg_pal::Image32& resTile) {
    REQUIRE( 3 == resTile.PixelGet( 0, 0 ).Value );
    REQUIRE( 2 == resTile.PixelGet( 1, 0 ).Value );
  });
}},
{"truncのテスト", []{
  auto src = R"(
def result_u8 |x, y| {
  u8[i32(5.0+trunc(-2.3)), 0, 0, 0]
}
)";
  ExecVerifyScript( src, 1, 1, [](mfg_pal::Image32& resTile) {
    REQUIRE( 3 == resTile.PixelGet( 0, 0 ).Value );
  });
}},
{"lengthのテスト", []{
  auto src = R"(
def result_u8 |x, y| {
  u8[i32(length([2.4, 3.3])), 0, 0, 0]
}
)";
  ExecVerifyScript( src, 1, 1, [](mfg_pal::Image32& resTile) {
    // sqrt(2.4^2+3.3^2) == 4.08
    REQUIRE( 4 == resTile.PixelGet( 0, 0 ).Value );
  });
}},
{"dotのテスト", []{
  auto src = R"(
def result_u8 |x, y| {
  u8[i32(dot([2.4, 3.3], [1.1, 2.2])), 0, 0, 0]
}
)";
  ExecVerifyScript( src, 1, 1, [](mfg_pal::Image32& resTile) {
    // 2.4*1.1+3.3*2.2=9.9
    REQUIRE( 9 == resTile.PixelGet( 0, 0 ).Value );
  });
}},
{"crossのテスト", []{
  auto src = R"(
def result_u8 |x, y| {
  let v = cross([1.0, 2.0, 3.0], [4.0, 5.0, 3.0])
  u8(i32[v.0, v.1, v.2, 0])
}
)";
  ExecVerifyScript( src, 1, 1, [](mfg_pal::Image32& resTile) {
    // 6-15, 12-3, 5-8 = -9, 9, -3 = 0xf7, 0x09, 0xfd
    REQUIRE( 0x00fd09f7 == resTile.PixelGet( 0, 0 ).Value );
  });
}},
{"distanceのテスト", []{
  auto src = R"(
def result_u8 |x, y| {
  u8[i32(10*distance([2.4, 3.3], [1.1, 2.2])), 0, 0, 0]
}
)";
  ExecVerifyScript( src, 1, 1, [](mfg_pal::Image32& resTile) {
    // sqrt((2.4-1.1)^2+(3.3-2.2)^2) = 1.7
    REQUIRE( 17 == resTile.PixelGet( 0, 0 ).Value );
  });
}},
{"mixのテスト、比率がスカラーのケース", []{
  auto src = R"(
def result_u8 |x, y| {
  let [x0, y0] = mix( [0.0, 1.2], [1.0, 0.0], 0.5)
  u8[i32(10*x0+100*y0), 0, 0, 0]
}
)";
  ExecVerifyScript( src, 1, 1, [](mfg_pal::Image32& resTile) {
    REQUIRE( 65 == resTile.PixelGet( 0, 0 ).Value );
  });
}},
{"mixのテスト、比率がベクターのケース", []{
  auto src = R"(
def result_u8 |x, y| {
  let [x0, y0] = mix( [0.0, 1.0], [1.0, 0.0], [0.3, 0.7])
  u8[i32(10*x0+100*y0), 0, 0, 0]
}
)";
  ExecVerifyScript( src, 1, 1, [](mfg_pal::Image32& resTile) {
    REQUIRE( 33 == resTile.PixelGet( 0, 0 ).Value );
  });
}},
{"mixのテスト、スカラーのケース", []{
  auto src = R"(
def result_u8 |x, y| {
  let res = 10.0*mix(3.0, 1.0, 0.3)
  u8[i32(res+0.01), 0, 0, 0]
}
)";
  // これはcommonで行われるのでMetalでだけテストしておけば十分
  #ifdef MFG_BACKEND_METAL
  if (SECTION("Metalで間違ってベクトルに展開しないかのテスト")) {SG g;
    auto actual = GenMSLResultDump( src );
    // cout << actual << endl;

    // hoistされていなければ大丈夫だろう
    CheckContains( actual, "10.000000f*mix(3.000000f, 1.000000f, 0.300000f)" );
  }
  #endif

  if (SECTION("実行して結果が合うか")) {SG g;
    ExecVerifyScript( src, 1, 1, [](mfg_pal::Image32& resTile) {
      REQUIRE( 24 == resTile.PixelGet( 0, 0 ).Value );
    });
  }
}},
{"signのテスト、ベクトルのケース", []{
  auto src = R"(
def result_u8 |x, y| {
  let [a, b] = sign( [0.5, -0.5] )
  u8[i32( 10*a+b), 0, 0, 0]
}
)";
  ExecVerifyScript( src, 1, 1, [](mfg_pal::Image32& resTile) {
    REQUIRE( 9 == resTile.PixelGet( 0, 0 ).Value );
  });  
}},
{"signのテスト、スカラーのケース", []{
  auto src = R"(
def result_u8 |x, y| {
  u8[i32( 10.0 + 2.0*sign(-0.5)), 0, 0, 0]
}
)";
  ExecVerifyScript( src, 1, 1, [](mfg_pal::Image32& resTile) {
    REQUIRE( 8 == resTile.PixelGet( 0, 0 ).Value );
  });  
}},
{"smoothstepのテスト", []{
  auto src = R"(
def result_u8 |x, y| {  
  let [a, b] = smoothstep( [1.0, 1.0], [2.0, 3.0], [1.3, 2.0] )
  let res = i32(10*a+100.0*b + 1000.0*smoothstep(1.0, 2.0, 1.3))
  u8[res%256, (res/256)%256, 0, 0]
}
)";
  ExecVerifyScript( src, 1, 1, [](mfg_pal::Image32& resTile) {
    // printf("%d\n", resTile.PixelGet( 0, 0 ).Value );
    // だいたい 10*0.21+100*0.5+1000*0.21くらいで268。
    REQUIRE( 268 == resTile.PixelGet( 0, 0 ).Value );
  });  
}},
{"stepのテスト", []{
  auto src = R"(
def result_u8 |x, y| {  
  let [a, b] = step( [1.0, 1.0], [0.9, 1.1] )
  let res = i32(10*a+100.0*b + 1000.0*step(1.1, 1.2))
  u8[res%256, (res/256)%256, 0, 0]
}
)";
  // resは1100。
  ExecVerifyScript( src, 1, 1, [](mfg_pal::Image32& resTile) {
    REQUIRE( 1100 == resTile.PixelGet( 0, 0 ).Value );
  });  
}},
{"smoothstepのエッジがスカラーのテスト", []{
  auto src = R"(
def result_u8 |x, y| {  
  let [a, b] = smoothstep( 1.0, 3.0, [1.3, 2.0] )
  u8[i32( 1000.0*a+100.0*b ), 0, 0, 0]
}
)";
  ExecVerifyScript( src, 1, 1, [](mfg_pal::Image32& resTile) {
    // printf("%d\n", resTile.PixelGet( 0, 0 ).Value );
    // だいたい 1000*0.06+100*0.5くらい
    REQUIRE( 110 == resTile.PixelGet( 0, 0 ).Value );
  });  
}},
{"stepのエッジがスカラーのテスト", []{
  auto src = R"(
def result_u8 |x, y| {  
  let [a, b] = step( 1.0, [0.9, 1.1] )
  u8[i32( 10*a+100.0*b ), 0, 0, 0]
}
)";
  ExecVerifyScript( src, 1, 1, [](mfg_pal::Image32& resTile) {
    REQUIRE( 100 == resTile.PixelGet( 0, 0 ).Value );
  });  
}},
{"isinfのテスト", []{
  auto src = R"(
def result_u8 |x, y| {  
  let [a, b] = isinf( [1.0/0.0, 1.0] )
  u8[i32( 2*isinf(1.0/0.0)+a*10+b*100 ), 0, 0, 0]
}
)";
  ExecVerifyScript( src, 1, 1, [](mfg_pal::Image32& resTile) {
    REQUIRE( 12 == resTile.PixelGet( 0, 0 ).Value );
  });  
}},
{"isnanのテスト", []{
  auto src = R"(
def result_u8 |x, y| {  
  let [a, b] = isnan( [1.0/0.0, sqrt(-1.0)] )
  u8[i32( 2*isnan(sqrt(-1.0))+a*10+b*100 ), 0, 0, 0]
}
)";

  // D3Dではなぜかisnanが0を返す。
  // 生成されているコードは正しそうに見えるが原因は良くわからない。
  // とりあえずifdefで切る。
  #ifndef MFG_BACKEND_D3D
  ExecVerifyScript( src, 1, 1, [](mfg_pal::Image32& resTile) {
    REQUIRE( 102 == resTile.PixelGet( 0, 0 ).Value );
  });  
  #endif
}},
{"logとlog2のテスト", []{
  auto src = R"(
def result_u8 |x, y| {  
  let [a, b] = log([3.0, 5.0])
  u8[i32( 10.0*log(10.0)+10.0*log2(10.0)+10.0*a+10.0*b ), 0, 0, 0]
}
)";
  ExecVerifyScript( src, 1, 1, [](mfg_pal::Image32& resTile) {
    // 23+33+10+16に端数
    REQUIRE( 83 == resTile.PixelGet( 0, 0 ).Value );
  });  
}},
{"minの4項のテスト", []{
  auto src = R"(
def result_u8 |x, y| {  
  u8[min(4, 2, 5, 4), 0, 0, 0]
}
)";
  ExecVerifyScript( src, 1, 1, [](mfg_pal::Image32& resTile) {
    REQUIRE( 2 == resTile.PixelGet( 0, 0 ).Value );
  });
}},
{"+の両方タプルのテスト", []{
  auto src = R"(
def result_u8 |x, y| {  
  let [a, b] = [1, 2.0] + [3, 4.2]
  u8[a + i32(10.0*b), 0, 0, 0]
}
)";
  ExecVerifyScript( src, 1, 1, [](mfg_pal::Image32& resTile) {
    REQUIRE( 4+62 == resTile.PixelGet( 0, 0 ).Value );
  });
}},
{"使っていない中間バッファがあっても落ちないテスト", []{ // 以前ReleaseDeadBufferで落ちたので（#543）
  auto src = R"(
  @bounds(4)
  def tmp |v| { 0 }

  def result_u8 |x, y| {
    u8[0, 0, 0, 0]
  }
)";
  ExecVerifyScript( src, 1, 1, [](mfg_pal::Image32& resTile) {
    REQUIRE( 0 == resTile.PixelGet( 0, 0 ).Value );
  });

}},
{"累乗のvectorize", []{
  auto src = R"(
  def result_u8 |x, y| {
    let [a, b, c] = [3, 3, 3]^[2, 2, 2]
    u8[a+b+c, 0, 0, 0]
  }
)";
  ExecVerifyScript( src, 1, 1, [](mfg_pal::Image32& resTile) {
    REQUIRE( 9*3 == resTile.PixelGet( 0, 0 ).Value );
  });

}},
{"condがベクトルのifel", []{
  auto src = R"(
  def result_u8 |x, y| {
    let [a, b, c] = ifel([1, 0, 1], [1, 1, 1], [10, 10, 10] )
    u8[a+b+c, 0, 0, 0]
  }
)";
  ExecVerifyScript( src, 1, 1, [](mfg_pal::Image32& resTile) {
    REQUIRE( 12 == resTile.PixelGet( 0, 0 ).Value );
  });
}},
{"|のベクトライズ（binop)", []{
  auto src = R"(
  def result_u8 |x, y| {
    let v1 = [1, 2]
    let v2 = [0, 1]
    let v3 = v1 | v2
    u8[v3.x+v3.y, 0, 0, 0]
  }
)";
  ExecVerifyScript( src, 1, 1, [](mfg_pal::Image32& resTile) {
    REQUIRE( 4 == resTile.PixelGet( 0, 0 ).Value );
  });
}},
{"!のベクトライズ", []{
  auto src = R"(
  def result_u8 |x, y| {
    let v1 = [2, 0]
    let v2 = !v1
    u8[v2.x+v2.y, 0, 0, 0]
  }
)";
  ExecVerifyScript( src, 1, 1, [](mfg_pal::Image32& resTile) {
    REQUIRE( 1 == resTile.PixelGet( 0, 0 ).Value );
  });
}},
{"!と|のベクトライズ（クラッシュしたケース）", []{
  auto src = R"(
  def result_u8 |x, y| {
    let v1 = [1, 2]
    let v2 = [0, 1]
    let v3 = v1 | !v2
    u8[v3.x+v3.y, 0, 0, 0]
  }
)";
  ExecVerifyScript( src, 1, 1, [](mfg_pal::Image32& resTile) {
    REQUIRE( 3 == resTile.PixelGet( 0, 0 ).Value );
  });
}},
{"logにintを渡すとexceptionになるかのテスト", []{
  auto src = R"(
def result_u8 |x, y| {
  let a = log(10)
  u8[a, 0, 0, 0]
}
)";
  VerifyParseError( src );
}},
{"式が並んでいる時にエラーになるか", []{
  auto src = R"(
def result_u8 |x, y| {
  u8[0, 0, 0, 0]
  u8[0, 0, 0, 0]
}
)";
  VerifyParseError( src );
}},
{"maxのvectorize", []{
  auto src = R"(
  def result_u8 |x, y| {
    let [a, b, c] = max([3, 2, 1], [0, 2, 4])
    u8[a+b+c, 0, 0, 0]
  }
)";
  ExecVerifyScript( src, 1, 1, [](mfg_pal::Image32& resTile) {
    REQUIRE( 9 == resTile.PixelGet( 0, 0 ).Value );
  });

}},
{"minのvectorize", []{
  auto src = R"(
  def result_u8 |x, y| {
    let [a, b, c] = min([3.0, 2.0, 1.0], [0.0, 2.0, 4.0])
    u8[i32(a+b+c), 0, 0, 0]
  }
)";
  ExecVerifyScript( src, 1, 1, [](mfg_pal::Image32& resTile) {
    REQUIRE( 3 == resTile.PixelGet( 0, 0 ).Value );
  });

}},
{"saturateのテスト", []{
  auto src = R"(
  def result_u8 |x, y| {
    let [a, b, c] = saturate([-1.0, 0.3, 2.0])
    let d = saturate(0.7)
    u8[i32(10*a+10*b+10*c+10*d), 0, 0, 0]
  }
)";
  ExecVerifyScript( src, 1, 1, [](mfg_pal::Image32& resTile) {
    REQUIRE( 20 == resTile.PixelGet( 0, 0 ).Value );
  });

}},
{"exp2のテスト", []{
  auto src = R"(
  def result_u8 |x, y| {
    let [a, b, c] = exp2([1.0, 2.0, 3.0])
    u8[i32(a+b+c+exp2(4.0)), 0, 0, 0]
  }
)";
  ExecVerifyScript( src, 1, 1, [](mfg_pal::Image32& resTile) {
    REQUIRE( (1+4+9+16) == resTile.PixelGet( 0, 0 ).Value );
  });

}},
{"atan2のベクトライズのテスト", []{
  // atan2は4要素までしか対応しない。2引数のベクトライズは対応してないから。
  auto src = R"(
def result_u8 |x, y| {
  let vec = atan2([1.0, 2.0, 3.0, 4.0], [5.0, 6.0, 7.0, 8.0])
  let a = vec.0+vec.1+vec.2+vec.3
  u8[a*100, 0, 0, 0]
}
)";

  ExecVerifyScript( src, 1, 1, [](mfg_pal::Image32& resTile) {
    // 1.387685
    REQUIRE( 138 == resTile.PixelGet( 0, 0 ).Value );
  });
}},

{"allのテスト", []{
  auto src = R"(
  def result_u8 |x, y| {
    ifel( all([x, y] == [1, 1]), u8[1, 0, 0, 0], u8[0xff, 0xff, 0xff, 0xff] )
  }
)";
  ExecVerifyScript( src, 2, 2, [](mfg_pal::Image32& resTile) {
    REQUIRE( 1 == resTile.PixelGet( 1, 1 ).Value );
    REQUIRE( 0xffffffff == resTile.PixelGet( 0, 0 ).Value );
    REQUIRE( 0xffffffff == resTile.PixelGet( 0, 1 ).Value );
  });
}},
{"anyのテスト", []{
  auto src = R"(
  def result_u8 |x, y| {
    ifel( any([x, y] == [1, 1]), u8[1, 0, 0, 0], u8[0xff, 0xff, 0xff, 0xff] )
  }
)";
  ExecVerifyScript( src, 2, 2, [](mfg_pal::Image32& resTile) {
    REQUIRE( 1 == resTile.PixelGet( 1, 1 ).Value );
    REQUIRE( 0xffffffff == resTile.PixelGet( 0, 0 ).Value );
    REQUIRE( 1 == resTile.PixelGet( 0, 1 ).Value );
  });
}},
{"swizzleのwのテスト", []{
  auto src = R"(
  def result_u8 |x, y| {
    let tup = [1, 2, 3, 4]
    let tup2 = tup.wywz
    u8(tup2)
  }
)";
  ExecVerifyScript( src, 1, 1, [](mfg_pal::Image32& resTile) {
    REQUIRE( 0x03040204 == resTile.PixelGet( 0, 0 ).Value );
  });

}},
{"swizzleのテスト、変数を介さないケース", []{
  auto src = R"(
  def result_u8 |x, y| {
    let tup2 = [1, 2, 3, 4].wywz
    u8(tup2)
  }
)";
  ExecVerifyScript( src, 1, 1, [](mfg_pal::Image32& resTile) {
    REQUIRE( 0x03040204 == resTile.PixelGet( 0, 0 ).Value );
  });

}},
{"swizzleのテスト、結果がスカラーになるケース", []{
  auto src = R"(
  def result_u8 |x, y| {
    let a = [1, 2, 3, 4].z
    u8[a, 0, 0, 0]
  }
)";
  ExecVerifyScript( src, 1, 1, [](mfg_pal::Image32& resTile) {
    REQUIRE( 0x00000003 == resTile.PixelGet( 0, 0 ).Value );
  });

}},
{"タプルのindexアクセスのテスト", []{
  auto src = R"(
  def result_u8 |x, y| {
    let a = [1, 2, 3, 4, 5, 6].4
    u8[a, 0, 0, 0]
  }
)";
  ExecVerifyScript( src, 1, 1, [](mfg_pal::Image32& resTile) {
    REQUIRE( 0x00000005 == resTile.PixelGet( 0, 0 ).Value );
  });

}},
{"1次元テンソルリテラル", []{
  auto src = R"(
  def lts by [1, 2, 3]

  def result_u8 |x, y| {
    u8[lts(0), lts(1), lts(2), 0]
  }
)";
  ExecVerifyScript( src, 1, 1, [](mfg_pal::Image32& resTile) {
    REQUIRE( 0x00030201 == resTile.PixelGet( 0, 0 ).Value );
  });
}},
{"2次元テンソルリテラル", []{
  auto src = R"(
  def lts by [[1, 2, 3], [4, 5, 6]]

  def result_u8 |x, y| {
    u8[lts(0, 0)+lts(0, 1), lts(1, 0)+lts(1, 1), lts(2, 0)+lts(2, 1), 0]
  }
)";
  ExecVerifyScript( src, 1, 1, [](mfg_pal::Image32& resTile) {
    REQUIRE( 0x00090705 == resTile.PixelGet( 0, 0 ).Value );
  });
}},
{"2次元テンソルリテラル、float", []{
  auto src = R"(
  def lts by [[1.5, 2.5, 3.5],
              [4.5, 5.5, 6.5]]

  def result_u8 |x, y| {
    u8[lts(0, 0)+lts(0, 1), lts(1, 0)+lts(1, 1), lts(2, 0)+lts(2, 1), 0]
  }
)";
  ExecVerifyScript( src, 1, 1, [](mfg_pal::Image32& resTile) {
    REQUIRE( 0x000a0806 == resTile.PixelGet( 0, 0 ).Value );
  });
}},
{"パッケージからのテクスチャロード", []{
  auto src = R"(
  def lts by load("4rect_texture16.png")

  def result_u8 |x, y| {
    let red = lts(0, 0)
    let green = lts(15, 0)
    let syan = lts(0, 15)
    let blue = lts(15, 15)
    ifel(x ==0, 
      ifel(y==0, red, green),
      ifel(y==0, syan, blue))
  }
)";
  ExecVerifyScript( src, 2, 2, [](mfg_pal::Image32& resTile) {
    REQUIRE( 0xFFFF0000 == resTile.PixelGet( 0, 0 ).Value );
    REQUIRE( 0xFF00FFFF == resTile.PixelGet( 1, 0 ).Value );
    REQUIRE( 0xFF00FF00 == resTile.PixelGet( 0, 1 ).Value );
    REQUIRE( 0xFF0000FF == resTile.PixelGet( 1, 1 ).Value );
  });
}},
{"ローカルテンソルのsumのテスト", []{
  ResetUniqueName();
  auto src = R"(

def result_u8 |x, y| {

  @bounds(4)  
  def loc |v| {
    u8(v+1)
  }

  let v = loc.sum |_, i| { 2*i}

  u8[v, 0, 0, 0]
}
)";

  ExecVerifyScript( src, 1, 1, [](mfg_pal::Image32& resTile) {
    REQUIRE( 2+4+6+8 == resTile.PixelGet( 0, 0 ).Value );
  });
}},
{"ユーザー定義関数", []{
  auto src = R"(
fn udf |x1: i32| {
  (x1+1)*3
}

def result_u8 |x, y| {
  let res = udf(x)
  let res2 = udf(10*y)
  u8[res, res2, 0, 0]
}
)";

  /*
  auto code = ParseAndLowerAndReturnTreeDump( src );
  cout << code << endl;
  */

  ExecVerifyScript( src, 2, 2, [](mfg_pal::Image32& resTile) {
    REQUIRE( 0x00000303 == resTile.PixelGet( 0, 0 ).Value );
    REQUIRE( 0x00000306 == resTile.PixelGet( 1, 0 ).Value );
    // 33 = 16*2+1
    REQUIRE( 0x00002103 == resTile.PixelGet( 0, 1 ).Value );
    REQUIRE( 0x00002106 == resTile.PixelGet( 1, 1 ).Value );
  });
}},
{"ユーザー定義関数, 2引数", []{
  auto src = R"(
fn udf |x1: i32, x2:i32| {
  (x1+1)*3+(x2+1)*4
}

def result_u8 |x, y| {
  let res = udf(x, y)
  u8[res, 0, 0, 0]
}
)";

  ExecVerifyScript( src, 2, 2, [](mfg_pal::Image32& resTile) {
    REQUIRE( 0x00000007 == resTile.PixelGet( 0, 0 ).Value );
    REQUIRE( 0x0000000a == resTile.PixelGet( 1, 0 ).Value );
    REQUIRE( 0x0000000b == resTile.PixelGet( 0, 1 ).Value );
    // 6+8 = 14
    REQUIRE( 0x0000000e == resTile.PixelGet( 1, 1 ).Value );
  });
}},
{"ユーザー定義関数, ベクトル", []{
  auto src = R"(
fn udf |xv: i32v2| {
  (xv.x+1)*3+(xv.y+1)*4
}

def result_u8 |x, y| {
  let res = udf([x, y])
  u8[res, 0, 0, 0]
}
)";

  /*
  auto code = ParseAndLowerAndReturnTreeDump( src );
  cout << code << endl;
  */

  // 2引数の時と同じ結果
  ExecVerifyScript( src, 2, 2, [](mfg_pal::Image32& resTile) {
    REQUIRE( 0x00000007 == resTile.PixelGet( 0, 0 ).Value );
    REQUIRE( 0x0000000a == resTile.PixelGet( 1, 0 ).Value );
    REQUIRE( 0x0000000b == resTile.PixelGet( 0, 1 ).Value );
    // 6+8 = 14
    REQUIRE( 0x0000000e == resTile.PixelGet( 1, 1 ).Value );
  });
}},
{"ユーザー定義関数、グローバル変数の参照", []{
  auto src = R"(
let a = 3
fn udf |x1: i32| {
  (x1+1)*a
}

def result_u8 |x, y| {
  let res = udf(x)
  let res2 = udf(10*y)
  u8[res, res2, 0, 0]
}
)";


  // ユーザー定義関数のテストと同じverify
  ExecVerifyScript( src, 2, 2, [](mfg_pal::Image32& resTile) {
    REQUIRE( 0x00000303 == resTile.PixelGet( 0, 0 ).Value );
    REQUIRE( 0x00000306 == resTile.PixelGet( 1, 0 ).Value );
    // 33 = 16*2+1
    REQUIRE( 0x00002103 == resTile.PixelGet( 0, 1 ).Value );
    REQUIRE( 0x00002106 == resTile.PixelGet( 1, 1 ).Value );
  });
}},
{"ユーザー定義関数の引数型違い", []{
  auto src = R"(
fn udf |x1: i32| {
  (x1+1)*3
}

def result_u8 |x, y| {
  let res = udf(0.1)
  u8[res, 0, 0, 0]
}
)";

  VerifyParseError( src );
}},
{"ユーザー定義関数のテンソル内定義の禁止", []{
  auto src = R"(
def result_u8 |x, y| {
  fn udf |x1: i32| {
    (x1+1)*3
  }

  let res = udf(1)
  u8[res, 0, 0, 0]
}
)";

  VerifyParseError( src );
}},
{"ユーザー定義関数を二回呼んだ時にローカル変数がぶつからないか？", []{
  auto src = R"(
fn udf |x1: i32| {
  let x = x1+1
  x*3
}

def result_u8 |x, y| {
  let res = udf(x)
  let res2 = udf(10*y)
  u8[res, res2, 0, 0]
}
)";
  auto actual = ParseAndLowerAndReturnTreeDump( src );
  // cout << actual << endl;

  REQUIRE( 1 == CountContains(actual, "let i0_u0_x") );
}},
{"ユーザー定義関数のローカル変数の名前の付け替えが、destructuring letでも行えているか？", []{
  auto src = R"(
fn udf |xv: i32v2| {
  let [e1, e2] = xv
  e1+e2
}

def result_u8 |x, y| {
  let res = udf([x, y])
  let res2 = udf([2*x, 2*y])
  u8[res, res2, 0, 0]
}
)";
  auto actual = ParseAndLowerAndReturnTreeDump( src );
  // cout << actual << endl;

  REQUIRE( 1 == CountContains(actual, "let i1_u0_e1") );
}},
{"ユーザー定義関数のローカル変数の名前の付け替えが、rsumでも行えているか？", []{
  auto src = R"(
fn udf |x: i32| {
  rsum(0..<3) |i| {
    i+x
  }
}

def result_u8 |x, y| {
  let res = udf(0)
  let res2 = udf(1)
  u8[res, res2, 0, 0]
}
)";

  if(SECTION("展開が正しく行われているか")){SG g;
    auto actual = ParseAndLowerAndReturnTreeDump( src );
    // cout << actual << endl;

    REQUIRE( 1 == CountContains(actual, "var rres.i1_r0.0 ") );
  }
  if (SECTION("実行して結果が合うか")){SG g;
    ExecVerifyScript( src, 1, 1, [](mfg_pal::Image32& resTile) {
      // printf("%x\n", resTile.PixelGet( 0, 0 ).Value );
      REQUIRE( 0x00000603 == resTile.PixelGet( 0, 0 ).Value );
    });
  }
}},
#endif
{"mfgでリークが無いかの確認。", []{
  #ifdef MFG_BACKEND_D3D
  ReleaseD3DAllDll();
  #endif

  REQUIRE( g_node_alloc_count == 0 );
  REQUIRE( g_irelem_alloc_count == 0 );
}}
};



static std::vector<TestPair> test_cases_addhook = {
{"MFGのアドホックなテスト", []{
}}
};


void RegisterMFGScriptTest(std::vector<TestPair>& testCases)
{
  testCases.insert(testCases.end(), test_cases.begin(), test_cases.end());
  testCases.insert(testCases.end(), test_cases_verify_script.begin(), test_cases_verify_script.end());
  // testCases.insert( testCases.end(), test_cases_addhook.begin(), test_cases_addhook.end() );
}