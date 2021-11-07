CC=gcc
CFLAGS = -g
LDFLAGS = -std=c11 -Wpedantic -Wall -Wextra

LEXER=lexer
PARSER=parser

decaf: decaf.o decaf_lex.o
	$(CC) $(CFLAGS) -o $@ $^

decaf_lex.c: $(LEXER)/decaf.lex
	flex -s -o $@ $< 


.PHONY: clean
clean c:
	rm -f decaf decaf_* *.o