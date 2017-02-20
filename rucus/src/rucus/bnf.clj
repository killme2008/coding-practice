(ns rucus.bnf
  "Parse BNF string, BNF's BNF:

    sytanx = rule syntax | rule
    rule = rule-name equal expr
    expr = list \"|\" expr | list
    list = term list | term
    term = \"[\" term' \"]\" | term'
    term' = literal  | rule-name
    literal = \"<\" literal' \">\" | literal'
    literal' = string | pattern | :e
    pattern = \"#\" string
    equal = \"=\" | \"::=\"
    rule-name = #\"[A-Za-z][A-Za-z0-9]*\"
    string=  \"\"\" double-quote-string-literal \"\"\"
  "
  (:refer-clojure :exclude [list])
  (:require [rucus.scanner :as sc]))

(declare syntax rule expr list term term-plus literal literal-plus
         rule-name pattern equal string rule-name
         epsilon)

(def bnf-tokens
  #{{ :name :lbracket    :pattern "[" }
    { :name :rbracket    :pattern "]" }
    { :name :equal    :pattern #"=|:="}
    { :name :or  :pattern "|"}
    { :name :epsilon  :pattern #":e"}
    { :name :lbrace    :pattern "<" }
    { :name :rbrace    :pattern ">" }
    { :name :lparen  :pattern "(" }
    { :name :rparen  :pattern ")" }
    { :name :newline  :pattern #"\n|\r\n" :priority 2}
    { :name :rule-name  :pattern #"[A-Za-z][A-Za-z0-9\-]*" :priority 1}
    { :name :string :pattern #"(\"([^\"\\]*|\\[\"\\bfnrt/]|\\u[0-9a-f]{4})*\")" :priority -1}
    { :name :pattern :pattern #"(\#\"([^\"\\]*|\\[\"\\bfnrt/]|\\u[0-9a-f]{4})*\")" :priority -1}
    { :ignore true   :pattern #"\s"}})

(defn pattern? [p]
  (instance? java.util.regex.Pattern p))

(defn ->pattern [s]
  (cond (pattern? s) s
        (string? s) (re-pattern (java.util.regex.Pattern/quote s))
        :else s))

(defn report-error
  [r & [msg]]
  (let [{:keys [consumed position]} (sc/peek r nil)]
    (throw (IllegalStateException.
            (str (or msg (format "Syntax error `%s`" consumed))
                 ", at: "
                 consumed
                 position)))))

(defn matches? [r s]
  (when-let [p (->pattern s)]
    #_(println "Matches" (sc/peek r nil) " to:" p)
    (sc/next-matches? r p)))

(defn starts-with? [r s]
  (cond
    (pattern? s) (sc/next-matches? r (re-pattern (str "(" s ").*")))
    (string? s) (sc/next-matches? r (re-pattern (str  s ".*")))
    :else
    false))

(defn matches [r p]
  (:consumed (sc/pop r (->pattern p))))

(defmacro ifm [test then else]
  (let [r (second test)
        s (-> test next second)]
    `(if ~test
       (do
         (matches ~r ~s)
         ~then)
       ~else)))

(defmacro whenm [test & body]
  `(ifm ~test
        (do ~@body)
        nil))


(defn syntax [r]
  (if-not (sc/eof? r)
    (if-let [re (rule r)]
      (if (sc/eof? r)
        [:syntax [re]]
        [:syntax (cons re (second (syntax r)))])
      (report-error r))
    (report-error r)))

(defn rule [r]
  (if-let [rn (rule-name r)]
    (if-let [eq (equal r)]
      (if-let [e (expr r)]
        (if (or (sc/eof? r)
                (matches? r :newline))
          (do
            (when (matches? r :newline)
              (matches r :newline))
            [:rule rn eq e])
          (report-error r "Expect an EOF or line end."))
        (report-error r "Expect an expr."))
      (report-error r "Expect \"=\" or \":=\""))
    (report-error r "Expect a rule name.")))

(defn expr [r]
  (if-let [l (list r)]
    (ifm (matches? r "|")
         (if-let [e (expr r)]
           [:expr l :or e]
           (report-error r "Expect an expr after \"|\""))
         [:expr l])
    (report-error r "Expect a list.")))

(defn list [r]
  (if-let [t (term r)]
    (if (starts-with? r #"\"|#|[a-zA-Z]|\[|<")
      (if-let [l (list r)]
        [:list (cons t (second l))]
        [:list [t]])
      [:list [t]])))

(defn term [r]
  (ifm (matches? r "[")
       (if-let [tp (term-plus r)]
         (ifm (matches? r "]")
              [:term tp :optional true]
              (report-error r "Expect \"]\""))
         (report-error r))
       (if-let [tp (term-plus r)]
         [:term tp]
         (report-error r))))

(defn term-plus [r]
  [:term-plus
   (if (starts-with? r #"[a-zA-Z]")
     (rule-name r)
     (literal r))])

(defn literal [r]
  (ifm (matches? r "<")
       (if-let [lp (literal-plus r)]
         (ifm (matches? r ">")
              [:literal lp :ignorable true]
              (report-error r "Expect \">\""))
         (report-error r))
       (when-let [lp (literal-plus r)]
         [:literal lp])))

(defn literal-plus [r]
  [:literal-plus
   (or (string r)
       (pattern r)
       (epsilon r)
       (report-error r))])

(defn epsilon [r]
  (when (matches? r :epsilon)
    [:epsilon (matches r :epsilon)]))

(defn rule-name [r]
  (when (matches? r #"[A-Za-z][A-Za-z0-9\-]*")
    [:rule-name (matches r #"[A-Za-z][A-Za-z0-9\-]*")]))

(defn- compile-pattern [p]
  (let [p (.substring p 2)
        p (.substring p 0 (dec (count p)))]
    (re-pattern p)))

(defn pattern [r]
  (when (matches? r :pattern)
    [:pattern (compile-pattern (matches r :pattern))]))

(defn equal [r]
  (when (matches? r #"=|::=")
    [:equal (matches r #"=|::=")]))

(defn- resolve-string [s]
  (.replaceAll s "\"" ""))

(defn string [r]
  (when (matches? r :string)
         [:string (resolve-string (matches r :string))]))

(defn string-literal [r]
  (when (matches? r #"([^\"\\]*|\\[\"\\bfnrt/]|\\u[0-9a-f]{4})*")
    (matches r #"([^\"\\]*|\\[\"\\bfnrt/]|\\u[0-9a-f]{4})*")))
