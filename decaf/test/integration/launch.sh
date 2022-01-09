#!/bin/bash
# Fichier lançant les tests d'intégration
# $1 : dossier où se trouve decaf

# Les fichiers de tests prennent en $1 le dossier où se trouve decaf et éventuellement le dossier vers les données de test en $2

INTE_DIR=$(dirname $0)
SUBTESTS_DIR=$INTE_DIR/tests
DATA_DIR=$INTE_DIR/data

GRE='\033[0;32m'
RED='\033[0;31m'
NOR='\033[0m'

tests=$SUBTESTS_DIR/*
count=0
for f in $tests; do
	count=$((count+1))
done

printf "Found $count tests...\n\n"

i=1
ok=0
for f in $tests; do
	printf "Executing test $i of $count: $f... "

	if OUTPUT=$(bash $f $1 $DATA_DIR 2>&1)
	then
		printf "${GRE}Ok!${NOR}\n"
		ok=$((ok+1))
	else
		printf "${RED}Error...${NOR}\n"
		printf "output:\n===\n$OUTPUT\n===\n"
	fi
	i=$((i+1))
done

if [ $ok = $count ]; then
	printf "${GRE}All tests passed!${NOR}\n"
	exit 0
else
	printf "${RED}Passed $ok/$count tests...${NOR}\n"
	exit 1
fi
