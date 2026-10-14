# meeplang
Why is it called meep you say? meep.

## Types
integer = ('u' | 's') ('8' | '16' | '32' | '64')
floating point = 'f' ('32' | '64')
pointer / array = '(' <type> ')'
struct
union

## Variables
declaration = 'let' 'mutable'? <identifier> '=' (<type> | <value>)
assignment = <identifier> '=' <value>

## Operators
logical = '!!' | '&&' | '||' | '^^'
bitwise = '!' | '&' | '|' | '^' | '|<' | '>|' | '<<' | '>>' | '#'
arithmetic = '+' | '-' | '*' | '/' | '%'
comparison = '<' | '<=' | '==' | '>=' | '>' | '!='
pointer = '?' | '$' | ('[' <value> ']')
parenthesis = '(' | ')'
list separator = ','
access = '.'

## Control Flow
'if' <expression>
    <block>
('or' <expression>
    <block>
)*
('else'
    <block>
)?

'loop' <identifier> ('while' <expression>)? (',' <assigment>)?
    <block>

skip iteration = 'skip' <identifier>
break loop = 'break' <identifier>

## Functions
'fn' <identifier> '(' (<declaration> ...) ')'
    <block>

return statment = 'return' <expression>
