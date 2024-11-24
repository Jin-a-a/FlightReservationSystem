#pragma once
#include <stdbool.h>
#include <D:\TechnicalProjects\C Libs\SQLite\sqlite3\sqlite3.h>

typedef struct SqlValueParameter {
	const char* name;
	const char* type;
	const char* value;
} SqlValueParameter;

typedef struct SqlParameter {
	const char* name;
	const char* type;
} SqlParameter;

typedef struct SqlValueParamArray {
	unsigned int length;
	SqlValueParameter* arr;
} SqlValueParamArray;

typedef struct SqlParamArray {
	unsigned int length;
	SqlParameter* arr;
} SqlParamArray;

SqlValueParameter* format_set(const SqlParameter parameters[], const int param_count, ...);
void free_value_set(SqlValueParameter parameters[], int param_count, bool free_values);
SqlValueParameter* combine_format_set(const SqlValueParameter a[], unsigned int a_len, const SqlValueParameter b[], unsigned int b_len);

char* combine_param_types(const SqlParameter parameters[], int param_count);
char* combine_param_values(const SqlValueParameter parameters[], unsigned int param_count);
char* combine_param_names(const SqlValueParameter parameters[], unsigned int param_count);
char* combine_param_names_v2(const SqlParameter parameters[], unsigned int param_count);
char* combine_param_explicit_values(const SqlValueParameter parameters[], unsigned int param_count);
char* combine_param_comparisons_and(const SqlValueParameter parameters[], unsigned int param_count, char* comparer);

int sqlite3_check_if_value_exists(sqlite3* data_base, const char* table_name, const SqlValueParameter parameters[], unsigned int param_count, char** err_msg, bool* result);
int sqlite3_set_value_where(sqlite3* data_base, const char* table_name, const SqlValueParameter select_value[], unsigned int selections, const SqlValueParameter values[], unsigned int set_val_len, char** err_msg);
int sqlite3_row_count(sqlite3* data_base, const char* table_name, int* result, char** err_msg);

int sqlite3_table_if_not_exists(sqlite3* data_base, const char* table_name, const SqlParameter parameters[], unsigned int param_count, char** err_msg);
int sqlite3_insert_value(sqlite3* data_base, const char* table_name, const SqlValueParameter parameters[], unsigned int param_count, char** err_msg);
int sqlite3_insert_value_w_defaults(sqlite3* database, const char* table_name, const SqlValueParameter parameters[], unsigned int param_count, char** err_msg);
int sqlite3_delete_value_where_all_and(sqlite3* data_base, const char* table_name, const SqlValueParameter parameters[], unsigned int param_count, char** err_msg);
int sqlite3_check_if_value_exists(sqlite3* data_base, const char* table_name, const SqlValueParameter parameters[], unsigned int param_count, char** err_msg, bool* result);
int sqlite3_set_value_where(sqlite3* data_base, const char* table_name, const SqlValueParameter select_value[], unsigned int selections, const SqlValueParameter values[], unsigned int set_val_len, char** err_msg);
int sqlite3_row_count(sqlite3* data_base, const char* table_name, int* result, char** err_msg);

int sqlite3_get_row_statement(sqlite3* data_base, char* table_name, SqlValueParameter* parameters, unsigned int param_count, char** err_msg, sqlite3_stmt** statement);