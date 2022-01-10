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
EHEXDIGIT ({EDIGIT}|[a-fA-F])
EALPHANUM ({EALPHA}|{EDIGIT})
%%
\/\/.*                  ; /* \/\/[^\n] */

int                     { yylval.BType = BT_INT; return TYPE; }
boolean                 { yylval.BType = BT_BOOL; return TYPE; }
class                   return CLASS;
void                    return VOID;
return                  return RETURN;
if 			return IF;
else 			return ELSE;
for 			return FOR;
WriteString 		return WRITESTRING;
true 			return TRUE;
false 			return FALSE;

0x{EHEXDIGIT}+          {
    long val = strtol(yytext, NULL, 16);
    if (val >= 2147483647 || val <= -2147483648) {
        fprintf(stderr, "Integer overflow\n");
        exit(1);
    }
    if (yylval.Integer == 0 && errno != 0) {
        fprintf(stderr, "Error converting integer\n");
        exit(1);
    }
    yylval.Integer = val;
    return HEXADECIMAL_CST; }
{EDIGIT}+               {
    long val = strtol(yytext, NULL, 10);
    if (val >= 2147483647 || val <= -2147483648) {
        fprintf(stderr, "Integer overflow\n");
        exit(1);
    }
    if (yylval.Integer == 0 && errno != 0) {
        fprintf(stderr, "Error converting integer\n");
        exit(1);
    }
    yylval.Integer = val;
    return DECIMAL_CST; }

{EALPHA}{EALPHANUM}*    {
    if (yyleng >= MAX_IDENTIFIER_SIZE) {
        fprintf(stderr, "Identifier max length reached (%d characters)\n", MAX_IDENTIFIER_SIZE);
        exit(1);
    }
    strcpy(yylval.Identifier, yytext);
    return ID;
}


"+=" 			{yylval.Aop = Q_ADD; return EQI; }
"-=" 			{yylval.Aop = Q_SUB; return EQI; }
"*=" 			{yylval.Aop = Q_MUL; return EQI; }
"/=" 			{yylval.Aop = Q_DIV; return EQI; }
"%=" 			{yylval.Aop = Q_MOD; return EQI; }

[+\-*/%]                return yytext[0];
[(){},;]                return yytext[0];
[=]                     return yytext[0];
[\[\]]                  return yytext[0];

"<=" 			{yylval.Relop = CMP_LE; return RELOP; }
"=="			{yylval.Relop = CMP_EQ; return RELOP; }
">="			{yylval.Relop = CMP_GE; return RELOP; }
"!=" 			{yylval.Relop = CMP_NQ; return RELOP; }
"<"			{yylval.Relop = CMP_LT; return RELOP; }
">" 			{yylval.Relop = CMP_GT; return RELOP; }

\"([\t\n\\\|]|[^\"])*\"	    {
    const char * str = yytext;
    for (size_t i = 0; i < strlen(str) - 1; i++) {
        if ( (str[i] < 32) || (str[i] > 126) ) { // (str[i] != 9) || (str[i] != 10) ||
            fprintf(stderr, "Character is not accepted\n");
            exit(1);
        }
        if (str[i] == 91 && ((str[i + 1] != 39) || (str[i + 1] != 34)) ) {
            fprintf(stderr, "Character is not accepted\n");
            exit(1);
        }
    }
    yylval.String = yytext; return CSTR; }

"&&"			return DAMP;
"||" 			return DPIPE;
[!] 			return yytext[0];

[[:space:]]             ;
.                       {fprintf(stderr, "(lex) Caractère illégal: %c (%d)\n", yytext[0], yytext[0]);}

%%
