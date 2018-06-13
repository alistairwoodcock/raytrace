#include "test.h"
#include "../shared/array.h"
/*
	TESTING FOR THE ARRAY LIBRARY
*/

TEST_S(test_create_array)
{

	char* array1 = a_new(char, 50);
	u64* array2 = a_new(u64, 100);
	double* array3 = a_new(double, 4000);


	TEST_A(array1 != null);
	TEST_A(array2 != null);
	TEST_A(array3 != null);
}
TEST_E


TEST_S(test_append_to_nothing)
{
	double* array;

	a_push(array, 10.5);
	a_push(array, 10.5);
	a_push(array, 10.5);
	a_push(array, 10.5);
	
	TEST_A(array != null)
	TEST_A(a_count(array) == 4)
}
TEST_E


TEST_S(test_array_size)
{
	char* array = a_new(char, 1);

	TEST_A(a_size(array) == 1);
}
TEST_E

TEST_S(test_array_count)
{
	float* array = a_new(float, 5);

	a_push(array, 0);	
	a_push(array, 1);	
	a_push(array, 2);	

	TEST_A(a_count(array) == 3)
}
TEST_E



RUN_TESTS
{
	test_create_array();
	test_append_to_nothing();
	test_array_size();
	test_array_count();
}
END_TESTS