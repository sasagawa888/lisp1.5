(comment this is punch card)

(defun foo (x) x)

(defun fact (n)
    (if (eq n 0)
        1
        (times n (fact (sub1 n)))))

(defun fib (n)
    (cond ((eq n 0) 0)
          ((eq n 1) 1)
          (t (plus (fib (sub1 n)) (fib (difference n 2))))))