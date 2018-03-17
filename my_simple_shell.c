#include <stdio.h> 
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>


const int BUFFER = 64;
const char * DELIMETER = " \t\n\r\a";

char* read_args() {
	char* line = NULL;
	size_t b = 0;
	int err;
	err = getline(&line, &b, stdin);
	if(err == -1) {
		int exitCode;
		if(errno != 0){
			perror("When read comand, error was occured.");
			exitCode = EXIT_FAILURE;
		} else {
			exitCode = EXIT_SUCCESS;
		}
		exit(exitCode);
	}
	return line;
}

char **parse_args(char* line) {
	size_t bufsize = BUFFER;
	size_t pos = 0;
	char **tokens = malloc(bufsize * sizeof(char *));
	char *token;
	if(!tokens) {
		perror("Can't allocate memmory. ");
		exit(EXIT_FAILURE);
	}

	token = strtok(line, DELIMETER);
	while(token != NULL) {
		tokens[pos] = token;
		pos++;
		if(pos >= bufsize) {
			bufsize += BUFFER;
			tokens = realloc(tokens, bufsize * sizeof(char *));
			if(!tokens){
				perror("Can't reallocate memmory. ");
				exit(EXIT_FAILURE);
			}
		}
		token = strtok(NULL, DELIMETER);
	}
	tokens[pos] = NULL;
	return tokens;
}

int exec_args(char** tokens) {
	if(tokens[0] == NULL) {
		return 1;
	}
	if(strcmp(tokens[0], "exit") == 0) {
		return 0;
	}
	const pid_t pid = fork();
	char** envp = NULL;
	int status = 0;
	if(pid < 0) {
		perror("When use fork error was occurred. ");
	} else if(pid == 0) {
		if(execve(tokens[0], tokens, envp) == -1) {
			perror("I don't know this comand.");
		}
		exit(EXIT_FAILURE);
	} else {
		do {
		int err = wait(&status);
		if(err == -1) {
			perror("When we wait child error was occured. ");
			exit(EXIT_FAILURE);
		}
		} while(!WIFEXITED(status) && !WIFSIGNALED(status));
		
	}
	return 1;
}



int main() {
	int status = 1;
	char* line;
	char** args;
	while(status) {
		printf("$ ");
		line = read_args();
		args = parse_args(line);
		status = exec_args(args);
		free(line);
		free(args);
	}
	return EXIT_SUCCESS;	
	
}