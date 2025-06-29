#pragma once

#include <stdio.h>

bool test_double_nearly_equal(double a, double b);

#define TEST_ENTRY(test_name) int main()

#define TEST_SUCCESS printf("test passed\n"); return 0;
#define TEST_FAILURE_NOPRINT return 1;


#define TEST_ISTRUE(expression) if(!(expression)) {printf("test failed: %s:%i: %s was not true", __FILE__, __LINE__, #expression); TEST_FAILURE_NOPRINT;}
#define TEST_ISFALSE(expression) if((expression)) {printf("test failed: %s:%i: %s was not false", __FILE__, __LINE__, #expression); TEST_FAILURE_NOPRINT;}
#define TEST_EQUAL(a, b) {auto a_ = a; auto b_ = b; if(!(a_ == b_)) {printf("test failed: %s:%i: %s is not equal to %s, got: \n", __FILE__, __LINE__, #a, #b); TEST_FAILURE_NOPRINT;}}

#define TEST_DOUBLE_EQUAL(a, b) if(!test_double_nearly_equal(a, b)) {printf("test failed: %s:%i: %s is not equal to %s\n", __FILE__, __LINE__, #a, #b); TEST_FAILURE_NOPRINT;}