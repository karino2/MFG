# ifel and Loops

- Prev: [Expression and Vectorize Operation](Expression.md)
- [Top](README.md)


Though loops and conditional branches are often combined as control structures in other languages,
because the GPU is a very different in this respect to the CPU, which is a random jump using a program counter,
this is a prominent aspect of MFG that is different from other languages.

Here we will look at the MFG condition selection and loop features.

As an MFG, there is no inherent reason to explain ifel and loop features in the same chapter,
we will cover them here to make comparisons with other languages easy.

## Conditional Selection Using ifel

Condition selection using MFG is achieved as a simple function.

```mfg
let s = ifel(x > 0, 1, 2)
```

s contains 1 if x is greater than 0, and 2 if it is less than 0.

The syntax is as follows:

```mfg
ifel( COND, TRUE_VALUE, FALSE_VALUE )
```

Returns TRUE_VALUE if COND is not 0, and FALSE_VLUE if 0.

Strictly speaking, MFG functions do not support overloading, but ifel behaves like it is overloaded,
it's slightly different from a function internally, but it's safe to assume it's a function as a user.

In other languages, it is called conditional branching, but in MFG, it does not branch, so we will call it conditional selection.

### Allowed Arguments and Vectorization

The types of COND, TRUE_VALUE, and FALSE_VALUE of `ifel` are mutually dependent and the allowed combinations of them are determined.

1. TRUE_VALUE and FALSE_VALUE must always be of the same type
2. COND is an integer (i or u) scalar or tuple
3. When COND is a tuple, TRUE_VALUE must be a tuple of the same dimension.

The rule 3 will be a bit more detailed, so let's take a look.
First of all, COND is basically an integer scalar or tuple.

When COND is, for example, `[0, 1, 2]`, this is a tuple of three elements of i32,
In this case, TRUE_VALUE must be a tuple of three elements.
The number of elements is important, and the types of each elements are arbitrary.
But TRUE_VALUE types and FALSE_VALUE types must match.

```mfg
ifel([0, 1, 2], [3, 3.0, 3u], [5, 0.0, 1u])
```

This is the same meaning to following code,
which execute ifel for each elements and combine result to tuple at the end:

```mfg
[ifel(0, 3, 5),
 ifel(1, 3.0, 0.0),
 ifel(2, 3u, 1u)]
```

The resulting type is a tuple of i32, f32, and u32.
The resulting value in this case is `[5, 3.0, 3u]`.

When combined with the vectorization of binary operations explained in [Expression and Vectorization Operations](Expression.md),
it may be possible to write the code concisely.

```mfg
let flag = [1, 2, 3]

ifel(flag%2 == 0, [3, 3.0, 3u], [5, 0.0, 1u])

# the result is [5, 3.0, 1u]
```

If the flag element has even numbers, select the TRUE_VALUE element, and if the flag is odd, select the FALSE_VALUE element.

This type constraint, which is a tuple of integers the same as the dimension of scalar or TRUE_VALUE, is quite special, and is only an ifel.

It may be helpful to paying attention to the fact that the result type of ifel always has the same as the type of TRUE_VALUE.

### Avoid Nesting by Using `...` Syntax Sugar

`ifel` often does the process of "If it's condition A, then do this, if it's condition B, then do that, if it's condition C if it's condition D, etc."
In this case, the calls will become deep nested, making parallel structures difficult to understand.

```mfg
ifel(aCond, aVal,
  ifel(bCond, bVal,
    ifel(cCond, cVal,
      ifel(dCond, dVal, otherVal))))
```

Therefore, by using the feature "If the last argument is set to `...`, the next expression in parenthesis becomes the last argument", you can write as follows:

```mfg
ifel(aCond, aVal, ...)
ifel(bCond, bVal, ...)
ifel(cCond, cVal, ...)
ifel(dCond, dVal, otherVal)
```

Both codes have the same meaning, but the nesting of calls is disappear.

When parsing, the code is converted into nested code and executed, so both have the exact same internal representation.

This `...` is a feature that can be used in all function calls, but is essentially only used in ifel or elif.

### Syntax Sugar of `elif`, `else`

You can also call the ifel function with the name elif.
For MFG compiler, `elif` and `ifel` are exactly the same and are indistinguishable.
However, for convenience to the writer, `ifel` is used for the first condition, and `elif` is used for following parallel conditions.

This is for reader's convinience and becomes similar to other languages.
Together with the aforementioned `...`, use it as follows:

```mfg
ifel(aCond, aVal, ...)
elif(bCond, bVal, ...)
elif(cCond, cVal, ...)
elif(dCond, dVal, otherVal)
```

Here is the real code from MLAA (morphological antialiasing):

```mfg
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
```

I think you can read it like a normal programming language (although it's quite complicated, so it's not easy to read).

There are also function called `else` which return the argument as is and do nothing.
This is also available only to make it easier to read when used in conjunction with the above.

In the example above, the last `otherVal` is placed as FALSE_VALUE in dCond,
In many cases, the value here is the value that does not match all previous conditions;
because it is used to mean something like that, it is misleading to be found inside dCond.

So, you can write following by using `else`:

```mfg
ifel(aCond, aVal, ...)
elif(bCond, bVal, ...)
elif(cCond, cVal, ...)
elif(dCond, dVal, ...)
else(otherVal)
```

Both `else` and `elif` are intended to make it easier to read, and as a function, you can write everything using `ifel` alone.

## Loop Based Features

For MFG, the loop can be roughly divided into three categories.

1. Returns a value based on a range
2. Returns a value based on a tensor
3. Returns a tensor based on other tensor

Here we will deal with 1 and 2.
3 will be explained at [trans and reduce](TransReduce.md).

Tensor reduce can be both 2 and 3, but this is also treated as "trans and reduce" chapter.

Let's start by looking at the case 1. `reduce` and `rsum`.

The loop feature of MFG have the following characteristic:

- Returns a value (the only exception is ts.for_each)
- You cannot exit the loop halfway through (all loops will always be executed)
- The range of the loop is fixed at the start of the loop and cannot be changed

Thanks to these, unnecessary side effects can be eliminated from the code, making it safer,
and the code looks and the actual GPU hardware behavior match naturally.

## rsum

`rsum` stands for reduce sum, and the same thing can be done using the more advanced function `reduce`.

Below is an example using rsum.

```mfg
let s = rsum(0..<4) |i| { i*2 }
```

This result is `0*2+1*2+2*2+3*2`, which means `12`.

### rsum Syntax

As will be explained later, rsum supports 1 and 2D. The syntax is as follows:

```mfg
# 1D rsum
rsum(RANGE) |i| {...}

# 2D rsum
rsum(RANGE1, RANGE2) |i, j| {...}
```

The range and block arguments are common to all loop-based functions and not special to rsum.
But `rsum` is the first example of loop-based functions, so let's take a closer look here.

### Range

The range is specified with `..<`, such as `0..<3`.
Currently, it can only be used as arguments to function calls.

The syntax is as follows:

````
StartIndex..<EndIndex
````

The range is a half-open section and does not include the ending index.
For example, `0..<3` represents a range from 0 to 2, that is, 0, 1, 2.

Currently, only support integers for start and end indexes.
Negative values ​​can also be used, and variables can also be used.

For example, `-2..<3`, which means `-2, -1, 0, 1, 2`.

If the starting index is larger, the loop will never be executed, but this behavior may change in the future, so write your code so that there will be no such thing.

### Block arguments

The part `|i| {...}` at the end of rsum, etc., is called a block argument.

The block has the syntax `|Formal Argument List| { BODY }`, and
The last part of this BODY must be an expression.
This last expression becomes the value (and type) of this block.

```mfg
let a = rsum(0..<5) |i| {
  let col = inputEx(x+i, y)
  col.x+col.y # This last expression is the value that results when evaluating this block.
}
```

Roughly speaking, the last of the body mus be a expressionn means that it must not end with a `let`.

What the formal argument list will be is always determined by the function that takes block arguments and the arguments of function up to that point.
In the case of rsum, the number of range arguments determines the number of formal argument lists for block arguments.

In 1D, it has one argument list, and in 2D, it has two formal arguments list, both of which have a type of i32.

```mfg
let a = rsum(0..<5, 0..<5) |i, j| {
  let col = inputEx(x+i, y+j)
  col.x+col.y
}
```

In the case of `reduce`, as described later, the number and type of the block's formal argument list are determined by the type of `init` and the number of range arguments.

How to use a block is determined by the function that passes the block argument.
In the case of rsum, the values ​​from each range are placed in the formal argument list in order, and the block is executed, and the result is added together.
Adding up the results is a feature of rsum, and the `reduce` function is used in a different way.

At this point, blocks can only be used as arguments.

### The Dimension of rsum

rsum supports 1 and 2D ranges.
Both results are single values.
This is different from tensor reduce.

The dimension is determined by the number of ranges of rsum arguments.
Additionally, the formal argument list for block arguments must be coincide with the dimension of rsum.

```mfg
let sum = rsum(0..<3, 0..<3) | i, j | {
            i*3+j
          }
```


## reduce

There are two types of reduce: the reduce function and the reduce used in tensor reduce. Here we will deal with the former, the stand alone reduce function.

The word "reduce" is used in MFG for operations that reduce dimensions.
This is clear when you look at the various reduces and the corresponding trans concepts in [trans and reduce](TransReduce.md),
at this point, don't go too deep and think of it as just a function name.

The reduce function is more powerful than rsum, and is more complicated. Everything you can do with rsum can also be achieved with reduce.

The `reduce` function is used as follows:

```mfg
let sum = reduce(init=1, 0..<4) | index, accm | {
            accm+index*2
          }
```

`accm` will initially contain the value of `init`, and from then on, the value of the previous block will assigned in turn.
In this example, the loop is executed four times with indexes ranging from 0 to 3, and each accm looks like this:

- accm: 1
- accm: 1
- accm: 3
- accm: 7

The final value is 13, and the variable `sum` contains 13.

### `reduce` Syntax

The reduce function has the following syntax:

```mfg
# 1D
reduce(init=INIT_VALUE, RANGE) |i, accm| {...}

# 2D
reduce(init=INIT_VALUE, RANGE1, RANGE2) |i, j, accm| {...}
```

i, j, accm can be any name.
accm is often used as an abbreviation for accumulator.

i and j contain the index of the loop, and accm contains the value from the previous block.
The initial value is the value specified in `init` argument.
The result of the last block executed becomes the result of this function itself.

The order of the block's formal arguments is the index first and accm is always the last.

The result type of reduce is the same as the type of the value specified in `init`.
The type that results from evaluating the block must be the same type as init.
accm is the same type as init.

You can also use a tuple as the value of init. In that case, accm will also contain tuples.

### `reduce` block execution order

Because of the nature of `reduce` which receives the results of the previous block with accm,
it depends on the order in which the loop is executed.

Consider the following example for the 2D case:

```mfg
let sum = reduce(init=1, 0..<3, 0..<3) | i, j, accm | {
            accm+i*3+j
          }
```

In this case, the order in which it is executed is executed so that j is the outer loop and i is the inner loop.

Specifically, in this case, it is the order of the indexes below.

1. i:0, j:0
2. i:1, j:0
3. i:2, j:0
4. i:0, j:1
5. i:1, j:1
6. i:2, j:1
7. i:0, j:2
8. i:1, j:2
9. i:2, j:2


## ts.sum

Though Tensor-based loops are discussed in detail in [trans and reduce](TransReduce.md),
ts.sum is a similar function to rsum, so I'll take a look here.

ts.sum executes blocks on each element of the tensor and returns the sum of the results.

```mfg
def weight by [
  [1.0, 2.0, 1.0],
  [2.0, 3.0, 2.0],
  [1.0, 2.0, 1.0]
]

let weightSum = weight.sum |i, j, elem| { elem }
```

i and j contain each index of the tensor, and elem contains the tensor value at that point.
The types of i and j are i32, and in this case elem is f32.

In this case, i and j are not used, so you can replace it with underscores.

```mfg
def weight by [
  [1.0, 2.0, 1.0],
  [2.0, 3.0, 2.0],
  [1.0, 2.0, 1.0]
]

let weightSum = weight.sum |_, _, elem| { elem }
```

## Next

[Sampler](Sampler.md)