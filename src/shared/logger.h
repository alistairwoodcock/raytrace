#ifndef LOGGER
#define LOGGER

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

FILE* logger_file_pointer = 0;
char logger_print_enabled = 0;

void logger_file(FILE* fp){
    logger_file_pointer = fp;
}

void logger_stdout_print(char print){
    logger_print_enabled = print;
}

void logger(char* tag, char* message){
    time_t now;
    time(&now);
    FILE* out = stdout;
    if(logger_file_pointer) out = logger_file_pointer;

    char* time_str = ctime(&now);
    time_str[strlen(time_str)-1] = '\0';

    if(out != stdout && logger_print_enabled){
        printf("%s [%s] %s\n", time_str, tag, message); 
    } 
    
    fprintf(out, "%s [%s] %s\n", time_str, tag, message); 

}


#endif