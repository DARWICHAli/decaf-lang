%{
extern int yylex();
void yyerror(const char *msg);

#include "ir.h"
%}

%union {
    int _int_literal;
    int _hex_literal;
}
%token <_int_literal> DECIMAL_CST HEXADECIMAL_CST

%type <_int_literal> decimal_literal hex_literal
%type <_int_literal> int_literal expr

%token ADD SUB MUL DIV MOD
%left ADD SUB
%left MUL
%right NEG

%start input

%%

input: expr {
    // init tds globale
    printf("%d\n", $1);
}

expr
: expr ADD expr {
    //! create new temp
    struct entry res = ctx_make_temp();
    $$ = res;
    //! check type
    if (!typedesc_equals($1.type, $2.type)) fprintf(stderr, "Type error :/\n");
    //! init quadop struct + TDS
    struct quad new_quad = {.lhs = $1, .rhs = $3, .op = Q_ADD, .res = $$};
    //! gencode
    struct quad_id_t qid = gencode(new_quad);
    // le type de $$ est connu à l'avance dans decaf 
    // puisque les variables sont déclarés et typée à l'avance 
    $$->type = typedesc_make_var(BT_INT);
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
    // constructeur
    // on perd la valeur du literal !
    // newtemp?
    struct typedesc type = typedesc_make_var(BT_INT);
    $$.type = type;
}


int_literal
: decimal_literal
| hex_literal

decimal_literal: DECIMAL_CST
hex_literal: HEXADECIMAL_CST

%%

void yyerror(const char *msg)
{
    fprintf(stderr, "%s\n", msg);
    return;
}