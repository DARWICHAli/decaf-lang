#!/bin/sh
# Vérifie que le parseur est capable de parser des programme simples
# C'est à dire des programmes vides ou bien seulement des opérations arithmétiques

printf "[Parsing de programmes simples]\n"

decaf_files=$2/arith/*.decaf
binary=$1/decaf

count=0
for f in $decaf_files; do
	count=$((count+1))
done

printf "Found $count files...\n"

i=1
ok=0
for f in $decaf_files; do
	printf "Parsing file $i of $count: $f... "

	if OUTPUT=$($binary < $f 2>&1)
	then
		printf "${GRE}Ok!${NOR}\n"
		ok=$((ok+1))
	else
		printf "${RED}Error...${NOR}\n"
		printf "output:\n===\n$OUTPUT\n===\n"
	fi
done

if [ $ok = $count ]; then
	printf "${GRE}Parsing was successfull${NOR}\n"
	exit 0
else
	printf "${RED}Parsing failed for $ok/$count files...${NOR}\n"
	exit 1
fi
