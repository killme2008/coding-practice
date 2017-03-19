#ifndef LVAL_H
#define LVAL_H

/* Add SYM and SEXPR as possible lval types */
enum { LVAL_ERR, LVAL_NUM, LVAL_SYM, LVAL_SEXPR };

struct spr;

/* Declare New lval Struct */
typedef struct {
  int type;
  /* Use union to store value */
  union {
    long num;
    char *err;
    char *sym;
    struct spr* list;
  } value;
} lval;

struct spr{
  int count;
  lval** cell;
};

#define lval_sexpr_count(x) (x->value.list->count)
#define lval_sexpr_cell(x) (x->value.list->cell)

typedef struct spr sexpr;

lval* lval_num(long x);
lval* lval_error(const char* e);
lval* lval_sym(const char* s);
lval* lval_sexpr(void);
void lval_del(lval* v);
lval* lval_add(lval* v, lval* x);
lval* lval_pop(lval* v, int i);
lval* lval_take(lval* v, int i);
void lval_print(lval* v);
void lval_println(lval* v);
#endif
