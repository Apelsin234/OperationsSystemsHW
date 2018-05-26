#include <signal.h>
#include <ucontext.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <execinfo.h>



#define handler_error(msg) \
		{perror(msg); exit(1);}

typedef struct sig_ucontext{
	unsigned long uc_flags;
	struct ucontext *uc_link;
	stack_t uc_stack;
	struct sigcontext uc_mcontext;
	sigset_t uc_sigmask;

} sig_ucontext_t;

void posix_signal(int signum, siginfo_t *info, void* uc_void) {

	sig_ucontext_t* uc = (sig_ucontext_t *)uc_void;

	void * call_addr;

#if defined(__i386__)
	call_addr = (void *)uc->uc_mcontext.eip;
#elif defined(__x86_64__)
	call_addr = (void *)uc->uc_mcontext.rip;
#else
#error Unsupported arc
#endif

	fprintf(stderr, "signal %d (%s), address is %p from %p\n", 
		signum, strsignal(signum), info->si_addr, (void *) call_addr);

	void *addr[50];
	int size = backtrace(addr, 50);

	addr[1] = (void *)call_addr;

	char ** mess = backtrace_symbols(addr, size);

	for(int i = 1; i < size && mess != NULL; i++) {
		fprintf(stderr, "[bt]: (%d) %s\n",i, mess[i]);

	}
	free(mess);

	exit(3);

}

void fall() {
	int p[10];

	p[1000000] = 13;

}


void foo2() {
	fall();
}

void foo1() {
	foo2();
}

int main() {
	struct sigaction act;

	act.sa_flags = SA_SIGINFO;
	act.sa_sigaction = posix_signal;

	if(sigaction(SIGSEGV, &act, (struct sigaction *)NULL) ) {
		handler_error("sigaction");
	}
	foo1();

	return 0;
}