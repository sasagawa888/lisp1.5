; Funarg Problem
; ex1
(DEFINE ((X (QUOTE IGNORED))))

(DEFINE ((MAPLIST
  (LAMBDA (X F)
    (COND ((EQ X NIL) NIL)
          (T (CONS (F (CAR X)) (MAPLIST (CDR X) F))))))))

(MAPLIST (QUOTE (1 2 3)) (LAMBDA (Y) (CONS Y X))) 
;=>((1 1 2 3) (2 2 3) (3 3))

;ex2
(DEFINE ((P (LAMBDA (X) (QUOTE OOPS)))))

(DEFINE ((COMPLEMENT (LAMBDA (P) (LAMBDA (X) (NOT (P X)))))))

;((COMPLEMENT ATOM) (QUOTE  FOO     )) ==> NIL
;((COMPLEMENT ATOM) (QUOTE (FOO BAR))) ==> NIL

