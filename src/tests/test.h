#ifndef TESTING_LIBRARY
#define TESTING_LIBRARY

#include <stdio.h>

#include "../shared/utils.h"

int _test_count = 0;
int _test_success_count = 0;


#define TEST(name) static bool name()\
{\
	printf("%s: ", __func__);\
	_test_count++;\
	bool _test_success = true;

#define ASSERT(assertion) _test_success = _test_success && (assertion);

#define FAILED(reason) printf("%sfailed%s - %s\n", KRED, KNRM, reason);\
	_test_success = false;\
	return _test_success;

#define END if(_test_success){\
	_test_success_count++;\
		printf("%ssucceded%s\n", KGRN, KNRM);\
	} else {\
		printf("%sfailed%s\n", KRED, KNRM);\
	}\
	\
	return _test_success;\
}

#define RUN_TESTS int main(void)\
{\
	_test_count = 0;\
	_test_success_count = 0;\
	printf("%sRunning tests in %s%s\n", KYEL, __FILE__, KNRM);

#define END_TESTS \
	printf("%sTests finished%s: %d/%d succeded", KYEL, KNRM, _test_success_count, _test_count);\
}


#endif