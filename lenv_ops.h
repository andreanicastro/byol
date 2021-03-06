#ifndef LENV_OPS_H
#define LENV_OPS_H

#include "lenv.h"
#include "lval.h"
#include "builtins.h"


lenv* lenv_new(void);

void lenv_del(lenv* e);

lval* lenv_get(lenv* e, lval* k);

void lenv_put(lenv* e, lval* k, lval* v);

void lenv_add_builtin(lenv* e, char* name, lbuiltin func);



#endif
