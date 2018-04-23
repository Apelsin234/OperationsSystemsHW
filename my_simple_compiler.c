#include <stdio.h> 
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include <sys/stat.h>
#include <sys/mman.h>

#define handle_error(msg) \
	{perror(msg); exit(EXIT_FAILURE); }

void *my_mmap(size_t si) {
	void * addr;
	if((addr = mmap(0, si, PROT_READ | PROT_WRITE,
		MAP_PRIVATE | MAP_ANON, -1, 0)) == MAP_FAILED){
		handle_error("mmap");
	}
	return addr;
}
void my_munmap(void* addr, size_t si){
	if(munmap(addr, si)){
		handle_error("munmap");
	}
}

void my_mprotect(void *addr, size_t si){
	if(mprotect(addr, si, PROT_READ | PROT_EXEC)){
		handle_error("mprotect");
	}
}

void modif(void *addr, int shift, char lo) {
	memcpy(addr + shift, &lo, sizeof(lo));
}

int main(int argc, char *argv[]) {
	void *addr;
	//() -> '3';
	char code[] = {
		0x55, 0x48, 0x89, 0xe5,
		0xb8, 0x33, 0x00, 0x00,
		0x00, 0x5d, 0xc3};
	
	size_t length = sizeof(code);

	addr = my_mmap(length);

	memcpy(addr, code, length);
	if(argc > 1) {
		modif(addr, 5, argv[1][0]);
	}
	my_mprotect(addr, length);

	int (*foo)() = (int (*)())(addr);
	int k = foo();
	printf("%c\n", k);

	my_munmap(addr, length);
}