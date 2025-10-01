# Expression and Vectorize Operation

- Prev: [Type](Type.md)
- [Top](README.md)


In MFG, many of the components are expressions.
Loops and condition selections, which are often sentences in other languages, are also implemented as expressions.

An expression is something that can come to the right side of a `let`, and is a value of some kind, and the type for it is determined.

Here we will look at arithmetic expressions such as basic binomial operations and function calls among expressions.

## Arithmetic Operations

Normal addition and subtraction. There are unary operators and binomial operators.

### Unary Operations

There are two unary operations in arithmetic operations:

| Operator | Description |
| ---- | ---- |
| `-` | Negate |
| `!` | Logical not |

Example:

```mfg
let b = -a
let c = !a
```

There is another unary operator other than arithmetic operations, the splat operator `*`.
Unlike arithmetic unary operators, this can only be used in special places.
I will explain it later.

### Binary Operations and Precedence

Binary operations include the following, starting with the lowest priority:

| Operator | Description |
| ---- | ---- |
| `\|\|` | Logical OR |
| `&&` | Logical AND |
| `==`, `!=`, `<` `<=` `>` `>=` `\|>` | Comparison, Pipe Operations |
| `\|` | Bit OR |
| `&` | Bit AND |
| `>>`, `<<` | Shift operator |
| `+`, `-` | Addition, Subtraction |
| `*`, `/`, `%` | Multiplication, division, modulo operations |
| `^` | Powers (right associative) |

Only the powers are right-associative.

Also, since `^` is used as a power, exclusive OR is a function rather than a binomial operation. 
It's a function called `xor`. For more information, see [Built-in Functions](BuiltinFunctions.md).

### Vectorization of Binary Operations

Tuples have the function of applying a certain operation to each element and then making the result a tuple.
This is called vectorization (although it may be complicated, it is a different function from vectors).

Several single argument functions, `ifel`, cast, and binomial operations support vectorization.
This section covers vectorization of binary operations.

The following a1 and a2 have the same meaning.

```mfg
let tup1 = [1.0, 2]
let tup2 = [3.0, 4]

# Add each element
let a1 = [tup1.x+tup2.x, tup1.y+tup2.y]

# Vectorise
let a2 = tup1+tup2
```

Please note that x in tup1 is f32 type and y is i32 type.
Vectorization is possible as long as the tup1 and tup2 types are all matches.
In other words, if the left and right sides of the binary calculation are matched for each element,
vectorization can be done even if the types of tup1.x and tup1.y are different.

Note that the results of binary operations are also tuples, so they can be stitched together.

```mfg
let tup1 = [1.0, 2.0]
let tup2 = [3.0, 4.0]
let tup3 = [5.0, 5.0]

let s = (tup+tup2)/tup3
```

It can do things like mathematical vector operations.

Equivalence operator `==` can also be used in Vectorization.

```mfg

let tup1 = [1, 2, 1]
let tup2 = [3, 2, 1]

# s becomes [0, 1, 1]
let s = tup1 == tup2
```

Strictly speaking, whether the element of s becomes 1 depends on the environment, so I should say it becomes `[0, non-zero, non-zero]`.

By combining this with the ifel vectorization that appears in a later chapter, multiple elements can be calculated at once.

### Binary Operations of Scalar and Tuple

Binary operations also support combinations of scalars and tuples.
In this case, it is called broadcast, and the result is the same as scalar operations being performed on each element of a tuple.

For example, the following three results have the same result:

```mfg
let tup = [3.0, 4.0]

let m1 = [tup.x*2.0, tup.y*2.0]
let m2 = 2.0*tup
let m3 = tup*2.0
```

Multiplication is the same as scalars and vectors in mathematics, but the same can be done with addition.
Below a1, a2, and a3 have the same values.

```mfg
let tup = [3.0, 4.0]

let a1 = 5.0+tup
let a2 = [5.0, 5.0] + [3.0, 4.0]
let a3 = [3.0+5.0, 4.0+5.0]
```

## splat Operator

A special unary operator is the splat operator, `*`.
This is an operator that can only be used in the following locations:

- Inside the literal of a tuple
- In the arguments of function calls
- In the arguments for tensors and sampler references

Among these, if you place `*` before any tuple,
it becomes as if the elements of tuple have been expanded in place as arguments.

**Tensor Example:**

```mfg
  let tup = [10, 20]

  # The same as: input_u8(10, 20)
  input_u8(*tup)
```

**Tuple Literal Example:**

```mfg
  let red = [0, 0, 0xff]

  # The same as: let bgra = [0, 0, 0xff, 0x88]
  let bgra = [*red, 0x88]
```

**Function Call Exaple:**

```mfg
let a = 0.7
let b = 0.6
let ratio = 0.3
let tup = [a, b, ratio]

# The same as: mix(a, b, ratio)
mix(*tup)
```

## Function Call

Function calls are also typical expressions.

Functions can be called by adding parentheses after the function name and adding arguments inside it.

For example, `sin(1.5)` is a function call.

### Vectorization of Function Calls

Single-argument functions such as trigonometric functions can be used in vectorization, just like binary operators.
The following s1 and s2 have the same meaning.

```mfg
let tup = [1.0, 2.0]

# sin for each element
let s1 = [sin(tup.x), sin(tup.y)]
# Vectorize Calling of sin
let s2 = sin(tup)
```

You will need to look at the individual function documentation to see which functions can be used as vectorization calls.
The rule of thumb is that functions that perform normal mathematical calculations with one argument can generally be used as vectorization calls.

### Function Calls and Splat Operators

The splat operator can be used in function calls.
For details, see the "splat operator" section above.

### Special treatment of last argument with `...` (Trailing Expression)

If the last argument of a function is set to `...`, the next expression in that call is considered to be the last argument,
There is a syntax sugar called.

```mfg
  # the same as: let d = distance( [1.2, 2.3], [4.5, 3.2])
  let d = distance( [1.2, 2.3], ...) [4.5, 3.2]
```

Although it is a feature that is not useful in functions, it is used in ifel to avoid nesting.
For more information, see [ifel and Loops](IfelLoop.md).

### Pipelining with Pipe Operator and `...`

`...` changes the location of the expression from which it is replaced by the pipe operator `|>`.
`|>` replaces `...` in the right side with the expression on the left side.

In other words, the following three expressions have the same meaning:

```mfg
f(a, b, c)
f(a, b, ...) c
c |> f(a, b, ...)
```

This allows pipelining to be performed as follows:

```mfg
let col = input_u8(x, y) |>
          to_ncolor(...) |>
          gamma2linearA(...)
```

In MFG, a line break is recognized as the end of an expression, but special treatment is given when the beginning of the next line is a pipe operator.
It is fine to write a pipe symbol at the beginning of a next line as in the example above.

### Named Arguments

Some functions have named arguments.
For example, the following `init` of reduce is the case.

```mfg
reduce(init=[0, 0], 0..<10) | index, accm | {
  let [a, b] = accm
  [a+index, b+index*2]
}
```

Named arguments can be used without worrying about order.

Named arguments can only be used in functions that provide them.
Named arguments are provided for functions with many arguments that are difficult to understand the meanings of arguments only by position.

See the explanation of each function to see if it provides named arguments.
If a named argument and an unnamed argument are used together,
It is assumed that the arguments for the named arguments are first filled in, and then the remaining arguments are specified in order.

Named arguments can be specified in the order of arguments without names, but many of them may change the order due to future changes in specifications.
Use named arguments wherever you can use named arguments.

Named arguments are covered in more detail using [Sampler](Sampler.md) and [trans and reduce](TransReduce.md).

### Block arguments

Functions that provide many loop-based functionality take a block as an argument at the end.
Place the last block right after the brackets.

```swfit
rsum(0..<3) |i| { (i+3)*2 }
```

The last `|i| { (i+3)*2 }` is the block argument, and this block is passed to rsum.
Block arguments are explained in the [ifel and Loops](IfelLoop.md) "Loop Based Features" section.

## Other Expressions

MFG has most of the elements of the program as expressions, so many places in this reference talk about expressions that were not covered on this page.

- Tensor call is [Tensor](Tensor.md)
- sampler call is [sampler](Sampler.md)
- Tuple's swizzle operator is [Type](Type.md)
- reduce, rsum, ifel, etc. are [ifel and Loops](IfelLoop.md)
  - Also see Ranges that represent the ranges here
- Tensor reduce can also be an expression, which is [trans and reduce](TransReduce.md)

## Next

[ifel and Loops](IfelLoop.md)