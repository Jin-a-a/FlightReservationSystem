#include <stdio.h>
#include <stdarg.h>
#include <malloc.h>
#include "StringHelper.h"

static void copy_args(int* buffer, int count, va_list args) {
	for (int i = 0; i < count; i++) {
		int value = va_arg(args, int);
		buffer[i] = value;
	}
}

void reset_rich_format() {
	printf("\033[0m");
}

void _vbegin_rich_printing(int arg_count, int* arr) {
	char temp[30];
	unsigned int capacity = 100;
	char* text_form = (char*)malloc(sizeof(char) * capacity);
	if (text_form == NULL) {
		printf("Heap Allocation Failed.");
		return;
	}

	text_form[0] = '\0';

	dynamic_concat(&text_form, "\033[", &capacity);

	for (int i = 0; i < arg_count; i++) {
		int a = arr[i];
		sprintf_s(temp, sizeof(char) * 30, "%d", a);
		dynamic_concat(&text_form, temp, &capacity);

		if (i < arg_count - 1) {
			dynamic_concat(&text_form, ";", &capacity);
		}
	}

	dynamic_concat(&text_form, "m", &capacity);
	printf(text_form);
	free(text_form);
}

void vbegin_rich_printing(int arg_count, ...) {
	va_list args;
	int* arr = (int*)malloc(sizeof(int) * arg_count);
	if (arr == NULL) {
		printf("Heap Allocation Failed.");
		return;
	}

	va_start(args, arg_count);
	copy_args(arr, arg_count, args);
	va_end(args);
	_vbegin_rich_printing(arg_count, arr);
	free(arr);
}

void vprintf_rich(int arg_count, char* str, ...) {
	va_list args;
	int* arr = (int*)malloc(sizeof(int) * arg_count);
	if (arr == NULL) {
		printf("Heap Allocation Failed.");
		printf(str);
		return;
	}

	va_start(args, str);
	copy_args(arr, arg_count, args);
	va_end(args);
	_vbegin_rich_printing(arg_count, arr);
	free(arr);

	printf(str);

	reset_rich_format();
}