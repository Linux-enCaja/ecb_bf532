(define list (lambda n n))

(define (sublis l exp)
  (if (cons? exp)
      (cons (sublis l (car exp))
	    (sublis l (cdr exp)))
      (let ((cell (assq exp l)))
	(if cell (cdr cell) exp))))

(define (cadr x) (car (cdr x)))
(define (caddr x) (car (cdr (cdr x))))
(define (cdddr x) (cdr (cdr (cdr x))))

(define (replace before after)
  (set-car! before (car after))
  (set-cdr! before (cdr after))
  after)

(define (push-macro form)
  (replace form
	   (list 'set! (caddr form)
		 (list 'cons (cadr form) (caddr form)))))

(define (pop-macro form)
  (replace form
	   (list 'let (list (list 'tmp (cadr form)))
		 (list 'set! (cadr form) '(cdr tmp))
		 '(car tmp))))

(define push 'push-macro)
(define pop 'pop-macro)

(define setq set!)

(define progn begin)

(define the-empty-stream ())

(define empty-stream? null?)

(define head car)

(define (tail x)
  (if (car (cdr x))
      (car (cdr (cdr x)))
      (let ((value ((car (cdr (cdr (cdr x)))))))
	(set-car! (cdr x) t)
	(set-car! (cdr (cdr x)) value))))

(define (enumerate-interval low high)
  (if (> low high)
      nil
      (cons low (enumerate-interval (+ low 1) high))))

(define (print-stream-elements x)
  (if (empty-stream? x)
      ()
      (begin (print (head x))
	     (print-stream-elements (tail x)))))

(define (standard-fib x) 
    (if (< x 2) 
        x 
        (+ (standard-fib (- x 1)) 
           (standard-fib (- x 2)))))
(standard-fib 10)

(console)
