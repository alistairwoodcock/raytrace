// Tests for the simple logger we have
#include "test.h"
#include <stdio.h>
#include <string.h>

#include "../shared/utils.h"
#include "../shared/array.h"
#include "../shared/logger.h"

FILE* default_out = null;
char* log_file = "logs/test.log";
char* logger_test_filename = "logs/logger_test.log";

void setup(){
    default_out = fopen(log_file, "a");
    logger_file(default_out);
}

void shutdown(){
    fclose(default_out);
    remove(log_file);
}


TEST(test_empty_tag)
{
    logger(null, "this log should go to stdout");
}
END

TEST(test_empty_log)
{
    logger(null, null);
    logger("empty", null);
}
END

TEST(test_writing_to_file)
{
    
    FILE* file = fopen(logger_test_filename, "w");

    if(file == null){
        FAILED("could not create logger_test file");
    }

    logger_file(file);

    char* find = "find_this";

    logger("info","~%s~", find);

    fclose(file);

    file = fopen(logger_test_filename, "r");

    char* found = a_new(char, 9);
    bool grab = false;

    do
    {
        if(feof(file)) break;
        
        int c = fgetc(file);

        if(c == '~' && grab) break;
        
        if(grab)a_push(found, c);

        if(c == '~' && !grab) grab = true;
    }
    while(true);

    found[a_count(found)] = '\0';

    fclose(file);
    remove(logger_test_filename);

    ASSERT(strcmp(find, found) == 0);
}
END

TEST(test_writing_to_closed_file)
{
    FILE* file = fopen("logs/logger_test.log", "w");

    int c = fgetc(file);

    fclose(file);

    logger_file(file);

    logger("test", "first byte read %c", c);

    remove(logger_test_filename);
}
END

// TEST(test_writing_concurrently)
// {
    //TODO(AL): Testing concurrent stuff. 
    //          This will likely require a 
    //          major refactor for the logger
// }
// END


RUN_TESTS
{
    setup();
    test_empty_tag();
    test_empty_log();
    // test_writing_concurrently();
    test_writing_to_file();
    test_writing_to_closed_file();

    shutdown();
}
END_TESTS