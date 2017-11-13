#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void error(const char *msg) { 
	perror(msg); 
	exit(1); 
}

int main(int argc, char *argv[]){
	
	printf("Hello, from the Server!\n");

	int server_pid = fork();
	
	if (server_pid < 0)
	{
		error("ERROR on fork");
	}

	if (server_pid == 0)  
	{
		//We are a forked process
		printf("Hello from the FORK\n");
		exit(0); //exit if we return from server_proc
	}
	else
	{
		printf("Hello from the Forks Parent\n");
	}

	return 0;	
}