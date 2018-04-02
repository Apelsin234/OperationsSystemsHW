#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/wait.h>
#include <stdio.h> 
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

struct {
	ino_t _inum;
	nlink_t _nlink;
	off_t _size;
	int _cmpSize;
	char _exec[1024];

	char _name[256];

	
} pattern;

void init(){
	pattern._inum = -1;
	pattern._nlink = -1;
	pattern._size = -1;
	pattern._cmpSize = 0;
	pattern._name[0] = '.';
	pattern._name[1] = '\0';
	pattern._exec[0] = '\0';
}

int cmpSize(off_t size){
	if(pattern._size == size){
		return 0;
	}
	if(pattern._size > size) {
		return -1;
	}
	return 1;
}


void exec(char pathName[256]){
	if(strcmp(pattern._exec, "") == 0) {
		printf("%s\n", pathName);
		return;
	} 
	const pid_t pid = fork();
	char ** envp = NULL;
	char * tokens[3] = {pattern._exec, pathName, NULL};
	
	int status = 0;
	if(pid < 0) {
		fprintf(stderr, "When use fork error was occurred. %s\n", strerror(errno));
	} else if(pid == 0) {
		if(execve(pattern._exec, tokens, envp) == -1) {
			fprintf(stderr, "Path in -exec was incorect. %s\n", strerror(errno) );
		}
		exit(EXIT_FAILURE);
	} else {
		do {
		int err = wait(&status);
		if(err == -1) {
			fprintf(stderr, "When we wait child error was occured. %s\n", strerror(errno) );
			exit(EXIT_FAILURE);
		}
		} while(!WIFEXITED(status) && !WIFSIGNALED(status));
		
	}
	
}

void check(char *name, ino_t inum, nlink_t nlink, off_t size, char pathName[256]) {
	if(strcmp(pattern._name, ".") != 0 && strcmp(pattern._name, name) != 0) {
		return;
	}
	if(pattern._inum != -1 && inum != pattern._inum) {
		return;
	}

	if(pattern._nlink != -1 && nlink != pattern._nlink) {
		return;
	}
	if(pattern._size != -1 && cmpSize(size) != pattern._cmpSize) {
		return;
	}
	exec(pathName);
}

void visiting(char *nameDir){
	
	struct dirent *entry = NULL;
	DIR* dir = opendir(nameDir);
	char pathName[256];
	if(dir == NULL) {
		fprintf(stderr, "Error opening directory on path %s: %s\n", nameDir, strerror(errno));
		return;
	}
	
	while( (entry = readdir(dir) ) ) {
		struct stat info;
		if(strcmp(entry -> d_name, ".") == 0 ||
			strcmp(entry -> d_name, "..") == 0){
			continue;
		}
		strcpy(pathName, nameDir);
		strcat(pathName, "/");
		strcat(pathName, entry -> d_name);
		if(stat(pathName, &info) == 0) {
			if(S_ISDIR(info.st_mode)) {
				visiting(pathName);
			} else if(S_ISREG(info.st_mode)) {
				check(entry -> d_name, info.st_ino,
				 info.st_nlink, info.st_size, pathName);
			}
		} else {
			
			fprintf(stderr, "Can't get stat file with name %s in directory on the path %s: %s\n",
				 entry -> d_name, nameDir, strerror(errno));
		}

	}
	closedir(dir);

}

void createPattern(int argv, char * argc[]) {
	for(int i = 2; i < argv; i += 2){
		if(i + 1 == argv) {
			printf("Invalid arguments. Run this programm with one argument {--help} \
				\nto get help and information about the arguments.\n");
			exit(EXIT_FAILURE);
		}
		if(strcmp("-inum", argc[i]) == 0) {
			pattern._inum = atoi(argc[i + 1]);

		} else if(strcmp("-name", argc[i]) == 0) {
			strcpy(pattern._name, argc[i + 1]);
		} else if(strcmp("-size", argc[i]) == 0) {
			int k = 0;
			switch(argc[i+1][0]){
				case '-': pattern._cmpSize = -1; k = 1; break;
				case '+': pattern._cmpSize = 1; k = 1; break;
				case '=': pattern._cmpSize = 0; k = 1; break;
			}
			pattern._size = atoi(argc[i + 1] + k);
			
		} else if(strcmp("-nlinks", argc[i]) == 0) {
			pattern._nlink = atoi(argc[i + 1]);
			
		} else if(strcmp("-exec", argc[i]) == 0) {
			strcpy(pattern._exec, argc[i + 1]);


		} else {
			printf("Unexpected token or Invalid arguments - { %s }. \
				\nRun this programm with one argument {--help} to get help and information about the arguments.\n", argc[i]);
			exit(EXIT_FAILURE);
		}
	}
}

void checkHelp(int n, char * args []) {
	if(n != 2 || strcmp(args[1], "--help") != 0) {
		return;
	}
	printf("\n\nThis programm search for files in a directory hierarchy\n\n");
	printf("Synopsis:  ... [ path - where need to look ] [{option} {argument for option}]*\n\n\n");
	printf("Supports many options {-inum, -nlinks, -size, -exec, -name}:\n\n");
	printf("inum - The option specifies the inode number. Argument type { int }.\n\n");
	printf("size - The option specifies the size of the file. Argument type [=-+]{ int }.\
		\n\tif '+' find file with size more than 'number' \
		\n\tif '-' find file with size less than 'number' \
		\n\tif '=' find file with size equally 'number' - default argument. \n\n");
	printf("exec - The option specifies the path to the executable file. Argument type { string }.\n\n");
	printf("name - The option specifies the name of the file. Argument type { string }.\n\n");
	printf("nlinks - The option specifies the hard links of the file. Argument type { int }.\n");
	exit(EXIT_SUCCESS);
}

int main(int argN, char * args []) {
	if(argN < 2) {
		printf("Expected more arguments. Run this programm with one argument {--help} \
			\nto get help and information about the arguments.\n");
		return EXIT_SUCCESS;
	}
	checkHelp(argN, args);
	init();
	createPattern(argN, args);
	visiting(args[1]);
	return EXIT_SUCCESS;
}