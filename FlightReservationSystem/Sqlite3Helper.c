#include <D:\TechnicalProjects\C Libs\SQLite\sqlite3\sqlite3.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <stdarg.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>
#include "Sqlite3Helper.h"
#include "StringHelper.h"

#define def_buffer 100

static int callback(void* count, int argc, char** argv, char** azColName) {
	int* c = count;
	*c = atoi(argv[0]);
	return 0;
}

static SqlValueParameter format_param(SqlParameter param, __int64 value) {
	const char* type = param.column_type;
	const char* format;

	if (str_lower_case_equal(type, "TEXT")) format = "%s";
	else if (str_lower_case_equal(type, "REAL")) format = "%.4f";
	else if (str_lower_case_equal(type, "INT")) format = "%d";
	else if (str_lower_case_equal(type, "INTEGER")) format = "%d";
	else format = "%d";

	int length = _scprintf(format, value) + 1;
	char* str_val = malloc(sizeof(char) * length);
	if (str_val == NULL) {
		exit(-1);
	}
	sprintf_s(str_val, sizeof(char) * length, format, value);

	SqlValueParameter target = (SqlValueParameter){
		.name = param.name,
		.type = param.column_type,
		.value = str_val
	};

	return target;
}

SqlValueParamArray vformat_param_set(const SqlParamArray parameters, int arg_count, ...) {
	if (parameters.length != arg_count) {
		printf("Format count does not match parameter length.");
		exit(-1);
	}

	SqlValueParameter* set = malloc(sizeof(SqlValueParameter) * arg_count);
	__int64* data_arr = malloc(sizeof(__int64) * arg_count);
	if (set == NULL || data_arr == NULL) {
		printf("Malloc failed.");
		exit(-1);
	}

	va_list ap;

	va_start(ap, arg_count);
	for (int i = 0; i < arg_count; i++) {
		data_arr[i] = va_arg(ap, __int64);
	}
	va_end(ap);

	for (int i = 0; i < arg_count; i++) {
		SqlParameter param = parameters.arr[i];
		set[i] = format_param(param, data_arr[i]);
	}

	free(data_arr);
	return (SqlValueParamArray) {
		.arr = set,
		.length = arg_count
	};
}

//Deep copies values.
SqlValueParamArray combine_param_set(const SqlValueParamArray a, const SqlValueParamArray b) {
	SqlValueParameter* param_array = malloc(sizeof(SqlValueParameter) * (a.length + b.length));
	if (param_array == NULL) {
		printf("Heap allocation failed.");
		exit(-1);
	}

	for (int i = 0; i < a.length; i++) {
		SqlValueParameter* curr = &param_array[i];
		SqlValueParameter* curr_source = &a.arr[i];

		*curr = *curr_source;

		int value_len = strlen(curr_source->value) + 1;
		char* str = malloc(sizeof(char) * value_len);
		if (str == NULL) {
			printf("Heap allocation failed.");
			exit(-1);
		}

		strcpy_s(str, sizeof(char) * value_len, curr_source->value);
		curr->value = str;
	}

	for (int i = 0; i < b.length; i++) {
		int j = i + a.length;

		SqlValueParameter* curr = &param_array[j];
		SqlValueParameter* curr_source = &b.arr[i];

		*curr = *curr_source;

		int value_len = strlen(curr_source->value) + 1;
		char* str = malloc(sizeof(char) * value_len);
		if (str == NULL) {
			printf("Heap allocation failed.");
			exit(-1);
		}

		strcpy_s(str, sizeof(char) * value_len, curr_source->value);
		curr->value = str;
	}

	return (SqlValueParamArray) {
		.arr = param_array,
		.length = a.length + b.length
	};
}

static void validate_range(int base_length, Range range) {
	if (range.index + range.range > base_length) {
		printf("The slice contains parts that is outside the parameters.\n");
		exit(-1);
	}

	if (range.index < 0) {
		printf("Starting index is negative.\n");
	}

	if (range.range <= 0) {
		printf("Length is negative or zero.\n");
	}
}

SqlParamArray param_array_slice(SqlParamArray parameters, Range range) {
	validate_range(parameters.length, range);
	return (SqlParamArray) {
		.arr = &parameters.arr[range.index],
		.length = range.range
	};
}

SqlValueParamArray value_param_array_slice(SqlValueParamArray parameters, Range range) {
	validate_range(parameters.length, range);
	return (SqlValueParamArray) {
		.arr = &parameters.arr[range.index],
		.length = range.range
	};
}

char* combine_param_types(const SqlParamArray parameters) {
	unsigned int capacity = 100;
	char* destination = malloc(sizeof(char) * capacity);
	if (destination == NULL) {
		printf("Null Reference Error.");
		exit(-1);
	}
	destination[0] = '\0';

	unsigned int param_count = parameters.length;
	SqlParameter* param_arr = parameters.arr;

	for (int i = 0; i < param_count; i++) {
		const char* curr_name = param_arr[i].name;
		const char* curr_type = param_arr[i].column_type;

		char* param_arg = dynamic_format("%s %s", curr_name, curr_type);
		dynamic_concat(&destination, param_arg, &capacity);

		if (i < param_count - 1) {
			dynamic_concat(&destination, ", ", &capacity);
		}

		free(param_arg);
	}

	return destination;
}

char* combine_param_values(const SqlValueParamArray parameters) {
	const char* form_literal = "%s";
	const char* form_str = "'%s'";

	unsigned int capacity = def_buffer;
	char* destination = malloc(sizeof(char) * def_buffer);
	if (destination == NULL) {
		printf("Null Reference Error");
		exit(-1);
	}
	destination[0] = '\0';

	int param_count = parameters.length;
	SqlValueParameter* param_array = parameters.arr;

	for (int i = 0; i < param_count; i++) {
		const char* curr_type = param_array[i].type;
		const char* curr_value = param_array[i].value;
		const char* using_format;

		if (str_lower_case_equal(curr_type, "text")) {
			using_format = form_str;
		}
		else {
			using_format = form_literal;
		}

		char* param_arg = dynamic_format(using_format, curr_value);
		dynamic_concat(&destination, param_arg, &capacity);

		if (i < param_count - 1) {
			dynamic_concat(&destination, ", ", &capacity);
		}

		free(param_arg);
	}

	return destination;
}

char* combine_param_names(const SqlValueParamArray parameters) {
	unsigned int capacity = def_buffer;
	char* destination = malloc(sizeof(char) * def_buffer);
	if (destination == NULL) {
		printf("Null Reference Error");
		exit(-1);
	}
	destination[0] = '\0';

	int param_count = parameters.length;

	for (int i = 0; i < param_count; i++) {
		const char* curr_name = parameters.arr[i].name;

		dynamic_concat(&destination, curr_name, &capacity);
		if (i < param_count - 1) {
			dynamic_concat(&destination, ", ", &capacity);
		}
	}

	return destination;
}

char* combine_param_names_v2(const SqlParamArray parameters) {
	unsigned int capacity = def_buffer;
	char* destination = malloc(sizeof(char) * def_buffer);
	if (destination == NULL) {
		printf("Null Reference Error");
		exit(-1);
	}
	destination[0] = '\0';

	for (int i = 0; i < parameters.length; i++) {
		const char* curr_name = parameters.arr[i].name;

		dynamic_concat(&destination, curr_name, &capacity);
		if (i < parameters.length - 1) {
			dynamic_concat(&destination, ", ", &capacity);
		}
	}

	return destination;
}

char* combine_param_explicit_values(const SqlValueParamArray parameters) {
	const char* form_literal = "%s = %s";
	const char* form_str = "%s = '%s'";

	unsigned int capacity = def_buffer;
	char* destination = malloc(sizeof(char) * def_buffer);
	if (destination == NULL) {
		printf("Null Reference Error");
		exit(-1);
	}
	destination[0] = '\0';

	SqlValueParameter* param_array = parameters.arr;

	for (int i = 0; i < parameters.length; i++) {
		const char* curr_type = param_array[i].type;
		const char* curr_value = param_array[i].value;
		const char* curr_name = param_array[i].name;
		const char* using_format;

		if (str_lower_case_equal(curr_type, "text")) {
			using_format = form_str;
		}
		else {
			using_format = form_literal;
		}

		char* param_arg = dynamic_format(using_format, curr_name, curr_value);
		dynamic_concat(&destination, param_arg, &capacity);

		if (i < parameters.length - 1) {
			dynamic_concat(&destination, ", ", &capacity);
		}

		free(param_arg);
	}

	return destination;
}

char* combine_param_comparisons_and(const SqlValueParamArray parameters, char* comparer) {
	const char* base_format = "%s %s %s"; //Name Comparer Value
	char* form_literal = "%s";
	char* form_str = "'%s'";
	unsigned int capacity = 100;
	char* destination = malloc(sizeof(char) * capacity);
	if (destination == NULL) {
		printf("Null Reference Error.");
		exit(-1);
	}
	destination[0] = '\0';

	int s_param_count = parameters.length;
	SqlValueParameter* param_array = parameters.arr;
	for (int i = 0; i < s_param_count; i++) {
		const char* curr_name = param_array[i].name;
		const char* curr_type = param_array[i].type;
		const char* curr_value = param_array[i].value;

		char* using_format;

		if (str_lower_case_equal(curr_type, "text")) {
			using_format = form_str;
		}
		else {
			using_format = form_literal;
		}

		char* casted_value = dynamic_format(using_format, curr_value);
		char* conditional = dynamic_format(base_format, curr_name, comparer, casted_value);
		free(casted_value);
		dynamic_concat(&destination, conditional, &capacity);
		free(conditional);

		if (i < s_param_count - 1) {
			dynamic_concat(&destination, " AND ", &capacity);
		}
	}

	return destination;
}



int sqlite3_table_if_not_exists(sqlite3* data_base, const char* table_name, const SqlParamArray parameters, char** err_msg) {
	const char* exec_sql_base = "CREATE TABLE IF NOT EXISTS %s(%s) STRICT";
	char* insert_arg = combine_param_types(parameters);
	char* exec_sql = dynamic_format(exec_sql_base, table_name, insert_arg);

	int res = sqlite3_exec(data_base, exec_sql, NULL, NULL, err_msg);
	free(insert_arg);
	free(exec_sql);
	return res;
}

int sqlite3_insert_value_w_defaults(sqlite3* database, const char* table_name, const SqlValueParamArray parameters, char** err_msg) {
	const char* exec_sql_base = "INSERT INTO %s(%s) VALUES(%s)";
	char* target_columns = combine_param_names(parameters);
	char* insert_arg = combine_param_values(parameters);
	char* exec_sql = dynamic_format(exec_sql_base, table_name, target_columns, insert_arg);

	int res = sqlite3_exec(database, exec_sql, NULL, NULL, err_msg);
	free(target_columns);
	free(insert_arg);
	free(exec_sql);
	return res;
}

int sqlite3_insert_value(sqlite3* data_base, const char* table_name, const SqlValueParamArray parameters, char** err_msg) {
	const char* exec_sql_base = "INSERT INTO %s VALUES(%s)";
	char* insert_arg = combine_param_values(parameters);
	char* exec_sql = dynamic_format(exec_sql_base, table_name, insert_arg);

	int res = sqlite3_exec(data_base, exec_sql, NULL, NULL, err_msg);
	free(insert_arg);
	free(exec_sql);
	return res;
}

int sqlite3_delete_value_where_all_and(sqlite3* data_base, const char* table_name, const SqlValueParamArray parameters, char** err_msg) {
	const char* exec_sql_base = "DELETE FROM %s WHERE %s";
	char* where_arg = combine_param_comparisons_and(parameters, "=");
	char* exec_sql = dynamic_format(exec_sql_base, table_name, where_arg);

	int res = sqlite3_exec(data_base, exec_sql, NULL, NULL, err_msg);
	free(where_arg);
	free(exec_sql);
	return res;
}

int sqlite3_get_row_statement(sqlite3* data_base, char* table_name, SqlValueParamArray parameters, char** err_msg, sqlite3_stmt** statement) {
	char* exec_sql_base = "SELECT * FROM %s WHERE %s";

	char* where_arg = combine_param_comparisons_and(parameters, "=");
	char* exec_sql = dynamic_format(exec_sql_base, table_name, where_arg);

	int res = sqlite3_prepare_v2(data_base, exec_sql, -1, statement, err_msg);

	free(where_arg);
	free(exec_sql);

	return res;
}

int sqlite3_check_if_value_exists(sqlite3* data_base, const char* table_name, const SqlValueParamArray parameters, char** err_msg, bool* result) {
	char* exec_sql_base = "SELECT * FROM %s WHERE %s";
	sqlite3_stmt* statement;

	char* where_arg = combine_param_comparisons_and(parameters, "=");
	char* exec_sql = dynamic_format(exec_sql_base, table_name, where_arg);

	int res = sqlite3_prepare_v2(data_base, exec_sql, -1, &statement, err_msg);
	if (res == SQLITE_OK) {
		*result = sqlite3_step(statement) == SQLITE_ROW;
	}

	free(where_arg);
	free(exec_sql);
	sqlite3_finalize(statement);

	return res;
}

int sqlite3_set_value_where(sqlite3* data_base, const char* table_name, const SqlValueParamArray selections, const SqlValueParamArray values, char** err_msg) {
	char* exec_sql_base = "UPDATE %s SET %s WHERE %s";

	char* values_arg = combine_param_explicit_values(values);
	char* where_arg = combine_param_comparisons_and(selections, "=");
	char* exec_sql = dynamic_format(exec_sql_base, table_name, values_arg, where_arg);

	int res = sqlite3_exec(data_base, exec_sql, NULL, NULL, err_msg);

	free(values_arg);
	free(where_arg);
	free(exec_sql);

	return res;
}

int sqlite3_row_count(sqlite3* data_base, const char* table_name, int* result, char** err_msg) {
	char* exec_sql_base = "SELECT COUNT(*) FROM %s";

	char* exec_sql = dynamic_format(exec_sql_base, table_name);

	int res = sqlite3_exec(data_base, exec_sql, callback, result, err_msg);

	return res;
}

void free_value_set(SqlValueParamArray parameters) {
	for (int i = 0; i < parameters.length; i++) {
		SqlValueParameter p = parameters.arr[i];
		char* ptr = (char*)p.value;
		free(ptr);
	}

	free(parameters.arr);
}

//Not used.
//void int_to_alpha_number(int value, char* buffer, size_t size) {
//	int alpha_amount = 'Z' - 'A' + 1;
//	int current = value;
//	int index = 0;
//	while (current != 0) {
//		int division_result = current / alpha_amount;
//		int quotient = current - division_result;
//		current = division_result;
//
//		buffer[index] = quotient + 'A';
//		index++;
//
//		while (index >= size - 1) {
//			for (int j = 0; j < size - 2; j++) {
//				buffer[j] = buffer[j + 1];
//			}
//		}
//	}
//
//	buffer[index] = '\0';
//}