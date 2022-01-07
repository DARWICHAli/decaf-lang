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
}


%union {
    int Integer;
    char Identifier[MAX_IDENTIFIER_SIZE];
    struct entry* Entry;
    enum BTYPE BType;
    enum Q_OP Binop;
    struct context* Context;
}

%token CLASS PROGRAM VOID IF ELSE

%token <Integer> DECIMAL_CST HEXADECIMAL_CST
%token <BType> TYPE
%token <Identifier> ID

%type <Entry> new_entry existing_entry arithmetique_exp negation_exp
%type <Integer> integer

%left '-' '+'
%left '*' '/' '%'
%left MUNAIRE

%start program

%%

program: CLASS PROGRAM '{' {ctx_pushctx();} optional_var_declarations optional_method_declarations '}' {/*ctx_popctx();*/ /* Ne pas dépiler ce contexte !*/}
;
/*
 * Entrées et identifiants
 */

// Déclaration de variables optionnelle
optional_var_declarations: %empty
		     | var_declarations
;
// Déclaration de variables
var_declarations: TYPE new_entry ';' { $2->type = typedesc_make_var($1); }
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
// identifiant préalablement déclaré
existing_entry: ID {
	      		struct entry* ent = ctx_lookup($1);
			SERRL(ent == NULL, fprintf(stderr, "%s not declared in this context\n", $1));
			$$ = ent;
		   }
;
/*
 * Constantes et litteraux
 */

// Littérauux entiers
integer: DECIMAL_CST { $$ = $1; }
       | HEXADECIMAL_CST { $$ = $1; }
;
/*
 * Méthodes et fonctions
 */

// Déclaration optionnelle de méthode
optional_method_declarations: %empty
			    | method_declarations
;
// Plusieurs déclarations de méthodes
method_declarations: method_declaration
		   | method_declaration method_declarations
;
// Déclaration de méthodes
method_declaration: VOID  new_entry '(' ')' {
		  	struct typelist* tl = typelist_new();
			$2->type = typedesc_make_function(BT_VOID, tl);
			} proc_block
;
/*
 * Blocs et code
 */

// bloc de procédure
proc_block: '{' { ctx_pushctx(); } optional_var_declarations optional_instructions '}' { gencode(quad_endproc()); ctx_popctx();}
;

// blocks
blocks: block
      | block blocks
;
// Bloc de code
block: '{' { ctx_pushctx(); } optional_var_declarations optional_instructions '}' { ctx_popctx();}
;
// liste optionnelle d'instructions
optional_instructions: %empty
		     | instructions
		     | blocks
			 | instructions_condi
;
// liste d'instructions
instructions: affectation ';'
	    | affectation ';' instructions
;
// instructions conditionnelles
instructions_condi: instruction_condi
	| instruction_condi instructions_condi
;

instruction_condi: IF '(' expression ')' block 
	| IF '(' expression ')' block ELSE block
;
/*
 * Expressions
 */
expression: location
	| instructions
	| arithmetique_exp
;

location: ID
;

/*
 * Expressions arithmétiques
 */

// affectation
affectation: existing_entry '=' arithmetique_exp { gencode(quad_aff($1, $3)); }
;
// expression arithmétique
arithmetique_exp: arithmetique_exp '+' arithmetique_exp { $$ = ctx_make_temp(); gencode(quad_arith($$, $1, Q_ADD, $3)); }
		| arithmetique_exp '-' arithmetique_exp { $$ = ctx_make_temp(); gencode(quad_arith($$, $1, Q_SUB, $3)); }
		| arithmetique_exp '*' arithmetique_exp { $$ = ctx_make_temp(); gencode(quad_arith($$, $1, Q_MUL, $3)); }
		| arithmetique_exp '/' arithmetique_exp { $$ = ctx_make_temp(); gencode(quad_arith($$, $1, Q_DIV, $3)); }
		| arithmetique_exp '%' arithmetique_exp { $$ = ctx_make_temp(); gencode(quad_arith($$, $1, Q_MOD, $3)); }
		| negation_exp { $$ = $1; } %prec MUNAIRE
		| '(' arithmetique_exp ')' { $$ = $2; }
		| integer { $$ = ctx_make_temp(); gencode(quad_cst($$, $1)); }
		| existing_entry {$$ = $1; }
;
// moins unaire
negation_exp: '-' arithmetique_exp {
	    				$$ = ctx_make_temp();
					gencode(quad_neg($$, $2));
				   }
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
