# User Defined and Bultin Functions

- Prev: [Attributes and Input Widgets](AttrWidget.md)
- [To Top](README.md)

This page summarizes user-defined functions and standard bultin functions not covered in other sections.

## User-defined functions (from v1.0.03)

User-defined functions start with the keyword `fn` and then follows block with explicit type specification for formal parameters.

```
fn add2|x:i32, y:i32| {
  x+y
}
```

Unlike blocks such as tensors and rsums, type specifications are required for formal parameters.
There is no return type notation, and the type of the expression at the end of the block becomes the return type of this function.

Functions defined in this way can be used by calling them in parentheses, just like regular built-in functions.

```
def result_u8 |x, y| {
  let res = add2(x, y)
  u8[res, 0, 0, 0]
}
```

Only scalars and vectors are available. Tuples and tensors cannot be used.
Vectors are expressed like `f32v2`.

```
fn adder |v:f32v2| {
  v.x+v.y
}
```

The space after function name is optional.

## Bultin API notation

Below we provide documentation of built-in functions for MFG.

However, in MFG, the return type of a function is not explicitly stated in the source, so there is a shortage from a documentation perspective.
To do this, we first decide on a notation in this document to describe the parameters and return types.
This is only for documentation purpose.

Write the return type before the function name, and the parameter type with a colon after the parameter name.

For example:

```mfg
f32 sin |x:f32|
```

This means that there is a single parameter, and the type is f32 and the return is f32.

Separate multiple arguments with commas.

```mfg
f32 clamp|x:f32, minVal:f32, maxVal:f32|
```

### T, Ti representing a special vector

There are many functions in MFG that correspond to genType in OpenGL terms and T in Metal terms.
If you know these, following is the same explanatin of them.

In MFG, there are functions that support either vectors or scalars.
As explained in [Type](Type.md), vectors are tuples of the same type and have 2-4 elements.
The reason for the constraint of up to four elements is the constraints of the underling environment (Metal, OpenGL, DirectX) constraints.

For example, length is a typical function that supports vectors.

```mfg
length([1.0, 2.0, 3.0])
```

length supports only 2 to 4 dim vector.
Also, all elements must be f32. If there is an i32 element, it will be a compilation error.

Thus, there are several functions that support only the vectors of f32 of the tuple.
Vectors can be 2D, 3D, or 4D, so they cannot be written normally, such as f32v2 or f32v3.
It's easier to create a special notation.

Furthermore, in some functions, although the dimensions of the vector can be from 2 to 4, the dimensions of the two arguments must be aligned.

A distance, which determines the distance between two vectors, is an example.

```mfg

# OK
distance([1.0, 1.0], [2.0, 2.0])
distance([1.0, 1.0, 1.0], [2.0, 2.0, 2.0])

# NG
distance([1.0, 1.0], [2.0, 2.0, 2.0])
```

In this way, the dimensions can be 2D, 3D, or 4D, but the dimensions of the two arguments must be aligned.
A function like this is often found.

Most of these types of functions have several vector arguments and return types, all of which must be in the same dimension.

To describe the confusing circumstances like this,
write the vector of f32 in the notation T, and the same T represents a vector of the same dimension.

```mfg
f32 distance|x:T, y:T|
```

In this case, T is a tuple of two to four elements of f32, and x and y are of the same dimension.

Also, similar to T, but with an element i32 is referred to as Ti.
This can be seen through logical operations such as all or any.

Some functions support only vectors, and others support both vectors and scalars.
This is not distinguished by notation, and if it is possible to use scalars, we will note as such.

### Functions that can be used with Vectorization

A similar but different to the case that supports vectors is the function Vectorization.
Vectorization is basically a function that can be used with a function that takes an argument of one element f32, which can be used for tuples of unlimited number of elements, not just four elements.
Anyone who knows about vectorization in the R language should think of it as the same thing.

For example, sin is the following type:

```mfg
f32 sin |x:f32|
```

This can be used with Vectorize.

Vectorizing is a feature that acts as if you called a function on the individual elements of a tuple when you pass tuple as an argument.

```mfg
let res = sin([1.0, 2.0, 3.0, 4.0, 5.0])
```

This is a syntax sugar, and internally translates as follows:

```mfg
let res = [sin(1.0) sin(2.0), sin(3.0), sin(4.0), sin(5.0)]
```

In this case, the API is expressed as a single argument, followed by an additional sentence **vectorization possible**.

## Color conversion and supported colors

MFG supports conversions between colors as it requires gamma correction or CIE XYZ colors depending on the required calculation.

### Color short notation

Each color has an short notation.

| Color Name | Abbreviation | Description |
| ---- | ---- |
|u8 color, u16 color| u8color, u16color | This is a color that holds each BGRA in a 4-dimensional vector of integers such as u8 or u16, each element is not linearized. input_u8 is the most used example. BGRA is treated as the 4D vector of u8. For linearization, see Gamma correction. |
| Normalized colors | ncolor | Normalized color or ncolor is what u8 colors and u16 colors are simply scaled to the range of 0.0 to 1.0. This is also not linearized. |
| Linearized BGR, BGRA | lbgr, lbgra | Linearized ncolor is called linealized bgr, and is abbreviated as lbgr. lbgra is the one an alpha added to it. But since alpha is linear from the beginning, it retains the same value. |
| CIE XYZ, and its alpha | xyz, xyza | lbgr is converted to CIE XYZ colors, and is abbreviated as xyz. Please be careful as it overlaps with the swizzle xyz. xyz is always linearized. |


### Conversion between colors

Basically, you convert it with the function name `[original color name]_to_[result color name]`. For example, `lbgr_to_xyz`.

Except for u8color, it is basically f32v3 if there is no alpha, and f32v4 if there is alpha.
`lbgr_to_xyz` etc. returns f32v3 with f32v3 as the argument.

```mfg
f32v3 lbgr_to_xyz |col:f32v3|
u8v4 xyza_to_u8color |col:f32v4|
```

Also, it is omitted if the original color name is u8color.

```mfg
f32v4 to_xyza |col:u8v4|
f32v4 to_lbgra |col:u8v4|
```

### to_ncolor

```mfg
f32v4 to_ncolor |col:u8v4|
```

Converts the u8v4 BGRA color to normalized colors.
f32 ranges from 0.0 to 1.0

**Example**

```mfg
let ncolor = to_ncolor(input_u8(x, y))
```

### to_u8color

```mfg
u8v4 to_u8color| col:f32v4 |
```

Converts from ncolor to u8v4 BGRA.
The col element must be between 0.0 and 1.0.

Since to_u8color is clamped from 0.0 to 1.0 to u8, values ​​greater than 1.0 are treated as 1.0, and negative values ​​are treated as 0.0.

**Example**

```mfg
let u8_bgra = to_u8color(ncolor)
```

### Linearized BGRA related

The gamma-corrected linearized BGRA-related functions are: (Implemented in v1.0.01)

```mfg
f32v4 to_lbgra|col:u8v4|
u8v4 lbgra_to_u8color|col:f32v4|
```

**Example**

```mfg
def result_u8 |x, y| {
  let lcol = to_lbgra(input_u8(x, y))
  lbgra_to_u8color(lcol)
}
```

### CIE XYZ color related

There are the following functions: (Implemented in v1.0.01)

```mfg
f32v4 to_xyza| col:u8v4 |
u8v4 xyza_to_u8color| col:f32v4 |

f32v3 lbgr_to_xyz| col:f32v3 |
f32v3 xyz_to_lbgr| col:f32v3 |
```


**Example**

```mfg
def result_u8 |x, y| {
  let xcol = to_xyza(input_u8(x, y))
  xyza_to_u8color(xcol)
}
```

### Gamma correction

When you want to process gamma correction, it is more common to just call to_lbgra or to_xyza to obtain linearized colors.
But if you want to do more detailed processing, we also provide a function that performs only linearization.

The following two functions are the basic functions for gamma correction:

- `T gamma2linear| ncolor:T |`
- `T linear2gamma| ncolor:T |`

T is a vector or scalar of f32.

It moves up to four elements of tuples, but gamma correction is done for RGB and alpha is usually linear.
Usually it is used as a tuple of three elements.

The argument is a normalized color (colors from 0.0 to 1.0 of float).

Since normal image files have gamma correction values, you can return them to linear scale and proceed with calculations.
After that, the usual process is to do gamma correction again.

Typically the code would look like this:

```
let bgra = to_ncolor(input_u8(x, y))
let lin_bgr = gamma2linear(bgra.xyz)
let alpha = bgra.w
# Something to do

to_u8color([*linear2gamma(lin_bgr), alpha])
```

Usually, it's easier to get lbgr directly by writing the following.

```
let lbgra = to_lbgra(input_u8(x, y))
let lin_bgr = lbgra.xyz
let alpha = lbgra.w
# Something to do

lbgra_to_u8color(lbgra)
```


There is also a function with a trailing `A` that passes alpha as it is and applies only to other elements.

- `f32v4 gamma2linearA| ncolor:f32v4 |` (since v1.0.01)
- `f32v4 linear2gammaA| ncolor:f32v4 |` (since v1.0.01)

In this case, it is fixed to 4D. The following two have the same meaning:

```mfg
let lcol1 = gamma2linearA(ncol)
let lcol2 = [*gamma2linear(ncol.xyz), ncol.w]
```

### gamma2linear

```mfg
# T may be scalar

T gamma2linear| ncolor:T |
f32v4 gamma2linearA| ncolor:f32v4 | # since v1.0.01
```

The argument is a gamma corrected component of a normalized color. Since the calculation is performed for each component, it does not need to be in the order of BGR. For example, you can use this function with all component red.
input_u8 is the gamma corrected value.

### linear2gamma

```mfg
# T may be scalar

T linear2gamma| ncolor:T |
f32v4 linear2gammaA| ncolor:f32v4 | # since v1.0.01
```

Converts a linearized normalized-color into a gamma-corrected normalized-color.
Before returning to result_u8, the value must be gamma corrected.


## rand

```mfg
f32 rand| |
```

Returns random numbers between 0.0 and 1.0.

## Trigonometric functions and log

You can generally use vectorization if you have one argument.
atan2 has two arguments, so vectorization is not possible, only T is supported.

| API | Can vectorize? | Explanation |
| ---- | ---- | ---- |
| `f32 exp\| x:f32 \|` | yes | exponential power of e |
| `f32 exp2\| x:f32 \|` | yes | exponential power of 2 |
| `f32 sin\| x:f32 \|` | yes | Angle is radian |
| `f32 cos\| x:f32 \|` | yes | |
| `f32 tan\| x:f32 \|` | yes | |
| `f32 log\| x:f32 \|` | yes | Calculate the logarithm of base e |
| `f32 log2\| x:f32 \|` | yes | Calculate logarithm with base 2 |
| `f32 sqrt\| x:f32 \|` | yes | |
| `T atan2\| y:T, x:T \|` | no | Calculate arctangent for y/x |


## Calculating fractions and signs of decimals

| API |Can vectorize? | Explanation |
| ---- | ---- | ---- |
| `f32 abs\|x: f32\|` | yes | i32 is also possible |
| `f32 ceil\|x: f32\|` | yes | round up. Returns the smallest integer greater than or equal to x. |
| `f32 floor\|x: f32\|` | yes | Returns the largest integer less than x |
| `f32 fract\|x: f32\|` | yes | Returns the fractional part of x |
| `f32 round\|x: f32\|` | yes | Returns an integer rounded to x |
| `f32 saturate\|x: f32\|` | yes | Returns a clamp of x between 0.0 and 1.0 |
| `f32 sign\|x: f32\|` | yes | Returns 1.0 if x is positive, -1.0 if x is negative |
| `f32 trunc\|x: f32\|` | yes | Returns an integer truncated to the fraction of x |
| `i32 isnan\|x:f32\|` | yes | Returns 1 if x is nan, otherwise 0 |
| `i32 isinf\|x:f32\|` | yes | Returns 1 if x is inf, 0 if not |

## Functions which take a vector as an argument

A function that takes a vector as an argument. All cannot be vectorized.

| API | Explanation |
| ---- | ---- |
| `i32 all\|x: Ti\|` | Returns 1 if all elements of x are non-zero, otherwise 0 |
| `i32 any\|x: Ti\|` | Returns 1 if there is at lease one non-zero element of x, 0 if all zeros |
| `f32 distance\|x:T, y:T\|` | Find the distance between two vectors, x and y |
| `f32 dot\|x:T, y:T\|` | Find the inner product of x and y |
| `f32v3 cross\|x:f32v3, y:f32v3\|` | Find the cross product of x and y |
| `f32 length\|x: T\|` | Find the length of vector x |
| `T normalize\|x: T\|` | Returns the vector in the same orientation, normallyizing vector x to length 1 (same as `x/length(x)`) |

## Functions that can be used with both scalars and vectors

A function that takes an argument of scalar f32 or a vector of f32. In other words, all of the following Ts are also possible with f32.
All functions cannot be used for vectorization.

| API | Explanation |
| ---- | ---- |
| `T mix\|x:T, y:T, a:T\|` | Returns linear interpolation of x and y, `x+(y-x)*a`. a must be in the range from 0.0 to 1.0.|
| `T step\|edge:T, x:T\|` | Returns 0.0 if x is smaller than edge, otherwise return 1.0 |
| `T smoothstep\|edge0:T, edge1:T, x:T\|` | If x is smaller than edge0, 0.0 is returned, and if edge1 or more, 1.0 is returned. It performs smooth interpolation (see below) between edge0 and edge1.|

smoothstep is a common function in shaders.
Completion of smoothstep is done using the following formula:

```mfg
let t = clamp((x – edge0)/(edge1 –edge0), 0.0, 1.0)
t * t * (3.0 – 2.0 * t);
```


## Functions which return vector vec3 and vec4

There is a vec3, which passes a scalar and returns a vector with that same value in three elements.
There is also a vec4, which returns a vector with four elements of the same value.

Neither can be vectorized.

- `i32v3 vec3|x:i32|` or `f32v3 vec3|x:f32|`
- `i32v4 vec4|x:i32|` or `f32v4 vec4|x:f32|`

**Example**

```mfg
let fvec = vec3(3.0)
let ivec = vec4(1)
```

**Detaile**

Returns the vector of f32 if the argument is f32, and the vector of i32 if the argument is i32.

Though it is common for shader languages ​​to support multiple elements too,
this is not supported in MFG.

In MFG, just tuples mean the same thing, so please use that.

```mfg
# let fvec = vec3(1.0, 2.0, 3.0) This is not supported, use following.
let fvec = [1.0, 2.0, 3.0]
```

## Functions which have exceptional relationships between scalar, vector, i32 and f32

The following functions have different argument constraints than any of the functions mentioned above.

- clamp
- min
- max

### clamp

Clamp can be used in scalars and vectors, and can also be used for both i32 and f32.

Strictly speaking, we can write as following four definitions:

```mfg
f32 clamp|x:f32, minVal:f32, maxVal:f32|
i32 clamp|x:i32, minVal:i32, maxVal:i32|
T clamp|x:T, minVal:T, maxVal:T|
Ti clamp|x:Ti, minVal:Ti, maxVal:Ti|
```

clamp returns minVal if x is less than minVal, maxVal if it is greater than maxVal, and otherwise returns x.

### min and max are variable length arguments

`min` and `max` are variable length arguments, and each element must be the same type.

The variable length argument means that:

```mfg
min(1.5, 3.2, 2.0)
max(3, 2, 5, 7)
```

At first glance, this may look like a vectorization, but it differ in which the arugments are not a tuple.

```mfg
#Vectrise. There is one argument, but the number of tuple elements is arbitrary
sin([1.0, 2.0, 3.0, 4.0])

# min has four arguments
min(1.5, 3.2, 2.0, 4.0)
```

It also supports vectors for each argument.

So you can do the following:

```mfg
# The result is [3, 3, 3]
max([1, 2, 3], [3, 2, 1], [1, 3, 2])
```

This is a tuple of the results of max for each element.

From the way I wrote it so far,

```mfg
T max|x1:T, x2:T, ...|
T min|x1:T, x2:T, ...|
```

Furthermore, T can also use Ti, and f32 and i32.


## Other functions

Here are some functions that have nowhere else to be placed.

| API | Explanation |
| ---- | ---- |
| `Ti xor\|x:Ti, y:Ti\|` | Bitwise exclusive OR of x and y(xor) |

Ti of xor also supports u32.
In normal shaders, it corresponds to `^`,
but in MFG, this is used as a power, so only xor is a function for bitwise operations.

