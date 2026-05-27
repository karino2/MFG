/* -*- coding: utf-8 -*- マルチバイト */

// Copyright 2026 Kazuma Arino. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#include "mfg_samples.h"

//
// mfgのサンプルスクリプト
// いろいろな所ですぐに動かせるスクリプトがあると便利なのでここに置く。
//
namespace mfg_samples {

const char* g_mfg_script_negaposi_invert = R"(
@title $TITLE_NEGAPOSI

def result_u8 |x, y| {
  let [b, g, r, a] = input_u8(x, y)
  u8[255-b, 255-g, 255-r, a]
}
)";

const char* g_mfg_script_mosaic = R"(
@title $TITLE_MOSAIC
@param_i32 MOSAIC_WIDTH(SLIDER, label=$LABEL_SIZE, min=2, max=256, init=16)

let clamped = sampler<input_u8>(address=.ClampToEdge)

@bounds( (input_u8.extent(0)-1)/MOSAIC_WIDTH+1, (input_u8.extent(1)-1)/MOSAIC_WIDTH+1)
def avg |x, y|{
  rsum(0..<MOSAIC_WIDTH, 0..<MOSAIC_WIDTH) |rx, ry|{
    let [b, g, r, a] = i32(clamped( MOSAIC_WIDTH*x+rx, MOSAIC_WIDTH*y+ry ))
    [*[b, g, r]*a, a]
  }
}

def result_u8 |x2, y2| {
  let [b2, g2, r2, a2] = avg( x2/MOSAIC_WIDTH, y2/MOSAIC_WIDTH )

  ifel(a2==0,
      u8[0, 0, 0, 0],
      u8[*[b2, g2, r2]/a2, a2/(MOSAIC_WIDTH*MOSAIC_WIDTH)] )
}
)";

const char* g_mfg_script_motion_blur = R"(
@title $TITLE_MOTION_BLUR

@param_f32 strength(SLIDER, label=$LABEL_STRENGTH, min=3.0, max=300.0, init=10.0)
@param_f32 angle(DIRECTION, init=0.0)

let sample = 2

let DeltaU = i32( cos(angle) * 65536 / sample )
let DeltaV = i32( sin(angle) * 65536 / sample )

let mw = i32( strength * sample )

let ar = 0.33 * mw

@bounds(mw+1)
def weights |m|{ 100 * exp( -m*m / (2*ar*ar) ) }

@bounds(input_u8.extent(0), input_u8.extent(1))
def mid_all |x, y| {
  rsum(-mw..<mw+1) |rx| {

    let px = x + ((DeltaU * rx) >> 16)
    let py = y + ((DeltaV * rx) >> 16)
  
    let inside = input_u8.is_inside(px, py)
    let gauss = ifel(inside, weights( abs(rx) ), 0.0)
  
    let [b, g, r, a] = f32(input_u8( px, py ))
    let ga = gauss * a
  
    [
      *[b, g, r]*ga,
      ga,
      gauss
     ]
  }
}

def result_u8 |x, y| {
  let [mid_b, mid_g, mid_r, mid_a, count] = mid_all(x, y)

  ifel( mid_a == 0,
    u8[0, 0, 0, 0],
    u8[*[mid_b, mid_g, mid_r]/mid_a,  (mid_a / count)]
  )
}
)";

const char* g_mfg_script_lens_blur = R"(

@title $TITLE_LENS_BLUR

@param_i32 radius(SLIDER, label=$LABEL_RADIUS, min=1, max=30, init=5)

@bounds(input_u8.extent(0), input_u8.extent(1))
def expbuf |x, y| {
  let [b, g, r, a] = f32(input_u8(x, y))

  let rcomp = r*r*a
  let gcomp = g*g*a
  let bcomp = b*b*a

  [bcomp, gcomp, rcomp, a]
}

@bounds(input_u8.extent(0), input_u8.extent(1))
def sumList |x, y| {
  rsum(0..<2*radius+1, 0..<2*radius+1) |r_edgex, r_edgey|
  {
    let sx = x+r_edgex-radius
    let sy = y+r_edgey-radius
    let inside = expbuf.is_inside(sx, sy)

    ifel(inside && (r_edgex-radius)^2 + (r_edgey-radius)^2 <= radius^2,
      [1.0 , *expbuf(sx, sy)] ,
      [0.0, 0.0, 0.0, 0.0, 0.0]
    )
  }
}

def result_u8 |x, y|{
  let [area, b, g, r, a] = sumList(x, y)
  let [b3, g3, r3, a3] = [b, g, r, a]/area

  u8[*sqrt([b3, g3, r3]/a3), a3]
}
)";

const char* g_mfg_script_sand_storm = R"(
@title $TITLE_SAND_STORM

def result_u8 |x, y| {
  ifel( rand() > 0.5, u8[0xff, 0, 0, 0], u8[0xff, 0xff, 0xff, 0xff] )
}
)";

const char* g_mfg_script_salt_and_pepper = R"(
@title $TITLE_SALT_AND_PEPPER_NOISE

@param_f32 SALT_THRESHOLD(SLIDER, label=$LABEL_SALT_THRESHOLD, min=0.0, max=1.0, init=0.1)
@param_f32 PEPPER_THRESHOLD(SLIDER, label=$LABEL_PEPPER_THRESHOLD, min=0.0, max=1.0, init=0.1)

def result_u8 |x, y| {
  let r = rand()
  ifel( r < SALT_THRESHOLD,
    u8[0xff, 0xff, 0xff, 0xff], ...)
  elif( (1.0 - r) < PEPPER_THRESHOLD,
    u8[0, 0, 0, 0xff],
    input_u8(x, y))
}
)";

const char* g_mfg_script_median_filter = R"(
@title $TITLE_WEIGHTED_MEDIAN_FILTER

# 1, 2, 1
# 2, 3, 2
# 1, 2, 1
@bounds(3, 3)
def weight |x, y| {
  3 - abs(x-1) - abs(y-1)
}

# To calculate median, the ranges are:
# x: 1 to w-1
# y: 1 to h-1
@bounds(input_u8.extent(0)-2, input_u8.extent(1)-2)
def median |x, y| {

  @bounds(9)
  def wcumsum |i| {
     let [ix, iy] = [i%3, i/3]
     weight(ix, iy)
  }

  mut! trans<wcumsum>.cumsum!(dim=0)

  # 15 is sum of weight.
  @bounds(15)
  def wmat |i| {
     let i3 = reduce<wcumsum>.find_first_index(dim=0) |_, val| { i < val }
     let [ix, iy] = [i3%weight.extent(0), i3/weight.extent(0)]
     i32(input_u8(ix+x, iy+y))
  }
  
  mut! trans<wmat>.sort!(dim=0)

  u8(wmat(wmat.extent(0)/2))
}

def result_u8 |x, y| {
  ifel( x == 0 || y == 0 || x == input_u8.extent(0)-1 || y == input_u8.extent(1) -1,
      input_u8(x, y),
      median(x-1, y-1))
}  
)";


const char* g_mfg_script_mirror = R"(
@title $TITLE_WATER_MIRROR

@param_f32v2 CENTER(POINTER, label=$LABEL_MIRROR_POS)

let [_, cy] = CENTER
let finput = sampler<input_u8>(coord=.NormalizedLinear)
# let finput = sampler<input_u8>(coord=.NormalizedNearest)


def result_u8 |x, y| {
  let [fx, fy] = to_ncoord([x, y])

  let diff = fy - cy
  let ratio =  diff/(1-cy)
  
  let srcY = cy -   cy*ratio

  let [b, g, r, _] = finput(fx, srcY)
  
  ifel( fy < cy, input_u8(x, y),  u8[b, g, r, 0x88])
}
)";


const char* g_mfg_script_cross_filter = R"(
@title "クロスフィルター"

@param_f32 strength(SLIDER, label="筋の大きさ", min=3.0, max=300.0, init=10.0)
@param_f32 angle(SLIDER, label="角度", min=0.0, max=3.2, init=0.0)
@param_i32 threshold(SLIDER, label="閾値", min=1, max=255, init=230)
@param_f32 lightStrength(SLIDER, label="光の強度", min=1.0, max=100.0, init=10.0)

let sample = 2

let mw = i32( strength * sample )

let ar = 0.33 * mw

@bounds(mw+1)
def weights |m|{ 100 * exp( -m*m / (2*ar*ar) ) }

let extended = sampler<input_u8>(address=.ClampToBorderValue, border_value=u8[0, 0, 0, 0] )

@bounds(input_u8.extent(0), input_u8.extent(1))
def mid_all |x, y| {
  let sum1 = rsum(-mw..<mw+1) |rx| {

    let angleOffset = 0.0

    let DeltaU = i32( cos(angle + angleOffset) * 65536 / sample )
    let DeltaV = i32( sin(angle + angleOffset) * 65536 / sample )

    let px = x + ((DeltaU * rx) >> 16)
    let py = y + ((DeltaV * rx) >> 16)

    let gauss = weights( abs(rx) )

    let [b, g, r, a] = f32(extended( px, py ))
    let ga = gauss * a

    let lum = dot([r / 255.0, g / 255.0, b / 255.0], [0.2126, 0.7152, 0.0722])

    ifel (lum * 255.0 > f32(threshold),
    [
      *[b, g, r]*ga,
      ga,
      gauss
     ],
    [0.0, 0.0, 0.0, 0.0, gauss]
    )
  }
  let sum2 = rsum(-mw..<mw+1) |rx| {

    let angleOffset = 3.141592 * 2.0 / 3.0

    let DeltaU = i32( cos(angle + angleOffset) * 65536 / sample )
    let DeltaV = i32( sin(angle + angleOffset) * 65536 / sample )

    let px = x + ((DeltaU * rx) >> 16)
    let py = y + ((DeltaV * rx) >> 16)

    let gauss = weights( abs(rx) )

    let [b, g, r, a] = f32(extended( px, py ))
    let ga = gauss * a
    let lum = dot([r / 255.0, g / 255.0, b / 255.0], [0.2126, 0.7152, 0.0722])

    ifel (lum * 255.0 > f32(threshold),
    [
      *[b, g, r]*ga,
      ga,
      gauss
     ],
    [0.0, 0.0, 0.0, 0.0, gauss]
    )
  }
  let sum3 = rsum(-mw..<mw+1) |rx| {

    let angleOffset = 3.141592 * 2.0 / 3.0 * 2.0

    let DeltaU = i32( cos(angle + angleOffset) * 65536 / sample )
    let DeltaV = i32( sin(angle + angleOffset) * 65536 / sample )

    let px = x + ((DeltaU * rx) >> 16)
    let py = y + ((DeltaV * rx) >> 16)

    let gauss = weights( abs(rx) )

    let [b, g, r, a] = f32(extended( px, py ))
    let ga = gauss * a
    let lum = dot([r / 255.0, g / 255.0, b / 255.0], [0.2126, 0.7152, 0.0722])

    ifel (lum * 255.0 > f32(threshold),
    [
      *[b, g, r]*ga,
      ga,
      gauss
     ],
    [0.0, 0.0, 0.0, 0.0, gauss]
    )
  }
  sum1 + sum2 + sum3
}

def result_u8 |x, y| {
  let [mid_b, mid_g, mid_r, mid_a, count] = mid_all(x, y)

  ifel( mid_a == 0,
    u8[0, 0, 0, 0],
    u8[*[mid_b, mid_g, mid_r]/mid_a,  clamp(i32((mid_a / count)) * i32(lightStrength), 0, 255)]
  )
}

)";


}///< mfg_samples
