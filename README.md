# Meep lang
An attempt to write a basic compiler.\
Why is it called meep you say? Meep.

## Data types
Boolean: bool (true == 1 or false == 0, bools are really u8)\
Signed integers: i8, i16, i32, i64\
Unsigned integers: u8, u16, u32, u64\
Floating point: f32, f64\
Arrays: \<type>[]\
References: ref \<type>, ref (ref \<type>), etc.\
(arrays and refs are really i64 memory addresses)

## Integer literals
Binary literal: 0b\<binary digits>\
Octal literal: 0o\<octal digits>\
Hex literal: 0x\<hex digits>\
Decimal literal: \<decimal digits>

## Variables
\<type> \<name>; \# Undefined value\
\<type> \<name> = \<value>;\
\<name> = \<value>; # Variable must be declared first\
Names must consist of only underscores and alphanumeric ASCII characters. The first character may not be a digit however!

## Arrays
\<type>[\<length>] myArray;\
\<type>[] myArray = \<initializer>;\
To get the length of an array, use its .length property

## Array initializers
Either a reference or a fancy initializer list like these:\
[..."Hello, World!", 0] -> [72, 101, 108, 108, 111, 44, 32, 87, 111, 114, 108, 100, 33, 0]\
[..."foo", 17.2, -1, ...[empty] * 2, ...[1, 2, 3] * 3, ..."a" * 2] -> [102.0, 111.0, 111.0, 17.2, -1.0, empty, empty, 1.0, 2.0, 3.0, 1.0, 2.0, 3.0, 1.0, 2.0, 3.0, 97.0, 97.0]\
[...[1, 2, 3] * 3] -> [1, 2, 3, 1, 2, 3, 1, 2, 3]

## Get/set array elements
item = myArray[index];\
myArray[index] = item;

## Referencing/dereferencing
ref \<type> myRef = ?\<variable>;\
\<type> value = $myRef;

## Type casts
reinterp\<value, type>: reinterprets the bits of \<value> as \<type>\
trunc\<value, type>: if demoting \<value>, high bits are discarded\
clamp\<value, type>: casts \<value> to \<type>, clamping it to avoid overflow

## Expressions
+, -, *, /, %, f(x), &, |, ^, ~\
\>, \<, >=, \<=, ==, !=, &&, ||, ^^, !\
+=, -=, *=, /=, %=, &&=, ||=, ^^=, &=, |=, ^=\
(\<nested expression>)

## Conditionals
if (\<cond>) {\
&emsp;\<code>\
} elif (\<cond>) {\
&emsp;\<code>\
} else (\<cond>) {\
&emsp;\<code>\
}\
\
check (\<value>) {\
&emsp;case \<match1>:\
&emsp;&emsp;\<code>\
&emsp;case \<match2>:\
&emsp;&emsp;\<code>\
&emsp;...\
&emsp;default:\
&emsp;&emsp;\<code>\
}

## Loops
for (\<init>; \<cond>; \<update>) {\
&emsp;\<code>\
}\
\
while (\<cond>) {\
&emsp;\<code>\
}\
\
do {\
&emsp;\<code>\
} while (\<cond>);\
\
skip:\
&emsp;skip to the next iteration\
&emsp;jump to next condition in switch\
\
break:\
&emsp;break out of the current loop\
&emsp;break out of switch

## Functions
func myFunction(arg1, arg2, kwarg1=1, kwarg2=2) [-> \<type>] {\
&emsp;\<code>\
&emsp;return \<value>; \# Optional, if empty or no return, don't define return type\
}

## Comments
\# Comment\
\
\# Comment comment\
\# Ranting comment\
\# Rant rant\
\# End of ranting comment

## Includes/imports
include \<path1>[, \<path2>[, \<path3>[...]]]