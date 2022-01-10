%{
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "ir.h"
#include "symbols.h"
#include "incomplete.h"

extern int yylineno;
extern int yylex();
void yyerror(const char *msg);

struct Boolexp_t;

void reification(struct entry* dst, struct Boolexp_t* bexp, struct quad_list* nexto);

#define SERR(x, msg) do { if ((x)) { fprintf(stderr, "Erreur semantique: " msg); exit(EXIT_FAILURE);} } while(0)
#define SERRL(x, fct) do { if ((x)) { fprintf(stderr, "Erreur semantique: "); fct; exit(EXIT_FAILURE);} } while(0)

%}
%define parse.error verbose

%code requires {
#include "ir.h"
#include "symbols.h"

struct Boolexp_t {
    	struct quad_list* qltrue;
	struct quad_list* qlfalse;
};

}


%union {
    char Identifier[MAX_IDENTIFIER_SIZE];
    struct entry* Entry;
    const struct entry* CEntry;
    enum BTYPE BType;
    enum Q_OP Binop;
    struct context* Context;
    struct typelist* TypeList;
    int Integer;
    struct quad_list* Statement;
    struct Boolexp_t Boolexp;
    enum CMP_OP Relop;
    quad_id_t Nextquad;
    const char* String;
    enum Q_OP Aop;
}

%token CLASS VOID RETURN IF THEN ELSE WRITESTRING TRUE FALSE FOR
%token <Aop> EQI

%token <Integer> DECIMAL_CST HEXADECIMAL_CST
%token <BType> TYPE
%token <Identifier> ID
%token <Relop> RELOP
%token <String> CSTR

%type <Entry> new_entry existing_entry arithmetique_expression call parameter integer litteral arg lvalue rvalue boolval
%type <CEntry> cstr
%type <TypeList> optional_parameters
%type <TypeList> parameters
%type <Integer> int_cst
%type <Statement> instruction instructions gom iblock control optional_instructions loop affectation
%type <Boolexp> boolexp
%type <Nextquad> nqm

%left '-' '+'
%left '*' '/' '%'
%left MUNAIRE

%left DPIPE
%left DAMP
%left RELOP
%nonassoc '!'

%start program

%%

program: CLASS ID {ctx_push_super_global(); } '{' {ctx_pushctx();} global_declarations '}' {/*ctx_popctx();*/ /* Ne pas dépiler ce contexte !*/}
;
/*
 * Entrées et identifiants
 */

// Déclarations dans le contexte global
global_declarations: %empty
	| var_declaration global_declarations
	| tab_declaration global_declarations
	| method_declarations
;
optional_var_declarations: %empty
	| var_declaration optional_var_declarations
;
// Déclaration de variables
var_declaration: TYPE new_entry ';' { $2->type = typedesc_make_var($1); }
	| TYPE new_entry ',' { $2->type = typedesc_make_var($1); } new_id_list ';'
;

// Déclarations de tableaux
tab_declaration: TYPE new_entry '[' int_cst ']' ';' { $2->type = typedesc_make_tab($1, $4); }
    | TYPE new_entry '[' int_cst ']' ',' { $2->type = typedesc_make_tab($1, $4); } new_id_list_tab ';'
;
// Liste de nouveelles entrées
new_id_list: new_entry { $1->type = $<Entry>-2->type; }
	    | new_entry ',' { $1->type = $<Entry>-2->type; } new_id_list
;
// Liste de nouvelles entrées tab
new_id_list_tab: new_entry '[' int_cst ']' { $1->type = $<Entry>-5->type; }
	    | new_entry '[' int_cst ']' ',' { $1->type = $<Entry>-5->type; } new_id_list_tab
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
integer: DECIMAL_CST { $$ = ctx_make_temp(BT_INT); gencode(quad_cst($$, $1)); }
       | HEXADECIMAL_CST { $$ = ctx_make_temp(BT_INT); gencode(quad_cst($$, $1));  }
;

// litteraux
litteral: integer { $$ = $1; }
;
int_cst: DECIMAL_CST {$$ = $1; }
       | HEXADECIMAL_CST {$$ = $1; }
;
cstr: CSTR { $$ = ctx_register_cstr($1); }
;

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
;
proc_declaration: VOID  new_entry '(' { ctx_pushctx(); } optional_parameters ')' {
		$2->type = typedesc_make_function(BT_VOID, $5);
	} proc_block { ctx_popctx(); /* dépile contexte des args */}
;

func_declaration: TYPE new_entry '(' { ctx_pushctx(); } optional_parameters ')' {
		$2->type = typedesc_make_function($1, $5);
	} block { ctx_popctx(); /* dépile contexte des args */}
;

// paramètres de fonction/proc (opt)
optional_parameters: %empty {$$ = typelist_new(); }
	| parameters { $$ = $1; }
;

// Liste de paramètres
parameters: parameter { $$ = typelist_new(); typelist_append($$, typedesc_var_type(&$1->type)); } // toujours la fin de la liste
	| parameter ',' parameters { $$ = typelist_append($3, typedesc_var_type(&$1->type)); } // toujours le milieu
;
parameter: TYPE new_entry { $2->type = typedesc_make_var($1); $$ = $2; } // factoriser TYPE + new_entry ?
	| litteral { $$ = $1; }
;
/*
 * Blocs et code
 */

// bloc de procédure
proc_block: '{' { ctx_pushctx(); } optional_var_declarations optional_instructions '}' { gencode(quad_endproc()); ctx_popctx();}
;

// Bloc de code
block: '{' { ctx_pushctx(); } optional_var_declarations optional_instructions '}' { ctx_popctx();}
;
// Bloc internes
iblock: '{' {ctx_pushctx(); } optional_instructions '}' {ctx_popctx();} {$$ = qlist_empty(); }
;
// liste optionnelle d'instructions
optional_instructions: %empty { $$ = qlist_empty(); }
	| instructions { $$ = $1; }
;
// liste d'instructions
instructions: instruction ';' {$$ = qlist_empty();}
	| instruction ';' nqm instructions {$$ = $4; qlist_complete($1, $3);}
	| iblock nqm optional_instructions { $$ = $3; qlist_complete($1, $2); }
	| control nqm optional_instructions {$$ = $3; qlist_complete($1, $2); }
	| loop nqm optional_instructions { $$ = $3; qlist_complete($1, $2); }
;

// marqueur nextquad
nqm: %empty {$$ = nextquad();}
;
// instruction
instruction: affectation {$$ = $1;}
	| proc { $$ = qlist_empty();}
	| return { $$ = qlist_empty();}
;
/*
 * Structures de controle
 */
control: IF boolexp nqm iblock gom ELSE nqm iblock { // pas de backpatching des blocks ???
		qlist_complete($2.qltrue, $3);
		qlist_complete($2.qlfalse, $7);
		$$ = qlist_concat($4, $5);
		$$ = qlist_concat($$, $8);
	}
	| IF boolexp nqm iblock {
		qlist_complete($2.qltrue, $3);
		$$ = qlist_concat($2.qlfalse, $4);
	}
;
// marqueur goto
gom: %empty {$$ = qlist_new(nextquad()); gencode(quad_goto(INCOMPLETE_QUAD_ID));}

/*
 * Expressions booléennes
 */

boolexp: rvalue RELOP rvalue 		{
		$$.qltrue = qlist_new(nextquad());
		$$.qlfalse = qlist_new(nextquad() + 1);
		gencode(quad_ifgoto($1, $2, $3, INCOMPLETE_QUAD_ID));
		gencode(quad_goto(INCOMPLETE_QUAD_ID));
	}
	| '(' boolexp ')' {$$ = $2;}
	| boolexp DPIPE nqm boolexp 	{
		qlist_complete($1.qlfalse, $3);
		$$.qltrue = qlist_concat($1.qltrue, $4.qltrue);
		$$.qlfalse = $4.qlfalse;
	}
	| boolexp DAMP nqm boolexp  	{
		qlist_complete($1.qltrue, $3);
		$$.qltrue = $4.qltrue;
		$$.qlfalse = qlist_concat($1.qlfalse, $4.qlfalse);
	}
	| '!' boolexp 		{$$.qltrue = $2.qlfalse; $$.qlfalse = $2.qltrue; }
	| TRUE 				{$$.qltrue = qlist_new(nextquad()); gencode(quad_goto(INCOMPLETE_QUAD_ID)); }
	| FALSE 			{$$.qlfalse = qlist_new(nextquad()); gencode(quad_goto(INCOMPLETE_QUAD_ID)); }
;
/*
 * Boucles
 */

// borne de fin inclue et dynamique
loop: FOR {ctx_pushctx();}
    new_entry '=' rvalue
    ',' rvalue { gencode(quad_aff($3, $5)); $3->type = typedesc_make_var(BT_INT); }
    nqm { gencode(quad_ifgoto($3, CMP_GT, $7, INCOMPLETE_QUAD_ID)); }
    iblock {
    	qlist_complete($11, nextquad()); // goto cond
		struct entry* one = ctx_make_temp(BT_INT);
		gencode(quad_cst(one, 1));
		gencode(quad_arith($3, $3, Q_ADD, one));
		gencode(quad_goto($9));
		$$ = qlist_new($9);
		ctx_popctx();
    }
;
/*
 * Appel de fonction / procédure
 */

// appel de fonction
call: existing_entry '(' args_list_opt ')' {
		$$ = ctx_make_temp(typedesc_function_type(&$1->type));
		gencode(quad_call($$, $1)); // no type test !!!
	}
;
// appel de procédure
proc: existing_entry '(' args_list_opt ')' { gencode(quad_proc($1)); }
    | WRITESTRING '(' cstr ')' {
					gencode(quad_param($3));
					gencode(quad_proc(ctx_lookup(tokenize("WriteString"))));
				}
;
// liste des arguments de fonction
args_list_opt: %empty
	     | args
;
// arguments
args: arg { gencode(quad_param($1)); }
    | arg ',' args { gencode(quad_param($1)); }

// argument
arg: rvalue { $$ = $1; }
;
// retour de fonction
return: RETURN rvalue { gencode(quad_return($2)); }
;
/*
 * Expressions arithmétiques
 */

// affectation
affectation: lvalue '=' rvalue {
    $$ = qlist_empty();
   SERRL(!typedesc_equals(&$1->type, &$3->type), yyerror("types are not equal\n"));
   gencode(quad_aff($1, $3));
}
	   | existing_entry '[' rvalue ']' '=' rvalue
      {
         $$ = qlist_empty();
        if(!typedesc_is_var(&$6->type)|| !typedesc_is_tab(&$1->type) )
            exit(EXIT_FAILURE);
        if(typedesc_tab_type(&$1->type) != typedesc_var_type(&$6->type))
            exit(EXIT_FAILURE);
          gencode(quad_aft($1, $3, $6));
      }
       | lvalue '=' boolexp { $$ = qlist_empty(); reification($1, &$3, $$); }
       | existing_entry '[' rvalue ']' '=' boolexp {$$ = qlist_empty();
            struct entry* tmp = ctx_make_temp(BT_INT);
            reification(tmp, &$6, $$);
            gencode(quad_aft($1, $3, tmp));
            }
       | lvalue EQI rvalue {
           SERRL(!typedesc_equals(&$1->type, &td_var_int), fprintf(stderr, "type of rexpr is not int in arithmetic statement\n"));
           SERRL(!typedesc_equals(&$3->type, &td_var_int), fprintf(stderr, "type of lexpr is not int in arithmetic statement\n"));

           gencode(quad_arith($1, $1, $2, $3)); $$ = qlist_empty(); }
       | existing_entry '[' rvalue ']' EQI rvalue { $$ = qlist_empty();
            struct entry* tmp = ctx_make_temp(typedesc_tab_type(&$1->type));
            gencode(quad_acc(tmp, $1, $3));
            gencode(quad_aft($1, $3, tmp)); }
;



arithmetique_expression: rvalue '+' rvalue {
			SERRL(!typedesc_equals(&$1->type, &td_var_int), fprintf(stderr, "type of rexpr is not int in arithmetic statement\n"));
        	SERRL(!typedesc_equals(&$3->type, &td_var_int), fprintf(stderr, "type of lexpr is not int in arithmetic statement\n"));
			 $$ = ctx_make_temp(BT_INT); gencode(quad_arith($$, $1, Q_ADD, $3)); }
		| rvalue '-' rvalue {
        	SERRL(!typedesc_equals(&$1->type, &td_var_int), fprintf(stderr, "type of rexpr is not int in arithmetic statement\n"));
        	SERRL(!typedesc_equals(&$3->type, &td_var_int), fprintf(stderr, "type of lexpr is not int in arithmetic statement\n"));
			$$ = ctx_make_temp(BT_INT); gencode(quad_arith($$, $1, Q_SUB, $3)); }
		| rvalue '*' rvalue {
			SERRL(!typedesc_equals(&$1->type, &td_var_int), fprintf(stderr, "type of rexpr is not int in arithmetic statement\n"));
        	SERRL(!typedesc_equals(&$3->type, &td_var_int), fprintf(stderr, "type of lexpr is not int in arithmetic statement\n"));
			$$ = ctx_make_temp(BT_INT); gencode(quad_arith($$, $1, Q_MUL, $3)); }
		| rvalue '/' rvalue {
			SERRL(!typedesc_equals(&$1->type, &td_var_int), fprintf(stderr, "type of rexpr is not int in arithmetic statement\n"));
        	SERRL(!typedesc_equals(&$3->type, &td_var_int), fprintf(stderr, "type of lexpr is not int in arithmetic statement\n"));
			$$ = ctx_make_temp(BT_INT); gencode(quad_arith($$, $1, Q_DIV, $3)); }
		| rvalue '%' rvalue {
			SERRL(!typedesc_equals(&$1->type, &td_var_int), fprintf(stderr, "type of rexpr is not int in arithmetic statement\n"));
        	SERRL(!typedesc_equals(&$3->type, &td_var_int), fprintf(stderr, "type of lexpr is not int in arithmetic statement\n"));
			$$ = ctx_make_temp(BT_INT); gencode(quad_arith($$, $1, Q_MOD, $3)); }
		| '-' rvalue {
			SERRL(!typedesc_equals(&$2->type, &td_var_int), fprintf(stderr, "type of rexpr is not int in arithmetic statement\n"));
			$$ = ctx_make_temp(BT_INT);
			gencode(quad_neg($$, $2));} %prec MUNAIRE
;

/*
 * rvalue/lvalue
 */

rvalue: arithmetique_expression { $$ = $1; }
	| '(' rvalue ')' { $$ = $2; }
	| integer { $$ = $1; }
	| lvalue {$$ = $1; }
	| call {$$ = $1;}
	| existing_entry '[' rvalue ']' {
		if (!typedesc_is_tab(&$1->type))
			yyerror("Pas un tableau");

		$$ = ctx_make_temp(typedesc_tab_type(&$1->type));
		gencode(quad_acc($$, $1, $3));
	}
	| boolval 	{ $$ = $1; }
;
boolval: TRUE 	{ $$ = ctx_make_temp(BT_BOOL); gencode(quad_cst($$, 1)); }
	| FALSE 	{ $$ = ctx_make_temp(BT_BOOL); gencode(quad_cst($$, 0)); }
;
lvalue: existing_entry {$$ = $1;}
;
%%
void yyerror(const char *msg)
{
    fprintf(stderr, "yyerror on line %d: %s\n", yylineno, msg);
    exit(EXIT_FAILURE);
}

void quads_print() {
    fprintf(stderr, "Not implemented");
}

void reification(struct entry* dst, struct Boolexp_t* bexp, struct quad_list* nexto) {
	if (!typedesc_equals(&dst->type, &td_var_bool))
		yyerror("lvalue not boolean");

	quad_id_t m;
	m = nextquad();
	gencode(quad_cst(dst, 1));
	qlist_complete(bexp->qltrue, m);

	m = nextquad();
	gencode(quad_goto(INCOMPLETE_QUAD_ID));
	qlist_append(nexto, m);

	gencode(quad_cst(dst, 0));
	qlist_complete(bexp->qlfalse, m);
}
