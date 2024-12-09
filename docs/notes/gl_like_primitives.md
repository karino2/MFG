# OpenGLなどのプリミティブの採用検討メモ

Created: 2023-03-14 17:12:50

注: このページは実装時のメモです。現在サポートされているもの一覧としては[リファレンス](../Reference.md)を参照してください。


distance, normalizeなどを用いたプログラミングと同じ感じで出来るようにプリミティブを揃えていきたい。 MFGにはベクトル型などが無いのでこういうのを揃える気もあまりしていなかったが、書いていると二次元のベクトルを扱う機会は多いので、なんらかの形でベクトルの特別扱いはしたいと思った。

-   [OpenGL 4.x Reference Pages](https://registry.khronos.org/OpenGL-Refpages/gl4/html/)
-   [abs - Win32 apps - Microsoft Learn](https://learn.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-abs)
-   [Metal Shading Language Specification.pdf](https://developer.apple.com/metal/Metal-Shading-Language-Specification.pdf)

----

シェーダーより便利に書けるのはいいけれど、シェーダーより面倒な部分は無くしていきたい。

プリミティブを足すにあたり、新しいものを追加しやすくするための仕組みを考える必要があるかもしれない。 これはいくつか追加してみたあとにあとに考えた方がいいかもしれない。
        
argbとclampのCall化をしたので、この流れでプリミティブを追加していけばいい気もする。

----
    
ドキュメントを上から眺めながらとりあえずこのチケットでサポートしても良さそうなものを列挙していく。 基本的にはHLSLとGLの両方でプリミティブがあるもの。膨大なので全部はサポートしないかなぁ。

-   ceil
-   clamp
-   cross
-   degrees
-   distance
-   dot
-   faceforward
-   floor
-   fract
-   isinf
-   isnan
-   ldexp
-   length
-   log
-   log2
-   mix
-   mod
-   normalize
-   pow
-   radians
-   reflect
-   round
-   sign
-   smoothstep
-   step
-   trunc

サポートしなくてもいいかなぁ、と思ったもの

-   fma
-   noise
-   refract

----

実装しても良いと思っているものを少しカテゴリごとにまとめてみよう。

端数など

-   ceil
-   floor
-   fract
-   trunc
-   round

ベクトル関連

-   cross
-   distance
-   dot
-   faceforward
-   length
-   normalize
-   reflect

スカラーまたはベクトル

-   clamp
-   mix
-   sign
-   smoothstep
-   step

その他

-   degrees
-   radians
-   isinf
-   isnan
-   ldexp
-   log
-   log2
-   mod
-   pow

----
    
genTypeの定義はSection 8. [GLSLangSpec.4.40.pdf](https://registry.khronos.org/OpenGL/specs/gl/GLSLangSpec.4.40.pdf)

float, vec2, vec3, vec4の事で、floatのベクトルかスカラー。

----

degrees, radiansはMetalに無いので実装しない事に。

----

modもMetalとHLSLに無いので実装しない事に。modfは全てにあるので実装しても良い。

----

## 実装したもの

### 端数など

-   ceil
-   floor
-   fract
-   trunc
-   round

### ベクトル関連

-   distance
-   dot
-   length
-   normalize
-   cross

### スカラーまたはベクトル

-   clamp
-   mix
-   sign
-   smoothstep
-   step

### その他

-   isinf
-   isnan
-   ldexp
-   log
-   log2
-   pow （これは前から実装してあったままでベクトル対応はまだ）

## 実装しなかったも

### 要らないと思って実装しなかったもの

-   faceforward
-   fma
-   noise
-   reflect
-   refract

### まだ実装しなくていいか、と思って後回しにしたもの

-   modf
-   ldexp
-   frexp

### シェーダー環境によってはbuiltinで提供されていないので実装しなかったもの

-   degrees
-   radians
-   mod