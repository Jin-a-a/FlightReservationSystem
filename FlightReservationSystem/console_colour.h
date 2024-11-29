#include <stdio.h>
#include <stdarg.h>
#include <malloc.h>
#include "StringHelper.h"

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
#define TEXTFORMAT_DOUBLEUNDERLINE 21
#define TEXTFORMAT_LOWUNDERLINE 52

#define TEXTCOLOR_BLACK 30
#define TEXTCOLOR_RED 31
#define TEXTCOLOR_GREEN 32
#define TEXTCOLOR_GOLD 33
#define TEXTCOLOR_BLUE 34
#define TEXTCOLOR_PURPLE 35
#define TEXTCOLOR_LIGHTBLUE 36
#define TEXTCOLOR_WHITE 37

#define TEXTCOLOR_GRAY 90
#define TEXTCOLOR_LIGHTRED 91
#define TEXTCOLOR_VIGRANTGREEN 92
#define TEXTCOLOR_LIGHTYELLOW 93
#define TEXTCOLOR_LIGHTBLUE2 94
#define TEXTCOLOR_MAGENTA 95
#define TEXTCOLOR_CYAN 96
#define TEXTCOLOR_EVENWHITER 97

#define TEXTHIGHLIGHT_BLACK 40
#define TEXTHIGHLIGHT_RED 41
#define TEXTHIGHLIGHT_GREEN 42
#define TEXTHIGHLIGHT_GOLD 43
#define TEXTHIGHLIGHT_BLUE 44
#define TEXTHIGHLIGHT_PURPLE 45
#define TEXTHIGHLIGHT_LIGHTBLUE 46
#define TEXTHIGHLIGHT_WHITE 47

#define TEXTHIGHLIGHT_GRAY 100
#define TEXTHIGHLIGHT_LIGHTRED 101
#define TEXTHIGHLIGHT_VIGRANTGREEN 102
#define TEXTHIGHLIGHT_LIGHTYELLOW 103
#define TEXTHIGHLIGHT_LIGHTBLUE2 104
#define TEXTHIGHLIGHT_MAGENTA 105
#define TEXTHIGHLIGHT_CYAN 106
#define TEXTHIGHLIGHT_EVENWHITER 107

void reset_rich_format();
//Use begin_rich_printing instead.
void vbegin_rich_printing(int arg_count, ...);
//Use printf_rich instead.
void vprintf_rich(int arg_count, char* str, ...);
//Use write_rich_format.
errno_t vwrite_rich_format(int arg_count, char* write_to, size_t size, ...);
//Use dynamic_write_rich_format. Requires you to free the char pointer afterwards.
char* vdynamic_write_rich_format(int arg_count, ...);

#define VA_NARGS2(...) ((int)(sizeof((int[]){ __VA_ARGS__ })/sizeof(int)))

#define begin_rich_printing(...) vbegin_rich_printing(VA_NARGS2(__VA_ARGS__), __VA_ARGS__);
#define printf_rich(str, ...) vprintf_rich(VA_NARGS2(__VA_ARGS__), str, __VA_ARGS__);
#define write_rich_format(write_to, size, ...) vwrite_rich_format(VA_NARGS2(__VA_ARGS__), write_to, size, __VA_ARGS__);
#define dynamic_write_rich_format(...) vdynamic_write_rich_format(VA_NARGS2(__VA_ARGS__), __VA_ARGS__);