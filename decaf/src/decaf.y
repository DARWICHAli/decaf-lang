%{
#include <stdio.h>
#include <string.h>

extern int yylex();
void yyerror(const char *msg);
%}
%define parse.error verbose
%union {
    int _int_literal;
    int _hex_literal;
    char _id[128];
}
%token CLASS PROGRAM
%token INT 

%token <_int_literal> DECIMAL_CST HEXADECIMAL_CST
%token <_id> ID

%type <_int_literal> decimal_literal hex_literal
%type <_int_literal> int_literal expr

%left '-' '+'
%left '*' '/' '%' 
%right UNEG

%start program

%%

program: CLASS PROGRAM '{' field_decl_opt statement_opt '}'
;

field_decl_opt: /* empty */
    | field_decl ';'
;
field_decl: type id_list
    | field_decl ';' type id_list
;

id_list: ID                 {printf("var decl: %s\n", $1);}
    | id_list ',' ID        {printf("var decl: %s\n", $3);}
;

type: INT
;

statement_opt: /* empty */
    | line
;

line: expr ';'              {printf("%d\n", $1);}
    | line expr ';'         {printf("%d\n", $2);}
;

expr: expr '+' expr         {$$ = $1 + $3;}
    | expr '-' expr         {$$ = $1 - $3;}
    | expr '*' expr         {$$ = $1 * $3;}
    | expr '/' expr         {$$ = $1 / $3;}
    | expr '%' expr         {$$ = $1 % $3;}
    | '-' expr %prec UNEG   {$$ = - $2;}
    | '(' expr ')'          {$$ = ($2);}
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
    fprintf(stderr, "yyerror: %s\n", msg);
    return;
}