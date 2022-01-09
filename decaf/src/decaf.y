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
    struct typelist* TypeList;
    quad_id_t Qid;
    struct {
        struct quad_list next_list;
    } IncompleteStatements;
}

%token CLASS VOID IF ELSE RETURN BREAK CONTINUE
%token LAND LOR EQUAL NEQUAL MORE_EQUAL LESS_EQUAL

%token <Integer> DECIMAL_CST HEXADECIMAL_CST
%token <Boolean> TRUE FALSE
%token <BType> TYPE
%token <Identifier> ID

%type <Entry> new_entry existing_entry method_call parameter literal integer bool_literal arg
%type <TypeList> optional_parameters
%type <TypeList> parameters
%type <Incomplete> expr
%type <IncompleteStatements> proc_block block statement statement_list g opt_statements 
%type <Qid> m


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
// Déclaration de plusieures méthodes 
method_declarations: method_declaration
    | method_declaration method_declarations
;
// Déclaration de méthodes
method_declaration: VOID  new_entry '(' { ctx_pushctx(); } optional_parameters ')' {
        $2->type = typedesc_make_function(BT_VOID, $5);
        } proc_block { ctx_popctx(); /* dépile contexte des args */}
    | TYPE new_entry '(' { ctx_pushctx(); } optional_parameters ')' {
		$2->type = typedesc_make_function($1, $5);
		} block { ctx_popctx(); /* dépile contexte des args */}
;

// Paramètres de fonctions
optional_parameters: %empty {$$ = typelist_new(); }
    | parameters { $$ = $1; }
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
proc_block: '{'         { ctx_pushctx(); } optional_var_declarations opt_statements '}' { $$.next_list = $<IncompleteStatements>3.next_list; gencode(quad_endproc()); ctx_popctx();}
;
// Bloc de code
block: '{'              { ctx_pushctx(); } optional_var_declarations opt_statements '}' { $$.next_list = $<IncompleteStatements>3.next_list; ctx_popctx();}
;
    // NOTE OK
optional_var_declarations: %empty
    | var_declaration optional_var_declarations
;
var_declaration: TYPE new_entry ';' { $2->type = typedesc_make_var($1); }
	| TYPE new_entry ',' { $2->type = typedesc_make_var($1); } new_id_list ';'
;
/*
 * Appel de fonction / procédure
 */

// appel de fonction
method_call: existing_entry '(' args_list_opt ')' {
		$$ = ctx_make_temp(typedesc_function_type(&$1->type));
		// $$->type = typedesc_make_var(typedesc_function_type(&$1->type));
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

/*-----------------*/

opt_statements: %empty              {$$.next_list = qlist_new(); }
    | statement_list                {$$.next_list = $1.next_list;}
;
statement_list: statement_list m statement {
        qlist_complete(&$1.next_list, $2);
        $$.next_list = $3.next_list; 
    }
    | statement {
        $$.next_list = $1.next_list;
    }
;
statement: existing_entry '=' expr ';' {
        SERRL(!typedesc_equals(&$1->type, &$3.Entry->type), fprintf(stderr, "type of values in assignement statement does not match\n"));
        gencode(quad_aff($1, $3.Entry));
    }
    | proc_call ';'       { }
    | IF '(' expr ')' m block             {
        SERRL(!typedesc_equals(&$3.Entry->type, &td_var_bool), fprintf(stderr, "type of expr is not boolean in if statement\n"));
        qlist_complete(&$3.true_list, $5);
        $$.next_list = qlist_concat(&$3.false_list, &$6.next_list);
    }
	| IF '(' expr ')' m block g ELSE m block  {
        SERRL(!typedesc_equals(&$3.Entry->type, &td_var_bool), fprintf(stderr, "type of expr is not boolean in if statement\n"));
        qlist_complete(&$3.true_list, $5);
        qlist_complete(&$3.false_list, $9);
        struct quad_list temp = qlist_concat(&$6.next_list, &$7.next_list);
        $$.next_list = qlist_concat(&temp, &$10.next_list);
    }
    | RETURN ';'            {}
    | RETURN expr ';'       { gencode(quad_return($2.Entry)); }
    | BREAK ';'             {}
    | CONTINUE ';'          {}
    | block                 { $$.next_list = $1.next_list;}
;
// Marqueur G
g: %empty {
    QLIST_NEWADD($$.next_list);
    gencode(quad_goto(INCOMPLETE_QUAD_ID));
}
;

// NOTE il faut effectuer les verification de types ici (== != && || -> BOOL)
expr: existing_entry                { $$.Entry = $1; }
    | method_call                   { $$.Entry = $1; }
    | integer                       { $$.Entry = $1; }
    | bool_literal                  { $$.Entry = $1; }
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
    | '-' expr                      {
        SERRL(!typedesc_equals(&$2.Entry->type, &td_var_int), fprintf(stderr, "type of expr is not int in arithmetic statement\n"));

        $$.Entry = ctx_make_temp(BT_INT);
        gencode(quad_neg($$.Entry, $2.Entry));
    } %prec MUNAIRE
    | expr EQUAL expr               {
        SERRL(!typedesc_equals(&$1.Entry->type, &$3.Entry->type), fprintf(stderr, "type of lexpr is not the same as rexpr\n"));

        QLIST_NEWADD($$.true_list); // $$.true_list = qlist_new(); // qlist_append(&$$.true_list, nextquad());
        gencode(quad_ifgoto($1.Entry, CMP_EQ, $3.Entry, INCOMPLETE_QUAD_ID));
        QLIST_NEWADD($$.false_list); // $$.false_list = qlist_new(); // qlist_append(&$$.false_list, nextquad());
        gencode(quad_goto(INCOMPLETE_QUAD_ID));
        $$.Entry->type = typedesc_make_var(BT_BOOL);
    }
    | expr NEQUAL expr              {
        SERRL(!typedesc_equals(&$1.Entry->type, &$3.Entry->type), fprintf(stderr, "type of lexpr is not the same as rexpr\n"));

        QLIST_NEWADD($$.true_list); // $$.true_list = qlist_new(); // qlist_append(&$$.true_list, nextquad());
        gencode(quad_ifgoto($1.Entry, CMP_NQ, $3.Entry, INCOMPLETE_QUAD_ID));
        QLIST_NEWADD($$.false_list); // $$.false_list = qlist_new(); // qlist_append(&$$.false_list, nextquad());
        gencode(quad_goto(INCOMPLETE_QUAD_ID));
        $$.Entry->type = typedesc_make_var(BT_BOOL);
    }
    | expr LOR m expr             {
        qlist_complete(&$1.false_list, $3);
        $$.true_list = qlist_concat(&$1.true_list, &$4.true_list);
        $$.false_list = $4.false_list;
        $$.Entry->type = typedesc_make_var(BT_BOOL);
    }
    | expr LAND m expr              {
        qlist_complete(&$1.false_list, $3);
        $$.true_list = $4.true_list;
        $$.false_list = qlist_concat(&$1.false_list, &$4.false_list);
        $$.Entry->type = typedesc_make_var(BT_BOOL);
    }
    | expr '<' expr                 {
        SERRL(!typedesc_equals(&$1.Entry->type, &td_var_int), fprintf(stderr, "type of expr is not int in comparison statement\n"));
        SERRL(!typedesc_equals(&$3.Entry->type, &td_var_int), fprintf(stderr, "type of expr is not int in comparison statement\n"));

        QLIST_NEWADD($$.true_list); // $$.true_list = qlist_new(); // qlist_append(&$$.true_list, nextquad());
        gencode(quad_ifgoto($1.Entry, CMP_LT, $3.Entry, INCOMPLETE_QUAD_ID));
        QLIST_NEWADD($$.false_list); // $$.false_list = qlist_new(); // qlist_append(&$$.false_list, nextquad());
        gencode(quad_goto(INCOMPLETE_QUAD_ID));
        $$.Entry->type = typedesc_make_var(BT_BOOL);
    }
    | expr '>' expr                 {
        SERRL(!typedesc_equals(&$1.Entry->type, &td_var_int), fprintf(stderr, "type of expr is not int in comparison statement\n"));
        SERRL(!typedesc_equals(&$3.Entry->type, &td_var_int), fprintf(stderr, "type of expr is not int in comparison statement\n"));

        QLIST_NEWADD($$.true_list); // $$.true_list = qlist_new(); // qlist_append(&$$.true_list, nextquad());
        gencode(quad_ifgoto($1.Entry, CMP_GT, $3.Entry, INCOMPLETE_QUAD_ID));
        QLIST_NEWADD($$.false_list); // $$.false_list = qlist_new(); // qlist_append(&$$.false_list, nextquad());
        gencode(quad_goto(INCOMPLETE_QUAD_ID));
        $$.Entry->type = typedesc_make_var(BT_BOOL);
    }
    | expr MORE_EQUAL expr          {
        SERRL(!typedesc_equals(&$1.Entry->type, &td_var_int), fprintf(stderr, "type of expr is not int in comparison statement\n"));
        SERRL(!typedesc_equals(&$3.Entry->type, &td_var_int), fprintf(stderr, "type of expr is not int in comparison statement\n"));

        QLIST_NEWADD($$.true_list);
        gencode(quad_ifgoto($1.Entry, CMP_GE, $3.Entry, INCOMPLETE_QUAD_ID));
        QLIST_NEWADD($$.false_list);
        gencode(quad_goto(INCOMPLETE_QUAD_ID));
        $$.Entry->type = typedesc_make_var(BT_BOOL);
    }
    | expr LESS_EQUAL expr          {
        SERRL(!typedesc_equals(&$1.Entry->type, &td_var_int), fprintf(stderr, "type of expr is not int in comparison statement\n"));
        SERRL(!typedesc_equals(&$3.Entry->type, &td_var_int), fprintf(stderr, "type of expr is not int in comparison statement\n"));

        QLIST_NEWADD($$.true_list);
        gencode(quad_ifgoto($1.Entry, CMP_LE, $3.Entry, INCOMPLETE_QUAD_ID));
        QLIST_NEWADD($$.false_list);
        gencode(quad_goto(INCOMPLETE_QUAD_ID));
        $$.Entry->type = typedesc_make_var(BT_BOOL);
    }
    | '(' expr ')'                  { $$ = $2; }
    | '!' expr                      {
        SERRL(!typedesc_equals(&($2.Entry->type), &td_var_bool), fprintf(stderr, "type of expr is not boolean following NOT\n"));
        $$.Entry = ctx_make_temp(BT_BOOL);
        struct quad_list temp = $$.true_list;
        $$.true_list = $$.false_list;
        $$.false_list = temp; 
    }
;


m: %empty      { $$ = nextquad();}
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
