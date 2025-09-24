
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