# Goal
The purpose of this project is to present the mechanisms of Lisp in the simplest possible form, and to faithfully reproduce them as a way of honoring Dr. John McCarthy.

## Startup
Start the system from the terminal by running:

```
lisp filename

e.g.
lisp punch.lsp
```

This simulates the original LISP 1.5 startup, where the system read punch cards from a deck.

## REPL
After startup, the system enters the REPL.
Enter S-expressions at the prompt.
To exit, use (quit).

## Comment
Comments are written in the original style as (comment ***).
see punch.lsp

## Builtin Functions
Displaying the variable oblist shows the names of built-in functions.
It contains most of the functions from LISP 1.5.

Example:

```
> oblist
(NIL T PLUS DIFFERENCE MINUS TIMES QUOTIENT DIVIDE ADD1 SUB1 MAX MIN
 RECIP REMAINDER EXPT QUIT HDMP CAR CDR CONS LIST REVERSE LENGTH APPEND NCONC RPLACA RPLACD MAPLIST MAPCON MAP EQ EQUAL NULL ATOM GBC READ
 EVAL APPLY PRINT PRIN1 GREATERP LESSP ZEROP ONEP MINUSP NUMBERP FIXP
 SYMBOLP LISTP ASSOC MEMBER SET NOT QUOTE SETQ DEFINE LAMBDA MACRO IF
 BEGIN COND AND OR COMMENT)
> 
```

## Sample Codes
In my reimplementation, I treat DEFINE as an FSUBR, so no explicit QUOTE is needed around lambda expressions.

```
(COMMENT THIS IS PUNCH CARD)

(DEFINE (
    
(FOO (LAMBDA (X) X))
    
(FACT (LAMBDA (N)
        (IF (EQ N 0)
            1
            (TIMES N (FACT (SUB1 N))))))

(FIB (LAMBDA (N)
        (COND ((EQ N 0) 0)
              ((EQ N 1) 1)
              (T (PLUS (FIB (SUB1 N)) (FIB (DIFFERENCE N 2)))))))

)) 

```

## Garbage Collection
Using (gbc t) enables output during garbage collection.
To disable it, use (gbc nil).
The system implements the classical mark-and-sweep algorithm.
You can force garbage collection by using (gbc 1).

## Funarg Problem
Due to dynamic scope, you can perform computational experiments on the funarg problem using mapping functions.

maplist mapcon map  see LISP1.5 user's manual

example:
```
> (setq x 2)
2
> (maplist '(1 2 3) (lambda (y) (plus x y)))
(3 4 5)

```
Note that the argument order of mapping functions is different from modern Lisp.

## Simplicity:
The system is written as simply as possible.
The entire implementation is about 2,300 lines in main.c alone.
It can serve as a reference for those interested in building their own LISP interpreter.

## Immediate values:
I made integers immediate values to save cells and improve speed. By setting the second-highest bit of the integer, they are treated as positive integers. Negative numbers are outside the cell area, so they remain immediate values as they are.

## Enjoy
For those who played with LISP 1.5 at the time, this will bring back nostalgic memories.
For those who learned LISP after Common Lisp, it may offer a fresh and surprising experience.
I believe there are joys and discoveries here that you won’t find in large “black-box” LISP implementations.