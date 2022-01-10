#!/bin/sh
# Vérifie que le parseur est capable de parser des programme

printf "[Tests de parsing]\n"

decaf_dirs=$2/*
binary=$1/decaf

GRE='\033[0;32m'
RED='\033[0;31m'
NOR='\033[0m'

gcount=0
for f in $decaf_dirs; do
	gcount=$((gcount+1))
done

printf "Found $count files...\n"

gi=1
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
		printf "Parsing file $i of $count: $f... "

		if OUTPUT=$($binary --nogen --ir /tmp/$(basename $f).ir < $f 2>&1)
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
		gok=$((gok+1))
	else
		printf "${RED}$ok/$count files parsed without errors...${NOR}\n"
	fi
	gi=$((gi+1))
done

if [ $gok = $gcount ]; then
	printf "${GRE}Parsing was successfull${NOR}\n"
	exit 0
else
	printf "${RED}$gok/$gcount TS parsed without errors...${NOR}\n"
	exit 1
fi

