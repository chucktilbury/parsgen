# simple_grammar

This is the grammar for the Simple programming language. It is a simplified
representation of a grammar.

## Reqular expressions

The regular expressions are used to specify loops and optional items. There is
no "regular expression" library.

* ``( item )?`` represents a single optional item
* ``( item )*`` represents an optional list of items
* ``( item )+`` represents a list of one or more items

An item can be any construct. For example ``( item ( ',' item )* )+``
