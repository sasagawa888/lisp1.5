# Lisp 1.5-like Interpreter

This project is an attempt to recreate **Lisp 1.5**.  
It is based on a small C Lisp interpreter and aims to provide a Lisp 1.5-like experience.

## Installation

Compile and install using:

```bash
sudo make install
```

This will create an executable named lisp in /usr/local/bin.
Usage

Start the interpreter by running:
```
lisp punch.lsp
```
Exit the interpreter by typing:
```
(quit)
```

## Functions

```
> oblist
(NIL T PLUS DIFFERENCE MINUS TIMES QUOTIENT DIVIDE ADD SUB MAX MIN
 RECIP REMAINDER QUIT HDMP CAR CDR CONS LIST REVERSE LENGTH APPEND
 EQ EQUAL NULL ATOM GBC READ EVAL APPLY PRINT PRIN GREATERP LESSP
 ZEROP ONEP MINUSP NUMBERP FIXP SYMBOLP LISTP ASSOC MEMBER SET NOT
 QUOTE SETQ DEFUN LAMBDA MACRO IF BEGIN COND AND OR)

```

## Example

```
LISP 1.5
> (defun fact (n) (if (eq n 0) 1 (times n (fact (sub1 n)))))
t
> (fact 10)
3628800
> 

```


## Notes

This is not a full Lisp 1.5 implementation, but a simplified version that captures the feel of the original.

Dynamic scoping is used.

Core functions like QUOTE, ATOM, EQ, CONS, CAR, CDR, and COND are implemented.