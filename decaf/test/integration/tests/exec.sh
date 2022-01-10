#!/bin/sh
# Vérifie qu'il est possible d'exécuter des programmes

printf "[Tests d'execution]\n"

decaf_dirs=$2/*
binary=$1/decaf

sim=spim
simopt="-exception -file"
killafter=1s

GRE='\033[0;32m'
RED='\033[0;31m'
NOR='\033[0m'

gcount=0
for f in $decaf_dirs; do
	gcount=$((gcount+1))
done

printf "Found $count files...\n"

gi=1
ok=0
gok=0
for d in $decaf_dirs; do
	printf "###Test suite $gi/$gcount : $(basename $d)###\n\n"
	decaf_files=$d/*

	count=0
	for f in $decaf_files; do
		count=$((count+1))
	done

	i=1
	ok=0
	for f in $decaf_files; do
		printf "Compiling file $i of $count: $f... "
		outfile="/tmp/$(basename $f).mips"

		expected=$(cat $f | head -n 1 | grep -Po "//\K.*")

		if OUTPUT=$($binary -o $outfile -m --ir /tmp/$(basename $f).ir < $f 2>&1)
		then
			printf "${GRE}Ok!${NOR}\n"

			echo -e "> Executing $sim "$simopt" $outfile..."
			MIPSOUT=$(timeout $killafter $sim $simopt $outfile 2>&1)
			GREPOUT=$(echo -e "$MIPSOUT" | grep -E "(error|Exception|undefined|out of bounds)")
			VALUEOUT=$(echo -e "$MIPSOUT" | tail -n 1)
			if [ "$expected" == "" ] || [ "$VALUEOUT" == "$expected" ]
			then
				printf "${GRE}Ok!${NOR}\n"
				ok=$((ok+1))
			else
				printf "${RED}Error... expected \"${expected}\", got \"${VALUEOUT}\".${NOR}\n"
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
		gok=$((gok+1))
	else
		printf "${RED}$ok/$count files executed without errors...${NOR}\n"
	fi
	gi=$((gi+1))
done

if [ $gok = $gcount ]; then
	printf "${GRE}Execution was successfull${NOR}\n"
	exit 0
else
	printf "${RED}$gok/$gcount TS executed without errors...${NOR}\n"
	exit 1
fi

