;verify

(DEFINE (
    (TEST (LAMBDA (S X Y)
            (IF (EQUAL X Y)
                T
                (PROGN (PRIN1 S) (prin1 '->) (PRINT 'ERROR)))))
))


;; Numeric operation tests
(TEST 'PLUS (PLUS 1 2) 3)
(TEST 'DIFFERENCE (DIFFERENCE 5 3) 2)
(TEST 'TIMES (TIMES 2 4) 8)
(TEST 'QUOTIENT (QUOTIENT 7 2) 3)
(TEST 'DIVIDE (DIVIDE 7 2) '(3 1))
(TEST 'ADD1 (ADD1 4) 5)
(TEST 'SUB1 (SUB1 4) 3)
(TEST 'REMAINDER (REMAINDER 7 3) 1)
(TEST 'EXPT (EXPT 2 3) 8)
(TEST 'RECIP (RECIP 2.0) 0.5)

;; Comparison tests
(TEST 'EQ (EQ 1 1) T)
(TEST 'EQ (EQ 1 2) NIL)
(TEST 'EQUAL (EQUAL '(1 2) '(1 2)) T)
(TEST 'EQUAL (EQUAL '(1 2) '(2 1)) NIL)
(TEST 'NULL (NULL NIL) T)
(TEST 'ATOM (ATOM 1) T)
(TEST 'ATOM (ATOM '(1 2)) NIL)
(TEST 'NUMBERP (NUMBERP 1) T)
(TEST 'NUMBERP (NUMBERP 'A) NIL)
(TEST 'SYMBOLP (SYMBOLP 'A) T)
(TEST 'SYMBOLP (SYMBOLP 1) NIL)
(TEST 'LISTP (LISTP '(1 2)) T)
(TEST 'LISTP (LISTP 1) NIL)

;; List operation tests
(TEST 'CAR (CAR '(1 2 3)) 1)
(TEST 'CDR (CDR '(1 2 3)) '(2 3))
(TEST 'CONS (CONS 1 '(2 3)) '(1 2 3))
(TEST 'LIST (LIST 1 2 3) '(1 2 3))
(TEST 'REVERSE (REVERSE '(1 2 3)) '(3 2 1))
(TEST 'LENGTH (LENGTH '(1 2 3)) 3)
(TEST 'APPEND (APPEND '(1 2) '(3 4)) '(1 2 3 4))
(TEST 'NCONC (NCONC '(1 2) '(3 4)) '(1 2 3 4))

;; Conditional/control tests
(TEST 'IF (IF T 1 2) 1)
(TEST 'IF (IF NIL 1 2) 2)
(TEST 'PROGN (PROGN (EQ 1 1) 2) 2)

;; Macro/function tests
(TEST 'LAMBDA ((LAMBDA (X) (PLUS X 1)) 5) 6)
;(TEST 'MACRO T T) ;; Macro definition only checked in form

;; Print/utility tests
(TEST 'GENSYM (GENSYM) 'g00001) ;; Only checking form; Gxxxx unique symbol generation
(PROP 'ASDF 'A 1)
(TEST 'GET (GET 'ASDF 'A) 1) ;; PROP operation form check


