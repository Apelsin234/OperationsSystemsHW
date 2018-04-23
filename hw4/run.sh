#!/bin/sh
if [ "${1}" -eq 1 ]; then
	make static > /dev/null
	./run1
	make clean1 > /dev/null
fi
if [ "${1}" -eq 2 ]; then
	make dynamic1 > /dev/null
	LD_LIBRARY_PATH="./dynamiclib/staticLinking"
	export LD_LIBRARY_PATH
	./run2
	make clean2 > /dev/null
fi
if [ "${1}" -eq 3 ]; then
	make dynamic2 > /dev/null
	LD_LIBRARY_PATH="./dynamiclib/dynamicLinking"
	export LD_LIBRARY_PATH
	./run3
	make clean3 > /dev/null
fi