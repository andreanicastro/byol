#include <stdio.h>
#include <stdlib.h>
#include "mpc.h"
#include "lval_ops.h"
#include "lenv.h"

#ifdef _WIN32
#include <string.h>


static char buffer[2048];

char* readline(char* prompt)
{
    fputs(prompt, stdout);
    fgets(buffer, 2048, stdin);
    char* cpy = malloc(strlen(buffer)+1);
    strcpy(cpy, buffer);
    cpy[strlen(cpy)-1] = '\0';
    return cpy;
}

void add_history(char* unused){}
#else
#include <editline/readline.h>
#include <editline/history.h>
#endif

#define LASSERT(args, cond, fmt, ...) \
    if (!(cond)) {\
        lval* err = lval_err(fmt, ##__VA_ARGS__); \
        lval_del(args); \
        return err;}



lenv* lenv_new(void)
{
    lenv* e = malloc(sizeof(lenv));
    e->count = 0;
    e->syms = NULL;
    e->vals = NULL;
    return e;
}

void lenv_del(lenv* e)
{
    for (int i = 0; i < e->count; i++)
    {
        free(e->syms[i]);
        lval_del(e->vals[i]);
    }
    free(e->syms);
    free(e->vals);
    free(e);
}

lval* lenv_get(lenv* e, lval* k)
{

    for (int i = 0; i < e->count; i++)
    {
        if (strcmp(e->syms[i], k->sym) == 0)
        {
            return lval_copy(e->vals[i]);
        }
    }

    return lval_err("Unbound symbol '%s'", k->sym);
}

void lenv_put(lenv* e, lval* k, lval* v)
{
    /* Iterate over all items in environment 
     * this is to see if a variable already exists */
    for (int i = 0; i < e->count; i++)
    {

        /* if variable is found delete item at that position
         * and replace with variable supplied by user */
        if (strcmp(e->syms[i], k->sym) == 0)
        {
            lval_del(e->vals[i]);
            e->vals[i] = lval_copy(v);
            return;
        }
    }

    /* If no eisting entry found allocate space for new entry */
    e->count++;
    e->vals = realloc(e->vals, sizeof(lval*) * e->count);
    e->syms = realloc(e->syms, sizeof(char*) * e->count);

    /* Copy contents of lval and symbol string int new location */
    e->vals[e->count-1] = lval_copy(v);
    e->syms[e->count-1] = malloc(strlen(k->sym)+1);
    strcpy(e->syms[e->count-1], k->sym);
}

void lenv_add_builtin(lenv* e, char* name, lbuiltin func)
{
    lval* k = lval_sym(name);
    lval* v = lval_fun(func);
    lenv_put(e, k, v);
    lval_del(k); lval_del(v);
}


lval* builtin_list(lenv* e, lval* a);
lval* builtin_head(lenv* e, lval* a);
lval* builtin_tail(lenv* e, lval* a);
lval* builtin_eval(lenv* e, lval* a);
lval* builtin_join(lenv* e, lval* a);
lval* builtin_add(lenv* e, lval* a);
lval* builtin_sub(lenv* e, lval* a);
lval* builtin_mul(lenv* e, lval* a);
lval* builtin_div(lenv* e, lval* a);
lval* builtin_def(lenv* e, lval* a);

void lenv_add_builtins(lenv* e)
{
    lenv_add_builtin(e, "list", builtin_list);
    lenv_add_builtin(e, "head", builtin_head);
    lenv_add_builtin(e, "tail", builtin_tail);
    lenv_add_builtin(e, "eval", builtin_eval);
    lenv_add_builtin(e, "join", builtin_join);

    lenv_add_builtin(e, "+", builtin_add);
    lenv_add_builtin(e, "-", builtin_sub);
    lenv_add_builtin(e, "*", builtin_mul);
    lenv_add_builtin(e, "/", builtin_div);

    lenv_add_builtin(e, "def", builtin_def);
}

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

//lval* builtin(lval* a, char* func)
//{
//    if (strcmp("list", func) == 0) 
//        return builtin_list(a);
//    if (strcmp("head", func) == 0)
//        return builtin_head(a);
//    if (strcmp("tail", func) == 0)
//        return builtin_tail(a);
//    if (strcmp("join", func) == 0)
//        return builtin_join(a);
//    if (strcmp("eval", func) == 0)
//        return builtin_eval(a);
//    if (strstr("+-/*", func)) 
//        return builtin_op(a, func);
//    lval_del(a);
//    return lval_err("Unknown Function!");
//}


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


int main(int argc, char** argv)
{

    /* Create some parsers */
    mpc_parser_t* Number = mpc_new("number");
    mpc_parser_t* Symbol = mpc_new("symbol");
    mpc_parser_t* Sexpr = mpc_new("sexpr");
    mpc_parser_t* Qexpr = mpc_new("qexpr");
    mpc_parser_t* Expr = mpc_new("expr");
    mpc_parser_t* Lispy = mpc_new("lispy");

    /* Define them with the following Language*/
    mpca_lang(MPCA_LANG_DEFAULT,
            "\
             number : /-?[0-9]+/ ; \
             symbol: /[a-zA-Z0-9_+\\-*\\/\\\\=<>!&]+/ ; \
             sexpr: '(' <expr>* ')';\
             qexpr: '{' <expr>* '}';\
             expr : <number> | <symbol> | <sexpr> | <qexpr> ; \
             lispy : /^/ <expr>* /$/ ; \
             ",
             Number, Symbol, Sexpr, Qexpr, Expr, Lispy);




    /* Print Version and Exit Information*/
    puts("Lispy Version 0.0.0.0.1");
    puts("Press Ctrl+c to Exit\n");

    lenv* e = lenv_new();
    lenv_add_builtins(e);

    while(1)
    {
        char* input = readline("lispy> ");

        add_history(input);
        
        // parse the input
        mpc_result_t r;
        if (mpc_parse("<stdin>", input, Lispy, &r))
        {
            lval* x = lval_eval(e, lval_read(r.output));
            lval_println(x);
            lval_del(x);

            mpc_ast_delete(r.output);
        }
        else
        {
            mpc_err_print(r.error);
            mpc_err_delete(r.error);
        }

        free(input);

    }
    lenv_del(e); 
    /* Undefine and Delete the Parser */
    mpc_cleanup(6, Number, Symbol, Sexpr, Qexpr, Expr, Lispy);

    return 0;
}




