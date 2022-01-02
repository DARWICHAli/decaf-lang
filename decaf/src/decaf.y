%{
#include <stdio.h>

extern int yylex();
void yyerror(const char *msg);
%}

%union {
    int _int_literal;
    int _hex_literal;
}
%token <_int_literal> DECIMAL_CST HEXADECIMAL_CST

%type <_int_literal> decimal_literal hex_literal
%type <_int_literal> int_literal expr

%token ADD SUB MUL DIV MOD
%token CLASS PROGRAM

%left SUB ADD
%left MUL DIV MOD 
%right UNEG

%start program

%%

program: CLASS PROGRAM '{' statement  '}'
;

statement: /* empty */
    | input
    /* | var_decl */
;


input: expr ';'             {printf("%d\n", $1);}
    | input expr ';'        {printf("%d\n", $2);}
;


expr: expr ADD expr         {$$ = $1 + $3;}
    | expr SUB expr         {$$ = $1 - $3;}
    | expr MUL expr         {$$ = $1 * $3;}
    | expr DIV expr         {$$ = $1 / $3;}
    | expr MOD expr         {$$ = $1 % $3;}
    | SUB expr %prec UNEG   {$$ = - $2;}
    | OPAR expr CPAR        {$$ = ($2);}
    | int_literal
;

int_literal: decimal_literal
    | hex_literal
;
decimal_literal: DECIMAL_CST
;
hex_literal: HEXADECIMAL_CST
;
%%

void yyerror(const char *msg)
{
    fprintf(stderr, "%s\n", msg);
    return;
}