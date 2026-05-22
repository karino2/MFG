# Internationalization and Resources

- Previous: [User Defined and Bultin Functions](BuiltinFunctions.md)
- [Top](README.md)

This page explains string resources for internationalization and how to include thumbnails and textures.

## String resources and internationalization

If a project (or MAR) contains a `strings.json` file, it is treated as a string resource.

In MFGStudio's file explorer, the T button can create a default resource file, which can be used to internationalize widget labels and similar text.

### Example overview

First, let's briefly describe the overall flow.

Suppose the project contains a `strings.json` like this:

```json
{
  "ja": {
    "LABEL_TITLE": "ハーフトーンディザ",
    "LABEL_THRESHOLD": "閾値"
  },
  "en": {
    "LABEL_TITLE": "Halftone Dither",
    "LABEL_THRESHOLD": "Threshold"
  }
}
```

If the MFG file originally contains:

```mfg
@title "Halftone Dither"

@param_f32 threshold(SLIDER, label="Threshold", init=1.0, min=0.0, max=1.0)
```

Then you can make the strings resource-based like this:

```mfg
@title $LABEL_TITLE

@param_f32 threshold(SLIDER, label=$LABEL_THRESHOLD, init=1.0, min=0.0, max=1.0)
```

Begin the resource with `$` and omit the quotation marks.

With this setup, FireAlpaca3 and MFGStudio will show the appropriate string when switching the language setting from Japanese to English.

Note that in MFG, only attributes and input widgets display strings, so internationalization applies only to these elements.

For attributes and input widgets, see [Attributes and Input Widgets](AttrWidget.md).

### `strings.json` format

The `strings.json` file has the following format:

```json
{
  "ja": {
    "LABEL_TITLE": "ハーフトーンディザ",
    "LABEL_THRESHOLD": "閾値"
  },
  "en": {
    "LABEL_TITLE": "Halftone Dither",
    "LABEL_THRESHOLD": "Threshold"
  }
}
```

Specify an ID and the corresponding string for each language.

### Supported languages

The supported languages are:

| Key | Description |
| ---- | ---- |
| zh_Hans | Chinese simplified |
| zh_Hant | Chinese traditional |
| en | English |
| fr | French |
| de | German |
| ja | Japanese |
| ko | Korean |
| pt | Portuguese |
| ru | Russian |
| es | Spanish |
| hi | Hindi |
| bn | Bengali |
| pl | Polish |

If the specified language is not available, `en` is used.

### Using string resources

You use string resources by replacing the entire quoted string with `$` plus the string ID.

For a title, if you have:

```mfg
@title "Halftone Dither"
```

replace it with:

```mfg
@title $LABEL_TITLE
```

For widgets like dropdowns, it looks like this:

```mfg
@param_i32 pattern(DROPDOWN, label=$LABEL_SHAPE, items=[$LABEL_SHAPE_SQUARE, $LABEL_SHAPE_SIN, $LABEL_NEWSPAPER, $LABEL_INTERLEAVE])
```

### Real-world examples

Many filters with internationalization examples are available in the MFG GitHub projects, so check them out for reference.

[MFG/projects](https://github.com/karino2/MFG/tree/main/projects)

## Image textures

If a project contains a PNG file, it can be used as a texture from MFG.

For example, if you place an image file named `4rect_texture16.png` in the project and define it like this, it can be used as a tensor named `lts`:

```mfg
  def lts by load("4rect_texture16.png")
```

Pass the filename to `load`.

At present, PNGs are always loaded as tensors with element type `u8v4` in BGRA order.

## Thumbnails

If a `thumbnail.png` file is present in a MAR, FireALpaca SE 3.0 and similar applications use it as the filter thumbnail.

`thumbnail.png` should be a 512x512 PNG image.

In MFGStudio's file explorer, the button above the panel can resize the current preview to 512x512 and include it as `thumbnail.png` in the project. Use this feature to include a thumbnail that clearly shows the filter's appearance.
