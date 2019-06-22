#!/usr/bin/env bash

# A script that invokes your compiler.

for i in ../examples/*.c; do 
	filename=`basename ${i}`
	./dfa.out ${i}.ta > cfg_res/${filename%.*}.ta.cfg
	echo "${filename%.*}.ta.cfg : "
	diff  cfg_res/${filename%.*}.ta.cfg ../examples/${filename%.*}.ta.cfg
done
