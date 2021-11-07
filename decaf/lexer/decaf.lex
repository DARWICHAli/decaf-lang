%{
#include <stdio.h>
// #include "token.h"
%}

%option nounput
%option noyywrap

    /* Definitions */


%%
    /* Hexadecimaux en premier, sinon (0)x(..) sont matcher comme étant 2 entiers  */
0x[0-9a-fA-F]+ printf("<hex>\n");


[+-]?[0-9]+ printf("<int>\n");


[[:space:]] ;
. {
    printf("LEX_Error\n");
    exit(1);
} 

%%