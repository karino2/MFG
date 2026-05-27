/* -*- coding: utf-8 -*- マルチバイト */

// Copyright 2026 Kazuma Arino. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#ifndef MFG_TOOL_HPP__
#define MFG_TOOL_HPP__

/*
  mfg_tool.hpp

  STLなどにだけ依存する、mfg共通のユーティリティなどを置くファイル。
  mfg_XXXに依存するものは置かない。  
*/

#include <mfg_pal/pal.hpp>
#include "forest.hpp"
#include <algorithm> // for_each
#include <cassert>
#include <cstring>
#include <iterator>
#include <stdexcept>
#include <sstream>
#include <string>
#include <vector>


namespace mfg_internal
{
using namespace mfg_pal;

//
// RevRange関連
// 
// リバースのNRangeでRevRange。
// end-1から0までのiterator
//
// 今の所mfgでしか使ってないのでここに置く。
// 他でも使う時が来たらlibneet_stl_util.hに昇格。

// ほとんどNCountableRangeと同じだが、Reverse。
// テンプレートで共通化するよりもdupする方がシンプルだったのでdupする事に。
template<typename T>
class RevCountableRange
{
  T _rangeEnd;

  
  class range_iterator
  {
    T _current;
    T _end;

  public:
    T operator *() const { return _end - _current -1; }
    const range_iterator &operator ++()
    {
      ++_current;
      return *this;
    }

    bool operator ==(const range_iterator &other) const { return _current == other._current; }
    bool operator !=(const range_iterator &other) const { return _current != other._current; }

    range_iterator(T pos, T end) : _current(pos), _end(end) { }
  };


public:
  RevCountableRange( T end ) : _rangeEnd(end) {}
  range_iterator begin() const { return range_iterator( 0, _rangeEnd ); }
  range_iterator end() const { return range_iterator( _rangeEnd, _rangeEnd ); }
};


// end-1から0までのiterator。
template<typename T>
RevCountableRange<T> RevRange( T end ) { return RevCountableRange<T>( end ); }


template<typename T>
void AppendTail( std::vector<T>& dest, std::vector<T>&& tail )
{
  dest.insert( dest.end(), std::make_move_iterator( tail.begin() ), std::make_move_iterator( tail.end() ) );
}

template<typename C, typename F>
void ForEachFn( C& container, F fun )
{
  std::for_each( container.begin(), container.end(), fun );
}

// 上と一つに出来そうだけど良く分からなかった。
template<typename C, typename F>
void ForEachFn( C&& container, F fun )
{
  std::for_each( std::make_move_iterator( container.begin() ), std::make_move_iterator( container.end() ), fun );
}

/*
  Error関連クラス
  https://www.notion.so/pixiv/Exception-API-cc8804ce5a034384a3a8ddc07510b266?pvs=4
*/

// MFGのエラーの共通基底クラス
struct Error : public std::runtime_error
{
  Error( const std::string& msg ) : std::runtime_error( msg ) {}
};

// 処理系内部のエラー。これが起きたら処理系を直すべきもの。
struct InternalError : public Error
{
  InternalError( const std::string& msg ) : Error( msg ) {}
};

// GPGPU関連のセットアップ失敗など
struct SystemError : public Error
{
  SystemError( const std::string& msg ) : Error( msg ) {}
};


// スクリプトの文法違反など、ユーザーの入力が誤っている事を表すエラー。
struct UserError : public Error
{
  UserError( const std::string& msg ) : Error( msg ) {}
};

/*
そのプレフィクスで始まる一意の名前を返す。
'r'なら、r0, r1, r2, など。

  ユーザー定義の名前が含まれるもののルールは大まかに以下になる。
  （ただし古いUnitTestなどでこれ以外のfというprefixが使われている事はある）
  
  カーネルの名前: k_
  カーネルに対応するバッファ変数名: b_
  ユーザー定義の変数: u_
  内部で生成される変数: t_
  インライン関数内のローカル変数: v_
  rsumなどで生成される変数: r_
  exntet: extent_[KERNEL_NAME]_

  基本的に二文字以上で始めればこれらの変数とはぶつからない(extentを除く)。  
*/
std::string UniqueName( char prefix );

void ResetUniqueName();

/*
  Format関連

  C++ 20のstd::formatのサブセットとして、{0}や{1}などだけ対応する。
  https://www.notion.so/pixiv/Flang-2d6532d82a3c43caa87ef73879f7c882?pvs=4

  引数の数ごとにオーバーロードして、単純なコードにしておく。
  リソース関連でしか使われないので、エラーのケースはそんなに真面目にチェックはせず、
  リソースのミスと思われるものをInternalErrorとしてthrowする程度に留める。
  
*/

// 0引数のFormat。variadic templateで使う
inline std::string Format( const std::string& fmt )
{
  return fmt;
}


// 1引数のFormat
template<typename T>
std::string Format( const std::string& fmt, T arg1 )
{
  std::ostringstream os;

  auto beg = fmt.find("{0}");
  if (beg == std::string::npos)
    throw InternalError( "Expect 1 arg, but no {0} in format." );

  os << fmt.substr( 0, beg ) << arg1 << fmt.substr( beg+3 );
  return os.str();
}

/*
  2引数のFormat
*/

// 引数の順番が小さい順になっているFormatのためのユーティリティ。
// beg1 < beg2 でエラーチェックも呼ぶ側でしてある。
template<typename T1, typename T2>
std::string FormatOrdered2( const std::string& fmt, size_t beg1, size_t beg2, T1 arg1, T2 arg2 )
{
  std::ostringstream os;

  os << fmt.substr( 0, beg1 ) << arg1 << fmt.substr( beg1+3, beg2-(beg1+3) ) << arg2 << fmt.substr( beg2+3 );
  return os.str();  
}


template<typename T1, typename T2>
std::string Format( const std::string& fmt, T1 arg1, T2 arg2 )
{
  auto beg1 = fmt.find("{0}");
  auto beg2 = fmt.find("{1}");
  if (beg1 == std::string::npos )
    throw InternalError( "Expect 2 arg, but no {0} in format." );
  if (beg2 == std::string::npos)
    throw InternalError( "Expect 2 arg, but no {1} in format." );

  if (beg1 < beg2)
    return FormatOrdered2( fmt, beg1, beg2, arg1, arg2 );
  else
    return FormatOrdered2( fmt, beg2, beg1, arg2, arg1 );
}

// 引数の順番が小さい順になっているFormatのためのユーティリティ。
// beg1 < beg2 < beg3でエラーチェックも呼ぶ側でしてある。
template<typename T1, typename T2, typename T3>
std::string FormatOrdered3( const std::string& fmt, size_t beg1, size_t beg2, size_t beg3, T1 arg1, T2 arg2, T3 arg3 )
{
  std::ostringstream os;

  os << fmt.substr( 0, beg1 ) << arg1 << fmt.substr( beg1+3, beg2-(beg1+3) ) << arg2 << fmt.substr( beg2+3, beg3-(beg2+3) ) << arg3 << fmt.substr( beg3+3 );
  return os.str();  
}

template<typename T1, typename T2, typename T3, typename T4>
std::string FormatOrdered4( const std::string& fmt, size_t beg1, size_t beg2, size_t beg3, size_t beg4, T1 arg1, T2 arg2, T3 arg3, T4 arg4 )
{
  std::ostringstream os;

  os << fmt.substr( 0, beg1 ) << arg1 << fmt.substr( beg1+3, beg2-(beg1+3) ) << arg2 << fmt.substr( beg2+3, beg3-(beg2+3) ) << arg3 << fmt.substr( beg3+3, beg4-(beg3+3) )<< arg4 << fmt.substr( beg4+3 );
  return os.str();  
}


template<typename T1, typename T2, typename T3>
std::string Format( const std::string& fmt, T1 arg1, T2 arg2, T3 arg3 )
{
  auto beg1 = fmt.find("{0}");
  auto beg2 = fmt.find("{1}");
  auto beg3 = fmt.find("{2}");
  if (beg1 == std::string::npos )
    throw InternalError( "Expect 3 arg, but no {0} in format." );
  if (beg2 == std::string::npos)
    throw InternalError( "Expect 3 arg, but no {1} in format." );
  if (beg3 == std::string::npos)
    throw InternalError( "Expect 3 arg, but no {2} in format." );


  if (beg1 < beg2)
  {
    if (beg2 < beg3)
    {
      return FormatOrdered3( fmt, beg1, beg2, beg3, arg1, arg2, arg3 );
    }
    else // beg2 > beg3 && beg2 > beg1
    {
      if (beg1 < beg3)
      {
        return FormatOrdered3( fmt, beg1, beg3, beg2, arg1, arg3, arg2 );
      }
      else
      {
        return FormatOrdered3( fmt, beg3, beg1, beg2, arg3, arg1, arg2 );
      }
    }
  }
  else // beg2 < beg1
  {
    if (beg1 < beg3) // beg2 < beg1 < beg3
    {
      return FormatOrdered3( fmt, beg2, beg1, beg3, arg2, arg1, arg3 );
    }
    else // beg2 < beg1 && beg3 < beg1
    {
      if (beg2 < beg3)
        return FormatOrdered3( fmt, beg2, beg3, beg1, arg2, arg3, arg1 );
      else
        return FormatOrdered3( fmt, beg3, beg2, beg1, arg3, arg2, arg1 );
    }

  }
}

// 3つくらいなら手作業ｄえいいかと思ったら4つが必要になった…
// 今更再帰で書き直す気も起こらないので、もう気合で全部場合分けする。
template<typename T1, typename T2, typename T3, typename T4>
std::string Format( const std::string& fmt, T1 arg1, T2 arg2, T3 arg3, T4 arg4 )
{
  auto beg1 = fmt.find("{0}");
  auto beg2 = fmt.find("{1}");
  auto beg3 = fmt.find("{2}");
  auto beg4 = fmt.find("{3}");
  if (beg1 == std::string::npos )
    throw InternalError( "Expect 4 arg, but no {0} in format." );
  if (beg2 == std::string::npos)
    throw InternalError( "Expect 4 arg, but no {1} in format." );
  if (beg3 == std::string::npos)
    throw InternalError( "Expect 4 arg, but no {2} in format." );
  if (beg4 == std::string::npos)
    throw InternalError( "Expect 4 arg, but no {3} in format." );


  if (beg1 < beg2)
  {
    if (beg2 < beg3)
    {
      // 1<2<3, 4の位置を探す
      if (beg3 < beg4)
      {
        return FormatOrdered4( fmt, beg1, beg2, beg3, beg4, arg1, arg2, arg3, arg4 );
      }
      else  if (beg2 < beg4) // 1<2<4<3
      {
        return FormatOrdered4( fmt, beg1, beg2, beg4, beg3, arg1, arg2, arg4, arg3 );
      }
      else if (beg1 < beg4) // 1<4<2<3
      {
        return FormatOrdered4( fmt, beg1, beg4, beg2, beg3, arg1, arg4, arg2, arg3 );
      }
      else // 4<1<2<3
      {
        return FormatOrdered4( fmt, beg4, beg1, beg2, beg3, arg4, arg1, arg2, arg3 );
      }
    }
    else // beg2 > beg3 && beg2 > beg1
    {
      if (beg1 < beg3)
      {
        // 1<3<2, 4の位置を探す
        //         return FormatOrdered3( fmt, beg1, beg3, beg2, arg1, arg3, arg2 );
        if (beg2 < beg4)
        {
          return FormatOrdered4( fmt, beg1, beg3, beg2, beg4, arg1, arg3, arg2, arg4 );

        }
        else if(beg3 < beg4)
        {
          return FormatOrdered4( fmt, beg1, beg3, beg4, beg2, arg1, arg3, arg4, arg2 );

        }
        else if(beg1 < beg4)
        {
          return FormatOrdered4( fmt, beg1, beg4, beg3, beg2, arg1, arg4, arg3, arg2 );

        }
        else
        {
          return FormatOrdered4( fmt, beg4, beg1, beg3, beg2, arg4, arg1, arg3, arg2 );
        }
      }
      else
      {
        // 3<1<2, 4の位置を探す
        if (beg2 < beg4)
        {
          return FormatOrdered4( fmt, beg3, beg1, beg2, beg4, arg3, arg1, arg2, arg4 );
        }
        else if(beg1 < beg4)
        {
          return FormatOrdered4( fmt, beg3, beg1, beg4, beg2, arg3, arg1, arg4, arg2 );
        }
        else if(beg3 < beg4)
        {
          return FormatOrdered4( fmt, beg3, beg4, beg1, beg2, arg3, arg4, arg1, arg2 );
        }
        else
        {
          return FormatOrdered4( fmt, beg4, beg3, beg1, beg2, arg4, arg3, arg1, arg2 );
        }
      }
    }
  }
  else // beg2 < beg1
  {
    if (beg1 < beg3) // beg2 < beg1 < beg3
    {
      if (beg3 < beg4)
      {
        return FormatOrdered4( fmt, beg2, beg1, beg3, beg4, arg2, arg1, arg3, arg4 );
      }
      else if(beg1 < beg4)
      {
        return FormatOrdered4( fmt, beg2, beg1, beg4, beg3, arg2, arg1, arg4, arg3 );
      }
      else if(beg2 < beg4)
      {
        return FormatOrdered4( fmt, beg2, beg4, beg1, beg3, arg2, arg4, arg1, arg3 );
      }
      else
      {
        return FormatOrdered4( fmt, beg4, beg2, beg1, beg3, arg4, arg2, arg1, arg3 );
      }
    }
    else // beg2 < beg1 && beg3 < beg1
    {
      if (beg2 < beg3)
      {
        // 2<3<1
        if (beg1 < beg4)
        {
          return FormatOrdered4( fmt, beg2, beg3, beg1, beg4, arg2, arg3, arg1, arg4 );
        }
        else if(beg3 < beg4)
        {
          return FormatOrdered4( fmt, beg2, beg3, beg4, beg1, arg2, arg3, arg4, arg1 );
        }
        else if(beg2 < beg4)
        {
          return FormatOrdered4( fmt, beg2, beg4, beg3, beg1, arg2, arg4, arg3, arg1 );
        }
        else
        {
          return FormatOrdered4( fmt, beg4, beg2, beg3, beg1, arg4, arg2, arg3, arg1 );
        }
      }
      else
      {
        // 3<2<1
        if (beg1 < beg4)
        {
          return FormatOrdered4( fmt, beg3, beg2, beg1, beg4, arg3, arg2, arg1, arg4 );
        }
        else if(beg2 < beg4)
        {
          return FormatOrdered4( fmt, beg3, beg2, beg4, beg1, arg3, arg2, arg4, arg1 );
        }
        else if(beg3 < beg4)
        {
          return FormatOrdered4( fmt, beg3, beg4, beg2, beg1, arg3, arg4, arg2, arg1 );
        }
        else
        {
          return FormatOrdered4( fmt, beg4, beg3, beg2, beg1, arg4, arg3, arg2, arg1 );
        }
      }
    }
  }
}

}///< mfg_internal


#endif

