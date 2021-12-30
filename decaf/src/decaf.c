#include <stdio.h>

extern int yyparse();
extern int yydebug;


int main (void)
{
    printf("-- Started\n");

    yydebug = 0; // avant yyparse
    int r = yyparse();
    printf("-- Finished\n");
    return r;
}
