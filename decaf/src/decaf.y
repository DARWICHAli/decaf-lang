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
// %type <Boolean> bool_literal
%type <Qid> m

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
// class_body: optional_g_var_declarations
//     | optional_g_var_declarations optional_method_declarations 
// ;


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
method_declaration: VOID  new_entry '(' ')' {struct typelist* tl = typelist_new();$2->type = typedesc_make_function(BT_VOID, tl);ctx_pushctx();/* Ici empiler les paramètres de la fonction */ctx_pushctx();} proc_block        {ctx_popctx(); ctx_popctx();}
    // | TYPE new_entry '(' { struct typelist* tl = typelist_new(); $2->type = typedesc_make_function(BT_INT, tl); ctx_pushctx(); ctx_pushctx();} ')' {} proc_block {ctx_popctx(); ctx_popctx();}
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
%%
void yyerror(const char *msg)
{
    fprintf(stderr, "yyerror: %s\n", msg);
    return;
}

void quads_print() {
    fprintf(stderr, "Not implemented");
}
