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
    struct {
        struct entry * Entry;
        struct quad_list true_list;
        struct quad_list false_list;
    } Incomplete;
    struct typelist* TypeList;
    quad_id_t Qid;
    struct {
        struct quad_list next_list;
    } IncompleteStatements;

    enum CMP_OP Relop;
}

%token CLASS VOID IF ELSE RETURN BREAK CONTINUE
%token AND OR

%token <Integer> DECIMAL_CST HEXADECIMAL_CST 
%token <Boolean> TRUE FALSE
%token <BType> TYPE
%token <Identifier> ID
%token <Relop> RELOP

%type <Entry> new_entry existing_entry func_call parameter literal integer bool_literal arg a_expr assignement
%type <Incomplete> expr
%type <IncompleteStatements> proc_block block g opt_instructions instructions instruction i_block control
%type <TypeList> optional_parameters
%type <TypeList> parameters
%type <Qid> m
%type <Integer> int_cst


%right '!'
%left '-' '+'
%left '*' '/' '%'
%nonassoc MUNAIRE
%left '<' LESS_EQUAL MORE_EQUAL '>'
%left EQUAL NEQUAL
%left LAND 
%left LOR

%start program

%%

program: CLASS ID '{' {ctx_pushctx();} global_declarations '}' {/*ctx_popctx();*/ /* Ne pas dépiler ce contexte !*/}
;

// Déclarations dans le contexte global
global_declarations: %empty
    | g_var_declaration global_declarations
    | method_declarations
;


/*
 * Entrées et identifiants
 */
// Déclaration de variables globales
g_var_declaration: TYPE new_entry ';'       { $2->type = typedesc_make_var($1); }
    | TYPE new_entry ','                    { $2->type = typedesc_make_var($1); } new_id_list ';'
    | TYPE new_entry '[' int_cst ']' ';'    { $2->type = typedesc_make_tab($1, $4); }
    | TYPE new_entry '[' int_cst ']' ','    { $2->type = typedesc_make_tab($1, $4); } new_id_list ';'
;
int_cst: DECIMAL_CST    { $$ = $1; }
    | HEXADECIMAL_CST   { $$ = $1; }
;
// Liste des nouvelles entrées
new_id_list: new_entry              { $1->type = $<Entry>-2->type; }
    | new_entry ','                 { $1->type = $<Entry>-2->type; } new_id_list
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
optional_var_declarations: %empty
    | var_declaration optional_var_declarations
;
var_declaration: TYPE new_entry ';' { $2->type = typedesc_make_var($1); }
	| TYPE new_entry ',' { $2->type = typedesc_make_var($1); } new_id_list ';'
;
/*
 * Méthodes et fonctions
 */
// Déclaration de plusieures méthodes 
method_declarations: method_declaration
    | method_declaration method_declarations
;
// Déclaration de méthodes
method_declaration: procedure_decl
    | function_decl
;
procedure_decl: VOID new_entry '(' { ctx_pushctx(); } optional_parameters ')' {
        $2->type = typedesc_make_function(BT_VOID, $5);
        } proc_block { ctx_popctx(); /* dépile contexte des args */}
;
function_decl: TYPE new_entry '(' { ctx_pushctx(); } optional_parameters ')' {
		$2->type = typedesc_make_function($1, $5);
		} block { ctx_popctx(); /* dépile contexte des args */}
;

// Paramètres de fonctions
optional_parameters: %empty     {$$ = typelist_new(); }
    | parameters                { $$ = $1; }
;

// Liste de paramètres
parameters: parameter           { $$ = typelist_new(); typelist_append($$, typedesc_var_type(&$1->type)); } // toujours la fin de la liste
    | parameter ',' parameters  { $$ = typelist_append($3, typedesc_var_type(&$1->type)); } // toujours le milieu
;
parameter: TYPE new_entry       { $2->type = typedesc_make_var($1); $$ = $2; } // factoriser TYPE + new_entry ?
    | literal                   { $$ = $1; }
;
/*
 * Blocs et code
 */
// Bloc de procédure
proc_block: '{' { ctx_pushctx(); } optional_var_declarations opt_instructions '}' { gencode(quad_endproc()); ctx_popctx();}
;
// Bloc de code
block: '{' { ctx_pushctx(); } optional_var_declarations opt_instructions '}' { ctx_popctx();}
;
// Bloc de code interne
i_block: '{' {ctx_pushctx(); } opt_instructions '}' {ctx_popctx();}
;
/*
 * Appel de fonction / procédure
 */

// appel de fonction
func_call: existing_entry '(' args_list_opt ')' {
		$$ = ctx_make_temp(typedesc_function_type(&$1->type));
    	gencode(quad_call($$, $1)); // no type test !!!
	}
;
// appel de procédure
proc_call: existing_entry '(' args_list_opt ')' { gencode(quad_proc($1)); }
;
// liste des arguments de fonction
args_list_opt: %empty
    | args
;
// arguments
args: arg           { gencode(quad_param($1)); }
    | arg ',' args  { gencode(quad_param($1)); }
;
// argument
arg: expr { $$ = $1.Entry; }
;

/*
 * Instructions
 */
opt_instructions: %empty    { $$.next_list = qlist_new(); }
    | instructions          { $$ = $1; }
;

instructions: instruction ';'           { $$.next_list = qlist_new(); }
    | instruction ';' m instructions    { $$.next_list = $4.next_list; qlist_complete(&$1.next_list, $3); }
    | i_block m opt_instructions        { $$.next_list = $3.next_list; qlist_complete(&$1.next_list, $2); }
	| control m opt_instructions        { $$.next_list = $3.next_list; qlist_complete(&$1.next_list, $2); }
;

control: IF expr m i_block g ELSE m i_block {
        qlist_complete(&$2.true_list, $3);
        qlist_complete(&$2.false_list, $7);
        $$.next_list = qlist_concat(&$4.next_list, &$5.next_list);
        $$.next_list = qlist_concat(&$$.next_list, &$8.next_list);
    }
    | IF expr m i_block {
        qlist_complete(&$2.true_list, $3);
        $$.next_list = qlist_concat(&$2.false_list, &$4.next_list);
    }
;

instruction: assignement    { $$.next_list = qlist_new(); }
    | proc_call             { $$.next_list = qlist_new(); }
    | RETURN                { $$.next_list = qlist_new(); }
;

assignement: existing_entry '=' expr        { gencode(quad_aff($1, $3.Entry)); }
    | existing_entry '[' expr ']' '=' expr  { gencode(quad_aft($1, $3.Entry, $6.Entry)); }
;

expr: a_expr                        { $$.Entry = $1; }
    | literal                       { $$.Entry = $1; }
    | func_call                     { $$.Entry = $1; }
    | existing_entry                { $$.Entry = $1; }
    | existing_entry '[' expr ']'   {
        if (!typedesc_is_tab(&$1->type))
            yyerror("Pas un tableau");
        // $$.Entry = ctx_make_temp($1.type);
        $$.Entry->type = typedesc_make_var(typedesc_tab_type(&$1->type));
        gencode(quad_acc($$.Entry, $1, $3.Entry));
    }
    | expr RELOP expr       { 
        $$.true_list = qlist_new(); qlist_append(&$$.true_list, nextquad());
        $$.false_list = qlist_new(); qlist_append(&$$.false_list, nextquad() + 1);
        gencode(quad_ifgoto($1.Entry, $2, $3.Entry, INCOMPLETE_QUAD_ID));
        gencode(quad_goto(INCOMPLETE_QUAD_ID));
    }
	| '(' expr ')' {$$ = $2;}

;

a_expr: a_expr '+' a_expr   {
        $$ = ctx_make_temp(BT_INT);
        gencode(quad_arith($$, $1, Q_ADD, $3));
}
    | a_expr '-' a_expr     {
        $$ = ctx_make_temp(BT_INT);
        gencode(quad_arith($$, $1, Q_SUB, $3));
    }
    | a_expr '*' a_expr     {
        $$ = ctx_make_temp(BT_INT);
        gencode(quad_arith($$, $1, Q_MUL, $3));
    }
    | a_expr '/' a_expr     {
        $$ = ctx_make_temp(BT_INT);
        gencode(quad_arith($$, $1, Q_DIV, $3));
    }
    | a_expr '%' a_expr     {
        $$ = ctx_make_temp(BT_INT);
        gencode(quad_arith($$, $1, Q_MOD, $3));
    }
    | '-' a_expr            {
        SERRL(!typedesc_equals(&$2->type, &td_var_int), fprintf(stderr, "type of expr is not int in arithmetic statement\n"));
        $$ = ctx_make_temp(BT_INT);
        gencode(quad_neg($$, $2));
    } %prec MUNAIRE
    | integer               { $$ = $1; }
;


/*
 * Marqueurs
 */
m: %empty       { $$ = nextquad();}
;
g: %empty {
    QLIST_NEWADD($$.next_list);
    gencode(quad_goto(INCOMPLETE_QUAD_ID));
}
;
/*
 * Constantes et litteraux
 */
literal: integer
    | bool_literal
;
// Littéraux entiers
integer: DECIMAL_CST    { 
        $$ = ctx_make_temp(BT_INT);
        gencode(quad_cst($$, $1));
    }
    | HEXADECIMAL_CST   {
        $$ = ctx_make_temp(BT_INT);
        gencode(quad_cst($$, $1));
    }
;
// Littéraux booléens
bool_literal: TRUE  {
        $$= ctx_make_temp(BT_BOOL);
        gencode(quad_cst($$, 1));    // cst, $$.true_list = qlist_new(); qlist_append(&$$.true_list, nextquad()); gencode(quad_goto(0));
    }
    | FALSE         {
        $$ = ctx_make_temp(BT_BOOL);
        gencode(quad_cst($$, 0));
    }
;
%%
void yyerror(const char *msg)
{
    fprintf(stderr, "yyerror on line %d: %s\n", yylineno, msg);
    return;
}

void quads_print() {
    fprintf(stderr, "Not implemented");
}
