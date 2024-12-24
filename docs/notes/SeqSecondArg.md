# シーケンスの第２引数の調査検討

- Created: 2023-01-31 11:29:35

`rsum(-2:5)` などは、-2から5個、という意味だったが、RやPythonは5まで、という意味になる。

Python

```
>>> i = [x for x in range(5)]
>>> i
[0, 1, 2, 3, 4]
>>> i[2:3]
[2]
>>> i[2:4]
[2, 3]
```

[R: Colon Operator](https://stat.ethz.ch/R-manual/R-devel/library/base/html/Colon.html)

```
> 2:3
[1] 2 3
> 3:2
[1] 3 2
> 3:4
[1] 3 4
```

わかりにくいのでメジャーなこれらにそろえておく。PythonはendがexcusiveでRはinclusive。どっちがいいかなぁ。Pythonに揃えるかな？

----

Pythonのドキュメントへのリンク。 [Built-in Types — Python 3.11.1 documentation](https://docs.python.org/3/library/stdtypes.html#common-sequence-operations)

----

    
JavaScriptは単なる関数だがやはりend exclusive。 [ArrayBuffer.prototype.slice() - JavaScript - MDN](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/ArrayBuffer/slice)

----

Pythonはスライスとrangeが違う（というかrangeは無い） 多くの言語では..や...が使われるが、end exclusiveかどうかはかなり混乱している。

Swiftはexclusiveは`..<`を使い、inclusiveは`...`

-   [Range - Apple Developer Documentation](https://developer.apple.com/documentation/swift/Range)
-   [...(_:_:) - Apple Developer Documentation](https://developer.apple.com/documentation/swift/comparable/'...(_:_:))

Rustは逆に`..`がexclusiveで、inclusiveに`..=`を使う。

-   [for と range - Rust By Example 日本語版](https://doc.rust-jp.rs/rust-by-example-ja/flow_control/for.html)

exclusiveが明示的なswiftが一番クリアな気がする（inclusiveは今の所使わないので）

Golangもend exclusive

- [A Tour of Go](https://go.dev/tour/moretypes/7)