(comment this is punch card)

(define ((foo (lambda (x) x))
         (fact (lambda (n)
                    (if (eq n 0)
                        1
                        (times n (fact (sub1 n))))))
         (fib (lambda (n)
                    (cond ((eq n 0) 0)
                          ((eq n 1) 1)
                          (t (plus (fib (sub1 n)) (fib (difference n 2)))))))))
