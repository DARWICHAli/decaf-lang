#include <stdio.h>

extern int yylex();



int main (void)
{
    int code;

    while ((code = yylex()) != 0);
    
    printf("\n[Log] - lexing finished\n");

    return 0;
}