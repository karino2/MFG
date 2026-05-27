// Copyright 2026 PGN Inc. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

/* -*- coding: utf-8 -*- マルチバイト */

#include <mfg_pal/pal.hpp>

#ifdef _WIN32
  #include <windows.h>
#else
  #include <sys/time.h>
#endif

namespace mfg_pal
{

// 大して重要では無いのでFNV1ハッシュにしておく
// FNV-1 32bit hash https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
uint32_t Hash32( const uint8_t* key, size_t len, uint32_t seed )
{
  uint32_t h = seed+2166136261;
  uint32_t prime = 16777619;

  for( auto i : NRange(len))
  {
    h = (h ^ key[i]) * prime;
  }

  return h;
}

uint32_t Rand32()
{
  return (uint32_t)rand();
}

size_t Tick()
{
  size_t res = 0;
#ifdef _WIN32
  res = GetTickCount();
#else
  struct timeval t;
  gettimeofday(&t, NULL);
  res = (size_t)(t.tv_sec*1000+(t.tv_usec/1000));
#endif
  return res;
}


} ///<mfg_pal
