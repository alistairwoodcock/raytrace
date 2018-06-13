#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void error(const char *msg) { 
	perror(msg); 
	exit(1); 
}

int main(int argc, char *argv[]){
	
	printf("Hello, from the Server!\n");

	return 0;	
}