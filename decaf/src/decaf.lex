%{
#include <stdio.h>
#include "decaf.tab.h"

extern YYSTYPE yylval;
extern char * yytext;

int fileno(FILE *stream);

%}

%option nounput
%option noinput
%option noyywrap

    /* Expressions rationnelles nommées */


%%

0x[0-9a-fA-F]+          { yylval._hex_literal = strtol(yytext, NULL, 16); return HEXADECIMAL_CST; }
[0-9]+                  { yylval._int_literal = strtol(yytext, NULL, 10); return DECIMAL_CST; }
"+"                     return ADD;
"-"                     return SUB;
"*"                     return MUL;
"/"                     return DIV;
"%"                     return MOD;
"("                     return OPAR;
")"                     return CPAR;
";"                     return SCOL;

[[:space:]]             ;
.                       {fprintf(stderr, "(lex) Caractère illégal (%d)\n", yytext[0]);}

%%