%{
#include <stdio.h>
#include <string.h>

extern int yylex();
void yyerror(const char *msg);

#include "ir.h"
#include "symbols.h"

%}
%define parse.error verbose
%code requires {
    #include "symbols.h"
}
%union {
    int _int_literal;
    int _hex_literal;
    struct {
        struct entry *_res;
        char _id[MAX_IDENTIFIER_SIZE];
    } exprval;
}
%token CLASS PROGRAM VOID
%token INT 

%token <_int_literal> DECIMAL_CST HEXADECIMAL_CST
%token <exprval> ID 

%type <exprval> expr location
%type <_int_literal> int_literal decimal_literal hex_literal

%left '-' '+'
%left '*' '/' '%' 
%nonassoc UNEG

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

id_list: ID                 {printf("var decl: %s\n", $1._id);}
    | id_list ',' ID        {printf("var decl: %s\n", $3._id);}
;


method_decl_opt: /* empty */
    | method_decl block
;

method_decl: VOID ID '(' ')'    {
        struct entry * e = ctx_newname($2._id);
        e->type.mtype = MT_FUN;
        printf("method_decl\n");}
    // | type ID '(' ')'        {struct entry * e = ctx_newname($2._id);ctx_pushctx();printf("method_decl\n");}
;

block: '{' var_decl_opt statement_opt '}' {
        struct context * n_c = ctx_pushctx();
        struct context * c_r = ctx_popctx();
}
;

var_decl_opt: /* empty */
    | var_decl ';'
;

var_decl: type id_list              // NOTE: Ajouter un moyen d'accéder au liste d'identifiant
    | var_decl ';' type id_list     // NOTE: pour remplir la table des symboles
;

type: INT
;

statement_opt:  /* empty */
    | statement ';'
;


statement: location assign_op expr {
        struct quad q = quad_aff($1._res, $3._res);
        quad_id_t qid = gencode(q);}
    | statement ';' location assign_op expr {
        struct quad q = quad_aff($3._res, $5._res);
        quad_id_t qid = gencode(q);}
;


assign_op: '='
;

location: ID    {
        $$._res = ctx_lookup($1._id);
}
;

expr: expr '+' expr {
        struct entry * res= ctx_make_temp();
        struct quad q = quad_arith(res, $1._res, Q_ADD, $3._res);
        quad_id_t qid = gencode(q);
        $$._res = res;}
    | expr '-' expr {
        struct entry * res = ctx_make_temp();
        struct quad q = quad_arith(res, $1._res, Q_SUB, $3._res);
        quad_id_t qid = gencode(q);
        $$._res = res;}
    | expr '*' expr {
        struct entry * res = ctx_make_temp();
        struct quad q = quad_arith(res, $1._res, Q_MUL, $3._res);
        quad_id_t qid = gencode(q);
        $$._res = res;}
    | expr '/' expr {
        struct entry * res = ctx_make_temp();
        struct quad q = quad_arith(res, $1._res, Q_DIV, $3._res);
        quad_id_t qid = gencode(q);
        $$._res = res;}
    | expr '%' expr {
        struct entry * res = ctx_make_temp();
        struct quad q = quad_arith(res, $1._res, Q_MOD, $3._res);
        quad_id_t qid = gencode(q);
        $$._res = res;}
    | '-' expr %prec UNEG {
        struct entry * res = ctx_make_temp();
        struct quad q = quad_neg(res, $2._res);
        quad_id_t qid = gencode(q);
        $$._res = res;}
    | '(' expr ')'      {$$ = $2;}
    | literal       //  NOTE: Bison types collides
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

%%
void yyerror(const char *msg)
{
    fprintf(stderr, "yyerror: %s\n", msg);
    return;
}

void quads_print() {
    fprintf(stderr, "Not implemented");
}