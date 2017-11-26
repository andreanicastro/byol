#ifndef LENV_H
#define LENV_H

#include "lval.h"

struct lenv;
typedef struct lenv lenv;

struct lenv 
{
    int count;
    char** syms;
    lval** vals;
};

#endif
