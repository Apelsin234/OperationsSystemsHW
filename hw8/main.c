
#include <signal.h>
#include <unistd.h>
#include <ucontext.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <execinfo.h>
#include <setjmp.h>
#include <stdint.h>


#define handler_error(msg) \
		{perror(msg); exit(1);}

void write2(char const* str){
	size_t si = strlen(str);
	for(int i = 0; i < si;) {
		int q;
		if(( q = write(2, str + i, si - i) ) == -1) {
			handler_error("write2");
		}
		i += q;
	}

}


jmp_buf jmpbuf;
void mem_sig(int signum){
	longjmp(jmpbuf, 1990);

}




#define PREG(NAME) \
	err = sprintf(mes, #NAME" - 0x%p\n", (void *)uc->uc_mcontext.gregs[REG_##NAME]);\
	if (err < 0) handler_error("PREG");\
	write2(mes);

void posix_signal(int signum, siginfo_t *info, void* uc_void) {
	ucontext_t* uc = (ucontext_t *)uc_void;

// #if __USE_GNU
// 	printf("Hi\n");
// #endif


	char * mes = (char *) malloc(100 * sizeof( char ));
	int err;
#if defined(__i386__)
	PREG(EDI);
	PREG(ESP);
	PREG(EAX);
	PREG(EBX);
	PREG(ECX);
	PREG(EDX);
	PREG(ESI);
	PREG(EBP);
	
#elif defined(__x86_64__)
	PREG(RDI);
	PREG(RSP);
	PREG(RAX);
	PREG(RBX);
	PREG(RCX);
	PREG(RDX);
	PREG(RSI);
	PREG(RBP);
#else
#error Unsupported compiler
#endif
	err = sprintf(mes, "\n\naddress is 0x%p\n", info->si_addr);
	if(err < 0 ){
		handler_error("sprintf");
	}
	write2(mes);
	write2("____MEM___DUMP____\n");

	struct sigaction act;

	memset(&act, 0, sizeof(act));

	act.sa_flags = SA_NODEFER;
	act.sa_handler = mem_sig;

	if(sigaction(SIGSEGV, &act, (struct sigaction *)NULL) ) {
		handler_error("sigaction");
	}



	uint64_t addr = (uint64_t)info->si_addr;
	for(uint64_t i = (addr & ~((uint64_t)15)) - 16, cnt = 0; cnt < 6 ; i += 8, ++cnt) {
		int f = 0;
		for(uint64_t j = i; j < i + 8; ++j) {
			if(j == addr) {
				write2("<");
				f = 17;
			} else if (f) {
				write2(">");
				f = 0;
			} else {
				write2(" ");
			}

			if(setjmp(jmpbuf)) {
				write2("--");
			} else {
				uint8_t byte = *((uint8_t*)(j));
				err = sprintf(mes, "%02x", byte);
				write2(mes);

			}
		}
		if (f) {
				write2(">");
		}
		write2("\n");

	}
	//в <> аддресс по которому мы обратились


	exit(3);

}

void fall() {
	char *p= calloc(100, sizeof(char));
	for(int i = 1; i; i++) {
		p[i - 1] = i;
	}

}


void foo2() {
	fall();
}

void foo1() {
	foo2();
}

int main() {
	struct sigaction act;

	memset(&act, 0, sizeof(act));

	act.sa_flags = SA_SIGINFO | SA_NODEFER;
	act.sa_sigaction = posix_signal;

	if(sigaction(SIGSEGV, &act, (struct sigaction *)NULL) ) {
		handler_error("sigaction");
	}
	foo1();

	return 0;
}