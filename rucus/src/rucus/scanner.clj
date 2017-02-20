(ns rucus.scanner
  (:import (java.io StringReader))
  (:require [cljcc :refer [make-lexer]]))

(defprotocol Scanner
  (eof? [this] "Return true if the scanner reach the end.")
  (next-matches? [this pattern] "Return true when the scanner has next which
                                 matches the pattern.")
  (peek [this pattern] "Return current token by pattern.")
  (pop [this pattern] "Read out the current token from reader by pattern,
                        and return it.")
  (return [this t] "Return token to the reader."))

(defprotocol ScannerFactory
  (get-scanner [this tokens]))

(defn- matches-pattern [word p]
  (when (:consumed word)
    (if p
     (if (keyword? p)
       (when (= (:token-name word) p)
         word)
       (when (re-matches p (:consumed word))
         word))
     word)))

(deftype TokenSeqScanner [ws tokens]
  Scanner
  (eof? [this]
    (empty? @ws))
  (next-matches? [this pattern]
    (peek this pattern))
  (peek [this pattern]
    (matches-pattern (first @ws) pattern))
  (pop [this pattern]
    (let [x (peek this pattern)]
      (swap! ws next)
      x))
  (return [this t]
    (swap! ws #(cons t %))))

(extend-type String
  ScannerFactory
  (get-scanner [s tokens]
    (TokenSeqScanner. (atom (vec
                             (butlast
                              ((make-lexer tokens)
                               s))))
                      tokens)))

(extend-type clojure.lang.IPersistentVector
  ScannerFactory
  (get-scanner [v tokens]
    (TokenSeqScanner. (atom v)
                      tokens)))

(extend-type java.io.File
  ScannerFactory
  (get-scanner [file]))
