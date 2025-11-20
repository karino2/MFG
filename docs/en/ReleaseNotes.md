# MFGStudio Release Notes

Update history. [MFG Studio download page](https://modernfilterlanguageforgpu.org/download/)

## 1.0.08 2025-11-20

Along with the official release of FireAlpaca SE 3.0, we released MFGStudio with the same engine.

- `fore_color()` and `back_color()` allow you to obtain the currently selected color of the app's brush and its back color([Reference: User Defined and Bultin Functions](Reference/BultinFunctions.md)）
- Added a color picker to MFGStudio (can be displayed from the window menu). Now you can specify the color that can be obtained with `fore_color()` and `back_color()`
- Added `ts.extentf()`, which is f32 version of `ts.extent()`([Reference: Tensor](Reference/Tensor.md))


## 1.0.07 2025-11-05

- Added thumbnail_base.mdz, which is the thumbnail_base.mdz, to the preview target
- Added ability to import current preview as thumbnail.png

The thumbnail is a 512x512 png image and will be included in the mar archive with the file name thumbnail.png.

## v1.0.06 2025-09-22

- Implement cancellation during long running times
- Implementing the `@version` attribute
- If the beginning of the next line is a pipeline operator, it is considered a continuation line (MEP 27)
- Bugfix
  - Fixed a bug where variable name conflicts could occur when destructuring lets in user-defined functions
  - Fixed the vectorization process for `!`.
  - Fixed a compile error when `else` was a non-vector tuple, make itthe same constraint as ifel
  - Fixed a bug where inline functions would crash when the number of inline functions reached a certain level.
  - Fixed crash because of missing catch of exception of compilation errors in HLSL and Metal layers.

## v1.0.05 2025-09-04

- Fixed an issue where the trial version check that I forgot to delete stopped running after 8/31

## v1.0.04 2025-09-01

- Fixed a bug where a compile error occurred when touching global variables in user-defined functions
- xor support
- Unsigned hex literals

## v1.0.03 2025-08-18

- Fixed random number seeds in preview and execution to make the same value
- Adding a user-defined function

## v1.0.02 2025-07-28

- Adding `@param_f32v4` and COLOR_PICKER (see perlin noise in examples)
- Introducing `@param_f32v2` to deprecated `@param_pos`
- Supports textures of png images
- Supports strings.json and internationalization using it. (Document is not yet ready.)
- Improved error message for unexpected token parsing errors

## v1.0.01 2025-06-24

- Implement vec2
- Fixed the 5th dimension or higher vectorization not working
- You can now choose where to create mar
- Display the time when filter is executed
- Fixed the modulo on f32 not working
- Added CRT filter
- Added preview image of copperplate tone filter and layer mode for hard mix to try it
- Added conversion functions related to CIE XYZ color (see below)
- Implement pipe operators (see below)

We implement following color-related conversion functions:

- lbgr_to_xyz, xyz_to_u8color, to_xyza
- lbgra_to_u8color, to_lbgra

The xyz is CIE XYZ color. lbgr is a linearized BGR (0.0 to 1.0) with gamma correction.
For more information, see "Color Conversion and Supported Colors" in [ja/Reference/BuiltinFunctions.md](../ja/Reference/BuiltinFunctions.md). (English version is not yet ready)

For pipe operators, see "Pipelining with Pipe Operator and `...`" in [Reference: Expressions and Vectorization Operation](Reference/Expression.md).

## v1.0.00 2025-06-03

Initial release.