%{
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

extern int yylex();
void yyerror(const char *msg);

#include "ir.h"
#include "symbols.h"

#define SERR(x, msg) do { if ((x)) { fprintf(stderr, "Erreur semantique: " msg); exit(EXIT_FAILURE);} } while(0)
#define SERRL(x, fct) do { if ((x)) { fprintf(stderr, "Erreur semantique: "); fct; exit(EXIT_FAILURE);} } while(0)

%}
%define parse.error verbose

%code requires {
#include "ir.h"
#include "symbols.h"
#include <stdbool.h>
}


%union {
    int Integer;
    bool Boolean;
    char Identifier[MAX_IDENTIFIER_SIZE];
    struct entry* Entry;
    enum BTYPE BType;
    enum Q_OP Binop;
    struct context* Context;
}

%token CLASS VOID IF ELSE RETURN BREAK CONTINUE
%token LAND LOR EQUAL NEQUAL MORE_EQUAL LESS_EQUAL

%token <Integer> DECIMAL_CST HEXADECIMAL_CST
%token <Boolean> TRUE FALSE
%token <BType> TYPE
%token <Identifier> ID

%type <Entry> new_entry existing_entry expr 
%type <Integer> integer
%type <Boolean> bool_literal

%right MUNAIRE
%right '!'
%left '*' '/' '%'
%left '-' '+'
%left '<' LESS_EQUAL MORE_EQUAL '>'
%left EQUAL NEQUAL
%left LAND 
%left LOR

%start program

%%

program: CLASS ID '{' {ctx_pushctx();} optional_g_var_declarations optional_method_declarations '}' {/*ctx_popctx();*/ /* Ne pas dépiler ce contexte !*/}
;
/*
 * Entrées et identifiants
 */

// Déclaration de variables globales optionnelle
optional_g_var_declarations: %empty
		    | g_var_declaration optional_g_var_declarations
;
// Déclaration de variables globales
g_var_declaration: TYPE new_entry ';' { $2->type = typedesc_make_var($1); }
		| TYPE new_entry ',' { $2->type = typedesc_make_var($1); } new_id_list ';'
;
// Liste de nouveelles entrées
new_id_list: new_entry { $1->type = $<Entry>-2->type; }
	    | new_entry ',' { $1->type = $<Entry>-2->type; } new_id_list
;
// Nouvel identifiant
new_entry: ID {
			struct entry* ent = ctx_newname($1);
			SERRL(ent == NULL, fprintf(stderr, "%s is already declared in this context\n", $1));
			$$ = ent;
	      }
;
// identifiant préalablement déclaré (location)
existing_entry: ID {
    struct entry* ent = ctx_lookup($1);
    SERRL(ent == NULL, fprintf(stderr, "%s not declared in this context\n", $1));
    $$ = ent;
}
;

/*
 * Méthodes et fonctions
 */

// Déclaration optionnelle de méthode
optional_method_declarations: %empty
			    | method_declaration optional_method_declarations
;

// Déclaration de méthodes
method_declaration: VOID  new_entry '(' ')' {
    struct typelist* tl = typelist_new();
    $2->type = typedesc_make_function(BT_VOID, tl);
    ctx_pushctx();
    /* Ici empiler les paramètres de la fonction */
    ctx_pushctx();
    } proc_block        {ctx_popctx(); ctx_popctx();}
;
/*
 * Blocs et code
 */

// Bloc de procédure
proc_block: '{'         { ctx_pushctx(); } optional_var_declarations opt_statements '}' { gencode(quad_endproc()); ctx_popctx();}
;
// Bloc de code
block: '{'              { ctx_pushctx(); } optional_var_declarations opt_statements '}' { ctx_popctx();}
;

optional_var_declarations: %empty
    | var_declaration optional_var_declarations
;
var_declaration: TYPE new_entry ';' { $2->type = typedesc_make_var($1); }
	| TYPE new_entry ',' { $2->type = typedesc_make_var($1); } new_id_list ';'
;

opt_statements: %empty
    | statement opt_statements
;
statement: existing_entry '=' expr ';'
    | IF '(' expr ')' block 
	| IF '(' expr ')' block ELSE block
    | RETURN ';'
    | RETURN expr ';'
    | BREAK ';'
    | CONTINUE ';'
    | block
;

expr: existing_entry                { $$ = $1; }
    | integer                       { $$ = ctx_make_temp(); }
    | bool_literal                  { $$ = ctx_make_temp(); }
    | expr '+' expr                 { $$ = ctx_make_temp(); gencode(quad_arith($$, $1, Q_ADD, $3)); }
    | expr '-' expr                 { $$ = ctx_make_temp(); gencode(quad_arith($$, $1, Q_SUB, $3)); }
    | expr '*' expr                 { $$ = ctx_make_temp(); gencode(quad_arith($$, $1, Q_MUL, $3)); }
    | expr '/' expr                 { $$ = ctx_make_temp(); gencode(quad_arith($$, $1, Q_DIV, $3)); }
    | expr '%' expr                 { $$ = ctx_make_temp(); gencode(quad_arith($$, $1, Q_MOD, $3)); }
    | expr EQUAL expr               { $$ = ctx_make_temp(); }
    | expr NEQUAL expr              { $$ = ctx_make_temp(); }
    | expr LAND expr                { $$ = ctx_make_temp(); }
    | expr LOR expr                 { $$ = ctx_make_temp(); }
    | expr '<' expr                 { $$ = ctx_make_temp(); }
    | expr '>' expr                 { $$ = ctx_make_temp(); }
    | expr MORE_EQUAL expr          { $$ = ctx_make_temp(); }
    | expr LESS_EQUAL expr          { $$ = ctx_make_temp(); }
    | '-' expr                      { $$ = ctx_make_temp();gencode(quad_neg($$, $2)); } %prec MUNAIRE
    | '(' expr ')'                  { $$ = $2; }
    | '!' expr                      { $$ = $2; }

;

/*
 * Constantes et litteraux
 */

// Littérauux entiers
integer: DECIMAL_CST    { $$ = $1; }
    | HEXADECIMAL_CST   { $$ = $1; }
;
// Littérauux booléens
bool_literal: TRUE      { $$ = $1;}
    | FALSE             { $$ = $1;}
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
