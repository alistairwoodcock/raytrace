#ifndef LOGGER
#define LOGGER

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

FILE* logger_file_pointer = 0;
char logger_print_enabled = 0;

void logger_file(FILE* fp){
    logger_file_pointer = fp;
}

void logger_stdout_print(char print){
    logger_print_enabled = print;
}

void logger(char* tag, char* message, ...){
    time_t now;
    time(&now);
    FILE* out = stdout;
    if(logger_file_pointer) out = logger_file_pointer;

    char* time_str = ctime(&now);
    time_str[strlen(time_str)-1] = '\0';
   
    fprintf(out, "%s [%s]", time_str, tag); 
    va_list args;
    va_start(args, message);
    vfprintf(out, message, args);
    va_end(args);    
    fprintf(out, "\n");

    if(out != stdout && logger_print_enabled)
    {
        fprintf(stdout, "%s [%s]", time_str, tag); 
        va_list args;
        va_start(args, message);
        vfprintf(stdout, message, args);
        va_end(args);    
        fprintf(stdout, "\n");        
    }

}


#endif