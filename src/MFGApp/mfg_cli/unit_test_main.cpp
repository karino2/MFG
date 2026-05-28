// Copyright 2026 Kazuma Arino. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#include "nfiftest.hpp"
using namespace nfiftest;

extern void RegisterForestTest(std::vector<TestPair>& testCases);
extern void RegisterMFGTest(std::vector<TestPair>& testCases);
#ifdef MFG_BACKEND_METAL
extern void RegisterMFGMetalTest(std::vector<TestPair>& testCases);
#endif
#ifdef MFG_BACKEND_D3D
extern void RegisterMFGD3DTest( std::vector<TestPair>& testCases );
#endif
extern void RegisterMFGScriptTest(std::vector<TestPair>& testCases);



int main()
{
  std::vector<TestPair> test_cases;

  RegisterForestTest( test_cases );
  RegisterMFGTest( test_cases );
#ifdef MFG_BACKEND_METAL
  RegisterMFGMetalTest( test_cases );
#endif
#ifdef MFG_BACKEND_D3D
  RegisterMFGD3DTest( test_cases );
#endif
  RegisterMFGScriptTest( test_cases );

  RunTests(test_cases);

  return 0;
}