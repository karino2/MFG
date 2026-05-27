/* -*- coding: utf-8 -*- マルチバイト */

// Copyright 2026 PGN Inc. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#ifndef MFG_PAL_MFG_CLI_HPP_
#define MFG_PAL_MFG_CLI_HPP_

#include <open_pal.hpp>

namespace mfg_pal {

// nstring はQtのアプリではwstirng
using nstring = std::wstring;
using nchar = wchar_t;
using nstringstream = std::wstringstream;


// MFGStudioではpercentしか使わない。
struct CallbackInfo
{
    double _percent = 0.0;
};

typedef bool (*TProgressCallback)( CallbackInfo );
inline bool CallbackPercent( TProgressCallback callback, double percent )
{
    CallbackInfo info;
    info._percent = percent;
    return callback( info );
}


bool DecodePng32( const std::vector<uint8_t>& buf, mfg_pal::Image32& dest );
std::unique_ptr<uz::File> GetZipFile( const nstring& filepath );
}


#endif

