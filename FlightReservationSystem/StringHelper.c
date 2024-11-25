#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <stdarg.h>
#include <stdlib.h>
#include <varargs.h>
#include <ctype.h>
#include <stdbool.h>

//#define def_buffer 100

bool str_equal(const char* a, const char* b) {
	while (*a != '\0' && *b != '\0') {
		if (*a != *b) return false;
		a++;
		b++;
	}

	return *a == *b;
}

void lower_case_str(char* destination, const char* str) {
	int len = strlen(str);

	for (int j = 0; j < len; j++) {
		destination[j] = tolower(str[j]);
	}

	destination[len] = '\0';
}

bool str_lower_case_equal(const char* x, const char* y) {
	while (*x != '\0' && *y != '\0') {
		if (tolower(*x) != tolower(*y)) return false;
		x++;
		y++;
	}

	return tolower(*x) == tolower(*y);
}

char* dynamic_format(const char* format, ...) {
	va_list ap;
	va_start(ap, format);
	int n = _vscprintf(format, ap) + 1;

	char* buffer = malloc(sizeof(char) * n);
	if (buffer == NULL) {
		printf("Null Reference Error.");
		exit(-1);
	}

	vsnprintf(buffer, sizeof(char) * n, format, ap);

	va_end(ap);
	return buffer;
}

void dynamic_concat(char** destination, const char* input, unsigned int* capacity) {
	if (*destination == NULL) {
		printf("Destination is null");
		return;
	}

	size_t a = strlen(*destination);
	size_t b = strlen(input);

	size_t total_len = a + b + 1;
	if (total_len > *capacity) {
		*capacity *= 2;
		char* new_destination = malloc(sizeof(char) * (*capacity));
		if (new_destination == NULL) {
			printf("Malloc failed.");
			return;
		}
		strcpy_s(new_destination, *capacity, *destination);
		free(*destination);
		*destination = new_destination;
	}

	strcat_s(*destination, *capacity, input);
}