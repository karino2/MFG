/* -*- coding: utf-8 -*- マルチバイト */

// Copyright 2026 PGN Inc. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#ifndef MFG_OPEN_PAL_HPP_
#define MFG_OPEN_PAL_HPP_

#include "cppunzip.hpp"
#include <stdint.h>
#include <stdlib.h>

#include <cmath>
#include <functional>
#include <string>

#ifdef __APPLE__
  #define MFG_BACKEND_METAL
#elif  _WIN32
  #define MFG_BACKEND_D3D

    // enumでcaseを全て列挙していてもreturnで値を返さないパスがあるといってくるワーニング。
    // clangはここにdefaultを足すと逆にワーニングを言ってきて向こうの方が正しいのでこちらをsuppress.
    #pragma warning( disable: 4715 )
    #pragma warning( disable: 4267 ) // narrowing cast, size_tからintへのキャストは許す。

#endif

// コンパイラ警告 "unused" 対応用マクロ
#define N_UNUSED(x) (void)x;

namespace mfg_pal {

namespace uz = cppunzip;

/*
STL util.
この辺は別に環境によらないが社内ライブラリとnamespaceが違う、とか。
*/

//////////////////////////////////////////////////////////////////////////////
// ScopeGuard
//////////////////////////////////////////////////////////////////////////////
/*
excceptionやreturnの時に自動で実行される後始末を行うクラス。
ラムダ式を渡す。

void * ptr = malloc(somesize);
if (ptr == nullptr)
   return nullptr;

auto guard = ScopeGuard([&]{ free( ptr ); });

// なにか処理

// ptrは解放せずに呼び出し元に返す、という場合。
guard.dismiss();
return ptr;
*/
struct ScopeGuard
{
  std::function<void(void)> _call;
  bool _dismiss = false;
  explicit ScopeGuard( std::function<void(void)> call ) : _call( call ) {}
  ~ScopeGuard()
  {
    if ( !_dismiss )
      _call();
  }
  void dismiss() { _dismiss = true; }
};


///////////////////////////////////
// NRange
///////////////////////////////////

/*
NCountableRange: NRangeのヘルパークラス。NRangeからしか使わない。（ユーザーはautoで受け取るので）
*/
template<typename T>
class NCountableRange
{
  T _rangeBegin;
  T _rangeEnd;

  class range_iterator
  {
    T _current;

  public:
    T operator *() const { return _current; }
    const range_iterator &operator ++()
    {
      ++_current;
      return *this;
    }

    bool operator ==(const range_iterator &other) const { return _current == other._current; }
    bool operator !=(const range_iterator &other) const { return _current != other._current; }

    range_iterator(T start) : _current (start) { }
  };


public:
  NCountableRange( T beg, T end ) : _rangeBegin(beg),_rangeEnd(end) {}
  range_iterator begin() const { return range_iterator(_rangeBegin); }
  range_iterator end() const { return range_iterator(_rangeEnd); }

  std::vector<T> ToVector()
  {
    std::vector<T> res;
    for (auto iter = begin(); iter != end(); ++iter)
    {
      res.push_back( *iter );
    }
    return res;
  }
};

/*
beg から end-1 までの数字を順番に返すiteratorを返す。(endは含まない)
range for文で使う。

例:
for (auto i : NRange(0, 100))
{
  ...
}
*/
template<typename T>
NCountableRange<T> NRange( T beg, T end ) { return NCountableRange<T>( beg, end ); }

/*
begに0を即値で指定すると、endがunsignedだったりint64_tだった時に型違いが発生するので、そのケースの型解決用のヘルパー。
*/
template<typename T>
NCountableRange<typename std::enable_if<!std::is_same<T, int>::value, T>::type> NRange( int beg, T end ) { return NCountableRange<T>( (T) beg, end ); }

/*
NRange(0, end)と同じ振る舞いをするヘルパー。０からend-1までのiteratorを返す。
*/
template<typename T>
NCountableRange<T> NRange( T end ) { return NCountableRange<T>( 0, end ); }

/*
 * Iterator実装の為の便利クラス。boostやfollyのiterator_facadeと似てる。
 * Iteratorを実装したい人はこのクラスを継承して以下のメソッドを実装すると、iter++, ++iter、など必要なモノをすべて実装してくれる。
 *
 *   void Increment();
 *   void Decrement(); // オプショナル、--したい人だけ実装
 *   V& Dereference() const;
 *   bool Equal( const D& other) const;
 *
 * Templateパラメータ:
 * D: 継承先クラス (CRTP)
 * V: 値の型
*/
template<class D, typename V>
class IteratorFacade
{
public:
  const V& operator*() const
  {
    return AsDerivedConst().Dereference();
  }

  V& operator*()
  {
    return AsDerived().Dereference();
  }

  D& operator++()
  {
    AsDerived().Increment();
    return AsDerived();
  }

  D operator++(int)
  {
    auto res = AsDerived(); // 進める前をコピー
    AsDerived().Increment();
    return res;
  }

  D& operator--()
  {
    AsDerived().Decrement();
    return AsDerived();
  }

  D operator--(int)
  {
    auto res = AsDerived(); // 戻す前をコピー
    AsDerived().Decrement();
    return res;
  }

  bool operator==(const D &other) const
  {
    return AsDerivedConst().Equal( other );
  }
  bool operator!=(const D &other) const
  {
    return !AsDerivedConst().Equal( other );
  }

 private:
  D& AsDerived() {
    return static_cast<D&>( *this );
  }

  const D & AsDerivedConst() const {
    return static_cast<const D&>( *this );
  }
};

/*
 * srcベクトルの各要素にfunを実行し、その結果を保持する同じサイズのvectorを返す。
 * いわゆるmap_fn。いつもpush_backを使う程度にはパフォーマンスは意識していない。
 *
 * 例:
 * std::vector<int> src { 1, 2, 3, 4 };
 * auto res = MapFn( src, [](int e) { return e*2; } );
 * for(auto i : res)
 * {
 *   cout << i << endl;
 * }
 *
 * 結果:
 * 2
 * 4
 * 6
 * 8
 *
*/
template<typename T, typename F>
auto MapFn( const std::vector<T>& src, F fun ) -> std::vector<decltype( fun(src[0]) )>
{
  std::vector<decltype( fun(src[0]) )> ret;
  for( auto i : NRange(src.size()) )
  {
    ret.push_back( fun( src[i] ) );
  }
  return ret;
}

// rvalueバージョン
template<typename T, typename F>
auto MapFn( std::vector<T>&& src, F fun ) -> std::vector<decltype( fun(std::move(src[0])) )>
{
  std::vector<decltype( fun(std::move(src[0])) )> ret;
  for( auto i : NRange(src.size()) )
  {
    ret.push_back( fun( std::move(src[i]) ) );
  }
  return ret;
}

/*
  画像バッファ関連
*/
#pragma pack(push,1)

union TBpp32
{
  uint32_t Value;
  struct
  {
    uint8_t B,G,R,A;
  };
  typedef uint8_t ChannelType;
};

inline TBpp32 Bpp32( uint32_t value )
{
  TBpp32 res;
  res.Value = value;
  return res;
}

inline TBpp32 Bpp32( uint8_t a, uint8_t r, uint8_t g, uint8_t b )
{
  TBpp32 res;
  res.A = a;
  res.R = r;
  res.G = g;
  res.B = b;
  return res;
}

inline TBpp32 Bpp32( uint8_t r, uint8_t g, uint8_t b )
{
  TBpp32 res;
  res.A = 255;
  res.R = r;
  res.G = g;
  res.B = b;
  return res;
}

///////////////////////////////////////
// 64bpp型 (16:16:16:16)
///////////////////////////////////////
union TBpp64
{
  uint64_t Value;
  struct
  {
    uint16_t B,G,R,A;
  };
  struct
  {
    // 16:16 固定小数座標
    int32_t fx,fy;
  };
  typedef uint16_t ChannelType;
};

inline TBpp64 Bpp64( uint64_t value )
{
  TBpp64 res;
  res.Value = value;
  return res;
}

inline TBpp64 Bpp64( uint16_t a, uint16_t r, uint16_t g, uint16_t b )
{
  TBpp64 res;
  res.A = a;
  res.R = r;
  res.G = g;
  res.B = b;
  return res;
}

inline TBpp64 Bpp64( uint16_t r, uint16_t g, uint16_t b )
{
  TBpp64 res;
  res.A = 0xFFFF;
  res.R = r;
  res.G = g;
  res.B = b;
  return res;
}

inline TBpp64 Bpp64( const TBpp32& col )
{
  TBpp64 res;
  res.A = (col.A << 8) + col.A;
  res.R = (col.R << 8) + col.R;
  res.G = (col.G << 8) + col.G;
  res.B = (col.B << 8) + col.B;
  return res;
}

inline TBpp32 Bpp32( const TBpp64& col )
{
  TBpp32 res;
  res.A = col.A >> 8;
  res.R = col.R >> 8;
  res.G = col.G >> 8;
  res.B = col.B >> 8;
  return res;
}


#pragma pack(pop)

template<typename TBPP, int PIXEL_BYTE>
class Image
{
protected:
  int m_width, m_height; // 幅・高さ
  int m_widthByte; // 幅のバイト数
  TBPP* m_buffer; // 画像バッファへのptr
  TBPP m_1px; // 必ず確保される 1px

  void ForceWidthHeight(int& width, int& height)
  {
    if (width < 1) width = 1;
    if (height < 1) height = 1;
  }
  // 幅と高さを指定する
  void SetWidthHeight(int w, int h, double pixelByte)
  {
    // サイズ設定
    m_width = w;
    m_height = h;
    m_widthByte = (int)(std::ceil(pixelByte * m_width));
  }

public:
  TBPP* PixelAddress(int x, int y)
  {
    if ((size_t)x >= (size_t)m_width) return NULL;
    if ((size_t)y >= (size_t)m_height) return NULL;

    TBPP* ptr = m_buffer + (m_width * y + x);
    return ptr;
  }

  const TBPP* PixelAddress(int x, int y) const
  {
    if ((size_t)x >= (size_t)m_width) return NULL;
    if ((size_t)y >= (size_t)m_height) return NULL;

    const TBPP* ptr = m_buffer + (m_width * y + x);
    return ptr;
  }

  // 幅、高さ、幅のバイト数、バッファのサイズ（バイト数）、2の累乗サイズ？
  int Width() const { return m_width; }
  int Height() const { return m_height; }
  int WidthByte() const { return m_widthByte; }
  int Size() const { return m_widthByte * m_height; }

  Image()
  {
    m_buffer = &m_1px;
    Resize( 1, 1 );
  }

  ~Image() { Free(); }

  void Free()
  {
    // 新規確保されてない
    if (m_buffer == &m_1px) return;

    // 確保されてるバッファを解放
    if (m_buffer != NULL)
    {
      free( m_buffer );
      m_buffer = NULL;
    }
  }

  TBPP PixelGet(int x, int y) const
  {
    TBPP res;
    res.Value = 0;
    const TBPP* ptr = PixelAddress( x, y );
    if (ptr != NULL) res = *ptr;

    return res;
  }

  void PixelSet(int x, int y, TBPP color)
  {
    TBPP* ptr = PixelAddress( x, y );
    if (ptr == NULL) return;

    *ptr = color;
  }

  // 社内ライブラリとの互換性のためdouble
  double PixelByte() const { return (double)PIXEL_BYTE; };

  bool Resize( int width, int height )
  {
    // サイズ変更なし
    if ((width == Width()) && (height == Height()))
    {
      return true;
    }

    ForceWidthHeight( width, height );

    Free();

    if (m_buffer == &m_1px && width*height == 1)
    {
      auto deb = PixelByte();
      SetWidthHeight( width, height, PixelByte() );
      return true;
    }


    m_buffer = (TBPP*)malloc( width * height * sizeof(TBPP) );

    if (m_buffer != NULL)
    {
      // 確保できた
      SetWidthHeight( width, height, PixelByte() );
      return true;
    }

    // できなかったので、1x1px
    m_buffer = &m_1px;
    SetWidthHeight( 1, 1, PixelByte() );
    return false;
  }

  bool Copy( const Image& src )
  {
    if(!Resize( src.Width(), src.Height() ))
      return false;

    // Imageのバッファが連続領域なケースを想定した簡易実装
    memcpy( m_buffer, src.PixelAddress( 0, 0 ),  Width() * Height() * sizeof(TBPP) );
    return true;
  }

};

////////////////////////////////////////////////////////////////////////////////
// 32bpp画像バッファ
////////////////////////////////////////////////////////////////////////////////
class Image32 : public Image<TBpp32, 4>
{
public:
  typedef TBpp32 PixelType;
  typedef TBpp32 PixelType2x2;

  typedef uint8_t ChannelType;
  Image32() : Image() {}

  // Image32だけFillを実装する。テストで使っているのと社内うライブラリとの切り分けがちょっと難しいので。
  void Fill( TBpp32 color )
  {
    for (auto i : NRange(Width()*Height()))
    {
      m_buffer[i] = color;
    }
  }
};

////////////////////////////////////////////////////////////////////////////////
// 64bpp画像バッファ
////////////////////////////////////////////////////////////////////////////////
class Image64 : public Image<TBpp64, 8>
{

public:
  typedef TBpp64 PixelType;
  typedef TBpp64 PixelType2x2;

  typedef uint16_t ChannelType;
  Image64() : Image() {}
};

/*
 ハンドル関連。オープンソース版では単なるメモリとする。
*/

/* ロック、メモリの場合なので何もしない */
template<typename T>
struct ImageLockPixelGuard
{
  ImageLockPixelGuard( T* ) {};
  ImageLockPixelGuard( ImageLockPixelGuard<T> &&src ) noexcept = default;
};

struct MemHandle {};
inline MemHandle* HandleAlloc( size_t size ) { return (MemHandle*)malloc( size ); }
inline void HandleFree( MemHandle* handle, size_t size ) { free( (void*)handle ); }

template<typename T>
struct HandleLockGuard
{
  MemHandle* _target;
  HandleLockGuard( MemHandle* target ) : _target( target ){}
  T ReLock( MemHandle* newTarget )
  {
    _target = newTarget;
    return (T)_target;
  }
  T Ptr() { return (T)_target; }  
};

/*
* PixelLockerは使わないのでダミー実装。
*/
template<typename T>
int PixelLocker( T* ){ return 0; }

//////////////////////////////////////////////////////////////////////////////
// 言語情報
//////////////////////////////////////////////////////////////////////////////
enum NLanguage
{
  NL_ENGLISH,
  NL_JAPANESE,
  NL_CHINESE_SIMP,
  NL_CHINESE_TRAD,
  NL_KOREAN,
  NL_PORTUGUESE,
  NL_SPANISH,
  NL_GERMAN,
  NL_FRENCH,
  NL_RUSSIAN,
  NL_HINDI,
  NL_BENGALI,
  NL_POLISH,
};

uint32_t Hash32( const uint8_t* key, size_t len, uint32_t seed );
uint32_t Rand32();
size_t Tick();

}


#endif

