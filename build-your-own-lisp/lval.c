#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "lval.h"

lval* lval_num(long x) {
  lval* v = malloc(sizeof(lval));
  v->type = LVAL_NUM;
  v->value.num = x;
  return v;
}

lval* lval_error(const char* e) {
  lval* v = malloc(sizeof(lval));
  v->type = LVAL_ERR;
  v->value.err = malloc(strlen(e) + 1);
  strcpy(v->value.err, e);
  return v;
}

lval* lval_sym(const char* s) {
  lval* v = malloc(sizeof(lval));
  v->type = LVAL_SYM;
  v->value.sym = malloc(strlen(s) + 1);
  strcpy(v->value.err, s);
  return v;
}

static sexpr* new_sexpr() {
  sexpr* s = malloc(sizeof(sexpr));
  s->count = 0;
  s->cell = NULL;
  return s;
}

static void free_sexpr(sexpr* s) {
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
  v->value.list = new_sexpr();
  return v;
}

void lval_del(lval* v) {
  switch(v->type){
  case LVAL_NUM: break;
  case LVAL_ERR: free(v->value.err); break;
  case LVAL_SYM: free(v->value.sym); break;
  case LVAL_SEXPR:
    free_sexpr(v->value.list);
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
    case LVAL_NUM:   printf("%li", v->value.num); break;
    case LVAL_ERR:   printf("Error: %s", v->value.err); break;
    case LVAL_SYM:   printf("%s", v->value.sym); break;
    case LVAL_SEXPR: lval_expr_print(v, '(', ')'); break;
  }
}

void lval_println(lval* v) { lval_print(v); putchar('\n'); }
