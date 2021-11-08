%{
extern int yylex();
void yyerror(const char *msg);
%}

%union {
    int int_literal;
    int hex_literal;
}
%token <int_literal> DECIMAL_CST
%type <int_literal> expr

%token PLUS MINUS MULT
%left PLUS MINUS
%left MULT

%%

input: expr {printf("%d\n", $1);}


expr
: expr PLUS expr {$$ = $1 + $3;}
| expr MINUS expr {$$ = $1 - $3;}
| expr MULT expr {$$ = $1 * $3;}
| DECIMAL_CST {$$ = $1;}

    // bin_op:
    // | arith_op

    // arith_op:
    // | PLUS  {$$ = +;}
    // | MINUS {$$ = -;}
    // | MULT  {$$ = *;}


%%

void yyerror(const char *msg)
{
    fprintf(stderr, "%s\n", msg);
    return;
}