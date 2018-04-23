all: main

clean: 
	rm -f *.o *.a

main: 
	g++ -c -Wall -Werror MyLib.cpp -o MyLib.o
	ar rcs libstaticLib.a MyLib.o


