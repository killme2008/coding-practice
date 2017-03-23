#ifndef LVAL_H
#define LVAL_H

/* Add SYM and SEXPR as possible lval types */
enum { LVAL_ERR, LVAL_NUM, LVAL_SYM, LVAL_SEXPR, LVAL_QEXPR, LVAL_FUN};

struct spr;
struct lval;
struct lenv;

typedef struct spr spr;
typedef struct lval lval;
typedef struct lenv lenv;

typedef lval*(*lbuiltin)(lenv*, lval*);

/* Declare function struct*/
typedef struct {
  char* name;
  /* builtin functions */
  lbuiltin builtin;
  /* user defined functions,the builtin should be NULL */
  lenv* env;
  lval* formals;
  lval* body;
} func;

/* Declare New lval Struct */

struct lval{
  int type;
  /* Use union to store value */
  union {
    long num;
    char *err;
    char *sym;
    spr* list;
    func* fn;
  } value;
};

struct spr{
  int count;
  lval** cell;
};

struct lenv{
  lenv* par;
  int count;
  char** syms;
  lval** vals;
};

#define lval_sexpr_count(x) (x->value.list->count)
#define lval_sexpr_cell(x) (x->value.list->cell)

lval* lval_num(long x);
lval* lval_error(const char* fmt, ...);
lval* lval_sym(const char* s);
lval* lval_sexpr(void);
lval* lval_qexpr(void);
void lval_del(lval* v);
lval* lval_add(lval* v, lval* x);
lval* lval_pop(lval* v, int i);
lval* lval_take(lval* v, int i);
lval* lval_fun(const char* name, lbuiltin func);
lval* lval_lambda(lval* formals, lval* body);
lval* lval_copy(lval* v);
int lval_eq(lval* x, lval* y);
void lval_print(lval* v);
void lval_println(lval* v);
lenv* lenv_new(void);
lenv* lenv_copy(lenv* e);
void lenv_del(lenv* e);
lval* lenv_get(lenv* e, lval* k);
void lenv_put(lenv* e, lval* k, lval* v);
void lenv_def(lenv* e, lval* k, lval* v);
char* ltype_name(int t);

/* some assert macros */
#define LASSERT(args, cond, fmt, ...)                       \
  if (!(cond)) {                                            \
                lval* err = lval_error(fmt, ##__VA_ARGS__); \
                lval_del(args);                             \
                return err;                                 \
                }

#define LASSERT_TYPE(func, args, index, expect)                         \
  LASSERT(args, lval_sexpr_cell(args)[index]->type == expect,           \
            "Function '%s' passed incorrect type for argument %i. Got %s, Expected %s.", \
            func, index, ltype_name(lval_sexpr_cell(args)[index]->type), ltype_name(expect))

#define LASSERT_NUM(func, args, num)                                    \
  LASSERT(args, lval_sexpr_count(args) == num,                          \
            "Function '%s' passed incorrect number of arguments. Got %i, Expected %i.", \
            func, lval_sexpr_count(args), num)

#define LASSERT_NOT_EMPTY(func, args, index)                          \
  LASSERT(args, lval_sexpr_count(lval_sexpr_cell(args)[index]) != 0,  \
            "Function '%s' passed {} for argument %i.", func, index);

#endif
