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
; THIS IS COMMENT

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

example:
```
(DEFINE ((X (QUOTE IGNORED))))

(DEFINE ((MAPLIST
  (LAMBDA (X F)
    (COND ((EQ X NIL) NIL)
          (T (CONS (F (CAR X)) (MAPLIST (CDR X) F))))))))

(MAPLIST (QUOTE (1 2 3)) (LAMBDA (Y) (CONS Y X))) -> ((1 1 2 3) (2 2 3) (3 3))

```
Note that the argument order of mapping functions is different from modern Lisp.

## Debug

### Trace/Untrace
example
```
LISP 1.5
> (trace 'fact)
T
> (fact 3)
ENTER FACT(3)
 ENTER FACT(2)
  ENTER FACT(1)
   ENTER FACT(0)
   RETURN FACT 1
  RETURN FACT 1
 RETURN FACT 2
RETURN FACT 6
6
> (untrace '(fact))
T
> (fact 3)
6
> 

```

### Stepper
- (step t)  Enable the stepper
- (step nil)　Disable the steppter

A prompt >> appears each time eval is called, waiting for user input. Pressing Enter continues to the next step, and entering q aborts the execution.

example:
```
LISP 1.5
> (step t)
T
> (foo 1)
(FOO 1) in [] >> 1 in [] >> 
FOO in [] >> 
X in [(X . 1)] >> 
1
> (step nil)
(STEP NIL) in [] >> NIL in [] >> 
STEP in [] >> 
T
> 
```

## Simplicity
The system is written as simply as possible.
The entire implementation is about 2,300 lines in main.c alone.
It can serve as a reference for those interested in building their own LISP interpreter.

## Immediate values:
I made integers immediate values to save cells and improve speed. By setting the second-highest bit of the integer, they are treated as positive integers. Negative numbers are outside the cell area, so they remain immediate values as they are.

## Enjoy
For those who played with LISP 1.5 at the time, this will bring back nostalgic memories.
For those who learned LISP after Common Lisp, it may offer a fresh and surprising experience.
I believe there are joys and discoveries here that you won’t find in large “black-box” LISP implementations.