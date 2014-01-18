std-format
==========

Sample implementation of my concept for the [std-proposals@isocpp.org](https://groups.google.com/a/isocpp.org/forum/?fromgroups#!topic/std-proposals/89RQz9BP7mY) forum regarding IOstream.

It is far from complete and still has unfinished parts and room for improvement but it can already be used to do very basic formatting.

At the core lies the guarantee of a type-safe system as provided by `ostream` but which can be fed using format strings just as easily as `printf()` and makes undefined behavior, buffer overflows and similar adventures often associated with `printf()`-like methods much less likely.

The current implementation is templated on the types to be translated to strings. Therefore typical problems known from the `printf()`-family of methods, namely not mapping type specifiers correctly to the argument types at the call site, thus potentially resulting in disaster, are averted. Because of the templated approach the types are fixed and known at compile time (as opposed to the vararg situation), thus the type specifiers, like `%i`, `%s` or `%f` are redundant: the compiler knows exactly what type is present at which index. The only information that is needed in the format string is *how* to print a type.

The syntax used for this proof of concept is modeled after the `String.Format()` method of the .NET framework, albeit only a very small subset is currently used. Concretely this means the format syntax is:
```cpp
"Hello {0}!"
"Hello {0:asdf}!"
```

Instead of using a single character for marking format specifiers balanced, unnested braces are used. This makes it easier to detect ill-formed format string. Braces to be printed are escaped by duplicates (`"{{"` and `"}}"`). Contrary to the `printf()` format every specifier must provide a zero-based positional index determining which value in code it corresponds to. The index may optionally be followed by a colon, followed by formatting flags determining how the type is to be formatted (not yet used). Because each specifier has a clearly defined closing character any arbitrary user-provided string can be used for the flags, giving users the freedom of passing custom format specifiers for their own types. The runtime issues an error if the position index is out of range.
I want to emphasize that this syntax was mainly chosen for its simplicity and to have something to experiment with and needs to be specified at a later time.

### Formatting Interface

There are currently two usage strategies: inline formatting using a free standing `format()` method versus storing a format string (together with some state) in a `formatter` object.

The former has the advantage that it is very easy to use, just like `printf()` where all you do is call a function with a format string and parameters and the rest is done automatically. The latter is for cases where repeated (and frequent) usage of the same format string is expected, as `formatter` can do some preprocessing to make the actual printing more efficient.

The freestanding function is called simply as:
```cpp
auto str = format(format_string, a, b, c, d);
```

There are multiple overloads available for convenience, like printing directly to a stream or a string:
```cpp
format(cout, "{0}, {3}, {0}, {1}, {1}, {2}", a, b, c, d);
format(str, "{0}, {3}, {0}, {1}, {1}, {2}", a, b, c, d);
```
As you can see the number of format specifiers is not required to match the number of arguments provided. The only requirement is for each single positional index to be less than the number of arguments. The above is the convenience use case as there is no need to mess around with any template arguments. For the advanced uses one can create a `formatter` object:
```cpp
using Formatter = formatter<std::string, int, double, std::string, MyType>;
Formatter fmt{"{0}, {3}, {0}, {1}, {1}, {2}"};
str = fmt(a, b, c, d);
// or (for compatibility with generic code)
str = format(fmt, a, b, c, d);
```
The first template argument is how the format string should be stored inside `formatter` for later reference. This can be anything with a string-like interface and certain typedefs, for example `string_view` when you want to prevent the format string to be copied. The remainder is the list of types `operator()` accepts. Because the same argument can be formatted more than once all values are transformed to const reference when calling `operator()` (even rvalue refs) to prevent undefined behavior possibly resulting when reading a moved-from value multiple times.

Internally `formatter` parses the format string on construction and remembers the format arguments, their positions, flags, etc. thus saving this redundant work on subsequent invokations of `operator()`, potentially speeding up the transformation where a lot of text processing is involved.

### Formatting Values

So, how do the individual values get transformed to strings? This is very similar to how it is done with `ostream`, except it doesn't rely on strange `operator<<` syntax which is, from experience, something many C++ newcomers have problems with. Instead we rely on simple `to_string()` functions like the ones introduced in C++11 for the arithmetic types.

For the simple and naive case all one needs to define is a method with the following syntax placed in the necessary namespace to be found by argument dependent lookup (ADL):
```cpp
std::string to_string(const MyType& x);
```
This is the minimum required to make a custom type formattable. However it neither provides flexibility nor is it very efficient due to the temporary string allocated on every call. Imagine the not uncommon scenario of printing a compound type, invoking `to_string()` recursively for every member. That will pile up a load of temporary strings. Therefore the library looks for additional overloads and selects the most specialized one. These are (in descending order or precedence):
```cpp
void to_string(const MyType& x, std::streambuf& out, string_view flags);
void to_string(const MyType& x, string_view flags);
void to_string(const MyType& x, std::streambuf& out);
std::string to_string(const MyType& x);
```
The `flags` argument points to the substring located between the colon and the closing brace in the format string, enabling fully customized formatting directives. If one wants to avoid the temporary strings created by the single-argument overload then the ones accepting a `streambuf` allow outputting directly to the destination. The overloads accepting format flags have always precedence as correct output takes priority. 

*Note: for completeness sake the streambuf overloads should probably be templated on basic_streambuf&lt;CharT, Traits&gt;.*

### Open Issues

Well, there is a lot. From the top of my head:

- Error handling: exceptions? failbit/badbit?
- Locales (need to implement some stuff manually because the `put()` facet methods only work with streams)
- Preprocessing of format flags to skip repeated parsing in `formatter`.
- Is `streambuf` the correct choice? Probably should be a type that doesn't allow modification of existing content. Use an `OutputIterator` instead? Would it hurt performance when no longer able to output blocks of chars at once?
- Write the more efficient `to_string()` overloads for primitive types as they will get used a lot
- Discuss format string syntax
- Available format flags for the various builtin/std types

