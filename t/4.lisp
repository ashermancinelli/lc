(defvar a
  (+ 1 2 3))

; This is a comment
; everything until the end of the line will be ignored
(defvar (a
          (sum 1 2 3)
          ))

; Check that binops like these are replaced with the bulitins
(let 
  (a (- 1 2 3))
  (b (* 4 5 6))
  (c (/ 1 2)))
