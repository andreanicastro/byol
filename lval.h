#ifndef LVAL_H
#define LVAL_H

struct lval;
typedef struct lval lval;

struct lenv;
typedef struct lenv lenv;
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



#endif 
