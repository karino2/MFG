# Sampler

- Prev: [ifel and Loops](IfelLoop.md)
- [Top](README.md)


A sampler is a feature that allows you to customize how tensors are loaded.

```mfg
let finput = sampler<input_u8>(address=.ClampToEdge, coord=.NormalizedLinear)
```

From now on, this finput can be used like a normal tensor.

Basically, the following three are specified for sampler:

- Tensor
- Address mode (address argument)
- Coord argument (coord argument)

Let's take a look at these three in order.

## Specify Tensor

The sampler customizes the way in which values ​​are retrieved from the target tensor.
Therefore, always specify the tensor to be targeted.

In MFG, tensors are special and are different from variables, so syntax distinguishes them from other arguments.
When specifing tensors, the tensor is always enclosed with angle brackets `<>`.

Below is an example of specifying `input_u8` for sampler.

```
sampler<input_u8>(...)
```

## Address Mode (address)

Address mode is a method that specifies the behavior when a coordinate outside the boundary is specified.

The address mode is Enum type and you specify one of the following three:

| Enum symbol | Effect |
| ---- | ---- |
| None | Default. Does not do anything to extrude, the operation is undefined when extrude (use this when it is known that it will not extrude programmatically) |
| ClampToEdge | Repeat the edge values |
| ClampToBorderValue | Value specified in border_value |

Note that you need to prefix `.` to enum symbol when using as an arguments.

Only when ClampToBorderValue requires an additional argument called border_value.

```mfg
let extend = sampler<input_u8>(address=.ClampToBorderValue, border_value=u8[0, 0, 0, 0])
```

The type of the border_value value must match the type of the element of the tensor.

## Coord Argument (coord)

For coordinate specification, you can choose between the normal pixel i32 and the normalized f32 from 0.0 to 1.0.
With coordinates normalized to f32, there are two types of Enum depending on the behavior when a gap is specified.

| Enum symbol | Effect |
| ---- | ---- |
| Pixel |  Default. Specified with i32. |
| NormalizedNearest | 0.0 to 1.0, and the nearest pixel is used. |
| NormalizedLinear | Values ​​between 0.0 and 1.0 are interpolated with binlinear |

If you specify the NormalizedXXX, the argument will be specified with f32 as follows:

```mfg
let finput = sampler<input_u8>(coord=.NormalizedLinear)
finput(0.3, 0.7)
```

To convert the current coordinates to the coordinates normalized to this f32 in the tensor definition,
you can use tensor to_ncoord.

```mfg
let finput = sampler<input_u8>(coord=.NormalizedLinear)

def result_u8 |x, y| {
   let fxy = to_ncoord([x, y])
   let col = finput(*fxy)
   ...
}
```

Note that this asterisk is a splat operator.
For details, see [Tensor](Tensor.md).

## Sampler Arguments can be Used as Named Arguments

`coord`, `address`, and `border_value` can be (and should be) used as named arguments.

## Next

[trans and reduce](TransReduce.md)