%{
extern int yylex();
void yyerror(const char *msg);

#include "ir.h"
%}

%union {
    int _int_literal;
    int _hex_literal;
    struct {
        struct entry * result;
    } _exprval;
}
%token <_int_literal> DECIMAL_CST HEXADECIMAL_CST

%type <_exprval> decimal_literal hex_literal
%type <_exprval> int_literal expr

%token ADD SUB MUL DIV MOD
%left ADD SUB
%left MUL
%right NEG

%start input

%%

input: expr {
    printf("%d\n", $1);
}

expr
: expr ADD expr {
    //! create new temp
    struct entry res = ctx_make_temp();
    //! check type and is int type
    if (!typedesc_equals($1._exprval.result->type, $2._exprval.result->type)) fprintf(stderr, "Type error :/\n");
    if (!typedesc_primitif_equals($1._exprval.result->type, BT_INT)) fprintf(stderr, "Type error :/\n");
    if (!typedesc_primitif_equals($2._exprval.result->type, BT_INT)) fprintf(stderr, "Type error :/\n");
    //! init quadop struct + TDS
    struct quad new_quad = {.lhs = $1._exprval.result, .rhs = $3._exprval.result, .op = Q_ADD, .res = res};
    //! gencode
    struct quad_id_t qid = gencode(new_quad);
    //! assignation du nouveau temporaire
    $$._exprval.result = res;
}
| SUB expr %prec NEG {
    $$ = - $2;
}
| expr SUB expr {
    $$ = $1 - $3;
}
| expr MUL expr {
    $$ = $1 * $3;
}
| expr DIV expr {
    $$ = $1 / $3;
}
| expr MOD expr {
    $$ = $1 % $3;
}
| int_literal {
    $$ = $1;
}


int_literal
: decimal_literal
| hex_literal

decimal_literal: DECIMAL_CST {
    struct entry * res = ctx_make_temp();
    struct entry * lhs = ctx_make_cst($1._int_literal, BT_INT);
    struct quad new_quad = {.lhs = lhs, .rhs = NULL, .op = Q_AFF, .res = res};
    struct quad_id_t q_id = gencode(new_quad);
    $$._exprval.result = res;
}
hex_literal: HEXADECIMAL_CST

%%

void yyerror(const char *msg)
{
    fprintf(stderr, "%s\n", msg);
    return;
}