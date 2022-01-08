%{
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

extern int yylineno;
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
    char Identifier[MAX_IDENTIFIER_SIZE];
    struct entry* Entry;
    enum BTYPE BType;
    enum Q_OP Binop;
    struct context* Context;
    struct typelist* TypeList;
    int Integer;
}

%token CLASS VOID RETURN

%token <Integer> DECIMAL_CST HEXADECIMAL_CST
%token <BType> TYPE
%token <Identifier> ID

%type <Entry> new_entry existing_entry arithmetique_expression negation_exp call parameter integer litteral arg lvalue rvalue
%type <TypeList> optional_parameters
%type <TypeList> parameters

%left '-' '+'
%left '*' '/' '%'
%left MUNAIRE

%start program

%%

program: CLASS ID '{' {ctx_pushctx();} global_declarations '}' {/*ctx_popctx();*/ /* Ne pas dépiler ce contexte !*/}
;
/*
 * Entrées et identifiants
 */

// Déclarations dans le contexte global
global_declarations: %empty
		     | var_declaration global_declarations
		     | method_declarations

optional_var_declarations: %empty
		     | var_declaration optional_var_declarations
;
// Déclaration de variables
var_declaration: TYPE new_entry ';' { $2->type = typedesc_make_var($1); }
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

// Littéraux entiers
integer: DECIMAL_CST { $$ = ctx_make_temp(); gencode(quad_cst($$, $1)); $$->type = typedesc_make_var(BT_INT); }
       | HEXADECIMAL_CST { $$ = ctx_make_temp(); gencode(quad_cst($$, $1)); $$->type = typedesc_make_var(BT_INT); }
;

// litteraux
litteral: integer { $$ = $1; }
/*
 * Méthodes et fonctions
 */

// Plusieurs déclarations de méthodes
method_declarations: method_declaration
		   | method_declaration method_declarations
;
// Déclaration de méthodes
method_declaration: proc_declaration
		  | func_declaration

proc_declaration: VOID  new_entry '(' { ctx_pushctx(); } optional_parameters ')' {
			$2->type = typedesc_make_function(BT_VOID, $5);
			} proc_block { ctx_popctx(); /* dépile contexte des args */}
;

func_declaration: TYPE new_entry '(' { ctx_pushctx(); } optional_parameters ')' {
			$2->type = typedesc_make_function($1, $5);
			} block { ctx_popctx(); /* dépile contexte des args */}


// paramètres de fonction/proc (opt)
optional_parameters: %empty {$$ = typelist_new(); }
		   | parameters { $$ = $1; }
		   ;

// Liste de paramètres
parameters: parameter { $$ = typelist_new(); typelist_append($$, typedesc_var_type(&$1->type)); } // toujours la fin de la liste
	  | parameter ',' parameters { $$ = typelist_append($3, typedesc_var_type(&$1->type)); } // toujours le milieu

parameter: TYPE new_entry { $2->type = typedesc_make_var($1); $$ = $2; } // factoriser TYPE + new_entry ?
	 | litteral { $$ = $1; }

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
;
// liste d'instructions
instructions: instruction ';'
	    | instruction ';' instructions
;

// instruction
instruction: affectation
	   | proc
	   | return // il faudrait faire un truc pour ne pas pouvoir return dans une procédure !

/*
 * Appel de fonction / procédure
 */

// appel de fonction
call: existing_entry '(' args_list_opt ')' {
		$$ = ctx_make_temp();
		$$->type = typedesc_make_var(typedesc_function_type(&$1->type));
    		gencode(quad_call($$, $1)); // no type test !!!
		}

// appel de procédure
proc: existing_entry '(' args_list_opt ')' { gencode(quad_proc($1)); }

// liste des arguments de fonction
args_list_opt: %empty
	     | args

// arguments
args: arg { gencode(quad_param($1)); }
    | arg ',' args { gencode(quad_param($1)); }

// argument
arg: rvalue { $$ = $1; }

// retour de fonction
return: RETURN rvalue { gencode(quad_return($2)); }

/*
 * Expressions arithmétiques
 */

// affectation
affectation: lvalue '=' rvalue { gencode(quad_aff($1, $3)); }
;

arithmetique_expression: rvalue '+' rvalue { $$ = ctx_make_temp(); $$->type = typedesc_make_var(BT_INT); gencode(quad_arith($$, $1, Q_ADD, $3)); }
		| rvalue '-' rvalue { $$ = ctx_make_temp(); $$->type = typedesc_make_var(BT_INT); gencode(quad_arith($$, $1, Q_SUB, $3)); }
		| rvalue '*' rvalue { $$ = ctx_make_temp(); $$->type = typedesc_make_var(BT_INT); gencode(quad_arith($$, $1, Q_MUL, $3)); }
		| rvalue '/' rvalue { $$ = ctx_make_temp(); $$->type = typedesc_make_var(BT_INT); gencode(quad_arith($$, $1, Q_DIV, $3)); }
		| rvalue '%' rvalue { $$ = ctx_make_temp(); $$->type = typedesc_make_var(BT_INT); gencode(quad_arith($$, $1, Q_MOD, $3)); }
		| negation_exp { $$ = $1; } %prec MUNAIRE
;
// moins unaire
negation_exp: '-' rvalue {
	    				$$ = ctx_make_temp();
					$$->type = typedesc_make_var(BT_INT);
					gencode(quad_neg($$, $2));
				   }
;

/*
 * rvalue/lvalue
 */

rvalue: arithmetique_expression { $$ = $1; }
      | '(' rvalue ')' { $$ = $2; }
      | integer { $$ = $1; }
      | lvalue {$$ = $1; }
      | call {$$ = $1;}

lvalue: existing_entry {$$ = $1;}

%%
void yyerror(const char *msg)
{
    fprintf(stderr, "yyerror on line %d: %s\n", yylineno, msg);
    return;
}

void quads_print() {
    fprintf(stderr, "Not implemented");
}
