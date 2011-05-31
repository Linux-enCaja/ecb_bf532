(define yellow 1)
(define red 2)

(color yellow '(200 255 80 120 90 120))
(color red '(100 180 80 130 160 250))

(define (stop) (robot 5))
(define (left) (robot 4))
(define (right) (robot 6))
(define (forward)
    (begin
        (robot 8)
        (delay 500)
        (robot 5)
    )
)
(define (back)
    (begin
        (robot 2)
        (delay 500)
        (robot 5)
    )
)

(define (branch)
    (begin
        (set! x (range))
        (print x)
        (if (< x 30) (right) (forward))
    )
)

(while (not (signal))
    (branch)
)

(console)

