/* -*- coding: utf-8 -*- マルチバイト */

// Copyright 2026 PGN Inc. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#ifndef MFG_PAL_MFG_CLI_HPP_
#define MFG_PAL_MFG_CLI_HPP_

#include <open_pal.hpp>

namespace mfg_pal {

// nstring はmfg_cliではUTF-8 (std::string)
// Windowsの空白などのパス周りでトラブルがあるが、mfg_cliはそういうものとする。
using nstring = std::string;
using nchar = char;
using nstringstream = std::stringstream;

namespace uz = cppunzip;

// Callbackはcliでは何もしない
typedef bool (*TProgressCallback)();
inline bool CallbackPercent( TProgressCallback callback, double percent ) { return true; }

bool DecodePng32( const std::vector<uint8_t>& buf, mfg_pal::Image32& dest );
bool SavePng32( const mfg_pal::Image32& img, nstring filename );
std::unique_ptr<uz::File> GetZipFile( const nstring& filepath );
}


#endif

