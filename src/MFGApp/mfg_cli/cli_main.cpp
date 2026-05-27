// Copyright 2026 Kazuma Arino. All rights reserved.
// SPDX-License-Identifier: Apache-2.0


#include <iostream>
#include <chrono>

#include "mfg.hpp"


#ifdef MFG_BACKEND_METAL

#include "mfg_runtime_metal.hpp"
using namespace mfg_runtime_metal;
using TargetBackend = MetalBackend;

#elif MFG_BACKEND_D3D

#include "mfg_runtime_d3d.hpp"
using namespace mfg_runtime_d3d;
using TargetBackend = D3DBackend;

using namespace mfg_hlsl_converter;

#endif

#include "app_util.hpp"
using namespace mfg_cli_app_util;

using namespace std;

static void PrintUsage()
{
  cout << "Usage:\n";
  cout << "  mfg_cli [options] filename\n";
  cout << "Examples:\n";
  cout << "  mfg_cli -ir example.mfg\n";
  cout << "  mfg_cli -shader example.mfg\n";
  cout << "  mfg_cli -target input.png -result result.png example.mfg\n";
  cout << "Options\n";
  cout << "  -ir: print low level IR to stdout.\n";
  cout << "  -shader: compile and print result shader to stdout.\n";
  cout << "  -target filename: specify the target image file.\n";
  cout << "  -result filename: specify the result png name to store what the filter is applied to target image.\n";
  cout << "Notes\n";
  cout << "  There are three mode: -ir, -shader, and standard.\n";
  cout << "  -ir, -shader, standard is exclusive.\n";
  cout << "  For standard mode, you have to specify -target and -result options.\n";
  cout << "  Input mfg file is either .mfg or .mar file.\n";
}

struct Compiler
{
  mfg_parser::TreeBuilder _builder;
  mfg_parser::Parser _parser;

  Compiler( const std::string& source ) : _builder(), _parser( _builder, source )
  {
  }

  void Parse()
  {
    _parser.ParseAll();
  }

  void Lower()
  {
    _builder._binary->LowerAndAnalyze();
  }

  void ParseAndLower()
  {
    Parse();
    Lower();
  }

  std::vector<mfg_binary::TLEntityRef>& GetTopLevelEntities() { return _builder._binary->_tles; }
};

static void PrintIR( const std::string& path )
{
  auto source = ReadText( path );

  Compiler compiler( source );
  compiler.ParseAndLower();

  for( auto& tle : compiler.GetTopLevelEntities() )
  {
    auto ir = PrintExprS( tle.AsTopLevel()->GetRootElem()->GetReturnExpr() );
    cout << ir << endl;
    cout << endl;
  }
}

static void PrintShader( const std::string& path )
{
  auto source = ReadText( path );
  Compiler compiler( source );
  compiler.ParseAndLower();

  auto shader = GenShaderSources( *compiler._builder._binary );
  for (const auto& src : shader)
  {
    cout << src << endl;
    cout << endl;
  }
}

static void ApplyFilter( const std::string& mfgPath, const std::string& targetPath, const std::string& resultPath )
{
  auto source = ReadText( mfgPath );

  mfg::MFG facade;
  auto bin = facade.Compile( source );

  mfg_pal::Image32 input;
  mfg_pal::Image32 result;

  if (!mfg_pal::DecodePng32( ReadContent( targetPath ), input ))
  {
    cerr << "Failed to decode target image: " << targetPath << endl;
    return;
  }

  if (!result.Resize( input.Width(), input.Height() ))
  {
    cerr << "Failed to resize result image." << endl;
    return;
  }

  InputTiles tiles;
  tiles.Insert( 0, &input );
  TileReference resultRef( &result );

  if (!facade.Run( bin, tiles, resultRef ))
  {
    cerr << "Failed to run filter." << endl;
    return;
  }

  if (!mfg_pal::SavePng32( result, resultPath ))
  {
    cerr << "Failed to save result image: " << resultPath << endl;
    return;
  }

}

int main(int argc, char* argv[])
{
  bool DUMP_IR = false;
  bool PRINT_SHADER = false;

  string mfgPath;
  string targetPath;
  string resultPath;
  if (argc == 1 ||
     (argc == 2 && argv[1][0] == '-')) // -hとか-?のみだった時にもusageが出るように。
  {
    PrintUsage();
    return 1;
  }

  for (int i = 1; i < argc; i++)
  {
    if (argv[i][0] != '-')
    {
      mfgPath = argv[i];
    }
    else {
      string option(argv[i]);
      if (option == string("-ir"))
      {
        DUMP_IR = true;
      }
      else if(option == string("-shader"))
      {
        PRINT_SHADER = true;
      }
      else if(option == string("-target"))
      {
        if (i + 1 < argc)
        {
          targetPath = argv[++i];
        }
        else
        {
          PrintUsage();
          return 1;
        }
      }
      else if(option == string("-result"))
      {
        if (i + 1 < argc)
        {
          resultPath = argv[++i];
        }
        else
        {
          PrintUsage();
          return 1;
        }
      }
      else
      {
        PrintUsage();
        return 1;
      }
    }
  }
  if (mfgPath == string(""))
  {
    PrintUsage();
    return 1;
  }

  if (DUMP_IR)
  {
    PrintIR( mfgPath );
    return 0;
  }

  if (PRINT_SHADER)
  {
    PrintShader( mfgPath );
    return 0;
  }

  if (targetPath == string("") || resultPath == string(""))
  {
    PrintUsage();
    return 1;
  }

  ApplyFilter( mfgPath, targetPath, resultPath );

  return 0;
}