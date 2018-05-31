
#include <ucontext.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <execinfo.h>



#define handler_error(msg) \
		{perror(msg); exit(1);}



void posix_signal(int signum, siginfo_t *info, void* uc_void) {
	ucontext_t* uc = (ucontext_t *)uc_void;

	void * call_addr;
// #if __USE_GNU
// 	printf("Hi\n");
// #endif

#if defined(__i386__)
	call_addr = (void *)(uc->uc_mcontext.gregs[REG_EIP];
#elif defined(__x86_64__)
	call_addr = (void *)uc->uc_mcontext.gregs[REG_RIP];
#else
#error Unsupported compiler
#endif
	fprintf(stderr, "_____REG___DUMP_____\n\n");
	fprintf(stderr, "signal %d (%s), address is %p from %p\n", 
		signum, strsignal(signum), info->si_addr, call_addr);

	void *addr[50];
	int size = backtrace(addr, 50);

	addr[1] = call_addr;

	char ** mess = backtrace_symbols(addr, size);

	if(mess == NULL) {
		handler_error("backtrace_symbols");
	}
	fprintf(stderr, "\n\n_____BACKTRACE_____\n\n");
	for(int i = 1; i < size && mess != NULL; i++) {
		fprintf(stderr, "[bt]: (%d) %s\n",i, mess[i]);

	}
	free(mess);


	char* mem_addr_it = (char *)info->si_addr - 5;	

	char* mem_addr_fin = mem_addr_it + 10;	

	//printf("%p\n", mem_addr);
	fprintf(stderr, "\n\n_____MEMORY___DUMP_____\n\n");
	while(mem_addr_it != mem_addr_fin) {
		fprintf(stderr, "address 0x%zx\n", (size_t)mem_addr_it++);
	}
	fprintf(stderr, "_____END______\n");
	exit(3);

}

void fall() {
	int p[10];

	p[20000] = 13;

}


void foo2() {
	fall();
}

void foo1() {
	foo2();
}

int main() {
	struct sigaction act;

	//memset(&act, 0, sizeof(act));

	act.sa_flags = SA_SIGINFO;
	act.sa_sigaction = posix_signal;

	if(sigaction(SIGSEGV, &act, (struct sigaction *)NULL) ) {
		handler_error("sigaction");
	}
	foo1();

	return 0;
}