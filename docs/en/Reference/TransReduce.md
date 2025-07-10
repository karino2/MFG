# trans and reduce

- Prev: [Sampler](Sampler.md)
- [Top](README.md)

Local tensors can be used to create another new local tensors, or to transform existing tensors itself.
There are tensor `trans` and tensor `reduce`.

`trnas` and `reduce` are advanced features, so they are unnecessary in the early days,
but if you use them well, you will be able to realize algorithms that are slightly unsuitable for GPUs.

In this chapter, we will look at the `trans`, `reduce`, and side effects of local tensors.

Tensor reduce is a features different from function reduce, so strictly speaking, we must call "tensor reduce".
But because this chapter deals with tensor reduce only,
it's not ambiguous in this chapter. So we simply call them `reduce`.

## Keep Local Tensors Usage As Minimum As Possible!

As mentioned in the Local Tensor section of [Tensor](Tensor.md),
local tensors should be avoided as much as possible.
But there ARE also several algorithms needed, such as histogram of local regions.

MFG concentrate side effects related features only to local tensors,
there by all other parts are pure and clean expression worlds.
It loalize bugs caused by side effects.

Conversely, you need to use a local tensor to do things that require side effects.

Even though local tensors can be updated by side effects,
local tensors are also useful without side effects.
Local tensors also have features called `reduce`,
which creates a new tensor from existing tensors without using side effects.

Also, there are another features called `trans`, which use side effects update, but with relatively limited and safer way, by transforming existing tensors in place.

Currently, the only side effect that is neither `trans` nor `reduce` is `+=`.

This `trans` and `reduce` is unique to MFG and is not found in other languages, and is an important element, so we will take a closer look here.

## Tensor Reduce

Tensor reduce is the process of creating a new tensor from an existing tensor.

`reduce` creates a new tensor that somehow reduces the dimensions of the source tensor.
For example, let's say we calculate the cumulative sum of histograms for each RGB within a certain range and hold it in a tensor called histCumSum.
histCumSum is the i32 tensor of (3,256).

You can find a median from histCumSum by following code:

```
	def median by reduce<histCumSum>.accumulate(dim=0, init=-1) |i, rgb, val, accm| {
    ifel(accm != -1, accm, ...)
    elif(val < histCumSum(255, rgb)/2, -1, i)
  }
```

`median` reduces the dimensions of 256 of histCumSum to form the 1-dimensional tensor of 3 elements.

### Syntax of `reduce`

reduce have four syntax part:

1. Specifying tensors
2. Specifying a method
3. Named arguments according to the method
4. Block arguments

The syntax is as follows:

```swift
reduce<TENSOR_NAME>.METHOD_NAME(...) |...| {...}
```

It's MFG norm to use the angle brackets `<>` to specify tensor by TENSOR_NAME.
Currently, there are two METHOD_NAME supported, `accumulate` and `find_first_index`, which we'll look at them next.

The allowed combinations of named arguments depends on METHOD_NAME, but all method of reduce have `dim` argument in common,
and MFG run the block along this dimension's axis.

It's quite complicated, but
the tensor specification and method specification are the same in `trans`,
once you understand either one, you will be able to understand both.

### Reduce Method 1: find_first_index

Run the block along the axis of `dim` and return the first index which block returned non-zero.
If it cannot be found, it's -1.

`find_first_index` can be used as follows:

```swift
  # The tensor literal for local tensors is NYI, so the usual way to make it
  # [[1, 2, 1],
  #  [2, 3, 2],
  #  [1, 2, 1]]
  @bounds(3, 3)
  def weight |i, j| { 3-abs(i-1)-abs(j-1) }

	def upper by reduce<weight>.find_first_index(dim=0) |i, j, val| {
    val <= i+j
  }
```

Tensor `upper` runs along the dim=0, that is, the x-axis, and replaces this axis with some kind of one value.
In other words, while the source tensor is `(3, 3)`, the result turn it into a `(_, 3)`, which is a 3-element tensor just on the y-axis.

`reduce` always ends up replacing the dimension of `dim` with some value, which generates a new tensor.
In the case of find_first_index, the block is executed along the x-axis, and the index that first returns a non-zero value will be the replaced value.
The resulting tensor is always i32, as the result is index.

In the example above, on the line j=0, execute 1, 2, 1 in order, which means:

| i+j | val |
| ---- | ---- |
| 0 | 1 |
| 1 | 2 |
| 2 | 1 |

So the first index which becomes `i+j <= val` is 2, as well as the next one is 1 and the end is 0.
In other words, it becomes a one-dimensional tensor called `[2, 1, 0]`.

**Arguments**

The only arguments are `dim` and block arguments.

- dim: Specify which dimension to reduce in i32

**Block**

The formal parameters of block are the index and the value of the original tensor at that index.

The result of block must be integer value.
find_first_index find the first index that makes this block result non-zero.

### How to Think of Reduce

Reduce is something that aggregates `dim` in some way.

Regarding the NxM tensor, if dim=0, it becomes the M tensor, and if dim=1, it becomes the N tensor.

If we think of dim=0, we look at the column in the N direction for each M and make it some kind of one value.
For the `find_first_index` case, this one value is the first index in the N direction which the result of block returns non-zero value.

If you understand the meanings of "aggregate along one axis", you will understand the tensor `reduce`.

### Reduce Method 2: accumulate

`accumulate` is a more functional reduce. Internally, `find_first_index` is running as `accumulate`.

Following is the example of `accumulate`:

```swift
	def median by reduce<histCumSum>.accumulate(dim=0, init=-1) |i, rgb, val, accm| {
    ifel(accm != -1, accm, ...)
    elif(val < histCumSum(255, rgb)/2, -1, i)
  }
```

`accumulate` has two arguments followed by a block argument.

**Arguments**

- dim: Specify the axis to reduce
- init: Specify the value of the first accm

**Block**

The formal parameters of block are

- The index of the original tensor (in 2D, x, y)
- Value at the corresponding position of the source tensor
- Value of previous block execution result

If the source tensor is 1D, there will be three formal parameters for the block, and if it is 2D, there will be two indexes, so it will have four parameters.
In the example above, i, rgb is the index, val is the value of `histCumSum(i, rgb)`, and accm is the value of the previous block execution.

**Explanation**

The init argument and the way blocks are executed are roughly the same as "function reduce",
but the only difference is executing block for one axis only.

For "function reduce", see [ifel and Loops](IfelLoop.md).

When we talk about "Value of previous block execution result", "previous" refers to the location obtained by moving back one element along a specified axis relative to the current index.

For example, if the index is `(3, 3)` and dim is 0, the previous location is `(2, 3)`, and if dim is 1, the previous location is `(3, 2)`.

### reduce Which Result in a Single Value

`reduce` reduces the dimension of the source tensor by one.
So, if the source tensor is one dimension, the result is a zero dimension, which means single value.

In this case, the tensor reduce can be used as an expression, and can be put result into a variable using a normal let.

When you want to find a place larger than a certain index from the cumulative sum of one-dimensional weights called wcumsum,
the following two expressions are the same:

```swift
  # Definition by def. The result is 0 dimensions, so i3 is just a value
  def i3 by reduce<wcumsum>.find_first_index(dim=0) |_, val| { index < val }

  # Only when it becomes 0 dimensions, it can be assigned as a variable using let.
  let i3 = reduce<wcumsum>.find_first_index(dim=0) |_, val| { index < val }
```

This is an example taken from the calculations used to determine a weighted median,
not by computing a histogram, but by arranging colors with duplication.
It's used in the context of a median filter.

### reduce Currently Only Support Local Tensor for Source Tensor Too

At the moment, we have a constraint that all local tensor sizes are determined when generating code.
To ensure this constraint is realized, the source tensor must be also the local tensor.

Technically, it would be fine to use a global tensor literal, which has a fixed size.
This constraint may be relaxed in the future, but at this point, we can only create a local tensor based on a local tensor.

## Tensor trans

`reduce` was a process of creating a new tensor from the source tensor.
Tensor `trans` is the process of changing the original tensor in place.

This is one of the few features in MFG that has the side effect of changing existing values.

For example, if you calculate the cumulative sum of tensors called hist, then it would be as follows:

```swift
  mut! trans<hist>.cumsum!(dim=0)
```

This will change the tensor called hist to the cumulative sum value.

### Advantages and disadvantages of trans

`trans` use side effect to rewriting existing values.
Rewriting an existing local tensor may have the advantage of having fewer registers than recreating with reduce.

On the other hand, the code tends to be difficult to read as the contents of tensor of the same name changes depending on the location.

### Syntax of `trans`

The tensor trans syntax is as follows:

```swift
  mut! trans<TENSOR_NAME>.METHOD_NAME!(...)
```

`TENSOR_NAME` and `METHOD_NAME` will be replaced with concrete names.

First of all, in MFG, statements with side effects always start with `mut!`.
The method name is then marked with `!`.

This is done with the intention of making sentences with side effects particularly visible in syntax.

Otherwise, the syntax is almost the same as reduce.

Methods of `trans` include the following:

- sort
- cumsum

### trans Method: sort

`sort` sorts from small to large values ​​along the specified axis.

```swift
  mut! trans<wmat>.sort!(dim=0)
```

At the moment, sort is only for a one-dimensional i32 tensor.
Not just for technical reasons, but because there was no use, so if there is a purpose to use it in the future, I plan to implement it for f32 or 2D.

The only argument is dim.

### trans Method: cumsum

`cumsum` is replaced with the cumulative sum value along the specified axis.

For example, for tensors `[3, 2, 3, 1]`,
it becomes `[3, 5, 8, 9]` with the results that are added in order from the left.
Please note that it will be in the same dimension as the original.

Use it as follows:

```swift
  mut! trans<wcumsum>.cumsum!(dim=0)
```

The only argument is dim.

## All trans and reduce methods can be used with named arguments

All `dim` and `init` arguments can be used as named arguments.

## ts.for_each and `+=`

There is a feature called `+=`.
Even though it's neither trans nor reduce, but it's similar to `trans`, so I'll explain it here.
Furthermore, there is also a tensor loop-based method called `ts.for_each`.
In principle, the only way to use it is with `+=`, so I would like to explain it all together here.

Weighted histograms can be calculated in the 3x3 range of each pixel, as follows:

```swift
def weight by [[1, 2, 1],
               [2, 3, 2],
               [1, 2, 1]]

def result_u8 |x, y| {

  @bounds(256, 4)
  def hist |i, col| { 0 }

  weight.for_each |ix, iy, wval| {
     let [b, g, r, a] = input_u8(ix+x, iy+y)
     mut! hist(b, 0) += wval
     mut! hist(g, 1) += wval
     mut! hist(r, 2) += wval
     mut! hist(a, 3) += wval
  }
  ...
}
```

This article covers `weight.for_each` and four sentences starting with `mut!`.

### ts.for_each

The tensor's for_each method executes a block argument on each element.
No return value, just run the block.

```swift
ts.for_each | INDEX, TS_ELEMENT_AT_THAT_INDEX | { ... }
```

The number of INDEX arguments is one if ts is 1D, two if ts is 2D.

This statement is meaningless unless you do something with side effects in it, as it only executes the block without returning a value.

And at this point, the only side effect that can be performed in this context is `+=`.

### Side Effect by `+=`

For now, the only side effect for each element of the local tensor is by `+=`.
I think it's also possible to implement `=`, but it's not currently implemented.

We would like to reduce the number of functions related to side effects as much as possible, so we have put aside the implementation until we conclude that it is sufficiently necessary.

Use `+=` as follows:

```swift
  mut! hist(b, 0) += wval
```

The `+=` statement begins with `mut!`.
In MFG, all side-effect statements begin with `mut!`.

`+=` places a tensor reference on the left side and a value on the right side.
The value of the result referenced tensor on the left side and the value on the right side is added,
overwrites the reference destination of the tensor on the left side.

I think it behaves the same as `+=` in other languages.

## Next

[Attributes and Input Widgets](AttrWidget.md)