all: static dynamic1 dynamic2

clean: clean1 clean2 clean3
	rm -f *.zip

#1 статическа библиотека незнал точно что нужно поэтому 2 варианта 
static: 
	cd staticlib && ${MAKE}
	g++ -o run1 static_library.cpp -L./staticlib -lstaticLib

static1: 
	cd staticlib && ${MAKE}
	g++ -o run1 static_library.cpp staticlib/MyLib.o
	
#2
dynamic1:
	cd dynamiclib/staticLinking && ${MAKE}
	g++ -fPIC -o run2 dynamic_library_with_static_link.cpp -L./dynamiclib/staticLinking -ldsllib
#	LD_LIBRARY_PATH="./dynamiclib/staticLinking"
#	export LD_LIBRARY_PATH
#   Прописать две строки выше ручками если не будет находить библиотеку
#3
dynamic2:
	cd dynamiclib/dynamicLinking && ${MAKE}
	g++ -fPIC -o run3 dynamic_library_with_dynamic_link.cpp -ldl
#	LD_LIBRARY_PATH="./dynamiclib/dynamicLinking"
#	export LD_LIBRARY_PATH
#   Прописать две строки выше ручками если будет падать

#  	Или воспользуйтесь скриптом run.sh с аргументами 
#  	[ 1 == запуск static ] [ 2 == запуск dynamic1 ] [ 3 == запуск dynamic2 ]
clean1:
	rm -f run1
	cd staticlib && ${MAKE} clean

clean2:
	rm -f run2
	cd dynamiclib/staticLinking && ${MAKE} clean

clean3:
	rm -f run3
	cd dynamiclib/dynamicLinking && ${MAKE} clean
pack: clean
	zip homework.zip -r *
