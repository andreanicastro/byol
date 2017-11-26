#ifndef LVAL_OPS_H
#define LVAL_OPS_H

#include "mpc.h"
#include "lval.h"



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
