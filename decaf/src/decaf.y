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
%left ADD SUB
%left MUL

%start input

%%

input: expr {printf("%d\n", $1);}

expr
: int_literal ADD int_literal {$$ = $1 + $3;}
| int_literal SUB int_literal {$$ = $1 - $3;}
| int_literal MUL int_literal {$$ = $1 * $3;}
| int_literal DIV int_literal {$$ = $1 / $3;}
| int_literal MOD int_literal {$$ = $1 % $3;}
| int_literal


int_literal
: decimal_literal
| hex_literal

decimal_literal: DECIMAL_CST
hex_literal: HEXADECIMAL_CST

%%

void yyerror(const char *msg)
{
    fprintf(stderr, "%s\n", msg);
    return;
}