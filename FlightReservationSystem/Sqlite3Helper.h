#pragma once
#include <stdbool.h>
#include <D:\TechnicalProjects\C Libs\SQLite\sqlite3\sqlite3.h>

#define VA_NARGS2(...) ((int)(sizeof((int[]){ __VA_ARGS__ })/sizeof(int)))
#define CHECK_TYPE(type,var) { typedef void (*type_t)(type); type_t tmp = (type_t)0; if(0) tmp(var);}

typedef struct {
	unsigned int index;
	unsigned int range;
} Range;

typedef struct SqlValueParameter {
	const char* name;
	const char* type;
	const char* value;
} SqlValueParameter;

typedef struct SqlParameter {
	const char* name;
	const char* column_type;
} SqlParameter;

typedef struct SqlValueParamArray {
	unsigned int length;
	SqlValueParameter* arr;
} SqlValueParamArray;

typedef struct SqlParamArray {
	unsigned int length;
	SqlParameter* arr;
} SqlParamArray;

SqlParamArray param_array_slice(SqlParamArray parameters, Range range);
SqlValueParamArray value_param_array_slice(SqlValueParamArray parameters, Range range);

SqlValueParamArray vformat_param_set(const SqlParamArray parameters, int arg_count, ...);
#define format_param_set(parameters, ...) vformat_param_set(parameters, VA_NARGS2(__VA_ARGS__), __VA_ARGS__);\
	CHECK_TYPE(const SqlParamArray, parameters);\


SqlValueParamArray combine_param_set(const SqlValueParamArray a, const SqlValueParamArray b);
void free_value_set(SqlValueParamArray parameters);

char* combine_param_types(const SqlParamArray parameters);

char* combine_param_values(const SqlValueParamArray parameters);
char* combine_param_names(const SqlValueParamArray parameters);
char* combine_param_names_v2(const SqlParamArray parameters);
char* combine_param_explicit_values(const SqlValueParamArray parameters);
char* combine_param_comparisons_and(const SqlValueParamArray parameters, char* comparer);

int sqlite3_table_if_not_exists(sqlite3* data_base, const char* table_name, const SqlParamArray parameters, char** err_msg);
int sqlite3_insert_value(sqlite3* data_base, const char* table_name, const SqlValueParamArray parameters, char** err_msg);
int sqlite3_insert_value_w_defaults(sqlite3* database, const char* table_name, const SqlValueParamArray parameters, char** err_msg);
int sqlite3_delete_value_where_all_and(sqlite3* data_base, const char* table_name, const SqlValueParamArray parameters, char** err_msg);
int sqlite3_check_if_value_exists(sqlite3* data_base, const char* table_name, const SqlValueParamArray parameters, char** err_msg, bool* result);
int sqlite3_set_value_where(sqlite3* data_base, const char* table_name, const SqlValueParamArray selections, const SqlValueParamArray values, char** err_msg);

int sqlite3_row_count(sqlite3* data_base, const char* table_name, int* result, char** err_msg);
int sqlite3_get_row_statement(sqlite3* data_base, char* table_name, SqlValueParamArray parameters, char** err_msg, sqlite3_stmt** statement);