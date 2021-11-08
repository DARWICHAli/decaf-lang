%{
#include <stdio.h>
#include "decaf.tab.h"

extern YYSTYPE yylval;
extern char * yytext;

%}

%option nounput
%option noyywrap

    /* Expressions rationnelles nommées */


%%
    /* Hexadecimaux en premier, sinon (0)x(..) sont matcher comme étant 2 entiers  */
0x[0-9a-fA-F]+ {
    yylval._hex_literal = strtol(yytext, NULL, 16);
    return HEXADECIMAL_CST;
}

    /* [+-]? */
[0-9]+ {
    yylval._int_literal = strtol(yytext, NULL, 10);
    return DECIMAL_CST;
}

"+" return PLUS;
"-" return MINUS;
"*" return MULT;

    /*  "/" return DIVIDE;
        "%" return MODULO;
        "(" return OPAR;
        ")" return CPAR; */


[[:space:]] ;
. fprintf(stderr, "(lex) Caractère illégal (%d)\n", yytext[0]);

%%