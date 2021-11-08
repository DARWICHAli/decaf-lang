CC=gcc
CFLAGS =
LDFLAGS =-std=c11 -Wpedantic -Wall -Wextra

LEXER=lexer
PARSER=parser

decaf: decaf.tab.o decaf.o decaf_lex.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^
	rm *.o

decaf.o: decaf.c
	$(CC) $(CFLAGS) -o $@ -c $<

decaf_lex.o: decaf_lex.c
	$(CC) $(CFLAGS) -o $@ -c $<
# %.o: %.c
# 	$(CC) $(CFLAGS) -o $@ -c $<

decaf_lex.c: $(LEXER)/decaf.lex
	flex -s -o $@ $< 


decaf.tab.o: decaf.tab.c
	$(CC) $(CFLAGS) -o $@ -c $<

decaf.tab.c: $(PARSER)/decaf.y
	bison -t -d $<


.PHONY: clean
clean c:
	rm -f decaf decaf_* *.o *.tab.*