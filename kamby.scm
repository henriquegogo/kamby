(define (tokenize script-text)
  (let ((tkns (list "")) (in-str? #f))

    (define (append-str! str)
      (set! tkns (cons (string-append (car tkns) str) (cdr tkns))))

    (define (add-tkn! . strs)
      (if (string=? (car tkns) "") (set! tkns (cdr tkns)))
      (set! tkns (append (reverse strs) tkns)))

    (define (handle-char char)
      (let ((str (string char)))
        (if (and in-str? (not (eq? char #\"))) (append-str! str)
          (case char
            ((#\") (set! in-str? (not in-str?)) (append-str! str))
            ((#\( #\[ #\{) (add-tkn! str ""))
            ((#\; #\) #\] #\}) (add-tkn! str))
            ((#\space #\newline) (add-tkn! ""))
            (else (append-str! str))))))

    (for-each handle-char (string->list script-text))
    (reverse tkns)))

(define (transpile old-tkns)
  (let ((tkns (list "(")))

    (define (add-tkn! . strs)
      (set! tkns (append (reverse strs) tkns)))

    (define (swap lst)
      (cons (cadr lst) (cons (car lst) (cddr lst))))

    (define (handle-tkn tkn)
      (cond
        ((string=? tkn "") 0)
        ((string=? tkn "{") (if (string=? (car tkns) ")\n(")
                              (set! tkns (cdr tkns))) (add-tkn! "\n( begin\n("))
        ((string=? tkn "}") (set! tkns (cdr tkns)) (add-tkn! ") )" ")\n("))
        ((string=? tkn "[") (add-tkn! "( list"))
        ((string=? tkn "]") (add-tkn! ")"))
        ((string=? tkn ";") (add-tkn! ")\n("))
        ((string=? tkn "=") (add-tkn! "=!") (set! tkns (swap tkns)))
        ((member tkn (list "==" "!=" "&&" "||" "+" "-" "*" "/"
                           "<" ">" "<=" ">="))
         (add-tkn! tkn) (set! tkns (swap tkns)))
        (else (add-tkn! tkn))))

    (for-each handle-tkn old-tkns)
    (set! tkns (cdr tkns))
    (add-tkn! ")")
    (apply string-append(map (lambda (s) (string-append s " ")) (reverse tkns)))))

(define input-text
  (let loop ((char (read-char)) (acc ""))
    (if (eof-object? char) acc
      (loop (read-char) (string-append acc (make-string 1 char))))))

(define (eval-string str)
  (let ((port (open-input-string str)))
    (do ((expr (read port) (read port)))
      ((eof-object? expr))
      (eval expr (interaction-environment)))))

; Language extensions
(eval-string "\
(define-syntax ==
  (syntax-rules () ((_ a b) (= a b))))

(define-syntax !=
  (syntax-rules () ((_ a b) (not (= a b)))))

(define-syntax &&
  (syntax-rules () ((_ a b) (and a b))))

(define-syntax ||
  (syntax-rules () ((_ a b) (or a b))))

(define-syntax =!
  (syntax-rules () ((_ key val) (set! key val))))

(define-syntax var
  (syntax-rules (=!)
    ((_ =! key val) (define key val))
    ((_ key) (define key))))

(define-syntax func
  (syntax-rules () ((_ key args body) (define (key . args) body))))

(define-syntax print
  (syntax-rules () ((_ arg ...) (begin (display arg) ... (newline)))))

(define-syntax include-script
  (syntax-rules ()
    ((_ path)
     (let ((file-content (call-with-input-file path get-string-all)))
       (eval-string (transpile (tokenize file-content)))))))
\n")

(eval-string (transpile (tokenize input-text)))