#include "builtins.h"
#include "lval_ops.h"
#include "lenv_ops.h"

lval* builtin_op(lenv* e, lval* a, char* op)
{
    // ensure all arguments are numbers
    for (int i = 0; i < a->count; i++)
        LASSERT(a, a->cell[i]->type == LVAL_NUM,
                "Function '+' passed incorrect type for argument %i. "
                "Got %s, Expected %s",
                i,
                ltype_name(a->cell[i]->type),
                ltype_name(LVAL_NUM));
    // pop the first element
    lval* x = lval_pop(a, 0);

    // if no arguments and sub then perform unary negation
    if ((strcmp(op, "-") == 0) && a->count == 0)
        x->num = - x->num;

    while (a->count > 0)
    {
        
        // pop the next element
        lval* y = lval_pop(a, 0);

        if (strcmp(op, "+") == 0) 
            x->num += y->num;
        if (strcmp(op, "-") == 0)
            x->num -= y->num;
        if (strcmp(op, "*") == 0)
            x->num *= y->num;
        if (strcmp(op, "/") == 0)
        {
            if (y->num == 0)
            {
                lval_del(x); lval_del(y);
                x = lval_err("Division By Zero!"); break;
            }
            x->num /= y->num;
        }
        lval_del(y);
    }

    lval_del(a); return x;
}

lval* builtin_head(lenv* e, lval* a)
{
    /* check error condition*/
    LASSERT(a, a->count == 1, 
            "Function 'head' passed too many arguments."
            "Got %i, Expected %i.",
            a->count, 1);
    LASSERT(a, a->cell[0]->type == LVAL_QEXPR,
            "Function 'head' passed incorrect types for argument 0. "
            "Got %s, Exprected %s.",
            ltype_name(a->cell[0]->type), ltype_name(LVAL_QEXPR));
    LASSERT(a, a->cell[0]->count != 0,
            "Function 'head' passed {}!");

    /* otherwise take first argument */
    lval* v = lval_take(a, 0);


    /* Delete all elements that are not head and return */
    while (v->count > 1)
        lval_del(lval_pop(v,1));

    return v;
}

lval* builtin_tail(lenv* e, lval* a)
{
    /* Checking error conditions*/
    LASSERT(a, a->count == 1,
            "Function 'tail' passed too many arguments."
            "Got %i, Expected %i.",
            a->count, 1);
    LASSERT(a, a->cell[0]->type == LVAL_QEXPR,
            "Function 'tail' passed incorrect types for argument 0. "
            "Got %s, Exprected %s.",
            ltype_name(a->cell[0]->type), ltype_name(LVAL_QEXPR));
    LASSERT(a, a->cell[0]->count != 0,
            "Function 'tail' passed {}!");
    /* Take first argument */
    lval* v = lval_take(a, 0);

    /* delete first element and return */
    lval_del(lval_pop(v,0));
    return v;
}


lval* builtin_list(lenv* e, lval* a)
{
    a->type = LVAL_QEXPR;
    return a;
}

lval* builtin_join(lenv* e, lval* a)
{

    for (int i = 0; i < a->count; i++)
        LASSERT(a, a->cell[i]->type == LVAL_QEXPR,
                "Function 'join' passed incorrect types for argument 0. "
                "Got %s, Exprected %s.",
                ltype_name(a->cell[0]->type), ltype_name(LVAL_QEXPR));
        
    lval* x = lval_pop(a, 0);

    while (a->count)
        x = lval_join(x, lval_pop(a, 0));

    lval_del(a);
    return x;
}


lval* builtin_eval(lenv* e, lval* a)
{
    LASSERT(a, a->count == 1,
            "Function 'eval' passed too many arguments."
            "Got %i, Expected %i.",
            a->count, 1);
    LASSERT(a, a->cell[0]->type == LVAL_QEXPR,
            "function 'eval' passed incorrect types for argument 0. "
            "got %s, exprected %s.",
            ltype_name(a->cell[0]->type), ltype_name(LVAL_QEXPR));

    lval* x = lval_take(a, 0);
    x->type = LVAL_SEXPR;
    return lval_eval(e, x);
}

lval* builtin_add(lenv* e, lval* a)
{
    return builtin_op(e, a, "+");
}


lval* builtin_sub(lenv* e, lval* a)
{
    return builtin_op(e, a, "-");
}


lval* builtin_mul(lenv* e, lval* a)
{
    return builtin_op(e, a, "*");
}

lval* builtin_div(lenv* e, lval* a)
{
    return builtin_op(e, a, "/");
}

lval* builtin_def(lenv* e, lval* a)
{
    LASSERT(a, a->cell[0]->type == LVAL_QEXPR,
            "function 'def' passed incorrect types for argument 0. "
            "got %s, exprected %s.",
            ltype_name(a->cell[0]->type), ltype_name(LVAL_QEXPR));

    /* FIrst argument is symbol list */
    lval* syms = a->cell[0];

    /* Ensure all elements of first list are symbols */
    for (int i = 0; i < syms->count;  i++)
    {
        LASSERT(a, syms->cell[i]->type == LVAL_SYM,
                "Function 'def' cannot define non-symbol");
    }

    /* Check correct number of symbols and values */
    LASSERT(a, syms->count == a->count -1,
            "Function 'def' cannot define incorrect "
            "number of values to symbols");

    /* Assign copies of values to symbols */
    for (int i = 0; i < syms->count; i++)
    {
        lenv_put(e, syms->cell[i], a->cell[i+1]);
    }
    
    lval_del(a);
    return lval_sexpr();
}
