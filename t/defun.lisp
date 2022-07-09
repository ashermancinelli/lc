; External declaration
; (import (sin a))

; Function definition
(defun (plus5 a)
  (+ a 5))

(defun (times a b)
  (* a b))

(+
  (times 5 4)
  (plus5 3))
