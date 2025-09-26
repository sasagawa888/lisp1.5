
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

(SUM (LAMBDA (N)
        (PROG (I S)
             (SETQ I 1)
             (SETQ S 0)
             A (COND ((GREATERP I N) (RETURN S)))
            (SETQ S (PLUS S I))
            (SETQ I (ADD1 I))
            (GO A))))

(TARAI (LAMBDA (X Y Z)
         (IF (OR (LESSP X Y) (EQ X Y))
         Y
          (TARAI (TARAI (SUB1 X) Y Z)
                 (TARAI (SUB1 Y) Z X)
                 (TARAI (SUB1 Z) X Y)))))


)) 

