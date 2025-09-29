# 04 Intermediate tensors for complex algorithms

Previous: [03 Vectorized Operations and rsum](03_VectorRsum.md)

Up until this point, the only tensor to generate was result_u8.

Here we will look at how to realize algorithms with more than two passes by generating intermediate tensors.
Because a kernel, which is GPU program terminology, is generated for each tensor,
It can also be used to prevent timeouts and utilize hardware.

## 2-pass blur algorithm

The blur processing implemented [last time](03_VectorRsum.md) add the same set of pixels multiple times for neighboring pixel calculation.
Reusing the surrounding calculation results allows for further optimization.

Here, let's simply save the results from the horizontal average and change to average them vertically.

Creating an intermediate tensor is almost the same as how you create result_u8, but the only difference is that you need to specify the width and height.

The code is as follows:

```mfg
let inputEx = sampler<input_u8>(address=.ClampToEdge)

@bounds(input_u8.extent(0), input_u8.extent(1))
def xblur |x, y| {
  let sumCol = rsum(0..<3) |rx| {
    i32(inputEx(x+rx-1, y))
  }
  sumCol/3
}
```

This will generate a tensor named xblur. The width and height are the same size as the input layer.
Since sumCol is a vector of four elements of i32, xblur is also a vector of four elements of i32 (note that it is not u8).

Instead of writing input_u8.extent twice, you can also do the following:

```mfg

let inputEx = sampler<input_u8>(address=.ClampToEdge)

let [W, H] = input_u8.extent()

@bounds(W, H)
def xblur |x, y| {
  let sumCol = rsum(0..<3) |rx| {
    i32(inputEx(x+rx-1, y))
  }
  sumCol/3
}
```

## Closer look at xblur's code

Now let's take a look at the code for generating intermediate tensors.

First, the tensor definition begins with `@bounds`, which specifies the width and height.

```mfg
@bounds(W, H)
def xblur
```

These W and H becomes width and height of xblur tensor.

The tensor definition is almost the same as result_u8, but you can choose the type of element you want (Unlike results_u8 which is fixed to u8 4D vector).
The xblur code is as follows:

```mfg
def xblur |x, y| {
  let sumCol = rsum(0..<3) |rx| {
    i32(inputEx(x+rx-1, y))
  }
  sumCol/3
}
```

Like result_u8, we run this block by increasing x and y by 1 from 0 to width and height.
This execution is performed in parallel on the GPU.

The creation of one tensor becomes one kernel in GPU terms.
If the one kernel execution is too long, the process will be timed out.
But if the execution time of each kernel is shortened, the execution will not be timed out even though total execution time is long.

Internally, xblur has global DRAM memory reserved and stored there.

Other than that, I think it's the same as the definition of result_u8.

## Write a blur processing using xblur

You can achieve a blur effect by summing intermediate results of vertical average horizontally.
Let's actually do that process.

If you note that it's enough to just set the ClampToEdge and add outside of the boundary just as inside, the code becomes following:

```mfg
@title "Blur using intermediate tensor"

let inputEx = sampler<input_u8>(address=.ClampToEdge)

let [W, H] = input_u8.extent()

@bounds(W, H)
def xblur |x, y| {
  let sumCol = rsum(0..<3) |rx| {
    i32(inputEx(x+rx-1, y))
  }
  sumCol/3
}

let xblurEx = sampler<xblur>(address=.ClampToEdge)

def result_u8 |x, y| {
  let sum = rsum(0..<3) |i| {
    xblurEx(x, y+i-1)
  }
  u8(sum/3)
}
```

This will perform faster.
3x3 might not be big enough to notice the difference, but if you increase the range a little, the difference will become more noticeable.

## Allowing to change the width with the slider widget

Up until this point, the addition range was limited to 3x3.
However, normally, when you think of a blur filter, you would want to specify the degree of blur using the slider.

So, try using the slider widget to receive user input.

To use the slider widget, specify `SLIDER` in `@param_i32`.
THe code is as follows:

```mfg
@param_i32 BLUR_WIDTH(SLIDER, label="Size", min=2, max=100, init=5)
```

This will cause a slider to appear in the dialog when you run it, and the selected number will be in the variable `BLUR_WIDTH`.
You can specify various things such as min, max, and init, but it's best to use it as is for a while and adjust only the places you want to change.

If you use this to enter the width of the blur into the variable BLUR_WIDTH, you can just replace the number specified in hardcoded in rsum with this.

If I write just the results, it looks like this:

```mfg
@title "Blur using intermediate tensor"

@param_i32 BLUR_WIDTH(SLIDER, label="Size", min=2, max=100, init=5)

let inputEx = sampler<input_u8>(address=.ClampToEdge)

let [W, H] = input_u8.extent()

@bounds(W, H)
def xblur |x, y| {
  let sumCol = rsum(0..<BLUR_WIDTH*2+1) |rx| {
    i32(inputEx(x+rx-BLUR_WIDTH, y))
  }
  sumCol/(2*BLUR_WIDTH+1)
}

let xblurEx = sampler<xblur>(address=.ClampToEdge)

def result_u8 |x, y| {
  let sum = rsum(0..<BLUR_WIDTH*2+1) |i| {
    xblurEx(x, y+i-BLUR_WIDTH)
  }
  u8(sum/(2*BLUR_WIDTH+1))
}
```

When you run it, it will look like this:

![Adjustable blur with slider] (imgs/blur_with_slider.png)

This creates a great blur filter.

## Summary of this chapter

- The intermediate tensor is the same as result_u8 except that it is necessary to specify the width and height with `@bounds`.
- Intermediate tensors can be created other than u8 vectors
- The intermediate tensor becomes a different kernel for GPUs
- You can receive input using the slider with `@param_i32`

## For those who want to learn more

Getting Started will end at this point.

From here onwards, there are many samples in the "Examples Project" in the MFGStudio File menu.

Additionally, the [Reference](../Reference/README.md) provides a more detailed explanation.