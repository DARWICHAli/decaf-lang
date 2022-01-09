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
#define QLIST_NEWADD(lst) do { lst = qlist_new();qlist_append(&lst, nextquad()); } while(0)
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
    enum Q_OP Binop; // call in lexer, assign value directly
    struct context* Context;
    struct {
        struct entry * Entry;
        struct quad_list true_list;
        struct quad_list false_list;
    } Incomplete;
    quad_id_t Qid;
    struct typelist* TypeList;
}

%token CLASS VOID IF ELSE RETURN BREAK CONTINUE
%token LAND LOR EQUAL NEQUAL MORE_EQUAL LESS_EQUAL
    

%token <Integer> DECIMAL_CST HEXADECIMAL_CST
%token <Boolean> TRUE FALSE
%token <BType> TYPE
%token <Identifier> ID

%type <Entry> new_entry existing_entry  
%type <Incomplete> expr bool_literal
%type <Integer> integer
%type <Qid> m
%type <Entry> new_entry existing_entry arithmetique_expression negation_exp call parameter integer litteral arg lvalue rvalue
%type <TypeList> optional_parameters
%type <TypeList> parameters

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

// program: CLASS ID '{' {ctx_pushctx();} optional_g_var_declarations optional_method_declarations '}' {/*ctx_popctx();*/ /* Ne pas dépiler ce contexte !*/}
// ;
program: CLASS ID '{' {ctx_pushctx();} global_declarations '}' {/*ctx_popctx();*/ /* Ne pas dépiler ce contexte !*/}
;


/*
 * Entrées et identifiants
 */

    // Déclaration de variables globales optionnelle
    // optional_g_var_declarations: %empty
    // 		    | g_var_declaration optional_g_var_declarations
    // ;
    // Déclaration de variables globales
    // g_var_declaration: TYPE new_entry ';' { $2->type = typedesc_make_var($1); }
    //;
// Déclarations dans le contexte global
global_declarations: %empty
	| var_declaration global_declarations
	| method_declarations
;
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
// identifiant préalablement déclaré (location)
existing_entry: ID {
    struct entry* ent = ctx_lookup($1);
    SERRL(ent == NULL, fprintf(stderr, "%s not declared in this context\n", $1));
    $$ = ent;
}
;

// Littéraux entiers
integer: DECIMAL_CST { $$ = ctx_make_temp(); gencode(quad_cst($$, $1)); $$->type = typedesc_make_var(BT_INT); }
       | HEXADECIMAL_CST { $$ = ctx_make_temp(); gencode(quad_cst($$, $1)); $$->type = typedesc_make_var(BT_INT); }
;

// litteraux
litteral: integer { $$ = $1; }
/*
 * Méthodes et fonctions
 */

// Déclaration optionnelle de méthode
optional_method_declarations: %empty
			    | method_declaration optional_method_declarations
// Plusieurs déclarations de méthodes
method_declarations: method_declaration
		   | method_declaration method_declarations
;

// Déclaration de méthodes
                // method_declaration: VOID  new_entry '(' ')' {struct typelist* tl = typelist_new();$2->type = typedesc_make_function(BT_VOID, tl);ctx_pushctx();/* Ici empiler les paramètres de la fonction */ctx_pushctx();} proc_block        {ctx_popctx(); ctx_popctx();}
                // | TYPE new_entry '(' { struct typelist* tl = typelist_new(); $2->type = typedesc_make_function(BT_INT, tl); ctx_pushctx(); ctx_pushctx();} ')' {} proc_block {ctx_popctx(); ctx_popctx();}
                    ;
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
statement: existing_entry '=' expr ';' {
        SERRL(!typedesc_equals(&$1->type, &$3.Entry->type), fprintf(stderr, "type of values in assignement statement does not match\n"));
        gencode(quad_aff($1, $3.Entry));
    }
    | IF '(' expr ')' block {
        SERRL(!typedesc_equals(&$3.Entry->type, &td_var_bool), fprintf(stderr, "type of expr is not boolean in if statement\n"));
    }
	| IF '(' expr ')' block ELSE block
    | RETURN ';'
    | RETURN expr ';'
    | BREAK ';'
    | CONTINUE ';'
    | block
;
    // NOTE: il faut effectuer les vérification de types ici 
    // "==" "!=" "&&" "||" sont les seuls opérateurs sur les booléens autorisées
    // Il faut lever une erreur, (la syntaxe est bonne mais pas la sémantique)
expr: existing_entry                { $$.Entry = $1; }
    | integer                       { $$.Entry = ctx_make_temp(BT_INT);}
    | bool_literal                  { $$.Entry = ctx_make_temp(BT_BOOL);}
    | expr '+' expr                 {
        SERRL(!typedesc_equals(&$1.Entry->type, &td_var_int), fprintf(stderr, "type of expr is not int in arithmetic statement\n"));
        SERRL(!typedesc_equals(&$3.Entry->type, &td_var_int), fprintf(stderr, "type of expr is not int in arithmetic statement\n"));
        $$.Entry = ctx_make_temp(BT_INT);
        gencode(quad_arith($$.Entry, $1.Entry, Q_ADD, $3.Entry));
    }
    | expr '-' expr                 {
        SERRL(!typedesc_equals(&$1.Entry->type, &td_var_int), fprintf(stderr, "type of expr is not int in arithmetic statement\n"));
        SERRL(!typedesc_equals(&$3.Entry->type, &td_var_int), fprintf(stderr, "type of expr is not int in arithmetic statement\n"));
        $$.Entry = ctx_make_temp(BT_INT);
        gencode(quad_arith($$.Entry, $1.Entry, Q_SUB, $3.Entry));
    }
    | expr '*' expr                 {
        SERRL(!typedesc_equals(&$1.Entry->type, &td_var_int), fprintf(stderr, "type of expr is not int in arithmetic statement\n"));
        SERRL(!typedesc_equals(&$3.Entry->type, &td_var_int), fprintf(stderr, "type of expr is not int in arithmetic statement\n"));
        $$.Entry = ctx_make_temp(BT_INT);
        gencode(quad_arith($$.Entry, $1.Entry, Q_MUL, $3.Entry));
    }
    | expr '/' expr                 {
        SERRL(!typedesc_equals(&$1.Entry->type, &td_var_int), fprintf(stderr, "type of expr is not int in arithmetic statement\n"));
        SERRL(!typedesc_equals(&$3.Entry->type, &td_var_int), fprintf(stderr, "type of expr is not int in arithmetic statement\n"));
        $$.Entry = ctx_make_temp(BT_INT);
        gencode(quad_arith($$.Entry, $1.Entry, Q_DIV, $3.Entry)); }
    | expr '%' expr                 {
        SERRL(!typedesc_equals(&$1.Entry->type, &td_var_int), fprintf(stderr, "type of expr is not int in arithmetic statement\n"));
        SERRL(!typedesc_equals(&$3.Entry->type, &td_var_int), fprintf(stderr, "type of expr is not int in arithmetic statement\n"));
        $$.Entry = ctx_make_temp(BT_INT);
        gencode(quad_arith($$.Entry, $1.Entry, Q_MOD, $3.Entry));
    }
    | expr EQUAL expr               {
        SERRL(!typedesc_equals(&$1.Entry->type, &$3.Entry->type), fprintf(stderr, "type of lexpr is not the same as rexpr\n"));

        QLIST_NEWADD($$.true_list); // $$.true_list = qlist_new(); // qlist_append(&$$.true_list, nextquad());
        gencode(quad_ifgoto($1.Entry, CMP_EQ, $3.Entry, 0));
        QLIST_NEWADD($$.false_list); // $$.false_list = qlist_new(); // qlist_append(&$$.false_list, nextquad());
        gencode(quad_goto(0));
    }
    | expr NEQUAL expr              {
        SERRL(!typedesc_equals(&$1.Entry->type, &$3.Entry->type), fprintf(stderr, "type of lexpr is not the same as rexpr\n"));

        QLIST_NEWADD($$.true_list); // $$.true_list = qlist_new(); // qlist_append(&$$.true_list, nextquad());
        gencode(quad_ifgoto($1.Entry, CMP_EQ, $3.Entry, 0)); // NOTE: to change
        QLIST_NEWADD($$.false_list); // $$.false_list = qlist_new(); // qlist_append(&$$.false_list, nextquad());
        gencode(quad_goto(0));
    }
    | expr LOR m expr             {
        qlist_complete(&$1.false_list, $3);
        $$.true_list = qlist_concat(&$1.true_list, &$4.true_list);
        $$.false_list = $4.false_list;
    }
    | expr LAND m expr              {
        qlist_complete(&$1.false_list, $3);
        $$.true_list = $4.true_list;
        $$.false_list = qlist_concat(&$1.false_list, &$4.false_list);
    }
    | expr '<' expr                 {
        SERRL(!typedesc_equals(&$1.Entry->type, &td_var_int), fprintf(stderr, "type of expr is not int in comparison statement\n"));
        SERRL(!typedesc_equals(&$3.Entry->type, &td_var_int), fprintf(stderr, "type of expr is not int in comparison statement\n"));

        QLIST_NEWADD($$.true_list); // $$.true_list = qlist_new(); // qlist_append(&$$.true_list, nextquad());
        gencode(quad_ifgoto($1.Entry, CMP_LT, $3.Entry, 0));
        QLIST_NEWADD($$.false_list); // $$.false_list = qlist_new(); // qlist_append(&$$.false_list, nextquad());
        gencode(quad_goto(0));
    }
    | expr '>' expr                 {
        SERRL(!typedesc_equals(&$1.Entry->type, &td_var_int), fprintf(stderr, "type of expr is not int in comparison statement\n"));
        SERRL(!typedesc_equals(&$3.Entry->type, &td_var_int), fprintf(stderr, "type of expr is not int in comparison statement\n"));

        QLIST_NEWADD($$.true_list); // $$.true_list = qlist_new(); // qlist_append(&$$.true_list, nextquad());
        gencode(quad_ifgoto($1.Entry, CMP_GT, $3.Entry, 0));
        QLIST_NEWADD($$.false_list); // $$.false_list = qlist_new(); // qlist_append(&$$.false_list, nextquad());
        gencode(quad_goto(0));
    }
    | expr MORE_EQUAL expr          {
        SERRL(!typedesc_equals(&$1.Entry->type, &td_var_int), fprintf(stderr, "type of expr is not int in comparison statement\n"));
        SERRL(!typedesc_equals(&$3.Entry->type, &td_var_int), fprintf(stderr, "type of expr is not int in comparison statement\n"));

        QLIST_NEWADD($$.true_list);
        gencode(quad_ifgoto($1.Entry, CMP_GE, $3.Entry, 0));
        QLIST_NEWADD($$.false_list);
        gencode(quad_goto(0));
    }
    | expr LESS_EQUAL expr          {
        SERRL(!typedesc_equals(&$1.Entry->type, &td_var_int), fprintf(stderr, "type of expr is not int in comparison statement\n"));
        SERRL(!typedesc_equals(&$3.Entry->type, &td_var_int), fprintf(stderr, "type of expr is not int in comparison statement\n"));

        QLIST_NEWADD($$.true_list);
        gencode(quad_ifgoto($1.Entry, CMP_LE, $3.Entry, 0));
        QLIST_NEWADD($$.false_list);
        gencode(quad_goto(0));
    }
    | '-' expr                      {
        SERRL(!typedesc_equals(&$2.Entry->type, &td_var_int), fprintf(stderr, "type of expr is not int in arithmetic statement\n"));

        $$.Entry = ctx_make_temp(BT_INT);
        gencode(quad_neg($$.Entry, $2.Entry));
    } %prec MUNAIRE
    | '(' expr ')'                  { $$ = $2; }
    | '!' expr                      {
        $$.Entry = ctx_make_temp(BT_BOOL);
        SERRL(!typedesc_equals(&($2.Entry->type), &td_var_bool), fprintf(stderr, "type of expr is not boolean following NOT\n"));
        struct quad_list temp = $$.true_list;
        $$.true_list = $$.false_list;
        $$.false_list = temp; 
    }
;


m: %empty      { $$ = nextquad();}
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
 * Constantes et litteraux
 */

// Littéraux entiers
integer: DECIMAL_CST    { $$ = $1; }
    | HEXADECIMAL_CST   { $$ = $1; }
;
// Littéraux booléens
bool_literal: TRUE  {
        $$.Entry = ctx_make_temp(BT_BOOL);
        quad_cst($$.Entry, 1);    // cst, $$.true_list = qlist_new(); qlist_append(&$$.true_list, nextquad()); gencode(quad_goto(0));
    }
    | FALSE         {
        $$.Entry = ctx_make_temp(BT_BOOL);
        quad_cst($$.Entry, 0);
    }
;
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
