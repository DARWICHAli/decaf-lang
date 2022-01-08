#!/bin/sh
# Vérifie que le parseur est capable de parser des programme simples
# C'est à dire des programmes vides ou bien seulement des opérations arithmétiques

printf "[Exécution de programmes avec des fonctions]\n"

decaf_files=$2/func/*.decaf
binary=$1/decaf
sim=spim
simopt="-exception -file"
killafter=0.1s

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
	printf "Compiling file $i of $count: $f... "
	outfile="/tmp/$(basename $f).mips"

	if OUTPUT=$($binary -o $outfile < $f 2>&1)
	then
		printf "${GRE}Ok!${NOR}\n"

		echo -e "> Executing $sim "$simopt" $outfile..."
		MIPSOUT=$(timeout $killafter $sim $simopt $outfile 2>&1)
		GREPOUT=$(echo -e "$MIPSOUT" | grep -E "(error|Exception|undefined|out of bounds)")
		if [ "$GREPOUT" == "" ]
		then
			printf "${GRE}Ok!${NOR}\n"
			ok=$((ok+1))
		else
			printf "${RED}Error...${NOR}\n"
			printf "output:\n===\n$MIPSOUT\n===\n"
		fi
	else
		printf "${RED}Error...${NOR}\n"
		printf "output:\n===\n$OUTPUT\n===\n"
	fi
	i=$((i+1))
done

if [ $ok = $count ]; then
	printf "${GRE}Execution was successfull${NOR}\n"
	exit 0
else
	printf "${RED}$ok/$count files executed without errors...${NOR}\n"
	exit 1
fi