#pragma once

#include <stdio.h>
#include <malloc.h>
#include <varargs.h>

#define TEXTFORMAT_NORMAL 0
#define TEXTFORMAT_BOLD 1
#define TEXTFORMAT_FADED 2
#define TEXTFORMAT_ITALIC 3
#define TEXTFORMAT_UNDERLINE 4
#define TEXTFORMAT_BLINKING 5
#define TEXTFORMAT_BLINKING2 6
#define TEXTFORMAT_SWAPCOLOR 7
#define TEXTFORMAT_INVISIBLE 8
#define TEXTFORMAT_STRIKETHROUGH 9

#define TEXTCOLOR_BLACK 30
#define TEXTCOLOR_RED 31
#define TEXTCOLOR_GREEN 32
#define TEXTCOLOR_YELLOW 33
#define TEXTCOLOR_BLUE 34
#define TEXTCOLOR_PURPLE 35
#define TEXTCOLOR_CYAN 36
#define TEXTCOLOR_WHITE 37

static char* dynamic_format(const char* format, ...) {
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

void reset_format() {
	printf("\033[0m");
}

void begin_format(int format, int colour) {
	char* clr_format = dynamic_format("\033[%d;%dm", format, colour);
	printf(clr_format);
	free(clr_format);
}

void print_formatted(char* str, int format, int colour) {
	begin_format(format, colour);
	printf(str);
	reset_format();
}
