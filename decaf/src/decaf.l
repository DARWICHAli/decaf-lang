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

%option yylineno

    /* Expressions rationnelles nommées */
EDIGIT [0-9]
EALPHA [a-zA-Z_]
EHEXDIGIT {EDIGIT}[a-fA-F]
EALPHANUM ({EALPHA}|{EDIGIT})
%%
\/\/.*                  ; /* \/\/[^\n] */

int                     { yylval.BType = BT_INT; return TYPE; }
boolean                 { yylval.BType = BT_BOOL; return TYPE; }
class                   return CLASS;
void                    return VOID;
return                  return RETURN;
WriteString 		return WRITESTRING;

0x{EHEXDIGIT}+          { yylval.Integer = strtol(yytext, NULL, 16); return HEXADECIMAL_CST; }
{EDIGIT}+               { yylval.Integer = strtol(yytext, NULL, 10); return DECIMAL_CST; }

{EALPHA}{EALPHANUM}*    {
    if (yyleng >= MAX_IDENTIFIER_SIZE) {
        fprintf(stderr, "Identifier max length reached (%d characters)\n", MAX_IDENTIFIER_SIZE);
        exit(1);
    }
    strcpy(yylval.Identifier, yytext);
    return ID;
}


[+\-*/%]                return yytext[0];
[(){},;]                return yytext[0];
[=]                     return yytext[0];
[\[\]]                  return yytext[0];

 \"(\\.|[^\"])*\"	{ yylval.String = yytext; return CSTR; }

[[:space:]]             ;
.                       {fprintf(stderr, "(lex) Caractère illégal: %c (%d)\n", yytext[0], yytext[0]);}

%%
