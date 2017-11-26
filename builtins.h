#ifndef BUILTINS_H
#define BUILTINS_H

#include "lval.h"
#include "lenv.h"

#define LASSERT(args, cond, fmt, ...) \
    if (!(cond)) {\
        lval* err = lval_err(fmt, ##__VA_ARGS__); \
        lval_del(args); \
        return err;}

lval* builtin_op(lenv* e, lval* a, char* op);

lval* builtin_head(lenv* e, lval* a);

lval* builtin_tail(lenv* e, lval* a);

lval* builtin_list(lenv* e, lval* a);

lval* builtin_join(lenv* e, lval* a);

lval* builtin_eval(lenv* e, lval* a);

lval* builtin_add(lenv* e, lval* a);

lval* builtin_sub(lenv* e, lval* a);

lval* builtin_mul(lenv* e, lval* a);

lval* builtin_div(lenv* e, lval* a);

lval* builtin_def(lenv* e, lval* a);
#endif 
