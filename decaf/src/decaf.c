#include <stdio.h>

#include "symbols.h" // TDS - TDS entries - descripteur de type - liste de type

extern int yyparse();
extern int yydebug;

int main (void)
{
    
    yydebug = 0; // avant yyparse

    // init TDS globale avec ajout des fonctions/nom protégés
    struct context * ctx_global = ctx_pushctx();
    
    int parse_r_value = yyparse();
    
    return parse_r_value;
}
