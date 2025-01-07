# PARSGEN

This is a very simple parser generator for a generic programming language. It's a work in progress and I am actively working on it.

I decided to write this for two reasons. I am not happy with other parser generators such as ``YACC/Bison`` because they cannot parse constructs that I want to include in my programming language. 

For example:
```
module
	: compound_name {}
	| compound_reference {}
	;
	
compound_name
	: IDENTIFIER {}
	| compound_name '.' IDENTIFIER {}
	;
	
 compound_reference
	: compound_reference_element {}
	| compound_reference '.' compound_reference_element {}
	
compound_reference_element
	: array_reference {}
	| function_reference {}
	| compound_name {}
	;

...

```

I am also not happy with PEG style parser generators, such as  ``peg/leg`` and ``packcc``, even though they can parse a construct like the one above, because they require integrating the lexical analysis into the syntax of the grammar. To me this adds an unescessary level of complexity to the grammar and I have had a very hard time getting anything but the most trivial parsers to actually work. 

In addition, I enjoy the challenge of re-inventing the wheel. I like wheels very much and maybe someday I will invent a new one that people will like to use. If not, then the work itself is a sufficient reward.

## The Plan

I have basically thrown all of what I already know about parsers and parser generators out of the window and I have developed my own grammar format that I find easier to read and parse. The end goal is a recursive-descent that has backtracking and is simple to emit from the grammar and is completely human-readable. Right recursion is **not** supported and is not needed for the things that I want to use it for. 

### Output
The output of the generator is a library that has everything needed to input the text to be parsed and output an Abstract Syntax Tree (AST). The implementation is in ANSI C.

* Parser. A separate C file is generated for every non-terminal symbol in the grammar. The file contains all of the code to "recognize" the construct implemented by the non-terminal. A single header file is created that has all of the function prototypes so they can interact. The parser also has the public interface for the parser. This interface opens files and actually starts the parser. All of the other functions such as scanning symbols are hidden in the library.

* Scanner. The scanner is implemented using GNU Flex. This program and it's dependencies must be present in order to use the parser generator. An input file for Flex is generated along with the other transient files to generate a scanner. 

* AST. The code to traverse the AST is generated along with separate data structures that implement the non-terminal symbols in the grammar. Traversing the AST is done with user implemented code that the traverse function uses to perform functions before the node is traversed and after the node is traversed. All user code related to the actual implementation of their language is implemented in the context of traversing the AST.

There is no provision in the grammar for executing arbitrary code. The only output of a generated parse is the AST. Executing arbitrary code is done by traversing the AST using the functions provided. 

It's worthwhile to note that the output of the parser generator and the code that implements it are very similar. There is an example AST pass in the file ``regurge.c`` and ``regurge.h``. This simply prints out the grammar that was read in to verify that the parser and the traverse functions are actually working properly.

### Input Grammar

The format of a grammar is very simple and devoid of features intentionally. Both the scanner and the parser are defined by the grammar, but they are separate. Assumptions are made about spaces and new lines in the grammar and the only time that a space or new line is significant is to separate symbols.  You can take a look at the grammar that is accepted by the parser generator in the file ``parsgen-grammar.txt`` 

#### Comments

The input grammar supports comments that start with a ``#`` and run to the end of the line. 

#### Scanner Symbols

A ``SCANNER_SYMBOL`` is used to generate a "constructed" scanner object such as an identifier or a number. In the grammar a scanner symbol is defined as 
```
ALL_UPPER_CASE_WORD "followed by a regular expression"
```
The regular expression is enclosed in double quotes. The quotes are stripped off and the expression is copied verbatim into the scanner definition such that a symbol with the upper case prefix is returned to represent it.  In other words, a definition such as 
```
IDENTIFIER "[a-zA-Z][a-zA-Z0-9_]*" 
```
Will generate code that looks like
```
...
[a-zA-Z][a-zA-Z0-9_] {add_token(yytext); return IDENTIFIER;}
...
```
A scanner symbol is always significant to the parser and the text of the symbol as well as it's type is returned in the token.  The format of scanner symbols is not configurable from the grammar and they must conform to the Flex style regular expression of `` [A-Z_][A-Z_0-9]* ``.

#### Scanner Names

A ``SCANNER_NAME`` is a constant name that is used by the parser to name constant constructs. A scanner name is defined in the context of the scanner rules and is always surrounded with single quotes. for example ``'while' 'if' and 'import'`` are all scanner names. They are returned as tokens and the text is returned but the name is returned "decorated" to be all caps and with the string "_TOKEN" appended to make it different from possible SCANNER_SYMBOLS. Scanner names are not configurable from the grammar input and always conform to the regular expression `` [a-zA-Z_][a-zA-Z0-9_]+ ``

#### Scanner Operators

A ``SCANNER_OPERATOR`` is a constant name that only includes punctuation. It is always surrounded in single quotes. The object is decorated with the name of the punctuation. For example the operator ``'>='`` is decorated to be ``CPBRACE_EQUAL_TOKEN``.  The scanner regex for a scanner operator is ``\'[^a-zA-Z_\']+\'``. 

#### Non Terminal Symbols

A non-terminal symbol conforms to the scanner expression of ``[a-zA-Z_][a-zA-Z_0-9]*``

### Grammar Rules

A grammar rule is very simple. It is nothing more than a list of terminal and non-terminal symbols that are grouped together using functions. A function is roughly analogous to a regular expression, but simplified. 

#### General Format

The general format of a rule begins with a non-terminal symbol followed by a list of terminal and non-terminal symbols that are enclosed in curly braces. 

For example:
```
rule_name { THIS is THE rule body }
another_rule {another body}
```
The rules in the rule body are augmented by a notion of functions. These functions are used to change the precedence and implement option and looping constructs similar to that of a regular expression.

In a rule body, the notion of ``and`` is implied. For example in the second example above ``{another body}`` could be read as ``another _rule is a match of the group of "another" and a "body"``. Note that a grouping is implied by the ``{}`` operators so that the whole rule is considered to be a group, but they cannot be used interchangeably with parentheses.

##### Grouping

A grouping is a function that binds a list of items together so that they have a higher precedence than they otherwise would. Groups are indicated by enclosing the list in parentheses. 

For example:
```
non_terminal { something (else is here) like an egg }
```
This (nonsense) rule specifies that a ``something`` followed by a group that exactly matches the three symbols ``else is here``. The grouping is parsed as if it was a single entity and the AST traverses it like a group.  

##### Or Function

An or function is introduced with pipe or bar punctuation character, ``|``.  The ``or`` function also implies grouping. 

For example:
```
# implies (one and two and three and four)
rule_name { one two | three four }
# implies ((one and two) or (three and four))
rule_name { one (two | three) four }
```

##### One Or Zero Function

An one or zero function (AKA "optional") is introduced with the question mark punctuation character, ``?``.  The optional operator appears before the object it operates on and does not imply any grouping.

For example:
```
# match {one} or {one two} 
rule_name { one ?two }
# match {one four} or {one two three four}
rule_name { one ?( two three) four }
```

##### One Or More Function

A one or more function is introduced with the plus punctuation character, ``+``.   The optional operator appears before the object it operates on and does not imply any grouping.

For example:
```
# match {one two}  or {one two two two} but not {one} 
rule_name { one +two }
# match {one four} or {one two three four}  or {one two three two three four}
rule_name { one +( two three) four }
```


##### Zero Or More Function

An zero or more function is introduced with star punctuation character, ``*``.   The optional operator appears before the object it operates on and does not imply any grouping.

For example:
```
# match {one} or {one two}  or {one two two two}
rule_name { one *two }
# match {one four} or {one two three four}  or {one two three two three four}
rule_name { one *( two three) four }
```

## Building

Building the library should be as simple as typing ``make`` if you have development tools installed. You can include the ``parser.h`` header file in your own application and link to the library and that should satisfy most situations. I have intentionally kept the size of the code to a minimum and implemented it in a pedantic style to make it easy to modify. If you have problems or questions, feel free to drop a bug here and I will respond as quickly as I can.


