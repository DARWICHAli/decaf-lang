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

int                     { yylval.BType = BT_INT; return TYPE; }
boolean                 { yylval.BType = BT_BOOL; return TYPE; }
class                   return CLASS;
void                    return VOID;
if                      return IF;
else                    return ELSE;

return                  return RETURN;
break                   return BREAK;
continue                return CONTINUE;

true                    { yylval.Boolean = true; return TRUE;}
false                   { yylval.Boolean = false; return FALSE;}
"&&"                    return LAND;
"||"                    return LOR;
"=="                    return EQUAL;
"!="                    return NEQUAL;
[<>]                    return yytext[0];
">="                    return MORE_EQUAL;
"<="                    return LESS_EQUAL;

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

[[:space:]]             ;
.                       {fprintf(stderr, "(lex) Caractère illégal (%d)\n", yytext[0]);}

%%
