# Tensor

[Top](README.md)

A tensor is a one- or two-dimensional, tuple or scalar array.

MFG programs are performed by arranging the definitions of multiple tensors.
The defined tensor can be used elsewhere.

Tensors are unique to MFG, providing functions similar to functions in other languages,
It is deeply connected to the kernel and global memory, an important element of GPU programs.

## Example of tensor

Below is an example of a typical tensor:

**Result_u8 Example**

```mfg
def result_u8 |x, y|{
  u8[0, 0, 0xff, 0xff]
}
```

**Example of intermediate tensor**

```mfg
@bounds(640, 480)
def red_tensor |x, y| {
  u8[0, 0, 0xff, 0xff]
}
```

### Tensor definitions and references

Tensors usually have two aspects: definition and reference (the exception is input and result, only one side).

A definition describes the rules that create tensors, and the MFG actually constructs the tensor data based on those rules.
The definition of a tensor is, for example, as follows:

```mfg
@bounds(640, 480)
def red_tensor |x, y| {
  u8[0, 0, 0xff, 0xff]
}
```

For details on the definition of tensors, please refer to "Tensor Definitions" section below.

You can refer to the tensor you defined.
A reference is accessing elements contained in the target tensor.

Tensor references are made in brackets.

```mfg
  let v = red_tensor(10, 20)
```

The shape of a tensor is determined by two aspects: dimension and element type.

### References outside tensor range

For references to outside of ranges, the return value is undefined, but the reference itself is legal.
Internally, indexes that are out of range are forcibly changed to within range.

In GPU programs, it is often difficult to avoid referencing outside of bounds, but that referenced value is not used as a final result.
To allow this situation, we have clearly allowed references to the specifications.
The value which obtained by out of bounds indexing may change in future implementations, so avoid code that depends on the value when accessing out of range.

### Tensor Reference and Splat Operator

You can use the splat operator `*` to reference elements of tensors.
The following v1 and v2 have the same meaning.

```mfg
  let tup = [10, 20]

  let v1 = red_tensor(10, 20)
  let v2 = red_tensor(*tup)
```

For information on the splat operator, see [Expression and Vectorize Operation](Expression.md).

## Types of tensors

There are the following major types of tensors:

- input tensor
- result tensor
- Tensor literal
- Normal tensor (intermediate tensor)
- Local tensor

## input tensor

The input tensor is a tensor that holds the pixel values ​​of the layer to be filtered into.
This tensor has a special property that it is not defined by the user and is merely referred to.

There are two types of input tensors: input_u8 and input_u16.
u8 and u16 will be covered in the following section, "Regarding input and results of u8 and u16",
In short, the u8 handles BGRA each as 8bit, while the u16 handles BGRA each as 16bit.

### input_u8 tensor

input_u8 is a tensor that holds BGRA as an element with the 4-dimensional vector of u8.
Because the 4D vector of u8 and the 4D vector of u16 are often used in MFG, they are named u8v4 and u16v4, respectively.

The dimension of input_u8 is always two dimensions.
It corresponds to x and y of the input layer.
The top left is 0 and the positive coordinate system towards the bottom right.

**example**

```mfg
let [b, g, r, a] = input_u8(32, 24)
```

### input_u16 tensor

input_u16 is a tensor that holds BGRA as a 4-dimensional vector of u16.
Other than that, it is the same as input_u8.

"Regarding input and result of u8 and u16" will cover the meaning of u8 and u16.

### References to other layers

In MFG, in addition to the target layer, you can refer to the above and below layers relatively.
Other layers are specified in square brackets.

```mfg
# One layer below
input_u8[-1](x, y)

# One layer above
input_u8[1](x, y)
```

The minus represents the lower layer, and the plus represents the upper layer. input_u16 has a similar mechanism.

Any layers that do not exist are considered to contain zeros. There is no way to distinguish whether a layer is not present or the layer contains value 0 at this time.

## result tensor

There are two types of result tensors: result_u8 and result_u16.
An MFG program must contain only one of these at a time.
Defining both is an error.

The result tensor is also a special tensor in that there is only one in the program.
Also, there is no specification for width or height, and it is always the same size as the input tensor.
And only the definition exists and cannot be referenced.

The tensor elements are kept as u8v4 in result_u8 and as u8v16 in result_u16.

The result of this tensor will be the layer of the result of applying the MFG filter.

### result_u8 tensor

This is a tensor that holds the results in 8 bits each of BGRA.
The element type must be u8v4.

The following is an example of result_u8 that turns everything red.

```mfg
def result_u8 |x, y|{
  u8[0, 0, 0xff, 0xff]
}
```

### result_u16 tensor

A result tensor that holds the elements in u16v4.

## Regarding input and result of u8 and u16

The tensor of `input` and `result` are either `_u8` or `_u16` at the end.

This differs between holding BGRA color elements with u8 (unsigned 8bit) or u16 (unsigned 16bit) respectively.
(For u8 and u16, see [Type](Type.md)).

MFG is a language that apply filter to layer.
The target layer can be divided into 32bpp, 64bpp, and others.

At this time, we only target 32bpp and 64bpp, and do not support other layer types.
In the future, we will also support 8bpp grayscale target.

If you specify u8 for the 32bpp layer and u16 for the 64bpp layer,
Each is passed as is without being converted.

If you specify u16 for the 32bpp layer or u8 for the 64bpp layer,
The colors are automatically converted internally and passed.
If you write it as either a u8 or a u16, it will work with either a 32bpp layer or a 64bpp layer.

Generally, it is better to implement it with u16 to have a beautiful color filter. However, the GPU memory usage will be doubled.
This is the same as using the 32bpp and 64bpp layers.

## Tensor Literal

We often want to define tensors of small constants.
For example, the weight of a convolutional filter.

Such things can be defined using the tensor literal notation.

Below is an example of a tensor literal:

```mfg
def tensorName by [[1, 2, 3],
               [4, 5, 6],
               [7, 8, 9]]
```

Expressed in nested notation of `by` keyword and tuples.

A tensor literal must have all elements the same.
As an element, it currently only supports numbers, and tuples are not supported.

## Regular tensor

A regular tensor is a tensor that is not the special tensor mentioned above.
It will be used as an intermediate tensor.

A regular tensor must be sized at the definition with `@bounds`.

```mfg
@bounds(640, 480)
def red_tensor |x, y| {
  u8[0, 0, 0xff, 0xff]
}
```

## Local tensor

Local tensors are special tensors that are not linked to global memory and are generated by one thread.
Also, at present, local tensor is the only one that can update the value as a side effect in MFG.

### Example of a local tensor: Histogram

A typical example of use local tensor is to find a histogram.
In following code, `_hist` is the local tensor:

```mfg
def weight by [[1, 2, 1],
               [2, 3, 2],
               [1, 2, 1]]

@bounds(input_u8.extent(0)-2, input_u8.extent(1)-2)
def median |x, y| {

  # Defining a local tensor
  @bounds(256, 4)
  def _hist |i, col| { 0 }

  # Updated with local tensor side effects
  weight.for_each |ix, iy, wval| {
     let [b, g, r, a] = input_u8(ix+x, iy+y)
     mut! _hist(b, 0) += wval
     mut! _hist(g, 1) += wval
     mut! _hist(r, 2) += wval
     mut! _hist(a, 3) += wval
  }
  # Do something using _hist below
}
```

Below we'll look at the details of this code.

### Defining a local tensor

The definition is almost the same as regular tensor, as follows:

```mfg
  @bounds(256, 4)
  def _hist |i, col| { 0 }
```

The only difference is where they are defined.
They are defined inside other tensor definition.

Also, this `bounds` must be a constant.
In global tensor, variables can be used, but in local tensor, you can't use variables here.
This is a strong constraint due to the fact that local tensors must have a fixed size at the time of compilation.

The runtime behavior is also different.
The initialization by this code is performed on a single thread.
In addition, internally it becomes a register rather than a global memory (similar to the local array of shaders).

Local tensor can also be defined using tensor reduce, which will be described later.

### Updates local tensor with side effects

MFG is designed to avoid any side effects as much as possible.
The only exception at the moment is the local tensor.

Side effects always start with `mut!` to make them look syntax-specific.

In the previous example, the following `+=` is an update due to side effects:

```mfg
  mut! _hist(b, 0) += wval
```

This means updating the value of `_hist(b, 0)` with `_hist(b, 0)+wval`.
Since this is something that can be done normally in other languages, some people may be surprised that it can only be done in this limited way.

MFG only provide very limited set of operations for `mut!`.

Currently, there are only two major types of `mut!`:

- `+=`
- `trans` family
  - cumsum!
  - sort!

The `trans` family will be explained separately at [trans and reduce](TransReduce.md).

### Use local tensors as minimum as possible

Local tensors behave like normal programming language arrays, but this also has many drawbacks on the GPU.

First, since local tensors are expanded into registers, there is a very strong limit on the sizes that can be used compared to global tensors.
It also increases device dependency.
This increases the risk that a filter that runs on a particular device will not work on another device.

Furthermore, even if it moves, parallel execution performance is greatly reduced, resulting in a significant slowdown in the generation of the parent kernel.
This also causes a greater device dependency, which has a negative impact on filter portability.

There are cases where local tensors are absolutely necessary, such as histograms, but usage should be kept to a minimum.

## Definition of tensors

There are two main ways to define a (normal) tensor.

- Normal tensor definition
- Define using tensor reduce system function from other tensors (local tensors only)

Tensors are defined using `def` keyword.

### Tensor Definition: The Standard Approach

A standard tensor definition specifies the width and height with `@bouds` and returns the elements at each position in a block.

```mfg
@bounds(640, 480)
def red_tensor |x, y| {
  u8[0, 0, 0xff, 0xff]
}
```

After `def`, the name of the tensor follows. Next, arguments representing each position.

In this case, x will have a value between 0 and 639 and y will have a value between 0 and 479.
The dimension of a tensor is determined by the number of i32 passed to `@bouds`.
If this number does not match with the number of `|x, y|`, a parsing error will occur.

### Tensor Definition: From other tensors using tensor reduce

There is a concept called "tensor reduce" that can be used when creating a new tensor based on other tensors.
This is a feature that can only be used with local tensors.

The syntax is as follows:

```mfg
def TENSOR_NAME by reduce<SOURCE_TENSOR>.METHOD_NAME(ARGUMENTS) BLOCK
```

The argument part can be a "named argument" and the content of the argument is determined by the "METHOD_NAME".

Examples include the following:

```mfg
  def med by reduce<hist>.accumulate(dim=0, init=-1) |i, col, val, accm| {
    ifel(accm != -1, accm, ...)
    elif(val < _hist(255, col)/2, -1, i)
  }
```

This is a feature for advanced users, so it will be explained separately in detail in [trans and reduce](TransReduce.md).

### Tensor Value

You can use a number and its tuple as the tensor value.
A tuple does not need to be a vector, and for example, it can be returned as a mixture of i32 and f32.

```mfg
@bounds(640, 480)
def red_tensor |x, y| {
  # Returns i32 and f32 tuples
  [3, 1.2]
}
```

Tuple nesting is not permitted.

Internally, tuples are separate arrays of global memory.

### Optimizing tensors for u8v4 and u16v4

In general, in MFG, the u8 is not always 8bit, and 32bit is often used internally.
However, we guarantee that the u8v4 tensor is optimized to be 32 bit sized.

This is not guaranteed for u8v3 or u8v2, so u8v4 may be more efficient than u8v3 or u8v2.

In addition, the same optimization has been carried out for the u16v4, guaranteeing that only 64bits will be used.

## Tensor-related methods

Though the main way to use tensors is to refer to the elements inside,
there are also some methods for tensor as well.

Tensor has the following methods:

- extent
- is_inside
- to_ncoord
- sum
- for_each

Methods are used like `input_u8.extent(0)`, etc.

In general, tensor method is called in following manner:

```
TENSOR_NAME.METHOD_NAME(ARGUMENTS)
```

(But `to_ncoord` method is the exception, see below).

sum and for_each methods provide loop-based functionality that takes block arguments.

### ts.extent(dim)

Gets the width and height of the tensor. Specify the dimension as an argument.

```mfg
let w = input_u8.extent(0)
let h = input_u8.extent(1)
```

w and h are the "maximum index +1" of input_u8 (since it is 0 origin).

If no argument is specified, all values ​​will be returned as a vector.

```mfg
let [w, h] = input_u8.extent()
```

### ts.is_inside(x, y)

Returns non-zero if x and y are within the range of ts, and 0 if out of the range.

The same as below.

```mfg
 x < ts.extent(0) && y < ts.extent(1)
```

For one dimension tensor, there is one argument.

### to_ncoord([x, y])

Used within tensor definitions, returns coordinates normalized between 0.0 and 1.0. The argument is a 2D tuple.

```mfg
def result_u8 |x, y| {
   let [fx, fy] = to_ncoord([x, y])
   ...
}
```

Target tensor is implicitly specified as defining tensor and cannot be specified other one.

If the integer coordinates of the argument are greater than or equal to extent, the value greater than 1.0 is returned.

### ts.sum BLOCK

Runs a block on all elements of ts and returns the sum of all the results.

For example, to calculate the sum of all elements of 2D tensor `weight` as variable wsum:

```
let wsum = weight.sum |_, _, val| { val }
```

The block arguments are the index (coordinates) of the tensor and the value of the element at that index.

### ts.for_each BLOCK

`ts.for_each` is the only loop type function of MFG that does not return a value.
Since it does not return a value, this is always used in conjunction with features that have side effects, specifically `+=` for `mut!`.

`ts.for_each` executes a block for each element of ts.

```mfg
  weight.for_each |ix, iy, wval| {
     let [b, g, r, a] = input_u8(ix+x, iy+y)
     mut! _hist(b, 0) += wval
     mut! _hist(g, 1) += wval
     mut! _hist(r, 2) += wval
     mut! _hist(a, 3) += wval
  }
```

The formal arguments for block arguments are the values ​​of the index and the elements of that index, just like ts.sum.

## Supported Tensor Dimensions

At the moment, tensors only support 1D and 2D.
3D may be supported in the future, but it is not currently supported.

## Global Block

Outside the definition of a global tensor is an area called a global block.
You can also perform calculations here.

```mfg

# This is the global block
let a = 3*2

# Below this is a tensor definition, so it is not a global block.
def result_u8 |x, y| {
  u8[y, x, 0, a]
}
```

In global blocks, you can write information about UI input widgets using attributes, and
`@print_expr` allows you to output expressions for debugging purposes.

For more information, see [Attributes and Input Widgets](AttrWidget.md).

## Next

[Type](Type.md)