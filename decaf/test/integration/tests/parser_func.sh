#!/bin/sh
# Vérifie que le parseur est capable de parser des programme simples
# C'est à dire des programmes vides ou bien seulement des opérations arithmétiques

printf "[Parsing de programmes utilisant des fonctions]\n"

decaf_files=$2/func/*.decaf
binary=$1/decaf

GRE='\033[0;32m'
RED='\033[0;31m'
NOR='\033[0m'

count=0
for f in $decaf_files; do
	count=$((count+1))
done

printf "Found $count files...\n"

i=1
ok=0
for f in $decaf_files; do
	printf "Parsing file $i of $count: $f... "

	if OUTPUT=$($binary --nogen -o /tmp/$(basename $f).mips < $f 2>&1)
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
	printf "${GRE}Parsing was successfull${NOR}\n"
	exit 0
else
	printf "${RED}$ok/$count files parsed without errors...${NOR}\n"
	exit 1
fi