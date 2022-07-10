; Define a variable here
(defvar a 
  (+ 3 2))

(defvar b
  (* 3
     (+ 3 a)))

; Use it down here, see if the modified value is actually returned
; Should be 10
(printf "calculated value %f" a)
(puts "")

(0)
