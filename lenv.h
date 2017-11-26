#ifndef LENV_H
#define LENV_H


struct lval;
typedef struct lval lval;

struct lenv;
typedef struct lenv lenv;

struct lenv 
{
    int count;
    char** syms;
    lval** vals;
};

#endif
