#include "lval_ops.h"
#include <stdlib.h>
#include "lenv_ops.h"

lval* lval_fun(lbuiltin func)
{
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_FUN;
    v->fun = func;
    return v;
}


lval* lval_num(long x)
{
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_NUM;
    v->num = x;
    return v;
}


lval* lval_err(char* fmt, ...)
{
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_ERR;
   
    /* Create a va list and initialize it*/
    va_list va;
    va_start(va, fmt);

    /* Allocate 512 bytes of space */
    v->err = malloc(512);

    /* printf the error string with a maximum of 511 chars */
    vsnprintf(v->err, 511, fmt, va);

    /* cleanup out va list */
    va_end(va);

    return v;
}


lval* lval_sym(char* s)
{
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_SYM;
    v->sym = malloc(strlen(s) + 1);
    strcpy(v->sym, s);
    return v;
}

lval* lval_read_num(mpc_ast_t* t)
{
    errno = 0;
    long x = strtol(t->contents, NULL, 10);
    return errno != ERANGE ?
        lval_num(x) : lval_err("invalid number");
}

lval* lval_sexpr(void)
{
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_SEXPR;
    v->count = 0;
    v->cell = NULL;
    return v;
}

lval* lval_qexpr(void)
{
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_QEXPR;
    v->count = 0;
    v->cell = NULL;
    return v;
}

void lval_del(lval* v) 
{
    switch(v->type)
    {
        case LVAL_NUM: break;
        case LVAL_ERR: free(v->err); break;
        case LVAL_SYM: free(v->sym); break;
        case LVAL_FUN: break;
        case LVAL_QEXPR:
        case LVAL_SEXPR:
             for( int i = 0; i < v->count; i++)
             {
                 lval_del(v->cell[i]);
             }
             free(v->cell);
             break;
    }
    free(v);
}

lval* lval_copy(lval* v)
{
    lval* x = malloc(sizeof(lval));
    x->type = v->type;

    switch (v->type)
    {
        case LVAL_FUN: x->fun = v->fun; break;
        case LVAL_NUM: x->num = v->num; break;
        case LVAL_ERR:
                       x->err = malloc(strlen(v->err) + 1);
                       strcpy(x->err, v->err); 
                       break;

        case LVAL_SYM:
                       x->sym = malloc(strlen(v->sym) + 1);
                       strcpy(x->sym, v->sym);
                       break;

        case LVAL_SEXPR:
        case LVAL_QEXPR:
                       x->count = v->count;
                       x->cell = malloc(sizeof(lval*) * x->count);
                       for (int i = 0; i < x->count; i++)
                       {
                           x->cell[i] = lval_copy(v->cell[i]);
                       }
                       break;
    }
    return x;

}

lval* lval_add(lval* v, lval* x)
{
    v->count++;
    v->cell = realloc(v->cell, sizeof(lval*) * v->count);
    v->cell[v->count - 1] = x;
    return v;
}

lval* lval_pop(lval* v, int i)
{
    // find the item at "i"
    lval* x = v->cell[i];

    // shift the memory after the item at "i" over the top
    memmove(&v->cell[i], &v->cell[i+1], sizeof(lval*) * (v->count - i - 1));

    // decrease the count of items in the list
    v->count--;

    // reallocate the memory used
    v->cell = realloc(v->cell, sizeof(lval*) * v->count);
    return x;
}

lval* lval_join(lval* x, lval* y)
{
    /* For each cell in 'y' add it to 'x' */
    while (y->count)
        x = lval_add(x, lval_pop(y, 0));

    /* Delete the empty 'y and return 'x' */
    lval_del(y);
    return x;
}

lval* lval_take(lval* v, int i)
{
    lval* x = lval_pop(v, i);
    lval_del(v);
    return x;
}

void lval_expr_print(lval* v, char open, char close)
{
    putchar(open);
    for (int i = 0; i < v->count; i++)
    {
        // print value contained within
        lval_print(v->cell[i]);

        // Dont print trailing space if last element
        if( i != (v->count - 1))
            putchar(' ');
    }
    putchar(close);
}

void lval_print(lval* v)
{
    switch (v->type){
        case LVAL_NUM : printf("%li", v->num); break;
        case LVAL_ERR: printf("Error: %s", v->err); break;
        case LVAL_SYM: printf("%s", v->sym); break;
        case LVAL_SEXPR: lval_expr_print(v, '(', ')'); break;
        case LVAL_QEXPR: lval_expr_print(v, '{', '}'); break;
        case LVAL_FUN: printf("<function>"); break;
    }
}

void lval_println(lval* v) { lval_print(v); putchar('\n'); }

lval* lval_eval_sexpr(lenv* e, lval* v)
{
    // evaluate children
    for (int i = 0; i < v->count; i++)
    {
        v->cell[i] = lval_eval(e, v->cell[i]);
    }

    // error checking
    for (int i = 0; i < v->count; i++)
        if (v->cell[i]->type == LVAL_ERR) 
            return lval_take(v,i);

    // empty expression
    if (v->count == 0)
        return v;
    // single expression
    if (v->count == 1)
        return lval_take(v, 0);

    // ensure first element is symbol
    lval* f = lval_pop(v,0);
    if (f->type !=  LVAL_FUN)
    {
        lval_del(f); lval_del(v);
        return lval_err("first element is not a function");
    }

    // call builtin with operator
    lval* result = f->fun(e, v);
    lval_del(f);
    return result;
}

lval* lval_eval(lenv* e, lval* v)
{
    if (v->type == LVAL_SYM)
    {
        lval* x = lenv_get(e, v);
        lval_del(v);
        return x;
    }

    // evaluate s-expressions
    if (v->type == LVAL_SEXPR)
        return lval_eval_sexpr(e, v);
    
    return v;
}

lval* lval_read(mpc_ast_t* t) {
      
   if (strstr(t->tag, "number")) 
      return lval_read_num(t); 

   if (strstr(t->tag, "symbol")) 
      return lval_sym(t->contents); 
          
   lval* x = NULL;
   if (strcmp(t->tag, ">") == 0) 
       x = lval_sexpr();

   if (strstr(t->tag, "sexpr"))  
       x = lval_sexpr(); 
   if (strstr(t->tag, "qexpr"))
       x = lval_qexpr(); 
                  
   for (int i = 0; i < t->children_num; i++) {
       if (strcmp(t->children[i]->contents, "(") == 0) 
           continue;
       if (strcmp(t->children[i]->contents, ")") == 0) 
           continue;

       if (strcmp(t->children[i]->contents, "}") == 0) 
           continue;
       if (strcmp(t->children[i]->contents, "{") == 0) 
           continue;
       if (strcmp(t->children[i]->tag,  "regex") == 0) 
           continue;
       x = lval_add(x, lval_read(t->children[i]));
    
   }
                   
return x;
}

