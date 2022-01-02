%{
#include <stdio.h>

extern int yylex();
void yyerror(const char *msg);
%}


%union {
    int _int_literal;
}

// %token <_int_literal> TERMINAL
%token CLASS PROGRAM IF ELSE FOR RETURN BREAK CONTINUE
%token AS_EQUALS AS_PLUS_EQUALS AS_MINUS_EQUALS
%token INT BOOL TRUE FALSE
%token DECIMAL HEX
%token CHAR

%token LESS MORE LESS_EQUALS MORE_EQUALS
%token EQ_EQUALS NOT_EQUALS
%token AND OR
%token TRUE FALSE
// %type <_int_literal> rule

%start program

%%

program: CLASS PROGRAM '{' field_decl_star method_decl_star '}'
;

field_decl_star: /* empty (optional field_decl) */
    | field_decl_star field_decl
;

field_decl: type id_i ';'       /* Portée globale des variables déclarée */
;

id_i: id                        /* int id1; */
    | id '[' int_literal ']'    /* int id2[34]; */
    | id_i ',' id               /* int id1,id2,id3[21]; */
;

method_decl_star: /* empty (optional method_decl) */
    | method_decl_star method_decl
;

method_decl: id '(' m_parameters ')' block /* no return type */
    | type id '(' m_parameters ')' block
;
m_parameters: /* empty (no parameters) */
    | m_parameters_plus
;
m_parameters_plus: type id
    | m_parameters_plus ',' type id
;

block: '{' var_decl_star statement_star '}'
;

var_decl_star: /* empty */
    | var_decl ';'

;
var_decl: type id
    | var_decl ',' type id
;

type: INT /* TODO: to lex */
    | BOOL
;

statement_star: /* empty */
    | statement_star statement
;

statement: location assign_op expr ';' /* TODO: IF ELSE FOR RETURN BREAK CONTINUE */
    | method_call ';'
    | IF '(' expr ')' block
    | IF '(' expr ')' block ELSE block
    | FOR id '=' expr ',' expr block
    | RETURN ';'
    | RETURN expr ';'
    | BREAK ';'
    | CONTINUE ';'
    | block
;
assign_op: AS_EQUALS /* TODO: to lex */
    | AS_PLUS_EQUALS
    | AS_MINUS_EQUALS
;

method_call: method_name '(' expr ')' /* vide ou 1 ou >=2 */
;

method_name: id
;

location: id
    | id '[' expr ']'
;

expr: location
    | method_call
    | literal
    | expr bin_op expr
    | '-' expr
    | '!' expr
    | '(' expr ')'
;

bin_op: arith_op
    | rel_op
    | eq_op
    | cond_op
;

arith_op: '+'
    | '-'
    | '*'
    | '/'
    | '%'
;

rel_op: LESS /* TODO: to lex */
    | MORE
    | LESS_EQUALS
    | MORE_EQUALS
;

eq_op: EQ_EQUALS /* TODO: to lex */
    | NOT_EQUALS
;


cond_op: AND /* TODO: to lex */
    | OR
;

literal: int_literal
    | char_literal
    | bool_literal;

id:;

bool_literal: TRUE /* TODO: to lex */
    | FALSE
;

    /* ================= */
    /* ================= */
    /* ================= */
int_literal: decimal_literal
    | hex_literal
;

decimal_literal: DECIMAL
;
hex_literal: HEX
;

char_literal: '\'' CHAR  '\''
;

string_literal: '"' '"'
    | '"' string_literal_i '"'
;

string_literal_i: CHAR
    | string_literal_i CHAR
%%



void yyerror(const char *msg)
{
    fprintf(stderr, "%s\n", msg);
    return;
}
