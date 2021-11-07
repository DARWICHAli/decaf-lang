%{
#include <stdio.h>
#include "lexer/token.h"

int yyval_int;
%}

%option nounput
%option noyywrap

    /* Expressions rationnelles nommées */


%%
    /* Hexadecimaux en premier, sinon (0)x(..) sont matcher comme étant 2 entiers  */
0x[0-9a-fA-F]+ {
    yyval_int = strtol(yytext, NULL, 16);
    return HEXADECIMAL_CST;
}


[+-]?[0-9]+ {
    yyval_int = strtol(yytext, NULL, 10);
    return DECIMAL_CST;
}


[[:space:]] ;
. return LEX_ERROR;

%%