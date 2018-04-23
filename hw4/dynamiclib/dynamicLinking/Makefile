all: comp

clean:
	rm -f *.o *.so

comp:
	g++ -c -Wall -Werror -fPIC *.cpp
	g++ -shared -fPIC -o libddllib.so *.o
