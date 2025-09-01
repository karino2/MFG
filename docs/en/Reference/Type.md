# Type

- Prev: [Tensor](Tensor.md)
- [Top](README.md)

In MFG, variables and constants have types.
Basically, it's just a numeric type and its tuple.

Tensors and samplers are not type and treated differently in MFG.

It also has the function of treating special cases of tuples as vectors.
Here we will look at the story of such types.

There is also an Enum type, but at this point it can only be used in very limited locations, so I'll just touch it lightly at the end.

## Numerical type

There are three basic numerical types of MFG: unsigned integers, signed integers, and floating point numbers, each called u, i, and f.
The number of bits follows as suffix like i32, u8, f32, etc.

f only supports 32.

i supports 8, 16, 32.
But many of the hardware under MFG are treated them as 32-bit.
There is little benefit to using a small type.
Basically, we recommend using i32 for i.

We also recommend not using u32 very often and using i32 as integers as possible.

The exception is when using u8 and u16 as color BGRA components, in this case u8 and u16 will be used.

In short, the basic numerical types are as follows:

- f32
- i32
- u8 and u16 (as color components)

### Numerical literals

If you write an integer such as `123`, this is considered i32.
Any number of sizes is considered to be i32, and if you write 123 when requesting u8, it will cause an error due to a type mismatch.

Decimal literals such as `123.0` are f32.

There are also hexadecimal literals, starting with 0x. `0xff` etc. This is also an i32.

There is also a literal for u32, which has a trailing u like `123u`,
but it is not recommended to use it very often.
Try to calculate integers with i32 as much as possible.

### Numerical type cast

Cast numeric types with parentheses after the type name.
You can think of it as calling the same function as the type name.

For example:

```swift
let a = i32(12.0)
```

### Vectorization of cast operators

The cast operator is vectorized for the tuple.

````swift
let a = i32([12.0, 3u, 1])
````

a is an i32 tuple.
For vectorization, see [Expressions and Vectorization Operation](Expression.md).

### Integer as a logical value

There is no bool type, and the i32 is used instead.
0 is false, and non-0 is treated as true.

For example, `&&` and `!=` return i32.

## Tuple type

A tuple is a sequence of numbers.
Tuple elements can be of different types.

Tuples are created by separating elements with brackets with commas.

```swift
let tup = [1, 12.0, 3u]
```

All tuple elements are numeric types, and at this time we do not support nesting.

Access to tuple elements includes destructuring, swize operators, and index access.

### Destructuring

There is destructuring, which directly retrieves the contents of a tuple into a variable.

```swift
let tup = [1, 12.0, 3u]

# Destructuring
let [a, b, c] = tup
```

In the example above, we can create variables a, b, and c. The types are i32, f32, and u32, respectively.

You can use an underscore for elements that you do not use. If you want to use only the first and third elements, you can see the following:

```swift
let [a, _, c] = tup
```

### Swizzle Operator

Tuples support the swizzle operator.

- Supports xyzw combinations only
- Only rvalues ​​(mutation of tuples itself is not supported)
- It doesn't have to be a vector

```swift
let a = [1, 2, 3].xxyy
```

a is a tuple of `[1, 1, 2, 2]`.

If only one element is specified, it becomes a scalar.

### Access by index

I mainly use destructuring and swizzle, but you can also specify a numeric index to retrieve only one element.

You can retrive element with dot and numbers. 0 origin.

```swift
let tup = [1, 2, 3, 4, 5, 6]

# Access by numeric index, 0 origin, so a is 5
let a = tup.4
```

There is no way to make this number a variable.
This is because the type on the left side is undefined in that case.

It is theoretically possible to access element by variable if all the element have the same type in tuple (like vector),
so it may be supported in future versions.
But currently, we only support constant indexes.

### The type of the tuple must be determined statically

It's a bit more detailed, but in MFG, the type of tuple must be statically determined.
The number of elements and the type of each element must be determined at the time of parsing.
It does not support types that change dynamically or have a different number of content.

## Tuple and Vector

A tuple that meets special conditions is called a vector.

The special conditions are as follows:

- Dimensions from 2D to 4D (inclusive)
- All elements are the same numerical type

A tuple that meets these two conditions is called a vector.

In MFG, some functions only support scalars or vectors,
so it can be important whether a variable is a vector or not.

This is noted for each function item in [Builtin Functions](BultinFunctions.md).

MFG treats tuple as vector automatically if you only meet the conditions.
For example, normalize, a function that normalizes a vector, can be called as follows:

```swift
normalize([1.2, 3.2])
```

You can use results as tuples.

```swift
let [x0, y0] = normalize([1.2, 3.2])
```

It can also be used for arguments to other functions as follows:

```swift
let len = length(normalize([1.2, 3.2]))
```

However, since vector-compatible functions only support up to 4 dimensions, the following code will not work.

```swift
# NG! 5th dimension cannot be used as a vector
normalize([1.0, 2,0, 3.0, 4.0, 5.0])
```

### Vector type notation

The notation of vector types is like `f32v3` and `f32v4`.
The type of the component (in this case `f32`) is followed by a vector `v`, followed by a dimension number like 3 or 4. 

Another example is like `i32v2`.

### vec2, vec3, vec4 functions

There are functions called vec2, which creates two-dimensional vectors from scalars, vec3, which creates three-dimensional vectors, and vec4, which creates four-dimensional vectors.

```swift
let fvec = vec3(3.0)
let ivec = vec4(1)
```

If you want to create a vector with multiple elements specified, use the syntax sugar of the cast (explained next) instead of this.

### Tuple cast syntax sugar

Since vectors must be of the same type, it is often the case that casting is a quick way to create vectors.
Furthermore, since colors are often handled with u8 4D vectors,
in this case, you will also need to cast it to u8.

Since it is common to want to cast literals in tuples,
we offer syntactic sugar.

Specifically, you can omit the parenthesis after the type name and write square brackets directly.

For example, the following two codes have the same meaning:

```swift
u8([0, 0, 0xff, 0xff])
u8[0, 0, 0xff, 0xff]
```

Even though this syntax sugar was introduced to allow it to be read as if it were a literal of a vector in u8,
you can also use it with variables (non literals).

```swift
u8[b, g, r, a]
```

### splat Operator for Tuples

You can use what is called the splat operator `*` in square brackets and in the parenthesis of function calls.
This behaves as if you wrote down each element of tuple under splat operator.

For example:

```swift

let tup1 = [1, 2, 3]

# a1 becomes [1, 2, 3, 4]
let a1 = [*tup1, 4]
```

Since it appears to be arranging tup1 with expanded elements, a1 is a tuple of four elements.

It is often used when processing BGR only in BGRA, in combination with the swizzle operator.

```swift
let col = input_u8(x, y)
let bgr = col.xyz
let a = col.a

let bgr2 = bgr/128
u8[*bgr2, a]
```

## Enum type

Internally, there is also an Enum type.
Currently, Enum can only be used as arguments to some functions.

A common use is the sampler argument.

```swift
let finput = sampler<input_u8>(address=.ClampToEdge, coord=.NormalizedLinear)
```

`.ClampToEdge` and `.NormalizedLinear` are values ​​of type Enum.

In this example, the address argument can take an Enum type value called `SamplerAddressMode`, and the coord argument can take an Enum type value called `SamplerCoord`.
As there is no other way to specify these types other than type inference, currently Enum can only be used as function calls or sampler arguments.

A value of Enum type is an identifier that begins with `.`, and only the identifiers allowed by the Enum type found in the type inference of that argument can be included.

## Range type

There is also a range type.
However, this is also a special type that can only be used in special places.

The Range type is used as arguments to loop-based functions such as rsum.

```swift
let s = rsum(0..<5) |i| { i*2 }
```

For more information, see the "Range" section of [ifel and Loops](IfelLoop.md).

## Next

[Expression and Vectorize Operation](Expression.md)