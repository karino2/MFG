# Attributes and Input Widgets

- Prev: [trans and reduce](TransReduce.md)
- [Top](README.md)


In MFG, those that start with the at mark `@` are called attributes.
These include things related to input/output dialogs and `@bouds`.

In this chapter we will look at attributes.

Because attributes are parsed with rules that are different from the normal script type system,
there is a difference in syntax.

## List of attributes

MFG has the following attributes:

- `@title`
- `@bounds`
- `@print_expr`
- param related:
  - `@param_i32`, `@param_f32`
  - `@param_f32v2`, `@param_f32v4`
    - (deprecated `@param_pos` is the same as `@param_f32v2`)

Since input widgets are involved in param-related part, we will handle them all together in the second half.

Attributes start with the `@` mark and are not used in other expressions, and can only be used in global blocks except for `@bounds`. (Cannot be used inside defs)

For global blocks, see [Tensor](Tensor.md).

## `@title`

Specifies the title of the filter.

````
@title "Negative Positive Inverted Filter"
````

After `@title`, place space and write a string with double quotes.

MFG does not have a string type in his type system, and it is not possible to create a string variable.
Strings appear only in attribute.

The title specified by `@title` can be used in the menu in the filter list, or
it is also used in the title of a dialog at runtime.

## `@bounds`

`@bounds` is used prior to the tensor definition. It specifis the width and height of the tensor.

```mfg
@bounds(50, 50)
def gridNoise |x, y| {
  [2.0*rand()-1.0, 2.0*rand()-1.0]
}
```

The number of arguments in `@bounds` determines the dimensions of the tensor that follow.

Specify an integer for `@bounds`.
`@bounds` of local tensor have further constraints that it must be literal constants.

## `@print_expr`

This is a debugging feature.
It can only be used in global blocks. (Cannot be used in blocks in tensor definitions).

The values you put inside will be output to the output window.

```mfg
@print_expr(sin(3.2)/2.0)
```

This is used for simple debugging investigations.

When you need more serious debugging,
you can get a more detailed look by adding information to the resulting color and looking it up with an eyedropper.

## param-based attributes and input widgets

Param-based attributes are attributes as well as definitions of variables.
That's why it's very different from other attributes.

Typically, the following are param-based attributes:

```mfg
@param_f32 radius(SLIDER, label="Radius", min=0.5, max=300.0, init=3.0)
```

It is easy to understand param-based attributes when you consider two angles: the type of widget created and the variables created.

### The Variables Created

Param-based attributes create variables. The result of the user's input is then entered into this variable.

Let's take a look at the example above.

```mfg
@param_f32 radius(SLIDER, label="Radius", min=0.5, max=300.0, init=3.0)
```

In this case, a variable called `radius` is created with the type f32.

The attribute name corresponds to the type of the variable being created.

| Attribute Name | Type of variable to be created |
| ---- | ---- |
| `@param_i32` | i32 |
| `@param_f32` | f32 |
| `@param_f32v2` | 2D vectors of `[f32, f32]` |
| `@param_f32v4` | 4-dimensional vectors of `[f32, f32, f32, f32, f32]` |

From the IR of MFG points of view, the important thing is what variables are created.
It doesn't matter how it accepts, that is, from what widgets it accepts.

So for IR, only this attribute name and the identifier which follows matters.

The bracketed arguments beyond that are information used by the world outside of the MFG to display the UI.

### Arguments of param and Widgets

The first argument to param represents the type of widget.
The widget types are as follows:

- SLIDER 
- DIRECTION
- CHECKBOX
- POINTER
- COLOR_PICKER

The named arguments follow depends on this first argument.

The order of named arguments follow is optional, but the type of widget must come first.

Below, let's take a look at each widget.

### SLIDER

A slider that selects numbers.

This widget can be used with `@param_i32` and `@param_f32`.

**Example:**

````
@param_f32 radius(SLIDER, label="radius", min=0.5, max=300.0, init=3.0)
````

**Argument***

- `label`: The label that appears next to the slider. String.
- `min`: Minimum slider value. For `@param_i32`, specify i32, and for `@param_f32`, specify f32 (same below)
- `max`: Maximum slider value.
- `init`: The initial slider value.

### DIRECTION

A widget to select orientation.
This widget can be used with `@param_f32`.

The result is radians of f32 from 0.0 to 3.1415...

**Example:**

````
@param_f32 angle(DIRECTION, init=1.0)
````

**Argument***

- `init`: Initial value for the widget orientation. Specify in radians.

### CHECKBOX

Checkbox widget.
This widget can be used with `@param_i32`.

The result is an i32 of 0 or 1.

**Example:**

````
@param_i32 show_edge(CHECKBOX, label="Display edge", init=0)
````

**Argument***

- `label`: Label displayed in checkbox
- `init`: Initial value for checkbox. 0 is not checked, 1 is checked.

### POINTER

Specify the position within the canvas. The result is a two-dimensional vector of f32, returned between 0.0 and 1.0.

This widget can be used with `@param_f32v2`.

**Example:**

````
@param_f32v2 ORIGIN(POINTER, label="center")
````

**Argument:**

- `label`: The label that appears above the widget to select a position. String.

### COLOR_PICKER

Choose a color. The result is the 4D vector of f32, order of BGRA, from 0.0 to 1.0.

This widget can be used with `@param_f32v4`.

**Example:**

````
@param_f32v4 fgcolor(COLOR_PICKER, label="color", init=[0.0, 0.0, 1.0, 1.0])
````

**Argument:**

- `label`: The label that appears in the color picker. String.
- `init`: Initial value for the color picker.

## Next

[Bultin Functions](BuiltinFunctions.md)