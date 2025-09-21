# Lisp 1.5-like Interpreter

This project is an attempt to recreate **Lisp 1.5**.  
It is based on a small C Lisp interpreter and aims to provide a Lisp 1.5-like experience.

## Installation

Compile and install using:

```bash
make
sudo make install
```

This will create an executable named lisp in /usr/local/bin.
Usage

Start the interpreter by running:
```
lisp
```
Exit the interpreter by typing:
```
(quit)
```

functions

```
LISP 1.5
> oblist
(nil t plus minus times quotient add1 sub1 quit hdmp car cdr cons list length append eq null atom oblist gbc read eval apply print prin1 greaterp smallerp zerop onep numberp symbolp listp assoc set quote setq defun lambda macro if begin cond)
> 

```

## Notes

This is not a full Lisp 1.5 implementation, but a simplified version that captures the feel of the original.

Dynamic scoping is used.

Core functions like QUOTE, ATOM, EQ, CONS, CAR, CDR, and COND are implemented.