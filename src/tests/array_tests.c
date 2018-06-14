#include "test.h"
#include "../shared/array.h"
/*
    TESTING FOR THE ARRAY LIBRARY
*/

TEST(test_create_array)
{

    char* array1 = a_new(char, 50);
    u64* array2 = a_new(u64, 100);
    double* array3 = a_new(double, 4000);


    ASSERT(array1 != null);
    ASSERT(array2 != null);
    ASSERT(array3 != null);
}
END


TEST(test_append_to_nothing)
{
    double* array;

    a_push(array, 10.5);
    a_push(array, 10.5);
    a_push(array, 10.5);
    a_push(array, 10.5);
    
    ASSERT(array != null)
    ASSERT(a_count(array) == 4)
}
END


TEST(test_array_size)
{
    char* array = a_new(char, 1);

    ASSERT(a_size(array) == 1);
}
END

TEST(test_array_count)
{
    float* array = a_new(float, 5);

    a_push(array, 0);   
    a_push(array, 1);   
    a_push(array, 2);   

    ASSERT(a_count(array) == 3)
}
END

TEST(test_array_access) 
{
    u32* array = a_new(u32, 10);

    a_push(array, 1);
    a_push(array, 2);
    a_push(array, 3);
    a_push(array, 4);
    a_push(array, 5);


    ASSERT(array[0] == 1);
    ASSERT(array[1] == 2);
    ASSERT(array[2] == 3);
    ASSERT(array[3] == 4);
    ASSERT(array[4] == 5);
}
END


TEST(test_free_array)
{

    char* array = a_new(char, 100);

    ASSERT(array != null);

    a_push(array, 10);
    a_push(array, 11);
    a_push(array, 12);
    a_push(array, 13);

    a_free(array);

    ASSERT(array == null);
}
END

TEST(test_iterate_though_array)
{
    int* array = a_new(int, 1000);

    for(int i = 0; i < a_size(array); i++){
        array[i] = i;
    }

    for(int i = 0; i < a_count(array); i++){
        ASSERT(array[i] == i);
    }

    a_free(array);

    ASSERT(array == null);
}
END


TEST(test_grow_array)
{
    int* array = a_new(int, 50);

    ASSERT(a_size(array) == 50);

    a_grow(array, 50);

    ASSERT(a_size(array) == 100);

    ASSERT(a_count(array) == 0);
}
END

RUN_TESTS
{
    test_create_array();
    test_append_to_nothing();
    test_array_size();
    test_array_count();
    test_array_access();
    test_free_array();
    test_iterate_though_array();
    test_grow_array();
}
END_TESTS