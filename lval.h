#ifndef LVAL_H
#define LVAL_H

#include "mpc.h"

struct lenv;
typedef struct lenv lenv;
struct lval;
typedef struct lval lval;

enum { LVAL_NUM, LVAL_ERR, LVAL_SYM,
       LVAL_FUN, LVAL_SEXPR, LVAL_QEXPR };

enum { LERR_DIV_ZERO, LERR_BAD_OP, LERR_BAD_NUM };

char* ltype_name(int t);

typedef lval*(*lbuiltin)(lenv*, lval*);

struct lval{
    int type;

    long num;
    char* err;
    char* sym;
    lbuiltin fun;

    int count;
    lval** cell;
};


lval* lval_fun(lbuiltin func);


lval* lval_num(long x);

lval* lval_err(char* fmt, ...);

lval* lval_sym(char* s);


lval* lval_read_num(mpc_ast_t* t);

lval* lval_sexpr(void);



lval* lval_read(mpc_ast_t* t);

void lval_del(lval* v); 


lval* lval_copy(lval* v);


lval* lval_add(lval* v, lval* x);


lval* lval_pop(lval* v, int i);

lval* lval_join(lval* x, lval* y);


lval* lval_take(lval* v, int i);

void lval_expr_print(lval* v, char open, char close);






void lval_print(lval* v);

void lval_println(lval* v); 

lval* lval_eval_sexpr(lenv* e, lval* v);


lval* lval_eval(lenv* e, lval* v);
#endif
