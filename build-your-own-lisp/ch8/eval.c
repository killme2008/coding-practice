#include "../global.h"

/* Create Enumeration of Possible Error Types */
enum { LERR_DIV_ZERO, LERR_BAD_OP, LERR_BAD_NUM };

/* Create Enumeration of Possible lval Types */
enum { LVAL_NUM, LVAL_ERR };

/* Declare New lval Struct */
typedef struct {
  int type;
  /* Use union to store value */
  union {
    long num;
    int err;
  } value;
} lval;

lval lval_num(long x);
lval lval_error(int e);
void lval_print(lval v);

lval eval_op(lval x, char* op, lval y);
lval eval(mpc_ast_t* t);

int main(int argc, char** argv) {

  /* Create Some Parsers */
  mpc_parser_t* Number   = mpc_new("number");
  mpc_parser_t* Operator = mpc_new("operator");
  mpc_parser_t* Expr     = mpc_new("expr");
  mpc_parser_t* Lispy    = mpc_new("lispy");

  /* Define them with the following Language */
  mpca_lang(MPCA_LANG_DEFAULT,
            "                                                     \
      number   : /-?[0-9]+/ ;                             \
      operator : '+' | '-' | '*' | '/' ;                  \
      expr     : <number> | '(' <operator> <expr>+ ')' ;  \
      lispy    : /^/ <operator> <expr>+ /$/ ;             \
    ",
            Number, Operator, Expr, Lispy);

  puts("Lispy Version 0.0.0.0.2");
  puts("Press Ctrl+c to Exit\n");

  while (1) {

    char* input = readline("lispy> ");
    add_history(input);

    /* Attempt to parse the user input */
    mpc_result_t r;
    if (mpc_parse("<stdin>", input, Lispy, &r)) {
      /* On success print and delete the AST */
      /* mpc_ast_print(r.output); */
      lval_print(eval(r.output));
      mpc_ast_delete(r.output);
    } else {
      /* Otherwise print and delete the Error */
      mpc_err_print(r.error);
      mpc_err_delete(r.error);
    }

    free(input);
  }

  /* Undefine and delete our parsers */
  mpc_cleanup(4, Number, Operator, Expr, Lispy);

  return 0;
}

lval eval_op(lval x, char* op, lval y) {
  if(strlen(op) != 1)
    return lval_error(LERR_BAD_OP);
  long xv = x.value.num;
  long yv = y.value.num;
  switch(op[0]){
  case '+': return lval_num(xv + yv);
  case '-': return lval_num(xv - yv);
  case '*': return lval_num(xv * yv);
  case '/':
    if(yv == 0)
      return lval_error(LERR_DIV_ZERO);
    return lval_num(xv / yv);
  default:
    return lval_error(LERR_BAD_OP);
  }
}
lval eval(mpc_ast_t* t) {
  if(strstr(t->tag, "number")) {
    /* Check if there is some error in conversion */
    errno = 0;
    long x = strtol(t->contents, NULL, 10);
    return errno != ERANGE ? lval_num(x) : lval_error(LERR_BAD_NUM);
  }

  char* op = t->children[1]->contents;
  lval x = eval(t->children[2]);

  int i = 3;
  while(strstr(t->children[i]->tag, "expr")) {
    x = eval_op(x, op, eval(t->children[i]));
    i++;
  }
  return x;
}

lval lval_num(long x) {
  lval v;
  v.type = LVAL_NUM;
  v.value.num = x;
  return v;
}
lval lval_error(int e) {
  lval v;
  v.type = LVAL_ERR;
  v.value.err = e;
  return v;
}
void lval_print(lval v) {
  switch(v.type) {
  case LVAL_NUM:
    printf("%ld\n", v.value.num);
    break;
  case LVAL_ERR:
    switch(v.value.err){
    case LERR_DIV_ZERO:
      puts("Error: Division By Zero!");
      break;
    case LERR_BAD_OP:
      puts("Error: Invalid Operator!");
      break;
    case LERR_BAD_NUM:
      puts("Error: Invalid Number!");
      break;
    }
    break;
  }
}
