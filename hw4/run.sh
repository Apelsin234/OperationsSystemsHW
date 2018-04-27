#!/bin/sh
name_os="$(uname -s)"
case "$name_os" in
	Darwin*)	comand_for_path="DYLD_LIBRARY_PATH";;
	*)		comand_for_path="LD_LIBRARY_PATH";;
esac	
if [ "${1}" -eq 1 ]; then
	make static > /dev/null
	./run1
	make clean1 > /dev/null
fi
if [ "${1}" -eq 2 ]; then
	make dynamic1 > /dev/null
	DYLD_LIBRARY_PATH="./dynamiclib/staticLinking"
	LD_LIBRARY_PATH="./dynamiclib/staticLinking"
	export ${comand_for_path}
	./run2
	make clean2 > /dev/null
fi
if [ "${1}" -eq 3 ]; then
	make dynamic2 > /dev/null
	DYLD_LIBRARY_PATH="./dynamiclib/dynamicLinking"
	LD_LIBRARY_PATH="./dynamiclib/dynamicLinking"
	export ${comand_for_path}
	./run3
	make clean3 > /dev/null
fi