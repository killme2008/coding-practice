(ns rucus.parser
  (:require [clojure.set :as cs]
            [rucus.scanner :as sc]
            [rucus.bnf :as bnf]))

(defonce eof :eof)
(defonce epsilon :epsilon)
(defonce error :error)

(defn- resolve-beta [b]
  (or (:nt b)
      (:literal b)
      b))

(defn- first-set-of [beta f i]
  (get f (resolve-beta (nth beta i))
       #{}))

(defn reduce-beta [new a beta]
  (let [b (resolve-beta (first beta))
        rhs (atom (cs/difference (get new b #{})
                                 #{epsilon}))
        k (dec (count beta))
        i (atom 0)]
    (while (and
            (contains? (first-set-of beta new @i) epsilon)
            (<= @i (dec k)))
      (swap! rhs
             cs/union
             (cs/difference
              (first-set-of beta new (inc @i))
              #{epsilon}))
      (swap! i inc))
    (when (and (= @i k)
               (contains? (first-set-of beta new k)
                          epsilon))
      (swap! rhs
             cs/union
             #{epsilon}))
    (update new a cs/union @rhs)))


(defn first-set [{:keys [terminals non-terminals productions]}]
  (let [f (->> terminals
               (cons eof)
               (cons epsilon)
               (map (fn [a]
                      [a #{a}]))
               (into {}))
        f (merge f
                 (->> non-terminals
                      (map (fn [a]
                             [a #{}]))
                      (into {})))]
    (loop [old f
           new f]
      (if (or
           (not= old new)
           (= new f))
        (recur new
               (reduce
                (fn [new p]
                  (let [a (resolve-beta (first p))
                        beta (-> p nnext)
                        beta (if (sequential? beta)
                               beta
                               [beta])]
                    (reduce-beta new a beta)))
                new
                productions))
        new))))

(defn find-productions [productions nt]
  (filter (fn [p]
            (= (:nt (first p)))
            nt) productions))

(defn follow-set [first-set {:keys [terminals non-terminals productions start]}]
  (let [fls (->> non-terminals
                 (map (fn [nt]
                        [nt #{}]))
                 (into {}))
        fls (assoc fls (:nt start) #{eof})]
    (loop [old fls
           new fls]
      (if (or
           (not= old new)
           (= new fls))
        (recur new
               (reduce
                (fn [new p]
                  (let [a (resolve-beta (first p))
                        trailler (atom (get new a #{}))
                        beta (-> p nnext)
                        beta (if (sequential? beta)
                               beta
                               [beta])]
                    (->> beta
                         (reverse)
                         (reduce
                          (fn [new pp]
                            (if-let [nt (:nt pp)]
                              (let [new-new (assoc new nt
                                                   (cs/union
                                                    (get new nt #{})
                                                    @trailler))
                                    first-nt (get first-set nt #{})]
                                (if (contains? first-nt
                                               epsilon)
                                  (swap! trailler cs/union
                                         (cs/difference
                                          first-nt
                                          #{epsilon}))
                                  (reset! trailler first-nt))
                                new-new)
                              (do
                                (reset! trailler
                                        (get first-set (resolve-beta pp) #{}))
                                new)))
                          new))))
                new
                productions))
        new))))


(defn first-plus-set [first-set follow-set {:keys [productions]}]
  (->> productions
       (map (fn [p]
              (let [a (resolve-beta (first p))
                    beta (-> p nnext)
                    b1 (if (sequential? beta)
                         (first beta)
                         beta)
                    fsb (get first-set (resolve-beta b1) #{})]
                [p
                 (if (contains? fsb epsilon)
                   (cs/union fsb (get follow-set a #{}))
                   fsb)])))
       (into {})))

(defn expand-productions [ret nt beta]
  (if (= :or (second beta))
    (->> beta
         (reduce (fn [r pp]
                   (if (not= :or pp)
                     (expand-productions r nt (if (sequential? pp)
                                                pp
                                                [pp]))
                     r))
                 ret))
    (conj! ret
           (->> beta
                (cons :eq)
                (cons nt)))))

(defn expand-all-productions [productions]
  (->> productions
       (reduce (fn [ret p]
                 (let [nt (first p)
                       beta (-> p nnext first)
                       beta (if (sequential? beta)
                              beta
                              [beta])]
                   (expand-productions ret nt beta)))
               (transient []))
       (persistent!)))

(defn build-pred-table [first-plus-set
                        {:keys [productions non-terminals terminals]}]
  (let [table (->> non-terminals
                   (reduce (fn [m nt]
                             (->> terminals
                                  (reduce (fn [m t]
                                            (assoc-in m [nt t] error))
                                          m)))
                           {})
                   (atom))]
    (->> productions
         (map (fn [p]
                (let [fps (get first-plus-set p #{})
                      a (-> p first (resolve-beta))]
                  (->> fps
                       (map (fn [s]
                              (when (contains? terminals (resolve-beta s))
                                (swap! table assoc-in [a (resolve-beta s)] p))))
                       (dorun))
                  (when (contains? fps eof)
                    (swap! table assoc-in [a eof] p)
                    (swap! table assoc-in [a nil] p)))))
         (dorun))
    @table))

(declare parser-generator)

(defn- resolve-terminals [terminals productions]
  (let [rts (->> productions
                 (map (fn [p]
                        (let [beta (->> p nnext)]
                          (when (and (= 1 (count beta))
                                     (contains? terminals (resolve-beta
                                                           (first beta))))
                            {:name (keyword (resolve-beta (first p)))
                             :pattern (resolve-beta (first beta))}))))
                 (filter identity))]
    {:new-t (->> terminals
                 (remove (->> rts
                              (map :pattern)
                              (into #{})))
                 (cs/union (->> rts
                                (map :name)
                                (into #{})))
                 (into #{}))
     :tl-map rts}))

(defn- resolve-productions [terminals productions new-t]
  (->> productions
       (map (fn [p]
              (let [beta (->> p nnext)]
                (when-not (and (= 1 (count beta))
                               (contains? terminals (resolve-beta (first beta))))
                  p))))
       (filter identity)
       (clojure.walk/postwalk (fn [form]
                                (if (and (map? form)
                                         (:nt form)
                                         (contains? new-t (keyword (:nt form))))
                                  {:nt (keyword (:nt form))}
                                  form)))))

(defn resolve-non-terminals [terminals non-terminals]
  (->> non-terminals
       (remove terminals)
       (into #{})))

(defn parser [bnf-string]
  (let [terminals (atom #{})
        non-terminals (atom #{})
        start (atom nil)
        tree (bnf/syntax (sc/get-scanner bnf-string bnf/bnf-tokens))]
    (let [transformed (clojure.walk/postwalk
                       (fn [form]
                         (if (vector? form)
                           (case (first form)
                             :rule-name (let [nt (second form)]
                                          (swap! non-terminals conj nt)
                                          {:nt nt})
                             :equal :eq
                             :epsilon :epsilon
                             :literal-plus (second form)
                             :literal (if (= (second form) :epsilon)
                                        (next form)
                                        (apply hash-map form))
                             :term-plus (second form)
                             :term  (second form)
                             :list (if (= 1 (count (second form)))
                                     (-> form second first)
                                     (second form))
                             :expr (case (count form)
                                     2 (second form)
                                     (next form))
                             :rule (next form)
                             :syntax (do
                                       (when (nil? @start)
                                         (reset! start (-> form second ffirst)))
                                       (second form))
                             :string (let [s (second form)]
                                       (swap! terminals conj s)
                                       s)
                             :pattern (let [p (second form)]
                                        (swap! terminals conj p)
                                        p)
                             form)
                           form))
                       tree)
          productions (expand-all-productions transformed)
          {:keys [new-t tl-map]} (resolve-terminals @terminals productions)
          cfg {:productions (resolve-productions @terminals productions new-t)
               :start @start
               :terminals  new-t
               :tl-map tl-map
               :non-terminals (resolve-non-terminals @terminals @non-terminals)}
          fs (rucus.parser/first-set cfg)
          fls (rucus.parser/follow-set fs cfg)
          fps (rucus.parser/first-plus-set fs fls cfg)]
      (-> cfg
          (assoc
           :pred-table (rucus.parser/build-pred-table fps cfg))
          (parser-generator)))))

(def push conj)

(defn- make-lex-tokens [terminals tl-map]
  (cs/union
   tl-map
   (conj (->> terminals
              (map (fn [t]
                     (let [name (keyword (gensym))]
                       (when-not (keyword? t)
                         {:name name :pattern t}))))
              (filter identity)
              (into #{}))
         { :ignore true   :pattern #"\s"})))

(defn- find-pattern-by-token-name [tokens n]
  (or
   (->> tokens
        (filter (fn [{:keys [name pattern]}]
                  (= name n)))
        (first)
        (:pattern))
   n))

(defn placeholder? [x]
  (when (number? x)
    x))

(defn resolve-ast [ast]
  (->> ast
       (sort-by second (comparator
                        (fn [x y]
                          (compare (:__ts x)
                                   (:__ts y)))))
       (reverse)
       (map (fn [[k v]]
                 [k (if (map? v)
                      (if (and (:__ts v)
                               (:__token v))
                        (:__token v)
                        (->> v
                             (resolve-ast)))
                      v)]))
       (vec)))

(defn parser-generator
  [{:keys [productions pred-table terminals non-terminals start tl-map]}]
  (fn [language]
    (let [tokens (make-lex-tokens terminals tl-map)
          r (sc/get-scanner language tokens)
          stack (atom [eof start])]
      (loop [ret {}
             kseq []
             focus (resolve-beta (peek @stack))
             word (let [tn (:token-name (sc/peek r nil))]
                    (if (contains? terminals tn)
                      tn
                      (:consumed (sc/peek r nil))))]
        (cond
          ;;the end
          (and
           (= focus eof)
           (sc/eof? r)) (resolve-ast ret)
          ;;matches terminal word
          (and
           (contains? terminals focus)
           (bnf/matches? r focus)) (let [upper (atom false)]
                                     (swap! stack pop)
                                     (when-let [ts (placeholder? (peek @stack))]
                                       (swap! stack pop)
                                       (reset! upper ts))
                                     (recur
                                      (assoc-in ret (conj  (if @upper
                                                             (doall
                                                              (reduce (fn [ks _]
                                                                        (pop ks))
                                                                      kseq
                                                                      (range @upper)))
                                                             kseq)
                                                           (keyword focus)) {:__token (bnf/matches r word)
                                                                             :__ts (System/nanoTime)})
                                      (if @upper
                                        (doall
                                         (reduce (fn [ks _]
                                                   (pop ks))
                                                 kseq
                                                 (range @upper)))
                                        kseq)
                                      (resolve-beta (peek @stack))
                                      (let [tn (:token-name (sc/peek r nil))]
                                        (if (contains? terminals tn)
                                          tn
                                          (:consumed (sc/peek r nil))))))
          :else
          (do
            (if-let [p (let [x (get-in pred-table
                                       [focus word])]
                         (when-not (= x error)
                           x))]
              (let [beta (-> p nnext)
                    upper (atom false)
                    beta (if (sequential? beta)
                           beta
                           [beta])]
                (swap! stack pop)
                (when-let [ts (placeholder? (peek @stack))]
                  (swap! stack pop)
                  (reset! upper ts))
                (when-let [x (seq (filter #(not= % epsilon)
                                          beta))]
                  (swap! stack push (count x)))
                (->> beta
                     (reverse)
                     (map (fn [b]
                            (when-not (= b epsilon)
                              (swap! stack push b)
                              ())))
                     (dorun))
                (recur
                 ret
                 (if @upper
                   (conj (pop kseq) (keyword focus))
                   (conj kseq (keyword focus)))
                 (resolve-beta (peek @stack))
                 (let [tn (:token-name (sc/peek r nil))]
                   (if (contains? terminals tn)
                     tn
                     (:consumed (sc/peek r nil))))))
              (bnf/report-error r (str "Can't expand " focus)))))))))

(defn apply-op [nt form]
  form)

(defn mytest [& [keys]]
  (let [ast ((parser " expr = term expr-plus
       expr-plus = \"+\" term expr-plus | \"-\" term expr-plus | :e
       term = factor term-plus
       term-plus = \"*\" factor term-plus | \"/\" factor term-plus | :e
       factor = \"(\" expr \")\" | num | name
       num = #\"[1-9][0-9]*\"
       name = #\"[a-zA-Z]+\"")
             "abc+3-(d-c*d)")
        transformer (fn [form]
                      (if (vector? form)
                        (case (first form)
                          :* *
                          :+ +
                          :/ /
                          :- -
                          :expr (apply-op :expr (second form))
                          :term (second form)
                          :num (Integer/valueOf (second form))
                          :name (get keys (keyword (second form)))
                          :factor (if (= (keyword "(") (first (second form)))
                                    (-> form
                                        second
                                        (next)
                                        (butlast))
                                    (second form))
                          :expr-plus (apply-op :expr-plus (second form))
                          :term-plus (apply-op :term-plus (second form))
                          form)
                        form))]
    ast))
