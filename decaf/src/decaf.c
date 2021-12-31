#include <stdio.h>

extern int yyparse();
extern int yydebug;


int main (void)
{
    // avant yyparse
    yydebug = 0;
    int r = yyparse();

    return r;
}
