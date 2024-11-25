#pragma once

#include <stdbool.h>

bool str_equal(const char* a, const char* b);
void lower_case_str(char* destination, const char* str);
bool str_lower_case_equal(const char* x, const char* y);
char* dynamic_format(const char* format, ...);
void dynamic_concat(char** destination, const char* input, unsigned int* capacity);