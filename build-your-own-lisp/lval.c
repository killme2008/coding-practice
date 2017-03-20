#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "lval.h"
#include <stdarg.h>

lval* lval_num(long x) {
  lval* v = malloc(sizeof(lval));
  v->type = LVAL_NUM;
  v->value.num = x;
  return v;
}

lval* lval_error(const char* fmt, ...) {
  lval* v = malloc(sizeof(lval));
  v->type = LVAL_ERR;

  va_list va;
  va_start(va, fmt);
  v->value.err = malloc(512);
  vsnprintf(v->value.err, 511, fmt, va);
  v->value.err = realloc(v->value.err, strlen(v->value.err)+1);
  va_end(va);

  return v;
}

lval* lval_sym(const char* s) {
  lval* v = malloc(sizeof(lval));
  v->type = LVAL_SYM;
  v->value.sym = malloc(strlen(s) + 1);
  strcpy(v->value.err, s);
  return v;
}

static spr* new_spr() {
  spr* s = malloc(sizeof(spr));
  s->count = 0;
  s->cell = NULL;
  return s;
}

static void free_spr(spr* s) {
  for (int i = 0; i < s->count; i++) {
    lval_del(s->cell[i]);
  }
  /* free cell */
  /* free sexpr structure itself */;
  free(s->cell);
  free(s);
}

lval* lval_sexpr(void) {
  lval* v = malloc(sizeof(lval));
  v->type = LVAL_SEXPR;
  v->value.list = new_spr();
  return v;
}

/* A pointer to a new empty Qexpr lval */
lval* lval_qexpr(void) {
  lval* v = malloc(sizeof(lval));
  v->type = LVAL_QEXPR;
  v->value.list = new_spr();
  return v;
}

lval* lval_fun(const char* name, lbuiltin builtin) {
  lval* v = malloc(sizeof(lval));
  v->type = LVAL_FUN;
  v->value.fn = malloc(sizeof(func));
  v->value.fn->builtin = builtin;
  v->value.fn->env = NULL;
  v->value.fn->formals = NULL;
  v->value.fn->body = NULL;
  v->value.fn->name = malloc(sizeof(strlen(name)+1));
  strcpy(v->value.fn->name, name);
  return v;
}

lval* lval_lambda(lval* formals, lval* body) {
  lval* v = malloc(sizeof(lval));
  v->type = LVAL_FUN;
  v->value.fn = malloc(sizeof(func));

  /* Set Builtin and name to Null */
  v->value.fn->builtin = NULL;
  v->value.fn->name = NULL;

  /* Build new environment */
  v->value.fn->env = lenv_new();

  v->value.fn->formals = formals;
  v->value.fn->body = body;

  return v;
}

void lval_del(lval* v) {
  switch(v->type){
  case LVAL_NUM: break;
  case LVAL_FUN:
    if(!v->value.fn->builtin) {
      lenv_del(v->value.fn->env);
      lval_del(v->value.fn->formals);
      lval_del(v->value.fn->body);
    } else {
      free(v->value.fn->name);
    }
    free(v->value.fn);
    break;
  case LVAL_ERR: free(v->value.err); break;
  case LVAL_SYM: free(v->value.sym); break;
  case LVAL_QEXPR:
  case LVAL_SEXPR:
    free_spr(v->value.list);
    break;
  }
  /* Free the memory allocated for the "lval" struct itself */
  free(v);
}

lval* lval_add(lval* v, lval* x) {
  lval_sexpr_count(v)++;
  lval_sexpr_cell(v) = realloc(lval_sexpr_cell(v), sizeof(lval*) * lval_sexpr_count(v));
  lval_sexpr_cell(v)[lval_sexpr_count(v)-1] = x;
  return v;
}

lval* lval_pop(lval* v, int i) {
  /* Find the item at "i" */
  lval* x = lval_sexpr_cell(v)[i];

  /* Shift memory after the item at "i" over the top */
  memmove(&lval_sexpr_cell(v)[i], &lval_sexpr_cell(v)[i+1],
          sizeof(lval*) * (lval_sexpr_count(v)-i-1));

  /* Decrease the count of items in the list */
  lval_sexpr_count(v)--;

  /* Reallocate the memory used */
  lval_sexpr_cell(v) = realloc(lval_sexpr_cell(v), sizeof(lval*) * lval_sexpr_count(v));
  return x;
}

lval* lval_take(lval* v, int i) {
  lval* x = lval_pop(v, i);
  lval_del(v);
  return x;
}

void lval_print(lval* v);

void lval_expr_print(lval* v, char open, char close) {
  putchar(open);
  for (int i = 0; i < lval_sexpr_count(v); i++) {

    /* Print Value contained within */
    lval_print(lval_sexpr_cell(v)[i]);

    /* Don't print trailing space if last element */
    if (i != (lval_sexpr_count(v)-1)) {
      putchar(' ');
    }
  }
  putchar(close);
}

void lval_print(lval* v) {
  switch (v->type) {
  case LVAL_FUN:
    if(v->value.fn->builtin) {
      printf("<builtin '%s'>", v->value.fn->name);
    } else {
      printf("(\\ "); lval_print(v->value.fn->formals);
      putchar(' '); lval_print(v->value.fn->body);putchar(')');
    }
    break;
  case LVAL_NUM:   printf("%li", v->value.num); break;
  case LVAL_ERR:   printf("Error: %s", v->value.err); break;
  case LVAL_SYM:   printf("%s", v->value.sym); break;
  case LVAL_SEXPR: lval_expr_print(v, '(', ')'); break;
  case LVAL_QEXPR: lval_expr_print(v, '{', '}'); break;
  }
}

void lval_println(lval* v) { lval_print(v); putchar('\n'); }

lval* lval_copy(lval* v) {

  lval* x = malloc(sizeof(lval));
  x->type = v->type;
  int c;

  switch(v->type) {
  case LVAL_NUM:
    x->value.num = v->value.num;
    break;
  case LVAL_FUN:
    x->value.fn = malloc(sizeof(func));
    if(v->value.fn->builtin) {
      x->value.fn->builtin = v->value.fn->builtin;
      x->value.fn->name = malloc(strlen(v->value.fn->name)+1);
      strcpy(x->value.fn->name, v->value.fn->name);
      x->value.fn->env = NULL;
      x->value.fn->formals = NULL;
      x->value.fn->body = NULL;
    } else {
      x->value.fn->builtin = NULL;
      x->value.fn->name = NULL;
      x->value.fn->env = lenv_copy(v->value.fn->env);
      x->value.fn->formals = lval_copy(v->value.fn->formals);
      x->value.fn->body = lval_copy(v->value.fn->body);
    }
    break;
  case LVAL_ERR:
    x->value.err = malloc(strlen(v->value.err) + 1);
    strcpy(x->value.err, v->value.err);
    break;
  case LVAL_SYM:
    x->value.sym = malloc(strlen(v->value.sym) + 1);
    strcpy(x->value.sym, v->value.sym);
    break;
  case LVAL_SEXPR:
  case LVAL_QEXPR:
    c = lval_sexpr_count(v);
    x->value.list = new_spr();
    lval_sexpr_count(x) = c;
    lval_sexpr_cell(x) = malloc(sizeof(lval*) * c);
    for (int i = 0; i < c; i++) {
      lval_sexpr_cell(x)[i] = lval_copy(lval_sexpr_cell(v)[i]);
    }
    break;
  }
  return x;
}

/**Environment functions */
lenv* lenv_new(void) {
  lenv* e = malloc(sizeof(lenv));
  e->count = 0;
  e->par = NULL;
  e->syms = NULL;
  e->vals = NULL;
  return e;
}

void lenv_del(lenv* e) {
  for(int i = 0; i < e->count;i++){
    free(e->syms[i]);
    lval_del(e->vals[i]);
  }

  free(e->syms);
  free(e->vals);
  free(e);
}
lval* lenv_get(lenv* e, lval* k) {
  for(int i = 0; i < e->count; i++) {
    if(strcmp(e->syms[i], k->value.sym) == 0) {
      return lval_copy(e->vals[i]);
    }
  }
  if(e->par)
    return lenv_get(e->par, k);
  else
    return lval_error("Unbound symbol '%s'", k->value.sym);
}

lenv* lenv_copy(lenv* e) {
  lenv* n = malloc(sizeof(lenv));
  n->par = e->par;
  n->count = e->count;
  n->syms = malloc(sizeof(char*) * n->count);
  n->vals = malloc(sizeof(lval*) * n->count);
  for (int i = 0; i < e->count; i++) {
    n->syms[i] = malloc(strlen(e->syms[i]) + 1);
    strcpy(n->syms[i], e->syms[i]);
    n->vals[i] = lval_copy(e->vals[i]);
  }
  return n;
}

void lenv_put(lenv* e, lval* k, lval* v) {
  for(int i = 0; i < e->count; i++) {
    if(strcmp(e->syms[i], k->value.sym) == 0) {
      /*remember to delete old value*/
      lval_del(e->vals[i]);
      e->vals[i] = lval_copy(v);
      return;
    }
  }
  e->count++;
  e->syms = realloc(e->syms, sizeof(lval*) * e->count);
  e->vals = realloc(e->vals, sizeof(lval*) * e->count);
  e->syms[e->count-1] = malloc(strlen(k->value.sym)+1);
  strcpy(e->syms[e->count-1], k->value.sym);
  e->vals[e->count-1] = lval_copy(v);
}

void lenv_def(lenv* e, lval* k, lval* v) {
  while (e->par) { e = e->par; }
  lenv_put(e, k, v);
}

char* ltype_name(int t) {
  switch(t) {
  case LVAL_FUN: return "Function";
  case LVAL_NUM: return "Number";
  case LVAL_ERR: return "Error";
  case LVAL_SYM: return "Symbol";
  case LVAL_SEXPR: return "S-Expression";
  case LVAL_QEXPR: return "Q-Expression";
  default: return "Unknown";
  }
}
