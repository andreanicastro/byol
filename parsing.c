#include <stdio.h>
#include <stdlib.h>
#include "mpc.h"
#include "lval_ops.h"
#include "builtins.h" 
#include "lenv_ops.h"

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




