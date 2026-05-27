/* -*- coding: utf-8 -*- マルチバイト */

// Copyright 2026 Kazuma Arino. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#ifndef MFG_RESOURCE_EXPANDER_HPP_
#define MFG_RESOURCE_EXPANDER_HPP_

/*
  サンプルコードなどのように、MFGのスクリプトで、展開された結果が欲しい場合に使うutility。
  STLのみに依存していて、コアからは特に使われていない。
  使う人だけincludeして使う。
  ユーザー入力では無く内部の決まった種類の文字列に対してのみ使われる前提で、置き換えはあまりコンテキストを見ずに単純に部分一致だけで行っていく。
  置き換えがうまく行かない時は対象を手直しする運用。

  リソースは２つの種類があり、コメントとそれ以外に分かれる。
  コメントは以下のような形。

  #$MOSAIC_HEADER_COMMENT

  「#$」で始まり、そのあとリソースIDが続く。
  この結果はMOSAIC_HEADER_COMMENTが複数行あると、それぞれに#と空白をつけて出力する。
  例えば MOSAIC_HEADER_COMMENTが"これは一行目\nこれは二行目" とあれば、以下のように展開される。

  # これは一行目
  # これは二行目

  コメント以外では以下のような形。

  $LABEL_SIZE

  例えば以下。

  @param_i32 MOSAIC_WIDTH(SLIDER, label=$LABEL_SIZE, min=2, max=256, init=16)

  この$LABEL_SIZEが展開された文字列に置き換わる。
  
  $自体を使いたい時は$$とする。
*/


#include <cassert>
#include <cstring>
#include <functional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace mfg_resource_expander
{

/*
  std::stringをスキャンしていく時のutility
*/
struct SrcScanner
{
  std::string _src;
  size_t _pos;
  SrcScanner( std::string src ) : _src( std::move(src) ), _pos(0) {}

  void ResetPos() { _pos = 0; }

  // 現在の位置からoffsetだけ先の文字を読む。EOFなら-1
  int Peek( int offset = 0 )
  {
    if (_pos+offset >= _src.size())
      return -1;
    return _src[_pos+offset];
  }

  bool LookAt( const char* s )
  {
    size_t len = std::strlen( s );
    if (_pos+len > _src.size())
      return false;
    
    return 0 == std::strncmp( &_src[_pos], s, len );
  }

  void SkipCurLine()
  {
    int ch = Peek();
    while(ch != '\n' && ch != -1) {
      _pos++;
      ch = Peek();
    }
    if (ch == '\n')
      _pos++;
  }

  void Advance( size_t delta = 1 )
  {
    if (_pos+delta > _src.size())
    {
      _pos = _src.size();
      return;
    }
    _pos += delta;
  }

  std::string Substr( size_t len )
  {
    assert( _pos+len <= _src.size() );
    return _src.substr( _pos, len );
  }
  
  // デバッグ用に付近の文字を取り出す。最後の方で呼ぶ場合もありうるのでminで超えないように。
  std::string DebSubstr( size_t len )
  {
    return Substr( std::min( len, _src.size()-_pos) );
  }

  size_t Size() const { return _src.size(); }

  // ダブルクオートでくくられた文字列リテラルをscanする。_posは進める。結果にはダブルクオートを含まない。
  std::string ScanStringLiteral()
  {
    std::stringstream res;
    assert( LookAt( "\"" ) );
    Advance();
    int ch = Peek( 0 );
    while( ch != '\"' && ch != -1 )
    {
      res << (char)ch;
      if (ch == '\\')
      {
        Advance();
        ch = Peek( 0 );
        if (ch == -1)
          throw std::runtime_error("Invalid EOF just after back slash.");

        res << (char)ch;
        Advance();
        ch = Peek( 0 );
      }
      else
      {
        Advance();
        ch = Peek( 0 );
      }
    }
    if (ch == -1)
      throw std::runtime_error("No close double quote found");
    Advance();
    return res.str();
  }

  void SkipWord( const std::string& word )
  {
    assert( LookAt( word.c_str() ) );
    Advance( word.size() );
  }

  void SkipSpace()
  {
    int ch = Peek();
    while( ch == ' ' )
    {
      Advance();
      ch = Peek();
    }
  }

  bool IsIdChar( int ch )
  {
    if (ch >= 'a' && ch <= 'z')
      return true;
    if (ch >= 'A' && ch <= 'Z')
      return true;
    if (ch >= '0' && ch <= '9')
      return true;
    if (ch == '_')
      return true;
    return false;
  }

  /*
    現在の位置にあるidentifierを返す。
  */
  std::string ScanId()
  {
    if (!IsIdChar( Peek(0) ))
      throw std::runtime_error("Invalid identifier: " + DebSubstr( 3 ));

    size_t off = 0;
    while( IsIdChar( Peek(off)) )
    {
      off++;
    }
    assert( off >= 1 );
    auto ret = Substr( off );
    Advance( off );
    return ret;
  }
};

/*
  RESIDかSTRINGの文字列。
  型としては文字列だが解決が必要な場合がある。
*/
struct MFGString
{
  enum MFGStringType
  {
    NOT_FOUND,
    STRING,
    RESID
  };

  MFGStringType _type;
  std::string _stringVal;

  MFGString() : MFGString(MFGString::NOT_FOUND, "") {}
  MFGString( MFGStringType tt, const std::string& stringVal ) : _type( tt ), _stringVal( stringVal ) {}
};

/*
  UIでタイトルだけほしいことがある時に使う。
  わざわざパースしたくも無いので専用のものをここに作る。
*/
struct AttrScanner
{
  SrcScanner _scanner;
  AttrScanner( std::string src ) : _scanner( std::move(src) ) {}

  void ResetPos() { _scanner.ResetPos(); }
  int Peek() { return _scanner.Peek(); }
  bool LookAt( const char* s ) { return _scanner.LookAt( s ); }
  void Advance( size_t delta = 1 ) { _scanner.Advance( delta ); }
  void SkipCurLine() { _scanner.SkipCurLine(); }

  /*
    リソースでないと分かっているケース。
    Expand済みのソースをスキャンする時など。
  */
  std::string ScanTitleString()
  {
    auto title = ScanTitle();
    if (title._type == MFGString::STRING)
      return std::move(title._stringVal);
    return "";
  }

  /*
   @title "abc"
   や
   @title $RES_ID
   をパースする。
   attrNameはこの場合は"title"で、呼び出し時のポジションは@の次の文字。
  */
  MFGString ParseAttrString( const char* attrName )
  {
    assert( LookAt( attrName ) );
    _scanner.SkipWord( attrName );
    _scanner.SkipSpace();
    if (_scanner.LookAt( "\"" ))
    {
      return MFGString( MFGString::STRING, _scanner.ScanStringLiteral() );
    }
    else if(_scanner.LookAt( "$" ))
    {
      _scanner.Advance();
      return MFGString( MFGString::RESID, _scanner.ScanId() );
    }
    else
    {
      throw std::runtime_error("Invalid title valud: " + _scanner.DebSubstr( 3 ));
    }
  }

  /*
    attrNamesの文字列attrを探して、その結果のmapを返す。

   @version "1.2.0"
   や、
   @title $RES_ID
   をパースする。
   attrNamesはこの場合"version"と"title"
   で、returnのmapはキーがattrNames。
  */
  std::map<std::string, MFGString> ScanStringAttrList( const std::vector<std::string>& attrNames )
  {
    std::map<std::string, MFGString> ret;
    ResetPos();
    int ch = Peek();
    while( ch != -1 )
    {
      if (ch == '#')
      {
        SkipCurLine();
        ch = Peek();
        continue;
      }
      else if(ch == '@')
      {
        Advance();
        for( const auto& attrName : attrNames )
        {
          if (LookAt( attrName.c_str() ))
          {
            ret[attrName] = ParseAttrString( attrName.c_str() );
            if (attrNames.size() == ret.size())
              return ret;
          }
        }
        SkipCurLine();
        ch = Peek();
        continue;
      }
      else
      {
        Advance();
        ch = Peek();
        continue;
      }
    }
    return ret;
  }

  MFGString ScanStringAttr( const char* attrName )
  {
    std::string attrNameStr( attrName );
    std::vector<std::string> vec { attrNameStr };
    auto retMap = ScanStringAttrList( vec );
    auto iter = retMap.find( attrNameStr );
    if (iter == retMap.end())
      return MFGString();
    return iter->second;
  }

  MFGString ScanTitle()
  {
    return ScanStringAttr( "title" );
  }

  MFGString ScanVersion()
  {
    return ScanStringAttr( "version" );
  }

};



/*
  インデントをトラックするためのクラス。
  ただし行頭の空白だけをトラックし、そのほかの記号にぶつかったら以後は改行まで不明として扱う。
*/
struct ColumnTracker
{
  bool _knownCol = true;
  size_t _col = 0;

  void Reset()
  {
    _knownCol = true;
    _col = 0;
  }

  void Invalidate()
  {
    _knownCol = false;
  }

  size_t Indent()
  {
    if (_knownCol)
      return _col;
    else
      return 0;
  }
};

struct ResourceExpander
{
  SrcScanner _scanner;
  std::function<std::string(const char*)> _resolver;
  std::stringstream _result;

  // resolverはリソースIDを引数に対応する文字列を返す。
  ResourceExpander( std::string src,  std::function<std::string(const char*)> resolver ) : _scanner( std::move(src) ), _resolver( std::move(resolver) )
  {
  }

  int Peek( int offset = 0 ) { return _scanner.Peek( offset ); }
  bool LookAt( const char* s ) { return _scanner.LookAt( s ); }
  void Advance( size_t delta = 1 ) { _scanner.Advance( delta ); }
  void SkipCurLine() { _scanner.SkipCurLine(); }
  void SkipWord( const std::string& word ) { _scanner.SkipWord( word ); }
  size_t Pos() const { return _scanner._pos; }


  // '#'から始まって行末までをコピー。
  void ConsumeComment()
  {
    int ch = Peek();
    assert( ch == '#' );
    _result << (char)ch;
    while(ch != '\n') {
      Advance();
      ch = Peek();
      if (ch == -1)
        return;
      _result << (char)ch;
    }
    Advance();
  }


  bool IsIdChar( int ch )
  {
    return _scanner.IsIdChar( ch );
  }

  std::string ScanId()
  {
    return _scanner.ScanId();
  }

  void ConsumeResource()
  {
    assert( LookAt( "$" ) );
    _scanner.Advance();
    // このケースはサンプルソースの方をリリース前に直すべきケース。
    if (!IsIdChar( Peek(0) ))
    {
      auto deb = _scanner.DebSubstr( 3 );
      throw std::runtime_error( "Parse error, fix target: " + deb );
    }
    
    auto resId = ScanId();
    _result << '"';
    _result << _resolver( resId.c_str() );
    _result << '"';
  }

  void ConsumeStringLiteral()
  {
    _result << '"';
    _result << _scanner.ScanStringLiteral();
    _result << '"';
  }

  void EmitIndent( size_t indent )
  {
    for( size_t i = 0; i < indent; i++ )
    {
      _result << ' ';
    }
  }

  // #$で始まっている。展開して各行の先頭には"# "をつける
  void ConsumeResourceComment( size_t indent )
  {
    assert( LookAt( "#$") );
    Advance( 2 );

    if (!IsIdChar( Peek(0) ))
    {
      auto deb = _scanner.DebSubstr( 3 );
      throw std::runtime_error( "Parse error2, fix target: " + deb );
    }
    
    auto resId = ScanId();    
    auto expanded =  _resolver( resId.c_str() );
    SkipCurLine();

    _result << "# ";
    for (size_t i = 0; i < expanded.size(); i++)
    {
      _result << expanded[i];

      // 改行があってしかもそこが終わりでなければ、行頭のインデントと"# "を出力
      if (expanded[i] == '\n' && i != expanded.size() - 1)
      {
        EmitIndent( indent );
        _result << "# ";
      }
    }
    _result << '\n';
  }

  std::string ConsumeAll()
  {
    if (_scanner.Size() == 0)
      return _result.str();

    int ch = Peek( 0 );
    ColumnTracker tracker;

    while( ch != -1 )
    {
      if (ch == '#')
      {
        if ('$' == Peek( 1 ))
        {
          // #$で始まっている。リソースコメント。
          ConsumeResourceComment( tracker.Indent() );
          ch = Peek( 0 );
          tracker.Reset();
          continue;
        }
        else
        {
          // #$でない#、コメント。
          ConsumeComment();
          ch = Peek( 0 );
          tracker.Reset();
          continue;
        }
      }
      else if(ch == '$')
      {
        tracker.Invalidate();
        if ('$' == Peek(1) )
        {
          // $$のケース、$として展開。
          _result << "$";
          Advance( 2 );
          ch = Peek( 0 );
          continue;
        }
        else
        {
          ConsumeResource();
          ch = Peek( 0 );
          continue;
        }
      }
      else if(ch == '"')
      {
        tracker.Invalidate();
        ConsumeStringLiteral();
        ch = Peek( 0 );
        continue;
      }
      else
      {
        _result << (char)ch;
        Advance();

        if (ch == '\n') {
          tracker.Reset();
        }
        else if (ch == ' ')
        {
          tracker._col++;
        }
        else
        {
          tracker.Invalidate();
        } 
        
        ch = Peek( 0 );
        continue;
      }
    }
    return _result.str();
    
  }

};

} ///< mfg_resource_expander
#endif

