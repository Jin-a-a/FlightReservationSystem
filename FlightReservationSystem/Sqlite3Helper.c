#include <D:\TechnicalProjects\C Libs\SQLite\sqlite3\sqlite3.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <stdarg.h>
#include <stdlib.h>
#include <varargs.h>
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

static SqlValueParameter format_parameter(SqlParameter param, __int64 value) {
	const char* type = param.type;
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
		.type = param.type,
		.value = str_val
	};

	return target;
}

SqlValueParameter* format_set(const SqlParameter parameters[], const int param_count, ...) {
	SqlValueParameter* set = malloc(sizeof(SqlValueParameter) * param_count);
	__int64* data_arr = malloc(sizeof(__int64) * param_count);
	if (set == NULL || data_arr == NULL) {
		printf("Malloc failed.");
		exit(-1);
	}

	va_list ap;

	va_start(ap, param_count);
	for (int i = 0; i < param_count; i++) {
		data_arr[i] = va_arg(ap, __int64);
	}
	va_end(ap);

	for (int i = 0; i < param_count; i++) {
		SqlParameter param = parameters[i];
		set[i] = format_parameter(param, data_arr[i]);
	}
	
	free(data_arr);
	return set;
}

SqlValueParameter* combine_format_set(const SqlValueParameter a[], unsigned int a_len, const SqlValueParameter b[], unsigned int b_len) {
	SqlValueParameter* set = malloc(sizeof(SqlValueParameter) * a_len * b_len);
	if (set == NULL) {
		printf("Null Reference Error.");
		exit(-1);
	}

	for (int i = 0; i < a_len; i++) {
		set[i] = a[i];
	}

	for (int i = 0; i < b_len; i++) {
		SqlValueParameter* curr = &set[a_len + i];
		*curr = b[i];
	}

	return set;
}

char* combine_param_types(const SqlParameter parameters[], int param_count) {
	unsigned int capacity = 100;
	char* destination = malloc(sizeof(char) * capacity);
	if (destination == NULL) {
		printf("Null Reference Error.");
		exit(-1);
	}
	destination[0] = '\0';

	for (int i = 0; i < param_count; i++) {
		const char* curr_name = parameters[i].name;
		const char* curr_type = parameters[i].type;

		char* param_arg = dynamic_format("%s %s", curr_name, curr_type);
		dynamic_concat(&destination, param_arg, &capacity);

		if (i < param_count - 1) {
			dynamic_concat(&destination, ", ", &capacity);
		}

		free(param_arg);
	}

	return destination;
}

char* combine_param_values(const SqlValueParameter parameters[], unsigned int param_count) {
	const char* form_literal = "%s";
	const char* form_str = "'%s'";

	unsigned int capacity = def_buffer;
	char* destination = malloc(sizeof(char) * def_buffer);
	if (destination == NULL) {
		printf("Null Reference Error");
		exit(-1);
	}
	destination[0] = '\0';

	for (int i = 0; i < param_count; i++) {
		const char* curr_type = parameters[i].type;
		const char* curr_value = parameters[i].value;
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

char* combine_param_names(const SqlValueParameter parameters[], unsigned int param_count) {
	unsigned int capacity = def_buffer;
	char* destination = malloc(sizeof(char) * def_buffer);
	if (destination == NULL) {
		printf("Null Reference Error");
		exit(-1);
	}
	destination[0] = '\0';

	for (int i = 0; i < param_count; i++) {
		const char* curr_name = parameters[i].name;

		dynamic_concat(&destination, curr_name, &capacity);
		if (i < param_count - 1) {
			dynamic_concat(&destination, ", ", &capacity);
		}
	}

	return destination;
}

char* combine_param_names_v2(const SqlParameter parameters[], unsigned int param_count) {
	unsigned int capacity = def_buffer;
	char* destination = malloc(sizeof(char) * def_buffer);
	if (destination == NULL) {
		printf("Null Reference Error");
		exit(-1);
	}
	destination[0] = '\0';

	for (int i = 0; i < param_count; i++) {
		const char* curr_name = parameters[i].name;

		dynamic_concat(&destination, curr_name, &capacity);
		if (i < param_count - 1) {
			dynamic_concat(&destination, ", ", &capacity);
		}
	}

	return destination;
}

char* combine_param_explicit_values(const SqlValueParameter parameters[], unsigned int param_count) {
	const char* form_literal = "%s = %s";
	const char* form_str = "%s = '%s'";

	unsigned int capacity = def_buffer;
	char* destination = malloc(sizeof(char) * def_buffer);
	if (destination == NULL) {
		printf("Null Reference Error");
		exit(-1);
	}
	destination[0] = '\0';

	for (int i = 0; i < param_count; i++) {
		const char* curr_type = parameters[i].type;
		const char* curr_value = parameters[i].value;
		const char* curr_name = parameters[i].name;
		const char* using_format;

		if (str_lower_case_equal(curr_type, "text")) {
			using_format = form_str;
		}
		else {
			using_format = form_literal;
		}

		char* param_arg = dynamic_format(using_format, curr_name, curr_value);
		dynamic_concat(&destination, param_arg, &capacity);

		if (i < param_count - 1) {
			dynamic_concat(&destination, ", ", &capacity);
		}

		free(param_arg);
	}

	return destination;
}

char* combine_param_comparisons_and(const SqlValueParameter parameters[], unsigned int param_count, char* comparer) {
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

	int s_param_count = param_count;
	for (int i = 0; i < s_param_count; i++) {
		const char* curr_name = parameters[i].name;
		const char* curr_type = parameters[i].type;
		const char* curr_value = parameters[i].value;

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

int sqlite3_table_if_not_exists(sqlite3* data_base, const char* table_name, const SqlParameter parameters[], unsigned int param_count, char** err_msg) {
	const char* exec_sql_base = "CREATE TABLE IF NOT EXISTS %s(%s) STRICT";
	char* insert_arg = combine_param_types(parameters, param_count);
	char* exec_sql = dynamic_format(exec_sql_base, table_name, insert_arg);

	int res = sqlite3_exec(data_base, exec_sql, NULL, NULL, err_msg);
	free(insert_arg);
	free(exec_sql);
	return res;
}

int sqlite3_insert_value_w_defaults(sqlite3* database, const char* table_name, const SqlValueParameter parameters[], unsigned int param_count, char** err_msg) {
	const char* exec_sql_base = "INSERT INTO %s(%s) VALUES(%s)";
	char* target_columns = combine_param_names(parameters, param_count);
	char* insert_arg = combine_param_values(parameters, param_count);
	char* exec_sql = dynamic_format(exec_sql_base, table_name, target_columns, insert_arg);

	int res = sqlite3_exec(database, exec_sql, NULL, NULL, err_msg);
	free(target_columns);
	free(insert_arg);
	free(exec_sql);
	return res;
}

int sqlite3_insert_value(sqlite3* data_base, const char* table_name, const SqlValueParameter parameters[], unsigned int param_count, char** err_msg) {	
	const char* exec_sql_base = "INSERT INTO %s VALUES(%s)";
	char* insert_arg = combine_param_values(parameters, param_count);
	char* exec_sql = dynamic_format(exec_sql_base, table_name, insert_arg);

	int res = sqlite3_exec(data_base, exec_sql, NULL, NULL, err_msg);
	free(insert_arg);
	free(exec_sql);
	return res;
}

int sqlite3_delete_value_where_all_and(sqlite3* data_base, const char* table_name, const SqlValueParameter parameters[], unsigned int param_count, char** err_msg) {
	const char* exec_sql_base = "DELETE FROM %s WHERE %s";
	char* where_arg = combine_param_comparisons_and(parameters, param_count, "=");
	char* exec_sql = dynamic_format(exec_sql_base, table_name, where_arg);

	int res = sqlite3_exec(data_base, exec_sql, NULL, NULL, err_msg);
	free(where_arg);
	free(exec_sql);
	return res;
}

int sqlite3_get_row_statement(sqlite3* data_base, char* table_name, SqlValueParameter* parameters, unsigned int param_count, char** err_msg, sqlite3_stmt** statement) {
	char* exec_sql_base = "SELECT * FROM %s WHERE %s";

	char* where_arg = combine_param_comparisons_and(parameters, param_count, "=");
	char* exec_sql = dynamic_format(exec_sql_base, table_name, where_arg);

	int res = sqlite3_prepare_v2(data_base, exec_sql, -1, statement, err_msg);

	free(where_arg);
	free(exec_sql);

	return res;
}

int sqlite3_check_if_value_exists(sqlite3* data_base, const char* table_name, const SqlValueParameter parameters[], unsigned int param_count, char** err_msg, bool* result) {
	char* exec_sql_base = "SELECT * FROM %s WHERE %s";
	sqlite3_stmt* statement;

	char* where_arg = combine_param_comparisons_and(parameters, param_count, "=");
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

int sqlite3_set_value_where(sqlite3* data_base, const char* table_name, const SqlValueParameter select_value[], unsigned int selections, const SqlValueParameter values[], unsigned int set_val_len, char** err_msg) {
	char* exec_sql_base = "UPDATE %s SET %s WHERE %s";

	char* values_arg = combine_param_explicit_values(values, set_val_len);
	char* where_arg = combine_param_comparisons_and(select_value, selections, "=");
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

void free_value_set(SqlValueParameter parameters[], int param_count, bool free_values) {
	if (free_values) {
		for (int i = 0; i < param_count; i++) {
			SqlValueParameter p = parameters[i];
			char* ptr = (char*)p.value;
			free(ptr);
		}
	}

	free(parameters);
}

//Not used.
void int_to_alpha_number(int value, char* buffer, size_t size) {
	int alpha_amount = 'Z' - 'A' + 1;
	int current = value;
	int index = 0;
	while (current != 0) {
		int division_result = current / alpha_amount;
		int quotient = current - division_result;
		current = division_result;

		buffer[index] = quotient + 'A';
		index++;

		while (index >= size - 1) {
			for (int j = 0; j < size - 2; j++) {
				buffer[j] = buffer[j + 1];
			}
		}
	}

	buffer[index] = '\0';
}