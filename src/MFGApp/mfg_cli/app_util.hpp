// Copyright 2026 Kazuma Arino. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#ifndef APP_UTIL_HPP_
#define APP_UTIL_HPP_

#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include "forest.hpp"
#include "mfg.hpp"
#include <mfg_pal/pal.hpp>

#ifdef MFG_BACKEND_METAL
#include "mfg_runtime_metal.hpp"
#elif defined(MFG_BACKEND_D3D)
#include "mfg_runtime_d3d.hpp"
#endif


namespace mfg_cli_app_util
{
using namespace mfg_internal;
using namespace std;


#ifdef MFG_BACKEND_METAL
using namespace mfg_runtime_metal;
inline std::vector<std::string> GenShaderSources( IRBinary& target )
{
  std::vector<std::string> res;

  MetalBackend backend;
  auto libBeg = target._tles.begin();
  auto originItr = libBeg;
  while (libBeg != target._tles.end())
  {
    auto libEnd = backend.LookupLibraryBoundary( target._tles, libBeg );
    size_t originIndex = (size_t)(libBeg - originItr);

    LibraryExecutor executor( backend._ctx, backend._logStream, backend._bufMap, backend._renv, target, libBeg, libEnd, originIndex );
    executor.SetupGenerator();
    std::string src = executor.CreateLibrarySource( backend.GetDebugLogger() );
    res.push_back( std::move(src) );

    libBeg = libEnd;
  }

  return res;
}
#elif defined(MFG_BACKEND_D3D)
using namespace mfg_runtime_d3d;
// D3Dの場合LibraryとTLE一つは同じだが、Metalと共通化するために同じ名前にしておく。
inline std::vector<std::string> GenShaderSources( IRBinary& target )
{
  std::vector<std::string> res;

  D3DBackend backend;

  for( auto tlePos: NRange(target._tles.size()) )
  {
    auto& tle = target._tles[tlePos];

    // RunOneTLEと類似
    switch (tle._etype)
    {
      case TLEntityRef::TENSOR:
      {
        auto tltensor = tle.u._tensor;

        // 使われていないテンソルのカーネルはスキップする。
        if (tltensor->LastReference() >= 0)
        {
          TLTensorKernelExecutor exec( backend._ctx, backend._bufMap, backend._renv, target, *tltensor, backend.GetDebugLogger());
          res.push_back( exec.GenSource() );
        }

        break;
      }
      case TLEntityRef::TOP_LEVEL_BLOCK:
      {
        auto tblock = tle.u._tblock;

        TopLevelBlockKernelExecutor exec( backend._ctx, backend._logStream, backend._bufMap, backend._renv, target, *tblock, backend.GetDebugLogger(), target.GetSeedGen() );
        res.push_back( exec.GenSource() );

        break;
      }
      case TLEntityRef::UNBOUND:
        assert(false);
        break;
    }
 }

  return res;
}
#endif


inline std::string PrintExprS( const IRElem* elem )
{
  stringstream sstream;
  PrintExpr( sstream, elem );
  return sstream.str();
}



inline std::vector<uint8_t> ReadContent(const std::string& filename)
{
  std::ifstream file(filename, std::ios::binary | std::ios::ate);

  if (!file.is_open()) {
    throw std::runtime_error("Failed to open file: " + filename);
  }

  std::streamsize size = file.tellg();

  file.seekg(0, std::ios::beg);

  std::vector<uint8_t> buffer(size);

  if (file.read(reinterpret_cast<char*>(buffer.data()), size)) {
      return buffer;
  }

  throw std::runtime_error("Failed to read file: " + filename);
}

inline std::string ReadText(const std::string& filename)
{
  std::ifstream file(filename);

  if (!file.is_open()) {
    throw std::runtime_error("Failed to open file: " + filename);
  }

  std::stringstream buffer;
  buffer << file.rdbuf();
  return buffer.str();
}


}///<mfg_cli_app_util
#endif
