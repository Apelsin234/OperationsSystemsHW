all: comp


clean:
	rm -f *.so *.o


comp: 
	g++ -c -Wall -Werror -fPIC *.cpp
	g++ -shared -fPIC -o libdsllib.so *.o
