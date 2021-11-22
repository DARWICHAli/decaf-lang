#!/bin/bash
# Fichier lançant les tests unitaires
# $1 : dossier où se trouvent les binaires des tests unitaires

GRE='\033[0;32m'
RED='\033[0;31m'
NOR='\033[0m'

tests=($1/*)
count=0
for f in tests; do
	count=$((count+1))
done

echo $tests

printf "Found $count tests...\n\n"

i=1
ok=0
for f in $tests; do
	printf "Executing test $i of $count: $f... "
	if OUTPUT=$(./$f 2>&1)
	then
		printf "${GRE}Ok!${NOR}\n"
		ok=$((ok+1))
	else
		printf "${RED}Error...${NOR}\n"
		printf "output:\n===\n$OUTPUT\n===\n"
	fi
done

if [ $ok = $count ]; then
	printf "${GRE}All tests passed!${NOR}\n"
	exit 0
else
	printf "${RED}Passed $ok/$count tests...${NOR}\n"
	exit 1
fi
