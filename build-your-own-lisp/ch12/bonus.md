* Define a Lisp function that returns the first element from a list.

```
fun {getfirst x & y} {x}
fun {first x} {curry getfirst x}
first {1 2 3}
```

* Define a Lisp function that returns the second element from a list.

```
fun {second x} {first (tail x)}
second {1 2 3}
```

* Define a Lisp function that calls a function with two arguments in reverse order.

```
fun {rcall f x y} {f y x}
(rcall - 3 4)
```