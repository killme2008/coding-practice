#include "../mpc/mpc.h"
#include "../readline.h"
#include "../lval.h"

#define LASSERT(args, cond, err) do {                         \
    if (!(cond)) { lval_del(args); return lval_error(err); }  \
  }while(0)                                                   \

lval* builtin_op(lval* a, char* op) {

  /* Ensure all arguments are numbers */
  for (int i = 0; i < lval_sexpr_count(a); i++) {
    if (lval_sexpr_cell(a)[i]->type != LVAL_NUM) {
      lval_del(a);
      return lval_error("Cannot operate on non-number!");
    }
  }

  /* Pop the first element */
  lval* x = lval_pop(a, 0);

  /* If no arguments and sub then perform unary negation */
  if ((strcmp(op, "-") == 0) && lval_sexpr_count(a) == 0) {
    x->value.num = -x->value.num;
  }

  /* While there are still elements remaining */
  while (lval_sexpr_count(a) > 0) {

    /* Pop the next element */
    lval* y = lval_pop(a, 0);

    /* Perform operation */
    if (strcmp(op, "+") == 0) { x->value.num += y->value.num; }
    if (strcmp(op, "-") == 0) { x->value.num -= y->value.num; }
    if (strcmp(op, "*") == 0) { x->value.num *= y->value.num; }
    if (strcmp(op, "/") == 0) {
      if (y->value.num == 0) {
        lval_del(x); lval_del(y);
        x = lval_error("Division By Zero.");
        break;
      }
      x->value.num /= y->value.num;
    }

    /* Delete element now finished with */
    lval_del(y);
  }

  /* Delete input expression and return result */
  lval_del(a);
  return x;
}

lval* builtin_head(lval* a) {
  LASSERT(a, lval_sexpr_count(a) == 1,
          "Function 'head' passed too many arguments!");

  LASSERT(a, lval_sexpr_cell(a)[0]->type == LVAL_QEXPR,
          "Function 'head' passed incorrect types!");

  LASSERT(a, lval_sexpr_count(lval_sexpr_cell(a)[0]) != 0,
          "Function 'head' passed {}!");

  /* Otherwise take first argument */
  lval* v = lval_take(a, 0);

  /* Delete all elements that are not head and return */
  while (lval_sexpr_count(v) > 1) { lval_del(lval_pop(v, 1)); }
  return v;
}

lval* builtin_tail(lval* a) {
  /* Check Error Conditions */
  LASSERT(a, lval_sexpr_count(a) == 1,
          "Function 'tail' passed too many arguments!");

  LASSERT(a, lval_sexpr_cell(a)[0]->type == LVAL_QEXPR,
          "Function 'tail' passed incorrect types!");

  LASSERT(a, lval_sexpr_count(lval_sexpr_cell(a)[0]) != 0,
          "Function 'tail' passed {}!");

  /* Take first argument */
  lval* v = lval_take(a, 0);

  /* Delete first element and return */
  lval_del(lval_pop(v, 0));
  return v;
}

lval* builtin_list(lval* a) {
  a->type = LVAL_QEXPR;
  return a;
}

lval* lval_eval(lval* v);

lval* builtin_eval(lval* a) {
  LASSERT(a, lval_sexpr_count(a) == 1,
          "Function 'eval' passed too many arguments!");
  LASSERT(a, lval_sexpr_cell(a)[0]->type == LVAL_QEXPR,
          "Function 'eval' passed incorrect type!");

  lval* x = lval_take(a, 0);
  x->type = LVAL_SEXPR;
  return lval_eval(x);
}

lval* lval_join(lval *x, lval *y) {
  /* For each cell in 'y' add it to 'x' */
  while (lval_sexpr_count(y)) {
    x = lval_add(x, lval_pop(y, 0));
  }

  /* Delete the empty 'y' and return 'x' */
  lval_del(y);
  return x;
}

lval* builtin_join(lval* a) {
  for(int i = 0; i < lval_sexpr_count(a); i++){
    LASSERT(a, lval_sexpr_cell(a)[i]->type == LVAL_QEXPR,
            "Function 'join' passed incorrect type.");
  }

  lval* x = lval_pop(a, 0);

  while (lval_sexpr_count(a)) {
    x = lval_join(x, lval_pop(a, 0));
  }

  lval_del(a);
  return x;
}

lval* builtin_cons(lval* a) {
  LASSERT(a, lval_sexpr_count(a) == 2,
          "'cons' passed too many arguments!");

  LASSERT(a, lval_sexpr_cell(a)[1]->type == LVAL_QEXPR,
          "'cons' second argument expects a q-expression.");

  lval* x = lval_pop(a, 0);
  lval* y = lval_pop(a, 0);

  lval* v = lval_qexpr();
  v = lval_add(v, x);

  lval_del(a);
  return lval_join(v, y);
}

lval* builtin_init(lval* a) {
  LASSERT(a, lval_sexpr_count(a) == 1,
          "'init' passed too many arguments.");
  LASSERT(a, lval_sexpr_cell(a)[0]->type == LVAL_QEXPR,
          "'init' passed incorrect type.");


  lval* x = lval_sexpr_cell(a)[0];
  int c = lval_sexpr_count(x) - 1;

  lval* v = lval_qexpr();
  for(int i = 0 ; i < c; i++)
    v = lval_add(v, lval_pop(x, 0));

  lval_del(a);

  return v;
}

lval* builtin_len(lval* a) {
  LASSERT(a, lval_sexpr_count(a) == 1,
          "'len' passed too many arguments.");
  LASSERT(a, lval_sexpr_cell(a)[0]->type == LVAL_QEXPR,
          "'len' passed incorrect type.");

  lval* x = lval_num(lval_sexpr_count(lval_sexpr_cell(a)[0]));
  lval_del(a);
  return x;
}


lval* builtin(lval* a, char* func) {
  if (strcmp("list", func) == 0) { return builtin_list(a); }
  if (strcmp("head", func) == 0) { return builtin_head(a); }
  if (strcmp("tail", func) == 0) { return builtin_tail(a); }
  if (strcmp("join", func) == 0) { return builtin_join(a); }
  if (strcmp("cons", func) == 0) { return builtin_cons(a); }
  if (strcmp("init", func) == 0) { return builtin_init(a); }
  if (strcmp("len", func) == 0) { return builtin_len(a); }
  if (strcmp("eval", func) == 0) { return builtin_eval(a); }
  if (strstr("+-/*", func)) { return builtin_op(a, func); }
  lval_del(a);
  return lval_error("Unknown Function!");
}

lval* lval_eval_sexpr(lval* v) {

  /* Evaluate Children */
  for (int i = 0; i < lval_sexpr_count(v); i++) {
    lval_sexpr_cell(v)[i] = lval_eval(lval_sexpr_cell(v)[i]);
  }

  /* Error Checking */
  for (int i = 0; i < lval_sexpr_count(v); i++) {
    if (lval_sexpr_cell(v)[i]->type == LVAL_ERR) { return lval_take(v, i); }
  }

  /* Empty Expression */
  if (lval_sexpr_count(v) == 0) { return v; }

  /* Single Expression */
  if (lval_sexpr_count(v) == 1) { return lval_take(v, 0); }

  /* Ensure First Element is Symbol */
  lval* f = lval_pop(v, 0);
  if (f->type != LVAL_SYM) {
    lval_del(f); lval_del(v);
    return lval_error("S-expression Does not start with symbol.");
  }

  /* Call builtin with operator */
  lval* result = builtin(v, f->value.sym);
  lval_del(f);
  return result;
}

lval* lval_eval(lval* v) {
  /* Evaluate Sexpressions */
  if (v->type == LVAL_SEXPR) { return lval_eval_sexpr(v); }
  /* All other lval types remain the same */
  return v;
}

lval* lval_read_num(mpc_ast_t* t) {
  errno = 0;
  long x = strtol(t->contents, NULL, 10);
  return errno != ERANGE ?
    lval_num(x) : lval_error("invalid number");
}

lval* lval_read(mpc_ast_t* t) {

  /* If Symbol or Number return conversion to that type */
  if (strstr(t->tag, "number")) { return lval_read_num(t); }
  if (strstr(t->tag, "symbol")) { return lval_sym(t->contents); }

  /* If root (>) or sexpr then create empty list */
  lval* x = NULL;
  if (strcmp(t->tag, ">") == 0) { x = lval_sexpr(); }
  if (strstr(t->tag, "sexpr"))  { x = lval_sexpr(); }
  if (strstr(t->tag, "qexpr"))  { x = lval_qexpr(); }

  /* Fill this list with any valid expression contained within */
  for (int i = 0; i < t->children_num; i++) {
    if (strcmp(t->children[i]->contents, "(") == 0) { continue; }
    if (strcmp(t->children[i]->contents, ")") == 0) { continue; }
    if (strcmp(t->children[i]->contents, "}") == 0) { continue; }
    if (strcmp(t->children[i]->contents, "{") == 0) { continue; }
    if (strcmp(t->children[i]->tag,  "regex") == 0) { continue; }
    x = lval_add(x, lval_read(t->children[i]));
  }

  return x;
}

int main(int argc, char** argv) {

  mpc_parser_t* Number = mpc_new("number");
  mpc_parser_t* Symbol = mpc_new("symbol");
  mpc_parser_t* Sexpr  = mpc_new("sexpr");
  mpc_parser_t* Qexpr  = mpc_new("qexpr");
  mpc_parser_t* Expr   = mpc_new("expr");
  mpc_parser_t* Lispy  = mpc_new("lispy");

  mpca_lang(MPCA_LANG_DEFAULT,
            "                                                    \
      number : /-?[0-9]+/ ;                              \
      symbol : \"list\" | \"head\" | \"tail\" | \"cons\" | \"len\"   \
               | \"init\" | \"join\" | \"eval\" | '+' | '-' | '*' | '/' ;   \
      sexpr  : '(' <expr>* ')' ;                         \
      qexpr  : '{' <expr>* '}' ;                         \
      expr   : <number> | <symbol> | <sexpr> | <qexpr> ; \
      lispy  : /^/ <expr>* /$/ ;                         \
    ",
            Number, Symbol, Sexpr, Qexpr, Expr, Lispy);

  puts("Lispy Version 0.0.0.0.5");
  puts("Press Ctrl+c to Exit\n");

  while (1) {

    char* input = readline("lispy> ");
    add_history(input);

    mpc_result_t r;
    if (mpc_parse("<stdin>", input, Lispy, &r)) {
      lval* x = lval_eval(lval_read(r.output));
      lval_println(x);
      lval_del(x);
      mpc_ast_delete(r.output);
    } else {
      mpc_err_print(r.error);
      mpc_err_delete(r.error);
    }

    free(input);

  }

  mpc_cleanup(6, Number, Symbol, Sexpr, Qexpr, Expr, Lispy);

  return 0;
}
