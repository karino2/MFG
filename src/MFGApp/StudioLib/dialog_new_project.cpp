/* -*- coding: utf-8 -*- マルチバイト */

// Copyright 2026 PGN Inc. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

#include "dialog_new_project.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QLabel>
#include <QSettings>
#include <QFile>
#include <QFileDialog>

#include <mfg_pal/pal.hpp>
#include <qt_pal/studio_pal.hpp>
#include "app_qt.h"
#include "trans.h"
#include "trans_str_mfg.h"
#include <qt_pal/qt_pal.h>

// ExpandMFGResource関連
#include "trans.h"
#include "mfg_resource.hpp"
#include "mfg_resource_expander.hpp"
using namespace mfg_resource;
using namespace mfg_resource_expander;

#include "mfg_samples.h"
using namespace mfg_samples;

using app_qt::AQ;

extern QWidget* MainAsQWidget();

static std::string ResolveSymbolMFG( const char *resIdStr )
{
  auto resId = LookupUIResId( resIdStr );
  return std::string( UIResourceString( resId ) );
}

 // リソースを展開した結果を返す。
std::string ExpandMFGResource( std::string src )
{
  mfg_resource::SetLanguage( Trans().Language() );
  ResourceExpander expander( src, ResolveSymbolMFG );
  return expander.ConsumeAll();
}

static const char* g_mfg_script_blank = R"**(
@title "(New Filter)"

def result_u8 |x, y| {
  #
  # please modify code here.
  #
  let [b, g, r, a] = input_u8(x, y)
  u8[255-b, 255-g, 255-r, a]

}
)**";
  

static const char* g_mfg_script_crystallize = R"(
@title $TITLE_CRYSTALLIZE

@param_i32 GRID_INTERVAL(SLIDER, label=$LABEL_GRID_SIZE, min=10, max=256, init=50)
@param_f32 STRENGTH(SLIDER, label=$LABEL_RANDOM_STRENGTH, min=0.1, max=1.0, init=1.0)

let [GRID_WIDTH, GRID_HEIGHT] = (input_u8.extent() -1)/GRID_INTERVAL + 1

@bounds(GRID_WIDTH+3, GRID_HEIGHT+3)
def gridOffset |x, y|{
  STRENGTH*[rand()-0.5, rand()-0.5]
}

@bounds(input_u8.extent(0), input_u8.extent(1))
def nn |x, y| {
  let go_xy = [x, y]/GRID_INTERVAL # x y of grid coordinate, upper left

  let nearest = reduce(init=[0, 0, 999999.0], 0..<4, 0..<4) |rx, ry, accm| {
    let gxy = go_xy + [-1, -1] + [rx, ry]
    let ggxy = gxy*GRID_INTERVAL + i32(gridOffset(*(gxy+1))*GRID_INTERVAL) # global coordinate
    let d = distance(f32([x, y]), f32(ggxy))
    ifel(d <accm.z, 
        [gxy.x, gxy.y, d],
        accm
    )
  }
  nearest
}

let inputEx = sampler<input_u8>(address=.ClampToEdge)
let nnEx = sampler<nn>(address=.ClampToEdge)

@bounds(GRID_WIDTH+3, GRID_HEIGHT+3)
def gridCol |x, y|{
   # revert to grid coord.
   let gxy = [x, y] - 1

   # screen coord of gxy
   let gs_xy0 =  gxy* GRID_INTERVAL
   let candW = 3*GRID_INTERVAL
   let [sb, sg, sr, sa, count] = rsum(0..<candW, 0..<candW) |rx, ry|{
    # candidate input pixel coord.
    let sxy = gs_xy0+[rx, ry] - i32(0.5*candW)
   
    # nearest site of sxy
    let g_nn = nnEx(*sxy)

    let [b, g, r, a] = i32(inputEx( *sxy ))
    # whether nearest site is same as gxy?
    ifel( all(g_nn.xy == gxy),
        [*[b, g, r]*a, a, 1],
        [0, 0, 0, 0, 0]
    )
  }
  
  ifel(sa==0,
    u8[0, 0, 0, 0],
    u8[*[sb, sg, sr]/sa, sa/count] )
}

def result_u8 |x, y| {
  let g_nn = nn(x, y)
  gridCol(*(g_nn.xy +1))
}
)";
  
static const char* g_mfg_script_magnifier = R"(
@title $TITLE_MAGNIFIER

@param_pos CENTER(POINTER, label=$LABEL_CENTER)
@param_i32 R(SLIDER, label=$LABEL_RADIUS, min=10, max=100, init=50)

let [rx, ry] = f32(R)/f32(input_u8.extent())

# Use shorter ratio as r
let r = min(rx, ry)

let finput = sampler<input_u8>(coord=.NormalizedLinear)

def result_u8 |x, y| {
  let fv = to_ncoord([x, y])

  let deltaV = fv-CENTER
  let distCand = length(deltaV)^2

  let dist = clamp(distCand, 0.0001, r)

  let deltaO = deltaV*dist/r
  let fo = deltaO+CENTER
  finput(*fo)
}
)";

static const char* g_mfg_script_drag = R"(
@title $TITLE_DRAG

@param_pos CENTER(POINTER, label=$LABEL_CENTER)
@param_f32 STRENGTH(SLIDER, label=$LABEL_STRENGTH, min=0.0, max=1.0, init=0.5)

let [cx, cy] = CENTER

let finput = sampler<input_u8>(address=.ClampToEdge, coord=.NormalizedNearest)

def result_u8 |x, y| {
  let [fx, fy] = to_ncoord([x, y])

  let [deltaX, deltaY] = [fx -cx, fy-cy]
  let dist = (deltaX^2+deltaY^2)^0.5
  let distClamp = ifel( dist*3.0 > 1.0, 1.0, 3.0*dist)
  let t = 1.0 + (1.0-distClamp)^(1.0/STRENGTH)
  let [deltaXo, deltaYo] = [deltaX, deltaY]*t
  let [fxo,fyo] = [deltaXo+cx, deltaYo+cy]

  finput(fxo, fyo)
}
)";
  
static const char* g_mfg_script_fisheye = R"(
@title $TITLE_FISHEYE

@param_pos CENTER(POINTER, label=$LABEL_CENTER)
@param_f32 DISTORT(SLIDER, label=$LABEL_DISTORT, min=0.0, max=4.0, init=1.0)

let finput = sampler<input_u8>(coord=.NormalizedNearest, address=.ClampToBorderValue, border_value=u8[0, 0, 0, 0])
# let finput = sampler<input_u8>(coord=.NormalizedLinear, address=.ClampToBorderValue, border_value=u8[0, 0, 0, 0])

def result_u8 |x, y| {
  let fv = to_ncoord([x, y])

  let r2 = distance(fv,  CENTER)^2

  let newV = fv + (fv-CENTER)*DISTORT*r2

  finput(*newV)
}
)";
  
static const char* g_mfg_script_bloom = R"(
@title $TITLE_BLOOM

@param_f32 THRESHOLD(SLIDER, label=$LABEL_THRESHOLD, init=0.7, min=0.0, max=1.0)
@param_i32 ar(SLIDER, label=$LABEL_BLUR_WIDTH, min=2, max=20, init=5)

#$COMMENT_GAUSS_BLUR_WIDTH
let sigma = f32(ar)
let WR = 3*ar
let mWR = -(WR-1)
let [W, H] = input_u8.extent()

@bounds(W, H)
def high_lumi |x, y| {
   let ncol = to_ncolor(input_u8(x, y))
   let lumi = dot([ncol.x, ncol.y, ncol.z], [0.0722, 0.7152, 0.2127])
   ifel(lumi > THRESHOLD, ncol, vec4(0.0))
}

let extend = sampler<high_lumi>(address=.ClampToEdge)

#$COMMENT_LUMINANCE_GAUSS_BLUR_TWO_PASS

let sigma = f32(WR)/3.0

@bounds(WR)
def weight |x| {
  exp(- f32(x^2)/(2.0*sigma^2) )
}

let coeff = rsum(mWR..<WR) |rx| { weight(abs(rx)) }

@bounds(W, H)
def xblur |x, y| {
   let [bs, gs, rs, as] = rsum(mWR..<WR) |rx| {
      let col = extend( x + rx, y)
      let ga = col.w*weight(abs(rx))
      [*(col.xyz*ga), ga]
   }
   let a = as/coeff
   let bgr = ifel(abs(as) < 0.0001, vec3(0.0), [bs, gs, rs]/as)
   [*bgr, a]
}

let clamped_x = sampler<xblur>(address=.ClampToEdge)

def result_u8 |ix, iy| {
   let bloom = rsum(mWR..<WR) |ry| {
      let col = clamped_x( ix, iy+ry)
      let ga = col.w*weight(abs(ry))
      [*(col.xyz*ga), ga]
   }
   let ga = bloom.w
   let ba = ga/coeff
   let b_bgr = ifel(abs(ga) < 0.0001, vec3(0.0), bloom.xyz/ga)
   let b_x = dot(b_bgr.xyz, [0.1804, 0.3576, 0.4125])
   let b_y = dot(b_bgr.xyz, [0.0722, 0.7152, 0.2127])
   let b_z = dot(b_bgr.xyz, [0.9502, 0.1192, 0.0193])

   #$COMMENT_RGB_XYZ_BLOOM_ADD
   let org_col = to_ncolor(input_u8(ix, iy))
   let org_x = dot(org_col.xyz, [0.1804, 0.3576, 0.4125])
   let org_y = dot(org_col.xyz, [0.0722, 0.7152, 0.2127])
   let org_z = dot(org_col.xyz, [0.9502, 0.1192, 0.0193])

   let xyz2 = ba*[b_x, b_y, b_z]+[org_x, org_y, org_z]

   let r2 = dot(xyz2, [3.2405, -1.5371, -0.4985])
   let g2 =dot(xyz2, [-0.9693, 1.8760, 0.04156])
   let b2 = dot(xyz2, [0.05564, -0.204, 1.0572])
 
   to_u8color([b2, g2, r2, org_col.w])
}
)";

static const char* g_mfg_script_sphere = R"(
@title $TITLE_SPHERE_PROJECTION

let finput = sampler<input_u8>(coord=.NormalizedLinear)

def result_u8 |x, y| {
  let fxy = to_ncoord([x, y])
  let w = 2*(0.5^2-(fxy.y-0.5)^2)^(0.5)
  let r = length(fxy - [0.5, 0.5])
  
  let orig_x = 0.5 + (fxy.x - 0.5)*1.0/w
  ifel( r > 0.5, u8[0, 0, 0, 0],
        finput(orig_x, fxy.y))
}
)";

static const char* g_mfg_script_winding = R"(
@title $TITLE_WINDING

@param_f32 RAD(SLIDER, label="Phase of sin",  min=1.0, max=30.0, init=5.0)

let h = 0.1

# shrink ratio, small enough to add h.
let R = 0.8

let finput = sampler<input_u8>(coord=.NormalizedNearest)

def result_u8 |x, y| {
  let [fx, fy] = to_ncoord([x, y])

  let x0 = (fx-(1.0-R)/2.0)/R
  let x1 = x0+h*sin( RAD*fy )

  ifel( x1 < 0.0 || x1 > 1.0,
         u8[0, 0, 0, 0],
         finput(x1, fy)
   )
}
)";

static const char* g_mfg_script_zigzag = R"(
@title $TITLE_ZIGZAG

@param_pos CENTER(POINTER, label=$LABEL_CENTER)

let [cx, cy] = CENTER
let finput = sampler<input_u8>(address=.ClampToEdge, coord=.NormalizedNearest)

let PI = 3.141592

def result_u8 |x, y| {
  let [fx, fy] = to_ncoord([x, y])


  let r = sqrt( ((fx-cx)^2+(fy-cy)^2)/2.0 )
  let theta = 30.0*exp(-40.0*r)*sin((100.0-80.0*r)*PI*r)
  
  let [dx, dy] = [fx-cx, fy-cy]
  let dx2 = dx*cos(-theta)-dy*sin(-theta)
  let dy2 = dx*sin(-theta)+dy*cos(-theta)

  finput(dx2+cx, dy2+cy)
}
)";

static const char* g_mfg_script_perlin = R"(
@title $TITLE_PERLIN
@param_f32v4 fgcol(COLOR_PICKER, label="forground")

let [W, H] = input_u8.extent()

let GRID_W = 50

@bounds(50, 50)
def gridNoise |x, y| {
  [2.0*rand()-1.0, 2.0*rand()-1]
}

let gridNoiseEx = sampler<gridNoise>(address=.ClampToEdge)

def result_u8 |x, y| {
  let [fx, fy] = [f32(x)/f32(W), f32(y)/f32(H)]

  let [gridX0, gridY0] = [i32(fx*GRID_W), i32(fy*GRID_W)]
  let [gridX1, gridY1] = [gridX0+1, gridY0+1]

  # 0.0 < dx0, dy0 < 1.0
  # -1.0 < dx1, dy1 < 0.0
  let [dx0, dy0] = [fx*GRID_W-gridX0, fy*GRID_W-gridY0]
  let [dx1, dy1] = [dx0-1.0, dy0-1.0]
 
  # interpolator, 6 t^5 - 15 t^4 + 10 t^3
  let d0 = [dx0, dy0]
  let [wx, wy] = d0^3*(d0*(6.0*d0 - 15.0) + 10.0)

  let [noise00X, noise00Y] = gridNoiseEx(gridX0, gridY0)
  let dp00 = dx0*noise00X+dy0*noise00Y
  
  let [noise10X, noise10Y] = gridNoiseEx(gridX1, gridY0)
  let dp10 = dx1*noise10X+dy0*noise10Y

  let [noise01X, noise01Y] = gridNoiseEx(gridX0, gridY1)
  let dp01 = dx0*noise01X+dy1*noise01Y

  let [noise11X, noise11Y] = gridNoiseEx(gridX1, gridY1)
  let dp11 = dx1*noise11X+dy1*noise11Y

  let dp0 = dp00*(1.0-wx)+dp10*wx
  let dp1 = dp01*(1.0-wx)+dp11*wx

  # from -1 to 1
  let dp = dp0*(1.0-wy)+dp1*wy

  let normalized = dp*0.5+0.5
  let fcol = saturate(fgcol*normalized)
  to_u8color([*fcol.xyz, 1.0])
}
)";

static const char* g_mfg_script_line_concent = R"(
@title $TITLE_LINE_CONCENT

# Specify width by ratio of I.
@param_f32 TR(SLIDER, label=$LABEL_WIDTH,  init=0.5, min=0.1, max=1.0)
@param_f32 WRR(SLIDER, label=$LABEL_WIDTH_RANDOMNESS,  init=0.2, min=0.0, max=1.0)
@param_f32 DR(SLIDER, label=$LABEL_DENSITY, init=0.5, min=0.01, max=1.0)
@param_f32 LR(SLIDER, label=$LABEL_LENGTH_RANDOMNESS,  init=0.2, min=0.0, max=1.0)
@param_pos ORIGIN(POINTER, label=$LABEL_CENTER)

let PI = 3.141592
let N_MAX = 400
# Number of lines. Interval is 2PI/N
let N = f32(N_MAX)*DR
let I = 2.0*PI/N


# Shorter edge of canvas (width or height).
let SIDE = f32(min(*input_u8.extent()))

# Start line from about half of shorter edge.
let OFFSET = SIDE/4.0

# N random number, but N is not known  in advance, so create N_MAX.
@bounds(N_MAX, 2)
def RAND_TABLE |x, y| { rand() }

# return positive region theta.
fn atan2p |y: f32, x: f32| {
  let theta = atan2(y, x)
  ifel(theta > 0.0, theta, 2.0*PI+theta)
}

def result_u8 |x_0, y_0| {
  # ORIGIN to pixel pos.
  let fo = ORIGIN*f32(input_u8.extent())
  let fxy0 = f32[x_0, y_0]

  @bounds(3, 3)
  def occupy |xi, yi| {
    let fxy = fxy0 - [0.33, 0.33] + 0.33*f32[xi, yi]

    let [rx, ry] = fxy - fo

    let theta = atan2p(ry, rx)

    # Index of I.
    let A = round(theta/I)
    # Basic angle of this pos.
    let ATheta = A*I
    let AI = i32(A)

     # Width of line, stands for the angle of triangle. Randomness is added for each A.
     let T = TR*(1.0-WRR*RAND_TABLE(AI, 0))*I

     # Length of origin. Away from OFFSET+random.
     let RO = OFFSET*(1.0+2.0*LR*RAND_TABLE(AI, 1))

     # to: Origin of triangle
     let to =  fo + RO* [cos(ATheta), sin(ATheta)]

     let txy = fxy - to
     let ttheta = atan2p(txy.y, txy.x)

     ifel(abs(ttheta-ATheta) < T/2.0, 1.0, 0.0)
  }

  let avg_occupy = rsum(0..<3, 0..<3) |rx, ry| { occupy(rx, ry) }/9.0
  u8[0x0, 0x0, 0x0, round(255.0*avg_occupy)]
}
)";

static const char* g_mfg_script_line_parallel = R"(
@title $TITLE_LINE_PARALLEL

@param_f32 LENGTH0(SLIDER, label=$LABEL_LENGTH,  init=0.7, min=0.01, max=1.0)
@param_f32 LR(SLIDER, label=$LABEL_LENGTH_RANDOMNESS,  init=0.2, min=0.0, max=1.0)
@param_f32 TR0(SLIDER, label=$LABEL_WIDTH,  init=0.9, min=0.1, max=1.0)
@param_f32 TRR(SLIDER, label=$LABEL_WIDTH_RANDOMNESS,  init=0.2, min=0.0, max=1.0)
@param_f32 DR(SLIDER, label=$LABEL_DENSITY, init=0.9, min=0.01, max=1.0)

# Number of lines
let N_MAX = 60
let N = f32(N_MAX)*DR

let I = 1.0/N

@bounds(N_MAX, 2)
def RAND_TABLE |x, y| { rand() }

let WH = f32(input_u8.extent())
let DELTA = [1/WH.x, 1/WH.y]

def result_u8 |x, y| {
  let fxy0 = to_ncoord([x, y])

  @bounds(3, 3)
  def occupy |xi, yi| {
    let fxy = fxy0 - 0.33*DELTA + 0.33*DELTA*f32[xi, yi]

    # Index of compartment
    let A = round(fxy.y/I)
    let AI = i32(A)

    # Origin of this compartment
    let AO = A*I

    # Line width
    let TR = TR0*(1.0-TRR*RAND_TABLE(AI, 0))
    let T = TR*I

    # line length
    let LENGTH = LENGTH0*(1.0-LR*RAND_TABLE(AI, 1))

    # Origin of triangle
    let to = [LENGTH, AO]
    
    # Whether inside "to" triangle or not.
    # Based on angle: between [0, AO + I/2] to [0, AO-I/2].
    # But theta is between -pi to pi.
    # So bigger than thetaPllus or smaller than thetaMinus
    let aplus = [0.0, AO+T/2.0] - to
    let aminus = [0.0, AO-T/2.0] - to
    let thetaPlus = atan2(aplus.y, aplus.x)
    let thetaMinus = atan2(aminus.y, aminus.x)
    
    let txy = fxy - to
    let theta = atan2(txy.y, txy.x)

    ifel( theta >= thetaPlus || theta < thetaMinus,
          1.0,
          0.0)
  }
  let avg_occupy = rsum(0..<3, 0..<3) |rx, ry| { occupy(rx, ry) }/9.0
  u8[0x0, 0x0, 0x0, round(255.0*avg_occupy)]
}
)";

static const char* g_mfg_script_bezier = R"(
@title $TITLE_BEZIER

# Based on following paper.
# [Resolution Independent Curve Rendering using Programmable Graphics Hardware - Microsoft Research](https://www.microsoft.com/en-us/research/publication/resolution-independent-curve-rendering-using-programmable-graphics-hardware/)

@param_pos v2_pos(POINTER, label=$LABEL_CONTROL_POINT)

# Bottom line, hard coded to (0.2, 0.8), (0.8, 0.8). 3D notation.
let v0 = [0.2, 0.8, 0.0]
let v1 = [0.8, 0.8, 0.0]
let v2 = [*v2_pos, 0.0]

# uv coordinate 3 points.
let uv0 = [0.0, 0.0]
let uv1 = [0.5, 0.0]
let uv2 = [1.0, 1.0]

def result_u8 |x, y| {
  let fxy = [*to_ncoord([x, y]), 0.0]

  # Draw triangle based on v1. Sign of cross product.
  let first = cross((v1-v0), fxy - v0).z
  let second = cross((v2- v1), fxy-v1).z
  let third = cross((v0-v2), fxy-v2).z
  let insideTriangle = first*second > 0.0 && second*third > 0.0

  # calculate barycentric coordinate of v0, v1, v2 of fxy.
  let pv0 = v0 - fxy
  let pv1 =  v1 - fxy
  let pv2 = v2 - fxy
  let area = cross(v1-v0, v2-v0).z
  let bp0 = [cross(pv1, pv2).z, cross(pv2, pv0).z, cross(pv0, pv1).z]
  let bp = abs(bp0)/abs(area)

  let uvp =   bp.y*uv0+bp.z*uv1+bp.x*uv2
  let f = uvp.x*uvp.x-uvp.y

  ifel(insideTriangle && f < 0.0, u8[0, 0, 0xff, 0xff] ,...)
  elif(insideTriangle, u8[0, 0xff, 0, 0x88],  u8[0, 0, 0, 0])
}
)";

static const char* g_mfg_script_nightvision = R"(
@title $TITLE_NIGHTVISION

@param_f32 STR(SLIDER, label=$LABEL_NOISE_STRENGTH, min=0.0, max=1.0, init=0.3)

def result_u8 |x, y| {
  let r = rand()
  let ncol = to_ncolor(input_u8(x, y))
  let lum = dot([0.2126, 0.7152, 0.0722, 1.0], ncol)
  to_u8color([0.0, lum*(r*STR+0.25), 0.0, 1.0])
}
)";

static const char* g_mfg_script_grain = R"(
@title $TITLE_GRAIN

@param_f32 STR(SLIDER, label=$LABEL_STRENGTH, min=0.0, max=1.0, init=0.1)

def result_u8 |x, y| {
  let r = rand()
  let ncol = f32(input_u8(x, y))/255.0
  let delta = STR*r
  u8(min(vec4(255.0), 255.0*(ncol - [*vec3(delta) , 0.0])))
}
)";

static const char* g_mfg_script_rg_delta = R"(
@title $TITLE_RG_DELTA

let [dx_r, dy_r] = [3, 1]
let [dx_g, dy_g] = [-2, -3]

let extended = sampler<input_u8>(address=.ClampToEdge)

def result_u8 |x, y| {
  let [_, _, r, _] = extended(x+dx_r, y+dy_r)
  let [_, g, _, _] = extended(x+dx_g, y+dy_g)
  let [b, _, _, a] = extended(x, y)

  u8[b, g, r, a]
}
)";

static const char* g_mfg_script_region_delta = R"(
@title $TITLE_REGION_DELTA

@param_f32 OVERLAY_CY(SLIDER, label=$LABEL_DELTA_REGION_TOP, min=0.0, max=1.0, init=0.2)
@param_f32 OVERLAY_H(SLIDER, label=$LABEL_DELTA_REGION, min=0.01, max=0.5, init=0.1)
@param_f32 DELTA(SLIDER, label=$LABEL_DELTA_SIZE, min=0.01, max=0.5, init=0.1)
@param_f32 ALPHA(SLIDER, label=$LABEL_BLEND_ALPHA, min=0.01, max=1.0, init=0.2)
@param_f32 NOISE(SLIDER, label=$LABEL_NOISE_STRENGTH, min=0.01, max=1.0, init=0.1)

let finput = sampler<input_u8>(coord=.NormalizedLinear)

def result_u8 |x, y| {
  let [fx, fy] = to_ncoord([x, y])

  let d_with_noise = DELTA + NOISE*DELTA*rand()
  let moved = fx+d_with_noise
  let moved_wrap = ifel(moved > 1.0, moved - 1.0, moved)

  let mvec = finput(moved_wrap, fy)
  let ivec = input_u8(x, y)

  let blend = u8((1.0-ALPHA)*ivec+(ALPHA*mvec))

  ifel( fy >= OVERLAY_CY && fy <= OVERLAY_CY+OVERLAY_H,
          blend,
          input_u8(x, y))
}
)";

static const char* g_mfg_script_repeat = R"(
@title $TITLE_REPEAT

@param_pos pos(POINTER, label=$LABEL_POINT_LOWER_RIGHT)

let wh = i32(pos*f32(input_u8.extent()))

def result_u8 |x, y| {
  let xy0 = [x, y] % wh
  input_u8(*xy0)
}
)";

static const char* g_mfg_script_MLAA = R"(
@title $TITLE_MLAA

# For further detail, see.
# https://github.com/karino2/MFG/tree/main/docs/examples/MLAA.md

let EDGE_MAX_LENGTH = 7
let END_FOUND_MASK = i32(0x80)
let DIFF_THRESHOLD= 1.0/12.0 #$COMMENT_DIFF_THRESHOLD

#$COMMENT_EDGE_TENSOR
# u8[bottom, right, 0, 0]
@bounds( (input_u8.extent(0)-1), (input_u8.extent(1)-1))
def edge |x, y|{
   let lumiVec = [0.0722, 0.7152, 0.2127]
   let col0 = to_ncolor(input_u8(x, y))
   let col0L = [*gamma2linear(col0.xyz), col0.w]

   let colRight = to_ncolor(input_u8(x+1, y))
   let colBottom = to_ncolor(input_u8(x, y+1))
   let colRightL = [*gamma2linear(colRight.xyz), colRight.w]
   let colBottomL = [*gamma2linear(colBottom.xyz), colBottom.w]

   let eb = distance(colBottomL, col0L) > DIFF_THRESHOLD
   let er = distance(colRightL, col0L) > DIFF_THRESHOLD

   #$COMMENT_U8V4_OPTIMIZE
   u8[eb, er, 0, 0]
}


# let edgeEx = sampler<edge>(address=..ClampToBorderValue, border_value=u8(vec4(0)))
let edgeEx = sampler<edge>(address=.ClampToEdge)

# edgeLen
#
#$COMMENT_EDGE_LEN
#
# u8[bottomNeg, bottomPos, rightNeg, rightPos]
#
@bounds( (input_u8.extent(0)-1), (input_u8.extent(1)-1))
def edgeLen |x, y|{
   #$COMMENT_BOTTOM_RIGHT_POS
   #
   #$COMMENT_EDGE_LEN_ACCM
   let eLenBRPos0 = reduce(init=[0, 0], 0..<EDGE_MAX_LENGTH+1) |i, accm|{
     let edgeBP = edgeEx(x+i, y)
     let edgeRP = edgeEx(x, y+i)

     let curEdge = [edgeBP.x, edgeRP.y]
     let curOrtho = [edgeBP.y, edgeRP.x]

     let alreadyEnd = (accm & END_FOUND_MASK) != 0
     let prevEnd = (curEdge == 0)
     let curEnd = (curOrtho != 0)

     ifel(alreadyEnd,
         accm,
         ...)
      elif(i == 0,
         ifel(curEdge,
               ifel(curEnd,
                  [0, 0] | END_FOUND_MASK ,
                  [0, 0]
               ),
               [-1, -1]),
         ...)
      elif(prevEnd, #$COMMENT_NON_ZERO_NON_END
              accm|END_FOUND_MASK,
              ...)
      elif(curEnd, 
              (accm+1)|END_FOUND_MASK,
              accm+1
              )
   }

   #
   # Next, calculate bottomNegative, rightNegative direction.
   #
   let eLenBRNeg0 = reduce(init=[0, 0], 0..<EDGE_MAX_LENGTH+1) |i, accm2|{
     #$COMMENT_NEGATIVE_LEFT_HANDLING

     let edgeBN = edgeEx(x-i, y)
     let edgeRN = edgeEx(x, y-i)

     let curEdge = [edgeBN.x, edgeRN.y]
     let curOrtho = [edgeBN.y, edgeRN.x]
     
     let alreadyEnd = (accm2 & END_FOUND_MASK) != 0
     let prevEnd = (curOrtho != 0) || (curEdge == 0)

     #$COMMENT_NEGA_POSI_DIFF
     ifel(alreadyEnd,
         accm2,
         ...)
      elif(i == 0,
         ifel(curEdge,
               [0, 0],
               [-1, -1]),
         ...)
      elif(prevEnd,
              accm2|END_FOUND_MASK,
              accm2+1)
   }

   #
   # u8[bottomNeg, bottomPos, rightNeg, rightPos]
   #
   let eLen0 = [eLenBRNeg0.x, eLenBRPos0.x, eLenBRNeg0.y, eLenBRPos0.y]
   let eLen = ifel(eLen0 == -1, [0, 0, 0, 0], eLen0)
   u8(eLen)
}



let edgeLenEx = sampler<edgeLen>(address=.ClampToBorderValue, border_value=u8(vec4(0)))
let inputEx = sampler<input_u8>(address=.ClampToEdge)
)"
R"(
def result_u8 |x, y| {
  let eps = 0.0001
  let col0 = to_ncolor(input_u8(x, y))
  let col0L = [*gamma2linear(col0.xyz), col0.w]


  #
  # [bottomNeg, botomPos, rightNeg, rightPos]
  #
  let einfo = i32(edgeLenEx(x, y))
  let endFound = (einfo & END_FOUND_MASK) != 0
  let edgeLen = (einfo & (END_FOUND_MASK-1))


  #
  # Horizontal interpolation
  #
  # [bottomNeg, bottomPos, topNeg, topPos]
  #

  # top (neg, pos) einfo
  let einfoTNP = i32(edgeLenEx(x, y-1).xy)
  let TNPEnd = (einfoTNP & END_FOUND_MASK) != 0
  let TNPLen = (einfoTNP & (END_FOUND_MASK-1))

  # [bottomNeg, botomPos]
  let BNPLen = edgeLen.xy
  let BNPEnd = endFound.xy  

  # [bottomNeg, bottomPos, topNeg, topPos]
  let HLen = [*BNPLen, *TNPLen]
  let HEnd = [*BNPEnd, *TNPEnd]

  # Calculate Edge len (ELen)
  let B_ELen = f32(HLen.x+HLen.y+1)
  let bmid = B_ELen/2.0

  let T_ELen = f32(HLen.z+HLen.w+1)
  let tmid = T_ELen/2.0

  # HLen: horizontal len.
  # H_ELen: horizontal edge len
  let H_ELen = [B_ELen, B_ELen, T_ELen, T_ELen]
  let HMid = [bmid, bmid, tmid, tmid]


  # Inside L condition.

  let isHRange = HLen+eps < HMid

  # Opposite color of current pixel.
  # OpsB == Opposite color of Bottom.
  let colOpsB = to_ncolor(inputEx(x, y+1))
  let colOpsBL = [*gamma2linear(colOpsB.xyz), colOpsB.w]
  
  # Opposite color of top
  let colOpsT = to_ncolor(inputEx(x, y-1))
  let colOpsTL = [*gamma2linear(colOpsT.xyz), colOpsT.w]

  
  # next color of bottomNegative
  let colBN = to_ncolor(inputEx(x-1-HLen.x, y))
  let colBNL = [*gamma2linear(colBN.xyz), colBN.w]

  # next color of bottomPositive
  let colBP = to_ncolor(inputEx(x+1+HLen.y, y))
  let colBPL = [*gamma2linear(colBP.xyz), colBP.w]

  # same for topNegative
  let colTN = to_ncolor(inputEx(x-1-HLen.z, y))
  let colTNL = [*gamma2linear(colTN.xyz), colTN.w]

  # same for topPositive
  let colTP = to_ncolor(inputEx(x+1+HLen.w, y))
  let colTPL = [*gamma2linear(colTP.xyz), colTP.w]

  # Opposite of BN, BP, TN, TPs are OpsB, OpsB, OpsT, OpsT
  let sameToOpsBN = distance(colOpsBL, colBNL) < DIFF_THRESHOLD
  let sameToOpsBP = distance(colOpsBL, colBPL) < DIFF_THRESHOLD
  let sameToOpsTN = distance(colOpsTL, colTNL) < DIFF_THRESHOLD
  let sameToOpsTP = distance(colOpsTL, colTPL) < DIFF_THRESHOLD


  let sameToOpsH = [sameToOpsBN, sameToOpsBP, sameToOpsTN, sameToOpsTP]
  
  # Inside L? for horizontal
  let insideHL = isHRange && HEnd && sameToOpsH

  # Area of trapezoid, some simple calculus lead to (1-(2n+1)/L)*1/2
  let areaH = (1.0-(2.0*HLen+1.0)/H_ELen)*0.5
  # for triangle case (end of trapezoid becomes triangle)
  let isTriangleH = abs( (HMid-f32(HLen)) - 0.5 ) < 0.001

  # area of triangle, same for pos and neg.
  let triAreaH = 1.0/(8.0*H_ELen)

  # blending ratio of horizontal
  let ratioH = ifel(insideHL,
                          ifel(isTriangleH, triAreaH, areaH),
                          [0.0, 0.0, 0.0, 0.0])

  #
  # For vertial orientation, that is, interpolation to left and right.
  # Calculation is similar for horizontal, but is cumbersome to do it at once.
  # Just dup for easier understanding.
  #

  let RNPLen = edgeLen.zw
  let RNPEnd = endFound.zw

  # left einfo
  let einfoLNP = i32(edgeLenEx(x-1, y).zw)
  let LNPEnd = (einfoLNP & END_FOUND_MASK) != 0
  let LNPLen = (einfoLNP & (END_FOUND_MASK-1))


  # [rightNeg, rightPos, leftNeg, leftPos]
  # Right coming ealier is a bit tricky, but it's more natural to calculate for increasing order.
  # This is because stored edge is right, not left.
  let VLen = [*RNPLen, *LNPLen]
  let VEnd = [*RNPEnd, *LNPEnd]

  let R_ELen = f32(VLen.x+VLen.y+1)
  let rmid = R_ELen/2.0

  let L_ELen = f32(VLen.z+VLen.w+1)
  let lmid = L_ELen/2.0

  let V_ELen = [R_ELen, R_ELen, L_ELen, L_ELen]
  let VMid = [rmid, rmid, lmid, lmid]

  let isVRange = VLen+eps < VMid
  
  # opposite color, right and left.
  let colOpsR = to_ncolor(inputEx(x+1, y))
  let colOpsRL = [*gamma2linear(colOpsR.xyz), colOpsR.w]

  let colOpsL = to_ncolor(inputEx(x-1, y))
  # Left Linear.
  let colOpsLL = [*gamma2linear(colOpsL.xyz), colOpsL.w]

  let colRN = to_ncolor(inputEx(x, y-1-VLen.x))
  let colRNL = [*gamma2linear(colRN.xyz), colRN.w]

  let colRP = to_ncolor(inputEx(x, y+1+VLen.y))
  let colRPL = [*gamma2linear(colRP.xyz), colRP.w]

  let colLN = to_ncolor(inputEx(x, y-1-VLen.z))
  let colLNL = [*gamma2linear(colLN.xyz), colLN.w]

  let colLP = to_ncolor(inputEx(x, y+1+VLen.w))
  let colLPL = [*gamma2linear(colLP.xyz), colLP.w]

  # Opposite color of rightNeg, rightPos, leftNeg, leftPos is OpsR, OpsR, OpsL, OpsL
  let sameToOpsRN = distance(colOpsRL, colRNL) < DIFF_THRESHOLD
  let sameToOpsRP = distance(colOpsRL, colRPL) < DIFF_THRESHOLD
  let sameToOpsLN = distance(colOpsLL, colLNL) < DIFF_THRESHOLD
  let sameToOpsLP = distance(colOpsLL, colLPL) < DIFF_THRESHOLD

  let sameToOpsV = [sameToOpsRN, sameToOpsRP, sameToOpsLN, sameToOpsLP]

  let insideVL = isVRange && VEnd && sameToOpsV

  let areaV = (1.0-(2.0*VLen+1.0)/V_ELen)*0.5
  let isTriangleV = abs( (VMid-f32(VLen)) - 0.5 ) < 0.001

  let triAreaV = 1.0/(8.0*V_ELen)

  let ratioV = ifel(insideVL,
                          ifel(isTriangleV, triAreaV, areaV),
                          [0.0, 0.0, 0.0, 0.0])

   # Calculate ratio of bottom, top, right, left from ratioV and ratioH.
   # It's easier to calculate separately for special center handling and opposite color difference.
   let ratioB = ifel(insideHL.x && insideHL.y && isTriangleH.x, 
                         # Just center, 0.5 case. IsTriangle check is enought for one side.
                         ratioH.x+ratioH.y,...)
                       elif(insideHL.x,
                         ratioH.x, ratioH.y)

   let ratioT = ifel(insideHL.z && insideHL.w && isTriangleH.z, 
                          ratioH.z+ratioH.w,...)
                       elif(insideHL.z,
                          ratioH.z, ratioH.w)

   let ratioR = ifel(insideVL.x && insideVL.y && isTriangleV.x, 
                         ratioV.x+ratioV.y,...)
                       elif(insideVL.x,
                         ratioV.x, ratioV.y)

   let ratioL = ifel(insideVL.z && insideVL.w && isTriangleV.z, 
                          ratioV.z+ratioV.w,...)
                       elif(insideVL.z,
                          ratioV.z, ratioV.w)

  # Blend color.
  let destColBL = mix(col0L, colOpsBL, ratioB)
  let destColTL = mix(col0L, colOpsTL, ratioT)
  let destColRL = mix(col0L, colOpsRL, ratioR)
  let destColLL = mix(col0L, colOpsLL, ratioL)

  let destColB = to_u8color([*linear2gamma(destColBL.xyz), destColBL.w])
  let destColT = to_u8color([*linear2gamma(destColTL.xyz), destColTL.w])
  let destColR = to_u8color([*linear2gamma(destColRL.xyz), destColRL.w])
  let destColL = to_u8color([*linear2gamma(destColLL.xyz), destColLL.w])

   # If multiple interpolation is overwrap, use biggest ratio's one.
   ifel( ratioB+ratioL > ratioR+ratioL,
     ifel( ratioB > ratioT,
        destColB,
        destColT),
      ifel( ratioR > ratioL,
         destColR,
         destColL)
   )

}

)";

static const char* g_mfg_script_point_light = R"(
@title $TITLE_POINT_LIGHT

@param_f32 intensity(SLIDER, label=$LABEL_STRENGTH, init=0.1, min=0.01, max=1.0)
@param_pos sunPos(POINTER, label=$LABEL_LIGHT_SOURCE_POS)

let const = 0.1

def result_u8 |x, y| {
  let fxy = to_ncoord([x, y])
  let dist = distance(fxy, sunPos)
  let iat = 0.1*intensity*(dist+const)^(-2.0)

  let cy = saturate(iat)
  let cx = (0.95047/1.0)*cy
  let cz = (1.0888/1.0)*cy

  let xyza2 = to_xyza(input_u8(x, y))
  let resXYZ = [cx, cy, cz] + xyza2.xyz

  xyza_to_u8color([*resXYZ, xyza2.w])
})";

static const char* g_mfg_script_point_light_and_lens_flare = R"(
@title $TITLE_POINT_LIGHT_AND_LENS_FLARE

@param_f32 intensity(SLIDER, label=$LABEL_LIGHT_SOURCE_INTENSITY, init=0.15, min=0.01, max=1.0)
@param_pos sunPos(POINTER, label=$LABEL_LIGHT_SOURCE_POS)
@param_f32 THRESHOLD(SLIDER, label=$LABEL_THRESHOLD, init=0.6, min=0.0, max=1.0)
@param_i32 ar(SLIDER, label=$LABEL_BLUR_WIDTH, min=2, max=20, init=7)
@param_f32 ghostSpace(SLIDER, label=$LABEL_GHOST_INTERVAL, init=0.5, min=0.0, max=1.0)
@param_f32 ghostOffset(SLIDER, label=$LABEL_GHOST_INIT_POS, init=0.0, min=0.0, max=2.0)
@param_f32 scale(SLIDER, label=$LABEL_GHOST_STRENGTH, init=1.0, min=0.0, max=4.0)
@param_i32 ghostNum(SLIDER, label=$LABEL_COUNT_NUM, init=4, min=1, max=10)

let const = 0.1

#$COMMENT_GAUSS_BLUR_WIDTH
let sigma = f32(ar)
let WR = 3*ar
let mWR = -(WR-1)

let [W, H] = input_u8.extent()
#$COMMENT_DOWNSAMPLE_SIZE
let [BW, BH] = max([1, 1], [W, H]/8)

let finput = sampler<input_u8>(coord=.NormalizedLinear)

@bounds(BW, BH)
def point_light |x, y| {
   let fxy = to_ncoord([x, y])
   let dist = distance(fxy, sunPos)
   let iat = 0.1*intensity*(dist+const)^(-2.0)

   saturate(iat)*step(THRESHOLD, iat)
}

let lumiEx = sampler<point_light>(address=.ClampToEdge)

let sigma = f32(WR)/3.0

@bounds(WR)
def weight |x| {
  exp(- f32(x^2)/(2.0*sigma^2) )
}

let coeff = rsum(mWR..<WR) |rx| { weight(abs(rx)) }

@bounds(BW, BH)
def xblur |x, y| {
   let slumi = rsum(mWR..<WR) |rx| {
      let lumi = lumiEx( x + rx, y)
      lumi * weight(abs(rx))
   }
   slumi/coeff
}

let clamped_x = sampler<xblur>(address=.ClampToEdge)

@bounds(BW, BH)
def blur |x, y| {
   let slumi = rsum(mWR..<WR) |ry| {
      let lumi = clamped_x( x, y+ry)
      lumi*weight(abs(ry))
   }
   slumi/coeff
}

let fblur = sampler<blur>(coord=.NormalizedLinear)

@bounds(BW, BH)
def lensFlare |x, y| {
   let fxy = to_ncoord([x, y])
   let fixy = 1.0 - fxy
   let ghostVec = (0.5 - fixy)*ghostSpace
   let offset = ghostOffset - 1.0
   rsum(0..<ghostNum) |i| {
     let sxy = fract( fixy + (offset+i)*ghostVec )
     let d = distance(sxy, vec2(0.5))
     let weight = 1.0 - smoothstep(0.0, 0.75, d)
     weight*fblur(*sxy)
   }
}

let fLensFlare = sampler<lensFlare>(coord=.NormalizedLinear)

def result_u8 |x, y| {
  let fxy = to_ncoord([x, y])

  let flareLumi = fLensFlare(*fxy)

  let dist = distance(fxy, sunPos)
  let iat = 0.1*intensity*(dist+const)^(-2.0)

  let cy = saturate(iat+flareLumi*scale)
  let cx = (0.95047/1.0)*cy
  let cz = (1.0888/1.0)*cy

  let xyza2 = to_xyza(input_u8(x, y))
  let resXYZ = [cx, cy, cz] + xyza2.xyz
  xyza_to_u8color([*resXYZ, xyza2.w])
})";

static const char* g_mfg_script_lens_flare = R"(
@title $TITLE_LENS_FLARE

@param_f32 THRESHOLD(SLIDER, label=$LABEL_THRESHOLD, init=0.8, min=0.0, max=1.0)
@param_i32 ar(SLIDER, label=$LABEL_BLUR_WIDTH, min=2, max=20, init=7)
@param_f32 ghostSpace(SLIDER, label=$LABEL_GHOST_INTERVAL, init=0.4, min=0.0, max=1.0)
@param_f32 ghostOffset(SLIDER, label=$LABEL_GHOST_INIT_POS, init=0.0, min=0.0, max=2.0)
@param_f32 scale(SLIDER, label=$LABEL_GHOST_STRENGTH, init=1.0, min=0.0, max=4.0)
@param_i32 ghostNum(SLIDER, label=$LABEL_COUNT_NUM, init=4, min=1, max=10)

#$COMMENT_GAUSS_BLUR_WIDTH
let sigma = f32(ar)
let WR = 3*ar
let mWR = -(WR-1)

let [W, H] = input_u8.extent()
#$COMMENT_DOWNSAMPLE_SIZE
let [BW, BH] = max([1, 1], [W, H]/8)

let finput = sampler<input_u8>(coord=.NormalizedLinear)

@bounds(BW, BH)
def high_lumi |x, y| {
   let fxy = to_ncoord([x, y])
   let ncol = to_xyza(finput(*fxy))
   let lumi = ncol.y # xyz color, y is luminance.
   ifel(lumi > THRESHOLD, ncol, vec4(0.0))
}

let extend = sampler<high_lumi>(address=.ClampToEdge)

let sigma = f32(WR)/3.0

@bounds(WR)
def weight |x| {
  exp(- f32(x^2)/(2.0*sigma^2) )
}

let coeff = rsum(mWR..<WR) |rx| { weight(abs(rx)) }

@bounds(BW, BH)
def xblur |x, y| {
   let [bs, gs, rs, as] = rsum(mWR..<WR) |rx| {
      let col = extend( x + rx, y)
      let ga = col.w*weight(abs(rx))
      [*(col.xyz*ga), ga]
   }
   let a = as/coeff
   let bgr = ifel(abs(as) < 0.0001, vec3(0.0), [bs, gs, rs]/as)
   [*bgr, a]
}

let clamped_x = sampler<xblur>(address=.ClampToEdge)

@bounds(BW, BH)
def blur |x, y| {
   let bloom = rsum(mWR..<WR) |ry| {
      let col = clamped_x( x, y+ry)
      let ga = col.w*weight(abs(ry))
      [*(col.xyz*ga), ga]
   }
   let ga = bloom.w
   let ba = ga/coeff
   let b_xyz = ifel(abs(ga) < 0.0001, vec3(0.0), bloom.xyz/ga)
   [*b_xyz, ba]
}

let fblur = sampler<blur>(coord=.NormalizedLinear)

@bounds(BW, BH)
def lensFlare |x, y| {
   let fxy = to_ncoord([x, y])
   let fixy = 1.0 - fxy
   let ghostVec = (0.5 - fixy)*ghostSpace
   let offset = ghostOffset - 1.0
   rsum(0..<ghostNum) |i| {
     let sxy = fract( fixy + (offset + i)*ghostVec )
     let d = distance(sxy, vec2(0.5))
     let weight = 1.0 - smoothstep(0.0, 0.75, d)
     let scol = fblur(*sxy)
     let ga = weight*scol.w
     [*(scol.xyz*ga), ga]
   }
}

let fLensFlare = sampler<lensFlare>(coord=.NormalizedLinear)

def result_u8 |ix, iy| {
   let fxy = to_ncoord([ix, iy])

   let [b_x, b_y, b_z, ba] = fLensFlare(*fxy)
   let [org_x, org_y, org_z, org_a] = to_xyza(input_u8(ix, iy))
   let xyz2 =scale*[b_x, b_y, b_z]+[org_x, org_y, org_z]

   xyza_to_u8color([*xyz2, org_a])
}
)";

static const char* g_mfg_script_copper_engrave = R"(
@title $TITLE_COPPER_ENGRAVE

@param_f32 A(SLIDER, label=$LABEL_AMPLITUDE, init=40.0, min=10.0, max=100.0)
@param_f32 B0(SLIDER, label=$LABEL_PERIOD, init=0.2, min=0.001, max=1.0)
@param_f32 interval(SLIDER, label=$LABEL_INTERVAL, init=10.0, min=1.0, max=100.0)


def result_u8 |x, y| {
  let B = B0/20.0

  let x2 = x - A*sin(B*f32(y))+A
  let y2 = y - A*sin(B*f32(x))+A

  let offx = x2 % interval
  let offy = y2 % interval
  let y0_gray = mix(80.0, 0.0, f32(offx)/interval)
  let y1_gray = mix(110.0, 255.0, f32(offx)/interval)
  let gray = mix(y0_gray, y1_gray, f32(offy)/interval)
  let ig = i32(gray)
  u8[ig, ig, ig, 255]
}
)";

static const char* g_mfg_script_crt = R"(

@title $TITLE_CRT_FILTER

#
# Similar to following shader, but much simpler.
# [VHS and CRT monitor effect - Godot Shaders](https://godotshaders.com/shader/vhs-and-crt-monitor-effect/)

@param_f32 dratio(SLIDER, label=$LABEL_DISTORT, init=0.5, min=0.0, max=1.0)
@param_f32 roughness(SLIDER, label=$LABEL_ROUGHNESS, init=0.2, min=0.01, max=1.0)
@param_f32 vratio(SLIDER, label="Vignette", init=0.5, min=0.0, max=1.0)
@param_f32 brightness(SLIDER, label=$LABEL_BRIGHTNESS, init=1.4, min=1.0, max=3.0)

let resolution = input_u8.extent()*roughness
let grille_opacity = 0.3
let PI = 3.141592
let scanline_width = 0.25
let scanline_opacity = 0.4


let inputEx = sampler<input_u8>(coord=.NormalizedLinear, address=.ClampToBorderValue, border_value=u8[0, 0, 0, 255])

def result_u8 |x, y| {
  let fxy0 = to_ncoord([x, y]) 

  # distort
  let rxy0 = fxy0 - 0.5
  let d0 = length(rxy0)
  let d = d0^6.0
  let fxy = 0.5+(1.0+d*dratio)*rxy0

  #pixelate   
  let pxy = ceil(fxy*resolution)/resolution
  let col = inputEx(*pxy) |> to_ncolor(...) |> gamma2linearA(...)

  # grille
  let g_offset = [0.0, 1.05, 2.1]
  let g_ratio = smoothstep(vec3(0.85), vec3(0.95), abs(sin(g_offset+fxy.x*resolution.x*PI)))
  let g_col = mix(col.xyz, col.xyz*g_ratio, grille_opacity)

  # scan line
  let scan = smoothstep(scanline_width, scanline_width+0.5, abs(sin(fxy.y*resolution.y*PI)))
  let s_col = mix(g_col, g_col*scan, scanline_opacity)

  # Vignette
  let vd0 = distance(fxy, vec2(0.5))/length(vec2(0.5))
  let vd = vratio*(vd0^2.0)
  let v_col = s_col*(1.0-vd)

  let b_col = saturate(v_col*brightness)

  let res_col = [*b_col, col.w]

  linear2gammaA(res_col) |> to_u8color(...)
}
)";

///////////////////////////////////////////////////////////
// Dialog位置保存 (static)
///////////////////////////////////////////////////////////
int DialogNewProject::gPosX = 0;
int DialogNewProject::gPosY = 0;
bool DialogNewProject::gPosInit = false;

void DialogNewProject::AddTemplate( const QString& label, std::string script )
{
  mTemplateCombo->addItem( label );
  mTemplateContents.emplace_back( std::move(script) );
}


static std::vector<ExampleFilterEntry> g_exampleFilters;

static void AddOneExampleEntry( const QString& fileName, const char* script, bool isTemplate = true )
{
  auto expanded = ExpandMFGResource( script );
  AttrScanner ascanner( expanded );
  QString label = QString::fromStdString( ascanner.ScanTitleString() );

  g_exampleFilters.emplace_back( fileName, label, std::move(expanded), isTemplate );
}

static void SetupExamples()
{
  g_exampleFilters.clear();
  AddOneExampleEntry( "00_mosaic.mfg", g_mfg_script_mosaic ) ; // モザイク
  AddOneExampleEntry( "01_crystallize.mfg", g_mfg_script_crystallize ); // クリスタライズ
  AddOneExampleEntry( "02_magnifier.mfg", g_mfg_script_magnifier ); // 虫眼鏡
  AddOneExampleEntry( "03_bloom.mfg", g_mfg_script_bloom); // ブルームフィルタ
  AddOneExampleEntry( "04_grain.mfg", g_mfg_script_grain, false ); // Grainフィルタ
  AddOneExampleEntry( "05_zigzag.mfg", g_mfg_script_zigzag ); // ジグザグ
  AddOneExampleEntry( "06_mlaa_antialias.mfg", g_mfg_script_MLAA ); // MLAA
  AddOneExampleEntry( "07_nightvision.mfg", g_mfg_script_nightvision ); // 暗視スコープ
  AddOneExampleEntry( "08_perlin.mfg", g_mfg_script_perlin ); // パーリンノイズ
  AddOneExampleEntry( "09_mirror.mfg", g_mfg_script_mirror ); // 水鏡
  AddOneExampleEntry( "10_repeat.mfg", g_mfg_script_repeat ); // 繰り返し
  AddOneExampleEntry( "11_median_denoise.mfg", g_mfg_script_median_filter ); // メディアンフィルタ
  AddOneExampleEntry( "12_sphere.mfg", g_mfg_script_sphere, false ); // 球面射影
  AddOneExampleEntry( "13_rg_delta.mfg", g_mfg_script_rg_delta, false ); // RGずらし
  AddOneExampleEntry( "14_region_delta.mfg", g_mfg_script_region_delta, false ); // 領域ずらし
  AddOneExampleEntry( "15_motion_blur.mfg", g_mfg_script_motion_blur ); // 移動ぼかし
  AddOneExampleEntry( "16_lens_blur.mfg", g_mfg_script_lens_blur, false ); // レンズぼかし
  AddOneExampleEntry( "17_salt_and_pepper.mfg", g_mfg_script_salt_and_pepper ); // ごま塩ノイズ
  AddOneExampleEntry( "18_sandstorm.mfg", g_mfg_script_sand_storm, false ); // 砂嵐
  AddOneExampleEntry( "19_fisheye.mfg", g_mfg_script_fisheye, false );
  AddOneExampleEntry( "20_drag.mfg", g_mfg_script_drag, false );
  AddOneExampleEntry( "21_winding.mfg", g_mfg_script_winding, false );
  AddOneExampleEntry( "22_line_concent.mfg", g_mfg_script_line_concent, false );
  AddOneExampleEntry( "23_line_parallel.mfg", g_mfg_script_line_parallel, false );
  AddOneExampleEntry( "24_bezier.mfg", g_mfg_script_bezier, true );
  AddOneExampleEntry( "25_crt_filter.mfg", g_mfg_script_crt, false );
  AddOneExampleEntry( "26_copper_engrave_tone.mfg", g_mfg_script_copper_engrave, false );

  /*
  納得いってないので一時的にコメントアウト
  AddOneExampleEntry( "27_point_light.mfg", g_mfg_script_point_light, false );
  AddOneExampleEntry( "28_point_light_and_lens_flare.mfg", g_mfg_script_point_light_and_lens_flare, false );
  AddOneExampleEntry( "29_lens_flare.mfg", g_mfg_script_lens_flare, false );
  */
  /* 
  AddOneExampleEntry( 31, g_mfg_script_ ); // パース変換
  AddOneExampleEntry( 31, g_mfg_script_ ); // 半円中
  */

}

static bool g_example_setup_done = false;
static mfg_pal::NLanguage g_last_example_lang = mfg_pal::NL_ENGLISH;

static bool NeedSetup()
{
  if (!g_example_setup_done)
    return true;
  if (g_last_example_lang != Trans().Language())
    return true;
  return false;
}

static const std::vector<ExampleFilterEntry>& GetExampleFilters()
{
  if (NeedSetup())
  {
    SetupExamples();
    g_example_setup_done = true;
    g_last_example_lang = Trans().Language();
  }
  return g_exampleFilters;
}

static bool EnsureStaticFile( QDir& folder, const QString& fileName, const std::string& contents, bool forceGenerate )
{
  // すでにあって、
  if (folder.exists( fileName )) {
    if (forceGenerate)
    {
      // 作り直しなら削除
      folder.remove( fileName );
    }
    else
    {
      // 作り直しでなければすでにあるので正常終了。
      return true;
    }
  }
  
  QString path = folder.filePath( fileName );
  QFile qf( path );

  if (!qf.open( QIODevice::WriteOnly ))
    return false;

  return contents.size() == qf.write( contents.c_str(), contents.size() );
}

static QString SettingsPath()
{
  return AQ()->DataFolder() + "Project.ini";
}

static bool LangeChanged()
{
  QSettings s( SettingsPath(), QSettings::IniFormat );

  auto v = s.value( "proj/lastSaveLang" );
  if (v.isNull())
    return true;

  return v.toInt() != (int)Trans().Language();
}

static void SaveLastSaveLang()
{
  QSettings s( SettingsPath(), QSettings::IniFormat );
  s.setValue( "proj/lastSaveLang", (int)Trans().Language() );
}

bool EnsureExamplesProject( const QString& projDirPath )
{
  QDir projDir( projDirPath );
  if (!projDir.exists()) {
    if (!projDir.mkpath( projDirPath ))
      return false;
  }

  const auto& examples = GetExampleFilters();
  bool forceGenerate = LangeChanged();
  for( const auto& ent : examples )
  {
    if (!EnsureStaticFile( projDir, ent._fileName, ent._script, forceGenerate ))
      return false;
  }
  SaveLastSaveLang();
  return true;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
DialogNewProject::DialogNewProject( QWidget *parent, const QString& dir ) :
    QDialog(parent)
{
  // ？を表示しない
  qt_pal::RemoveQuestion( this );

  QVBoxLayout* lo = new QVBoxLayout();
  // lo->setContentsMargins( 4, 4, 4, 4 );

  QHBoxLayout* ln = new QHBoxLayout();
  QLabel* labelName = new QLabel( Str(3) );
  ln->addWidget( labelName, 1 );
  mNameEdit = new QLineEdit("NewProj");
  ln->addWidget( mNameEdit, 3 );
  ln->addStretch( 1 );

  QHBoxLayout* ld = new QHBoxLayout();
  QLabel* labelDir = new QLabel( Str(4) );
  ld->addWidget( labelDir, 1 );
  mDirEdit = new QLineEdit( dir );
  ld->addWidget( mDirEdit, 3 );
  mBrowse = new QPushButton( "..." );
  ld->addWidget( mBrowse , 1 );

  QHBoxLayout* lt = new QHBoxLayout();
  QLabel* labelTemplate = new QLabel( Str(28) );
  lt->addWidget( labelTemplate, 1 );

  mTemplateCombo = new QComboBox();
  AddTemplate( Str(29), ExpandMFGResource( g_mfg_script_blank ) ); // blank

  for( const auto& entry : GetExampleFilters() )
  {
    if (entry._isTemplate)
      AddTemplate( entry._label, entry._script );
  }

  lt->addWidget( mTemplateCombo, 3 );
  lt->addStretch( 1 );


  // OkCancel
  QHBoxLayout* lok = new QHBoxLayout();
  QDialogButtonBox* but = new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel );
  lok->addWidget( but );

  // Layout
  lo->addLayout( ln );
  lo->addLayout( ld );
  lo->addLayout( lt );
  lo->addLayout( lok );
  setLayout( lo );

  connect( but, SIGNAL(accepted()), this, SLOT(accept()) );
  connect( but, SIGNAL(rejected()), this, SLOT(reject()) );
  connect( mBrowse, SIGNAL(pressed()), this, SLOT(onBrowse()) );

  // Size (横だけ指定)
  setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Preferred );

  setWindowTitle( Str(5) );
}

///////////////////////////////////////////////////////////////////////////
QSize DialogNewProject::sizeHint() const
{
  return QSize( qt_pal::MagInt( 600 ), 1 );
}

///////////////////////////////////////////////////////////////////////////
QString DialogNewProject::Str( int idx )
{
  // 翻訳済みの文字列を取得
  return Trans_StrMFG( idx );
}


const std::string& DialogNewProject::getSelectedTemplate()
{
  return mTemplateContents[mTemplateCombo->currentIndex()];
}

///////////////////////////////////////////////////////////////////////////
void DialogNewProject::showEvent( QShowEvent* event )
{
  Q_UNUSED(event)

  if (!gPosInit)
  {
    qt_pal::InitDialogPos( this, MainAsQWidget() );
  }
  else
  {
    // 直近の値
    move( gPosX, gPosY );
  }

  /*
  Std()->mEditWidth->setFocus();
  Std()->mEditWidth->selectAll();
  Std()->updateControl();
  */
}

///////////////////////////////////////////////////////////////////////////
void DialogNewProject::hideEvent( QHideEvent* event )
{
  Q_UNUSED(event)

  gPosX = pos().x();
  gPosY = pos().y();
  gPosInit = true;
}

///////////////////////////////////////////////////////////////////////////
void DialogNewProject::onBrowse()
{
  QFileDialog fd( this );
  fd.setFileMode( QFileDialog::Directory );
  fd.setOption( QFileDialog::ShowDirsOnly, true );
  fd.setDirectory( getDir() );
  if (fd.exec())
  {
    QString fileName = fd.selectedFiles().at( 0 );
    mDirEdit->setText( fileName );
    // QMessageBox::warning( this, "", fileName, QMessageBox::Ok );  
  }

}
