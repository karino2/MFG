/* -*- coding: utf-8 -*- マルチバイト */

// Copyright 2026 PGN Inc. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#include "pal.hpp"
#include "png.h"
#include <stdio.h>

namespace mfg_pal {

/*
  DecodePng32関連
*/
struct PngBufferReader
{
  const std::vector<uint8_t>& _buf;
  size_t _pos;
  PngBufferReader( const std::vector<uint8_t>& buf ) : _buf( buf ), _pos( 0 ) {}
};

static void PngReadFunc( png_struct *Png, png_bytep buf, png_size_t size )
{
  PngBufferReader* bufp = (PngBufferReader*)png_get_io_ptr(Png);

  memcpy( buf, bufp->_buf.data() + bufp->_pos, size );

  bufp->_pos += size;
}


bool DecodePng32( const std::vector<uint8_t>& buf, mfg_pal::Image32& dest )
{
  int pngcheck = png_sig_cmp( buf.data(), 0, buf.size() );
  if (pngcheck != 0) return false;

  // setup buffer
  png_structp pngstruct = NULL;
  png_infop pnginfo = NULL;
  auto autoDelete = mfg_pal::ScopeGuard( [&]{
    if (pngstruct != NULL)
    {
      png_read_end( pngstruct, pnginfo );
      png_destroy_read_struct( &pngstruct, &pnginfo, (png_infopp)NULL);
    }
  });
  pngstruct = png_create_read_struct( PNG_LIBPNG_VER_STRING, NULL,NULL,NULL );

  PngBufferReader reader( buf );

  png_set_read_fn( pngstruct, (png_voidp)&reader, (png_rw_ptr)PngReadFunc);
  pnginfo = png_create_info_struct( pngstruct );
  png_read_info( pngstruct, pnginfo );


  // start decode.
  int bpp, colortype, interlacetype, depth;
  unsigned int uwidth, uheight;

  png_get_IHDR(
    pngstruct, pnginfo, &uwidth, &uheight, &depth,
    &colortype, &interlacetype, NULL, NULL );
  
  png_set_strip_16( pngstruct );
  png_set_packing( pngstruct );

  const bool typePal = (colortype == PNG_COLOR_TYPE_PALETTE);

  const bool typeGray = (colortype == PNG_COLOR_TYPE_GRAY);
  const bool typeGrayA = (colortype == PNG_COLOR_TYPE_GRAY_ALPHA);

  const bool typeRGB = (colortype == PNG_COLOR_TYPE_RGB);
  //const bool typeRGBA = (colortype == PNG_COLOR_TYPE_RGB_ALPHA);

  if (typeGray || typePal)
  {
    // 1,2,4bitは、8bitアクセスできるようにする
    if (typeGray)
    {
      png_set_expand_gray_1_2_4_to_8( pngstruct );
    }
    bpp = 8;
  }
  else
  if (typeGrayA)
  {
    // 8+8 bit
    bpp = 16;
  }
  else
  {
    // 24 or 32bit
    if (typeRGB)
    {
      bpp = 24;
    }
    else
    {
      bpp = 32;
    }
  }

  if (pngstruct == NULL || pnginfo == NULL)
    return false;

  const int width = (int) uwidth;
  const int height = (int) uheight;

  if(!dest.Resize( width, height ))
  {
    return false;
  }

  if ((bpp == 16) || (bpp == 24) || (bpp == 32))
  {
    // 取得初期化 (フルカラー
    uint8_t **image = (png_bytepp)malloc( height * sizeof(png_bytep));
    for (int i=0; i<height; i++)
    {
      image[i] = (png_bytep)malloc( width * sizeof(uint32_t) );
    }
    png_read_image( pngstruct, image );

    // 画像展開
    for (int j=0; j<height; j++)
    {
      for (int i=0; i<width; i++)
      {
        TBpp32 color;

        if (bpp == 16)
        {
          uint8_t v = *((uint8_t*)( *(image + j) + i*2 + 0));
          uint8_t a = *((uint8_t*)( *(image + j) + i*2 + 1));

          color.A = a;
          color.R = v;
          color.G = v;
          color.B = v;
        }

        if (bpp == 24)
        {
          color.A = 255;
          color.R = *((uint8_t*)( *(image + j) + i*3 + 0));
          color.G = *((uint8_t*)( *(image + j) + i*3 + 1));
          color.B = *((uint8_t*)( *(image + j) + i*3 + 2));
        }

        if (bpp == 32)
        {
          color.R = *((uint8_t*)( *(image + j) + i*4 + 0));
          color.G = *((uint8_t*)( *(image + j) + i*4 + 1));
          color.B = *((uint8_t*)( *(image + j) + i*4 + 2));
          color.A = *((uint8_t*)( *(image + j) + i*4 + 3));
        }

        dest.PixelSet( i,j, color );
      }
    }

    for (int i=0; i<height; i++) free( image[i] );
    free( image );
  }
  else
  {
    // 取得初期化 (パレット付き)
    uint8_t **image = (png_bytepp)malloc( height * sizeof(png_bytep));
    for (int i=0; i<height; i++)
    {
      image[i] = (png_bytep)malloc( width * sizeof(uint8_t) );
    }
    png_read_image( pngstruct, image );

    // パレット用意
    png_colorp palette;
    int palnum;
    png_get_PLTE( pngstruct, pnginfo, &palette, &palnum );

    TBpp32 pal[256];
    if (colortype == PNG_COLOR_TYPE_GRAY)
    {
      // グレースケールのパレット
      for (int i=0; i<256; i++ )
      {
        pal[i].A = 0xFF;
        pal[i].R = i;
        pal[i].G = i;
        pal[i].B = i;
      }
    }
    else
    {
      for (int i=0; i<256; i++ )
      {
        pal[i].Value = 0xFF000000 | ((palette[i].red <<16) + (palette[i].green << 8) + palette[i].blue);
      }
    }

    for (int j=0; j<height; j++)
    {
      for (int i=0; i<width; i++)
      {
        uint8_t p = *( *(image + j) + i);
        dest.PixelSet( i, j, pal[p] );
      }
    }

    for (int i=0; i<height; i++) free( image[i] );
    free( image );
  }

  return true;
}

/*
  SavePng32
*/
bool SavePng32( const mfg_pal::Image32& img, nstring filename )
{
  FILE* f = fopen( filename.c_str(), "wb" );
  if (f == NULL) return false;

  auto autoClose = mfg_pal::ScopeGuard( [&]{
    fclose( f );
  });

  png_structp pngstruct = NULL;
  png_infop pnginfo = NULL;
  auto autoDelete = mfg_pal::ScopeGuard( [&]{
    if (pngstruct != NULL)
    {
      png_write_end( pngstruct, pnginfo );
      png_destroy_write_struct( &pngstruct, &pnginfo );
    }
  });
  pngstruct = png_create_write_struct( PNG_LIBPNG_VER_STRING, NULL,NULL,NULL );

  if (pngstruct == NULL)
    return false;

  pnginfo = png_create_info_struct( pngstruct );
  if (pnginfo == NULL)
    return false;

  png_init_io( pngstruct, f );

  const int width = img.Width();
  const int height = img.Height();

  // 書き込み初期化
  png_set_IHDR(
    pngstruct, pnginfo, width,height, 8,
    PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE );

  png_color_8 sig_bit;
  memset( &sig_bit, 0, sizeof(sig_bit) );
  sig_bit.red = 8;
  sig_bit.green = 8;
  sig_bit.blue = 8;
  sig_bit.alpha = 8;
  png_set_sBIT( pngstruct, pnginfo, &sig_bit );

  png_write_info( pngstruct, pnginfo );

  png_set_packing( pngstruct );

  uint8_t **image = (png_bytepp)malloc( height * sizeof(png_bytep));
  for (int i=0; i<height; i++)
  {
    image[i] = (png_bytep)malloc( width * sizeof(uint8_t) * 4 );
  }

  for (int j=0; j<height; j++)
  {
    for (int i=0; i<width; i++)
    {
      TBpp32 color = img.PixelGet( i,j );

      *( *(image + j) + i*4 + 0) = color.R;
      *( *(image + j) + i*4 + 1) = color.G;
      *( *(image + j) + i*4 + 2) = color.B;
      *( *(image + j) + i*4 + 3) = color.A;
    }
  }
  png_write_image( pngstruct, image );

  for (int i=0; i<height; i++) free( image[i] );
  free( image );

  return true;
}

}///< mfg_pal

