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
EDIGIT [0-9]
EHEXDIGIT {EDIGIT}[a-fA-F]
%%

0x{EHEXDIGIT}+          { yylval._hex_literal = strtol(yytext, NULL, 16); return HEXADECIMAL_CST; }
{EDIGIT}+               { yylval._int_literal = strtol(yytext, NULL, 10); return DECIMAL_CST; }

[+\-*/%]                return yytext[0];
[(){},;]                return yytext[0];

"class"                 return CLASS;
[a-zA-Z]+               return PROGRAM;

[[:space:]]             ;
.                       {fprintf(stderr, "(lex) Caractère illégal (%d)\n", yytext[0]);}

%%