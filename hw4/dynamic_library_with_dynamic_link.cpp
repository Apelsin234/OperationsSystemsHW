#include <dlfcn.h>
#include <iostream> 
#include <stdio.h>

typedef int (*funPtr)(int);

#define handle_error(msg) \
	{printf("%s\n%s\n", msg, dlerror()); return -1; }

int main() {
	void *handle = dlopen("libddllib.so", RTLD_LAZY);

	if(handle == NULL) {
		handle_error("When call dlopen error was occurred. ");
	}

	funPtr fun = (funPtr)dlsym(handle, "addFour");

	if(fun == NULL) {
		handle_error("When call dlsym error was occurred. ");
	}

	std::cout << (*fun)(996) << std::endl;

	if(dlclose(handle)) {
		handle_error("When trying to free memory error was occured. ");
	}
	return 0;
}