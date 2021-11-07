#include <stdio.h>

#include "lexer/token.h"

extern int yylex();
extern int yyval_int;


int main (void)
{
    printf("\n[Log] - lexing started\n\n");
    int code;

    while ((code = yylex()) != 0)
    {
        switch (code)
        {
        case DECIMAL_CST:
            printf("Decimal: %d\n", yyval_int);
            break;
        case HEXADECIMAL_CST:
            printf("Hexadecimal: %d\n", yyval_int);
            break;
        case LEX_ERROR:
            fprintf(stderr, "Erreur lors de l'analyse lexicale!\n");
            break;
        default:
            break;
        }
        // if (code == DECIMAL_CST) printf("Decimal: %d\n", yyval_int);
        // else if (code == HEXADECIMAL_CST) printf("Hexadecimal: %d\n", yyval_int);
        // else if (code == LEX_ERROR) fprintf(stderr, "Error during lexical analysis!\n");
        
    }
    
    printf("\n[Log] - lexing finished\n");

    return 0;
}