# MFGStudio Release Notes

Update history.

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