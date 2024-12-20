# Notes

This grammar consists of a non-terminal followed by exactly one rule. The rule
consists of one or more functions. Functions can contain other functions, which
are nested to any depth.

For example the rule:
```

name1 {
    name2 |
    name3
}

read as

name1 is 
EXACTLY_ONE(
    OR(
        EXACTLY_ONE(name2),
        EXACTLY_ONE(name3)))

```
and where
```

name1 {
    name2 name3 |
    name4
}

name1 is 
EXACTLY_ONE(
    OR(
        EXACTLY_ONE(
            EXACTLY_ONE(name2),
            EXACTLY_ONE(name3))),
        EXACTLY_ONE(name4))

```

and where

```
name1 {
    '(' (name2 (',' name3)*)? ')'
}

read as

name1 is 
EXACTLY_ONE(
    EXACTLY_ONE('('),
    ZERO_OR_ONE(
        EXACTLY_ONE(name2),
        ZERO_OR_MORE(
            EXACTLY_ONE(','),
            EXACTLY_ONE(name3))),
    EXACTLY_ONE(')'))
            

```
.
