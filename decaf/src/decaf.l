%{
#include <stdio.h>
#include <string.h>
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
EALPHA [a-zA-Z_]
EHEXDIGIT {EDIGIT}[a-fA-F]
EALPHANUM ({EALPHA}|{EDIGIT})
%%
\/\/.*                  ; /* \/\/[^\n] */

int                     return INT;
class                   return CLASS;
Program                 return PROGRAM; // NOTE: conflict
void                    return VOID;

0x{EHEXDIGIT}+          { yylval._hex_literal = strtol(yytext, NULL, 16); return HEXADECIMAL_CST; }
{EDIGIT}+               { yylval._int_literal = strtol(yytext, NULL, 10); return DECIMAL_CST; }

{EALPHA}{EALPHANUM}*    { strcpy(yylval._id, yytext); return ID; }


[+\-*/%]                return yytext[0];
[(){},;]                return yytext[0];
[=]                     return yytext[0];

[[:space:]]             ;
.                       {fprintf(stderr, "(lex) Caractère illégal (%d)\n", yytext[0]);}

%%