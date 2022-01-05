%{
#include <stdio.h>
#include <string.h>

extern int yylex();
void yyerror(const char *msg);

#include "ir.h"
#include "symbols.h"

%}
%define parse.error verbose
%union {
    int _int_literal;
    int _hex_literal;
    char _id[128];
}
%token CLASS PROGRAM VOID
%token INT 

%token <_int_literal> DECIMAL_CST HEXADECIMAL_CST
%token <_id> ID

%type <_int_literal> int_literal decimal_literal hex_literal

%left '-' '+'
%left '*' '/' '%' 
%right UNEG

%start program

%%

program: CLASS PROGRAM '{' field_decl_opt method_decl_opt '}'
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


method_decl_opt: /* empty */
    | method_decl block
;

method_decl: VOID ID '(' ')'    {printf("method_decl: void: %s\n", $2);}
    // | type ID '(' ')'        {printf("method_decl: type: %s\n", $2);}
;

block: '{' var_decl_opt statement_opt '}'
;

var_decl_opt: /* empty */
    | var_decl ';'
;

var_decl: type id_list
    | var_decl ';' type id_list
;

type: INT
;

statement_opt:  /* empty */
    | statement ';'
;


statement: location assign_op expr                {printf("stmt\n");}
    | statement ';' location '=' expr
;


assign_op: '='
;

location: ID
;

expr: expr '+' expr
    | expr '-' expr
    | expr '*' expr
    | expr '/' expr
    | expr '%' expr
    | '-' expr %prec UNEG
    | '(' expr ')'
    | literal
    | location
;

literal: int_literal
;

int_literal: decimal_literal
    | hex_literal
;
decimal_literal: DECIMAL_CST
;
hex_literal: HEXADECIMAL_CST
;



arith_op: '+'
    | '-'
    | '*'
    | '/'
    | '%'
;

%%
void yyerror(const char *msg)
{
    fprintf(stderr, "yyerror: %s\n", msg);
    return;
}

void quads_print() {
    fprintf(stderr, "Not implemented");
}