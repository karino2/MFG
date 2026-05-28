/* -*- coding: utf-8 -*- マルチバイト */

// Copyright 2026 Kazuma Arino. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#include "mfg.hpp"
#include "mfg_runtime_d3d.hpp"
#include <iostream>

namespace mfg_runtime_d3d {

// COM周辺はnullptrでは無くNULLを使う
PFN_D3D11_CREATE_DEVICE g_D3DCreateDevice = NULL;
pD3DCompile g_D3DCompile = NULL;
pD3DDisassemble g_D3DDisassemble = NULL;

std::unique_ptr<D3DAllDllLoader> g_dllLoader;

void EnsureD3DAllDllLoaded()
{
  if (g_dllLoader.get() == nullptr)
  {
    g_dllLoader.reset( new D3DAllDllLoader() );
  }
}
void ReleaseD3DAllDll()
{
  g_dllLoader.reset( nullptr );
}


}///< mfg_runtime_d3d