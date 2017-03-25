#include "../mpc/mpc.h"
#include "../readline.h"
#include "../lval.h"

mpc_parser_t* Number;
mpc_parser_t* Symbol;
mpc_parser_t* String;
mpc_parser_t* Comment;
mpc_parser_t* Sexpr;
mpc_parser_t* Qexpr;
mpc_parser_t* Expr;
mpc_parser_t* Lispy;

lval* builtin_order(lenv* e, lval* a, char* op) {

  LASSERT_NUM(op, a, 2);

  /* Ensure all arguments are numbers */
  if(strcmp(op, "==") && strcmp(op, "!=")){
    for (int i = 0; i < lval_sexpr_count(a); i++) {
      LASSERT_TYPE(op, a, i, LVAL_NUM);
    }
  }

  lval* x = lval_pop(a, 0);
  lval* y = lval_pop(a, 0);

  int r = 0;
  if(strcmp(op, ">") == 0){
    r = (x->value.num >  y->value.num);
  } else if (strcmp(op, ">=") == 0){
    r = (x->value.num >=  y->value.num);
  } else if(strcmp(op, "<") == 0){
    r = (x->value.num <  y->value.num);
  } else if(strcmp(op, "<=") == 0){
    r = (x->value.num <=  y->value.num);
  } else if(strcmp(op, "==") == 0){
    r = lval_eq(x, y);
  } else if(strcmp(op, "!=") == 0){
    r = !lval_eq(x, y);
  }

  lval_del(x);
  lval_del(y);
  lval_del(a);
  return lval_num(r);
}

lval* builtin_gt(lenv* e, lval* a) {
  return builtin_order(e, a, ">");
}

lval* builtin_gte(lenv* e, lval* a) {
  return builtin_order(e, a, ">=");
}

lval* builtin_lt(lenv* e, lval* a) {
  return builtin_order(e, a, "<");
}

lval* builtin_lte(lenv* e, lval* a) {
  return builtin_order(e, a, "<=");
}

lval* builtin_eq(lenv* e, lval* a) {
  return builtin_order(e, a, "==");
}

lval* builtin_neq(lenv* e, lval* a) {
  return builtin_order(e, a, "!=");
}

lval* builtin_op(lenv* e, lval* a, char* op) {

  /* Ensure all arguments are numbers */
  for (int i = 0; i < lval_sexpr_count(a); i++) {
    LASSERT_TYPE(op, a, i, LVAL_NUM);
  }

  /* Pop the first element */
  lval* x = lval_pop(a, 0);

  /* If no arguments and sub then perform unary negation */
  if ((strcmp(op, "-") == 0) && lval_sexpr_count(a) == 0) {
    x->value.num = -x->value.num;
  }

  if(strcmp(op, "!") == 0) {
    LASSERT(a, lval_sexpr_count(a) == 0,
            "Function '%s' passed incorrect number of arguments. Got %i, Expected %i.",
            op, lval_sexpr_count(a), 1);
    x->value.num = !x->value.num;
  }

  /* While there are still elements remaining */
  while (lval_sexpr_count(a) > 0) {

    /* Pop the next element */
    lval* y = lval_pop(a, 0);

    /* Perform operation */
    if (strcmp(op, "+") == 0) { x->value.num += y->value.num; }
    if (strcmp(op, "-") == 0) { x->value.num -= y->value.num; }
    if (strcmp(op, "*") == 0) { x->value.num *= y->value.num; }
    if (strcmp(op, "&&") == 0) { x->value.num = x->value.num && y->value.num; }
    if (strcmp(op, "||") == 0) { x->value.num = x->value.num || y->value.num; }
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

lval* builtin_and(lenv* e, lval* a) {
  return builtin_op(e, a, "&&");
}

lval* builtin_or(lenv* e, lval* a) {
  return builtin_op(e, a, "||");
}

lval* builtin_not(lenv* e, lval* a) {
  return builtin_op(e, a, "!");
}

lval* builtin_add(lenv* e, lval* a) {
  return builtin_op(e, a, "+");
}

lval* builtin_sub(lenv* e, lval* a) {
  return builtin_op(e, a, "-");
}

lval* builtin_mul(lenv* e, lval* a) {
  return builtin_op(e, a, "*");
}

lval* builtin_div(lenv* e, lval* a) {
  return builtin_op(e, a, "/");
}                                                \

lval* builtin_head(lenv* e, lval* a) {
  LASSERT_NUM("head", a, 1);
  LASSERT_OR_TYPE("head", a, 0, LVAL_QEXPR, LVAL_STRING);
  LASSERT_NOT_EMPTY("head", a, 0);

  if(lval_sexpr_cell(a)[0]->type == LVAL_QEXPR) {
    lval* v = lval_take(a, 0);
    /* Delete all elements that are not head and return */
    while (lval_sexpr_count(v) > 1) { lval_del(lval_pop(v, 1)); }
    return v;
  } else {
    if(strlen(lval_sexpr_cell(a)[0]->value.str) > 0) {
      lval_sexpr_cell(a)[0]->value.str[1] = '\0';
      lval* v = lval_str(lval_sexpr_cell(a)[0]->value.str);
      lval_del(a);
      return v;
    } else {
      lval_del(a);
      return lval_sexpr();
    }
  }
}

lval* builtin_tail(lenv* e, lval* a) {
  LASSERT_NUM("tail", a, 1);
  LASSERT_OR_TYPE("tail", a, 0, LVAL_QEXPR, LVAL_STRING);
  LASSERT_NOT_EMPTY("tail", a, 0);

  /* Take first argument */
  lval* v = lval_take(a, 0);

  /* Delete first element and return */
  lval_del(lval_pop(v, 0));
  return v;
}

lval* builtin_list(lenv* e, lval* a) {
  a->type = LVAL_QEXPR;
  return a;
}

lval* lval_eval(lenv* e, lval* v);

lval* builtin_eval(lenv* e, lval* a) {
  LASSERT_NUM("eval", a, 1);
  LASSERT_TYPE("eval", a, 0, LVAL_QEXPR);

  lval* x = lval_take(a, 0);
  x->type = LVAL_SEXPR;
  return lval_eval(e, x);
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

lval* lval_str_join(lval* x, lval * y) {
  char* xs = x->value.str;
  char* ys = y->value.str;
  xs = realloc(xs, strlen(xs) + strlen(ys) + 1);
  strcat(xs, ys);
  lval* v = lval_str(xs);
  lval_del(x);
  lval_del(y);
  return v;
}

lval* builtin_join(lenv* e, lval* a) {
  LASSERT(a, lval_sexpr_count(a) > 0,
          "Function '%s' passed too few arguments", "join");

  LASSERT_OR_TYPE("join", a, 0, LVAL_QEXPR, LVAL_STRING);

  lval* x = lval_pop(a, 0);

  for(int i = 0; i < lval_sexpr_count(a); i++){
    LASSERT_TYPE("join", a, i+1, x->type);
  }

  while (lval_sexpr_count(a)) {
    if(x->type == LVAL_QEXPR)
      x = lval_join(x, lval_pop(a, 0));
    else
      x = lval_str_join(x, lval_pop(a, 0));
  }

  lval_del(a);
  return x;
}

lval* builtin_cons(lenv* e, lval* a) {
  LASSERT_NUM("cons", a, 2);
  LASSERT_TYPE("cons", a, 1, LVAL_QEXPR);

  lval* x = lval_pop(a, 0);
  lval* y = lval_pop(a, 0);

  lval* v = lval_qexpr();
  v = lval_add(v, x);

  lval_del(a);
  return lval_join(v, y);
}

lval* builtin_init(lenv* e, lval* a) {
  LASSERT_NUM("init", a, 1);
  LASSERT_TYPE("init", a, 0, LVAL_QEXPR);

  lval* x = lval_sexpr_cell(a)[0];
  int c = lval_sexpr_count(x) - 1;

  lval* v = lval_qexpr();
  for(int i = 0 ; i < c; i++)
    v = lval_add(v, lval_pop(x, 0));

  lval_del(a);

  return v;
}

lval* builtin_len(lenv* e, lval* a) {
  LASSERT_NUM("len", a, 1);
  LASSERT_TYPE("len", a, 0, LVAL_QEXPR);

  lval* x = lval_num(lval_sexpr_count(lval_sexpr_cell(a)[0]));
  lval_del(a);
  return x;
}

lval* builtin_exit(lenv* e, lval* a) {
  LASSERT_NUM("exit", a, 1);
  LASSERT_TYPE("exit", a, 0, LVAL_NUM);

  printf("Byte.\n");
  exit(lval_sexpr_cell(a)[0]->value.num);
}

lval* builtin_var(lenv* e, lval* a, char* func) {
  LASSERT_TYPE(func, a, 0, LVAL_QEXPR);

  int i;
  lval* syms = lval_pop(a, 0);

  for(i = 0; i < lval_sexpr_count(syms); i++)
    LASSERT(a, (lval_sexpr_cell(syms)[i]->type == LVAL_SYM),
            "Function '%s' cannot define non-symbol. "
            "Got %s, Expected %s.",func,
            ltype_name(lval_sexpr_cell(syms)[i]->type), ltype_name(LVAL_SYM));

  LASSERT(a, lval_sexpr_count(syms) == lval_sexpr_count(a),
          "Function '%s' passed too many arguments for symbols. "
          "Got %i, Expected %i.", func,
          lval_sexpr_count(a), lval_sexpr_count(syms));

  for(i = 0; i < lval_sexpr_count(syms); i++) {
    if(strcmp(func, "def") == 0) {
      lenv_def(e, lval_sexpr_cell(syms)[i], lval_sexpr_cell(a)[i]);
    }
    if(strcmp(func, "=") == 0) {
      lenv_put(e, lval_sexpr_cell(syms)[i], lval_sexpr_cell(a)[i]);
    }
  }

  lval_del(syms);
  lval_del(a);

  return lval_sexpr();
}

lval* builtin_def(lenv* e, lval* a) {
  return builtin_var(e, a, "def");
}

lval* builtin_put(lenv* e, lval* a) {
  return builtin_var(e, a, "=");
}

lval* builtin_lambda(lenv* e, lval* a) {
  /* Check Two arguments, each of which are Q-Expressions */
  LASSERT_NUM("\\", a, 2);
  LASSERT_TYPE("\\", a, 0, LVAL_QEXPR);
  LASSERT_TYPE("\\", a, 1, LVAL_QEXPR);

  /* Check first Q-Expression contains only Symbols */
  lval* fs = lval_sexpr_cell(a)[0];
  for (int i = 0; i < lval_sexpr_count(fs); i++) {
    LASSERT(a, (lval_sexpr_cell(fs)[i]->type == LVAL_SYM),
            "Cannot define non-symbol. Got %s, Expected %s.",
            ltype_name(lval_sexpr_cell(fs)[i]->type),ltype_name(LVAL_SYM));
  }

  /* Pop first two arguments and pass them to lval_lambda */
  lval* formals = lval_pop(a, 0);
  lval* body = lval_pop(a, 0);
  lval_del(a);

  return lval_lambda(formals, body);
}
lval* lval_eval_sexpr(lenv* e, lval* v);

lval* builtin_if(lenv* e, lval* a) {
  LASSERT_NUM("if", a, 3);
  LASSERT_TYPE("if", a, 0, LVAL_NUM);
  LASSERT_TYPE("if", a, 1, LVAL_QEXPR);
  LASSERT_TYPE("if", a, 2, LVAL_QEXPR);

  lval* t = lval_pop(a, 0);
  lval* then = lval_pop(a, 0);
  lval* other = lval_pop(a, 0);
  lval* r = NULL;

  if(t->value.num) {
    then->type = LVAL_SEXPR;
    r = lval_eval_sexpr(e, then);
  } else {
    other->type = LVAL_SEXPR;
    r = lval_eval_sexpr(e, other);
  }

  lval_del(t);
  lval_del(a);

  return r;
}

lval* lval_call(lenv* e, lval* f, lval* a) {
  /* If f is a builtin function */
  if(f->value.fn->builtin) {
    return f->value.fn->builtin(e, a);
  }

  /* bind formals */
  int given = lval_sexpr_count(a);
  int total = lval_sexpr_count(f->value.fn->formals);

  while(lval_sexpr_count(a)) {

    LASSERT(a, lval_sexpr_count(f->value.fn->formals) > 0,
            "Function passed too many arguments. "
            "Got %i, Expected %i.", given, total);

    lval* sym = lval_pop(f->value.fn->formals, 0);

    if(strcmp(sym->value.sym, "&") == 0) {
      LASSERT(a, lval_sexpr_count(f->value.fn->formals) == 1,
              "Function format invalid. "
              "Symbol '&' not followed by single symbol.");
      lval* nsym = lval_pop(f->value.fn->formals, 0);

      lenv_put(f->value.fn->env, nsym, builtin_list(e, a));
      lval_del(sym);lval_del(nsym);
      break;
    }

    lval* val = lval_pop(a, 0);
    lenv_put(f->value.fn->env, sym, val);
    lval_del(sym);lval_del(val);
  }

  lval_del(a);

  if (lval_sexpr_count(f->value.fn->formals) > 0 &&
      strcmp(lval_sexpr_cell(f->value.fn->formals)[0]->value.sym, "&") == 0) {

    LASSERT(a, lval_sexpr_count(f->value.fn->formals) != 2,
            "Function format invalid. "
            "Symbol '&' not followed by single symbol.");

    lval_del(lval_pop(f->value.fn->formals, 0));

    lval* sym = lval_pop(f->value.fn->formals, 0);
    lval* val = lval_qexpr();

    lenv_put(f->value.fn->env, sym, val);
    lval_del(sym); lval_del(val);
  }

  if(lval_sexpr_count(f->value.fn->formals) == 0) {

    f->value.fn->env->par = e;
    /* Call function with function local environment */
    return builtin_eval(f->value.fn->env,
                        lval_add(lval_sexpr(), lval_copy(f->value.fn->body)));
  }else {
    /* return partial bound function */
    return lval_copy(f);
  }
}

lval* lval_eval_sexpr(lenv* e, lval* v) {

  /* Evaluate Children */
  for (int i = 0; i < lval_sexpr_count(v); i++) {
    lval_sexpr_cell(v)[i] = lval_eval(e, lval_sexpr_cell(v)[i]);
  }

  /* Error Checking */
  for (int i = 0; i < lval_sexpr_count(v); i++) {
    if (lval_sexpr_cell(v)[i]->type == LVAL_ERR) { return lval_take(v, i); }
  }

  /* Empty Expression */
  if (lval_sexpr_count(v) == 0) { return v; }

  /* Single Expression */
  if (lval_sexpr_count(v) == 1) { return lval_take(v, 0); }

  /* Ensure First Element is Func */
  lval* f = lval_pop(v, 0);
  if (f->type != LVAL_FUN) {
    lval_del(f); lval_del(v);
    return lval_error("first element is not a function");
  }

  /* If so call function to get result */
  lval* result = lval_call(e, f, v);
  lval_del(f);
  return result;
}

lval* lval_eval(lenv* e, lval* v) {
  /* Evaluate variable */
  if (v->type == LVAL_SYM) {
    lval* x  = lenv_get(e, v);
    lval_del(v);
    return x;
  }
  /* Evaluate Sexpressions */
  if (v->type == LVAL_SEXPR) { return lval_eval_sexpr(e, v); }
  /* All other lval types remain the same */
  return v;
}

lval* lval_read_num(mpc_ast_t* t) {
  errno = 0;
  long x = strtol(t->contents, NULL, 10);
  return errno != ERANGE ?
    lval_num(x) : lval_error("invalid number");
}

lval* lval_read_str(mpc_ast_t* t) {
  /* Cut off the final quote character */
  t->contents[strlen(t->contents)-1] = '\0';
  /* Copy the string missing out the first quote character */
  char* unescaped = malloc(strlen(t->contents+1)+1);
  strcpy(unescaped, t->contents+1);
  /* Pass through the unescape function */
  unescaped = mpcf_unescape(unescaped);
  /* Construct a new lval using the string */
  lval* str = lval_str(unescaped);
  /* Free the string and return */
  free(unescaped);
  return str;
}

lval* lval_read(mpc_ast_t* t) {

  /* If Symbol or Number return conversion to that type */
  if (strstr(t->tag, "number")) { return lval_read_num(t); }
  if (strstr(t->tag, "symbol")) { return lval_sym(t->contents); }
  if (strstr(t->tag, "string")) { return lval_read_str(t); }

  /* If root (>) or sexpr then create empty list */
  lval* x = NULL;
  if (strcmp(t->tag, ">") == 0) { x = lval_sexpr(); }
  if (strstr(t->tag, "sexpr"))  { x = lval_sexpr(); }
  if (strstr(t->tag, "qexpr"))  { x = lval_qexpr(); }

  /* Fill this list with any valid expression contained within */
  for (int i = 0; i < t->children_num; i++) {
    if (strstr(t->children[i]->tag, "comment")) { continue; }
    if (strcmp(t->children[i]->contents, "(") == 0) { continue; }
    if (strcmp(t->children[i]->contents, ")") == 0) { continue; }
    if (strcmp(t->children[i]->contents, "}") == 0) { continue; }
    if (strcmp(t->children[i]->contents, "{") == 0) { continue; }
    if (strcmp(t->children[i]->tag,  "regex") == 0) { continue; }
    x = lval_add(x, lval_read(t->children[i]));
  }

  return x;
}

lval* builtin_load(lenv* e, lval* a) {
  LASSERT_NUM("load", a, 1);
  LASSERT_TYPE("load", a, 0, LVAL_STRING);

  mpc_result_t r;
  if(mpc_parse_contents(lval_sexpr_cell(a)[0]->value.str, Lispy, &r)) {
    lval* expr = lval_read(r.output);
    mpc_ast_delete(r.output);

    while(lval_sexpr_count(expr)) {
      lval* x = lval_eval(e, lval_pop(expr, 0));
      if (x->type == LVAL_ERR) { lval_println(x); }
      lval_del(x);
    }

    lval_del(expr);
    lval_del(a);
    return lval_sexpr();
  } else {
    char* err_msg = mpc_err_string(r.error);
    mpc_err_delete(r.error);

    lval* err = lval_error("Could not load Library %s", err_msg);
    free(err_msg);
    lval_del(a);

    return err;
  }
}

lval* builtin_print(lenv* e, lval* a) {

  for (int i = 0; i < lval_sexpr_count(a); i++) {
    lval_print(lval_sexpr_cell(a)[i]); putchar(' ');
  }

  putchar('\n');
  lval_del(a);

  return lval_sexpr();
}

lval* builtin_error(lenv* e, lval* a) {
  LASSERT_NUM("error", a, 1);
  LASSERT_TYPE("error", a, 0, LVAL_STRING);

  lval* err = lval_error(lval_sexpr_cell(a)[0]->value.str);

  lval_del(a);
  return err;
}

void lenv_add_builtin(lenv* e, const char* name, lbuiltin func) {
  lval* k = lval_sym(name);
  lval* v = lval_fun(name, func);
  lenv_put(e, k, v);
  /* remember to delete k and v*/
  lval_del(k); lval_del(v);
}

void lenv_add_builtins(lenv* e) {
  /* List Functions */
  lenv_add_builtin(e, "list", builtin_list);
  lenv_add_builtin(e, "head", builtin_head);
  lenv_add_builtin(e, "tail", builtin_tail);
  lenv_add_builtin(e, "eval", builtin_eval);
  lenv_add_builtin(e, "join", builtin_join);
  lenv_add_builtin(e, "init", builtin_init);
  lenv_add_builtin(e, "cons", builtin_cons);
  lenv_add_builtin(e, "len", builtin_len);

  /* Mathematical Functions */
  lenv_add_builtin(e, "+", builtin_add);
  lenv_add_builtin(e, "-", builtin_sub);
  lenv_add_builtin(e, "*", builtin_mul);
  lenv_add_builtin(e, "/", builtin_div);

  /* Order and Logic Functions */
  lenv_add_builtin(e, ">", builtin_gt);
  lenv_add_builtin(e, ">=", builtin_gte);
  lenv_add_builtin(e, "<", builtin_lt);
  lenv_add_builtin(e, "<=", builtin_lte);
  lenv_add_builtin(e, "==", builtin_eq);
  lenv_add_builtin(e, "!=", builtin_neq);
  lenv_add_builtin(e, "&&", builtin_and);
  lenv_add_builtin(e, "||", builtin_or);
  lenv_add_builtin(e, "!", builtin_not);


  /* Environments */
  lenv_add_builtin(e, "\\",  builtin_lambda);
  lenv_add_builtin(e, "def", builtin_def);
  lenv_add_builtin(e, "=", builtin_put);

  /* Conditions */
  lenv_add_builtin(e, "if", builtin_if);

  /* Core */
  lenv_add_builtin(e, "exit", builtin_exit);
  lenv_add_builtin(e, "load", builtin_load);
  lenv_add_builtin(e, "error", builtin_error);
  lenv_add_builtin(e, "print", builtin_print);
}

int main(int argc, char** argv) {

  Number = mpc_new("number");
  Symbol = mpc_new("symbol");
  String = mpc_new("string");
  Comment = mpc_new("comment");
  Sexpr  = mpc_new("sexpr");
  Qexpr  = mpc_new("qexpr");
  Expr   = mpc_new("expr");
  Lispy  = mpc_new("lispy");

  mpca_lang(MPCA_LANG_DEFAULT,
            "                                              \
      number  : /-?[0-9]+/ ;                       \
      symbol  : /[a-zA-Z0-9_+\\-*\\/\\\\=<>!&|]+/ ; \
      string  : /\"(\\\\.|[^\"])*\"/ ;             \
      comment : /;[^\\r\\n]*/ ;                    \
      sexpr   : '(' <expr>* ')' ;                  \
      qexpr   : '{' <expr>* '}' ;                  \
      expr    : <number>  | <symbol> | <string>    \
              | <comment> | <sexpr>  | <qexpr>;    \
      lispy   : /^/ <expr>* /$/ ;                  \
    ",
            Number, Symbol, String, Comment, Sexpr, Qexpr, Expr, Lispy);

  lenv* e = lenv_new();
  lenv_add_builtins(e);

  if (argc >= 2) {
    for (int i = 1; i < argc; i++) {
      lval* args = lval_add(lval_sexpr(), lval_str(argv[i]));
      lval* x = builtin_load(e, args);

      if (x->type == LVAL_ERR) { lval_println(x); }
      lval_del(x);
    }
  }

  puts("Lispy Version 0.0.0.0.5");
  puts("Press Ctrl+c or enter 'exit int' to Exit\n");

  while (1) {

    char* input = readline("lispy> ");
    add_history(input);

    mpc_result_t r;
    if (mpc_parse("<stdin>", input, Lispy, &r)) {
      lval* x = lval_eval(e, lval_read(r.output));
      lval_println(x);
      lval_del(x);
      mpc_ast_delete(r.output);
    } else {
      mpc_err_print(r.error);
      mpc_err_delete(r.error);
    }

    free(input);

  }

  mpc_cleanup(7, Number, Symbol, String, Comment, Sexpr, Qexpr, Expr, Lispy);

  return 0;
}
