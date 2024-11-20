#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <malloc.h>
#include <string.h>
#include "Sqlite3Helper.h"
#include "D:\TechnicalProjects\C Libs\SQLite\sqlite3\sqlite3.h"
#include "D:\TechnicalProjects\C Libs\LibFort\fort.h";

#define CANCELLED 2
#define DELAYED 1
#define ONSCHED 0

const char* data_base_file = "flights.db";

const char* flight_table = "FLIGHTS";
const SqlParameter* flight_table_params;
int flight_table_param_count;
int flight_table_identifiable_param_count;
int flight_table_status_index;

const char* seats_table = "SEATS";
const SqlParameter* seats_table_params;
int seats_table_param_count;
//Range to identify only the flight it belongs to.
int seats_table_identifiable_flight_param_count;
//Range to identify the flight and the position of the seat.
int seats_table_identifiable_seat_param_count;

static void empty_stdin(void);

void display_options(sqlite3* data_base);
void add_flight_sched(sqlite3* data_base);
void delete_flight_sched(sqlite3* data_base);
void delay_flight_schedule(sqlite3* data_base);
void cancel_flight_sched(sqlite3* data_base);
void normalize_flight_schedule(sqlite3* data_base);
void update_flight_seat(sqlite3* data_base);
void view_flight_schedule(sqlite3* data_base);
void view_flight_seats(sqlite3* data_base);

void get_flight_sched(unsigned int* day, unsigned int* month, unsigned int* year, unsigned int* hour, unsigned int* minute, char* source_ap, char* destination_ap);
SqlValueParameter* get_flight_sched_v2();
SqlValueParameter* get_identifiable_flight_seat(long long time_stamp, char source_ap[5], char destination_ap[5]);

long long get_timestamp(unsigned int day, unsigned int month, unsigned int year, unsigned int hour, unsigned int minute);

static void exit_on_err(int result, char* err_code) {
	if (result != SQLITE_OK) {
		if (err_code == NULL) {
			printf("   : Error message\n");
		}
		else {
			printf("   : Error message - %s\n", err_code);
		}
		free(err_code);
		exit(-1);
	}
}

int main()
{
	SqlParameter stack_flight_table_params[9] = {
		[0] = (SqlParameter){.name = "Day", .type = "INT" },
		[1] = (SqlParameter){.name = "Month", .type = "INT" },
		[2] = (SqlParameter){.name = "Year", .type = "INT" },
		[3] = (SqlParameter){.name = "Hour", .type = "INT" },
		[4] = (SqlParameter){.name = "Minute", .type = "INT" },
		[5] = (SqlParameter){.name = "SourceAirportCode", .type = "TEXT" },
		[6] = (SqlParameter){.name = "DestinationAirportCode", .type = "TEXT" },
		[7] = (SqlParameter){.name = "Delay", .type = "INT" },
		[8] = (SqlParameter){.name = "Status", .type = "INT" },
	};
	flight_table_param_count = 9;
	flight_table_identifiable_param_count = 7;
	flight_table_params = stack_flight_table_params;
	flight_table_status_index = 8;

	SqlParameter stack_seats_table_params[10] = {
		[0] = (SqlParameter){.name = "Timestamp", .type = "INT" },
		[1] = (SqlParameter){.name = "SourceAirportCode", .type = "TEXT" },
		[2] = (SqlParameter){.name = "DestinationAirportCode", .type = "TEXT" },
		[3] = (SqlParameter){.name = "Row", .type = "INT" },
		[4] = (SqlParameter){.name = "Column", .type = "INT" },
		[5] = (SqlParameter){.name = "GivenName", .type = "TEXT" },
		[6] = (SqlParameter){.name = "MiddleName", .type = "TEXT" },
		[7] = (SqlParameter){.name = "FamilyName", .type = "TEXT" },
		[8] = (SqlParameter){.name = "Country", .type = "TEXT" },
		[9] = (SqlParameter){.name = "PassportNumber", .type = "TEXT" },
	};
	seats_table_param_count = 10;
	seats_table_identifiable_flight_param_count = 3;
	seats_table_identifiable_seat_param_count = 5;
	seats_table_params = stack_seats_table_params;

	sqlite3* data_base;
	sqlite3_open(data_base_file, &data_base);
	display_options(data_base);
	sqlite3_close(data_base);
}

void display_options(sqlite3* data_base) {
	while (true) {
		printf("Choose a command.\n");
		printf("1. Add Flight Schedule \n");
		printf("2. Remove Flight Schedule \n");
		printf("3. Delay Flight Schedule \n");
		printf("4. Cancel Flight Schedule \n");
		printf("5. Restore Flight Schedule \n");
		printf("6. Update Flight Seat \n");
		printf("7. View Flight Schedule \n");
		printf("8. View Flight Seats \n");
		printf("- ");

		int option;
		int res = scanf_s("%d", &option);

		if (res == 0) {
			empty_stdin();
			return;
		}

		switch (option) {
		case 1:
			add_flight_sched(data_base);
			break;
		case 2:
			delete_flight_sched(data_base);
			break;
		case 3:
			delay_flight_schedule(data_base);
			break;
		case 4:
			cancel_flight_sched(data_base);
			break;
		case 5:
			normalize_flight_schedule(data_base);
			break;
		case 6:
			update_flight_seat(data_base);
			break;
		case 7:
			view_flight_schedule(data_base);
			break;
		default:
			return;
		}

		printf("\n");
	}
}

void add_flight_sched(sqlite3* data_base) {
	char* err_code;
	int result;
	unsigned int day, month, year, hour, minute;
	char source_ap[5];
	char destination_ap[5];
	SqlValueParameter* set;
	bool has_row;

	result = sqlite3_table_if_not_exists(data_base, flight_table, flight_table_params, flight_table_param_count, &err_code);
	exit_on_err(result, err_code);

	printf("Adding Flight Sched\n");
	get_flight_sched(&day, &month, &year, &hour, &minute, source_ap, destination_ap);

	set = format_set(flight_table_params, flight_table_param_count, day, month, year, hour, minute, source_ap, destination_ap, 0, ONSCHED);
	result = sqlite3_check_if_value_exists(data_base, flight_table, set, flight_table_identifiable_param_count, &err_code, &has_row);
	exit_on_err(result, err_code);

	printf("   :\n");

	if (has_row) {
		printf("   : Flight Schedule already exists %u/%u/%u, %u:%u  %s ~> %s\n", day, month, year, hour, minute, source_ap, destination_ap);
	}
	else {
		result = sqlite3_insert_value(data_base, flight_table, set, flight_table_param_count, &err_code);
		exit_on_err(result, err_code);

		printf("   : Successfully added %u/%u/%u, %u:%u  %s ~> %s\n", day, month, year, hour, minute, source_ap, destination_ap);
	}

	free_value_set(set, flight_table_param_count, true);

	printf("\n");
}

void delete_flight_sched(sqlite3* data_base) {
	char* err_code;
	int result = 0;
	unsigned int day, month, year, hour, minute;
	char source_ap[5];
	char destination_ap[5];
	SqlValueParameter* set;
	bool has_row;

	result = sqlite3_table_if_not_exists(data_base, flight_table, flight_table_params, flight_table_param_count, &err_code);
	exit_on_err(result, err_code);

	result = sqlite3_table_if_not_exists(data_base, seats_table, seats_table_params, seats_table_param_count, &err_code);
	exit_on_err(result, err_code);

	printf("Deleting Flight Sched\n");
	get_flight_sched(&day, &month, &year, &hour, &minute, source_ap, destination_ap);

	set = format_set(flight_table_params, flight_table_identifiable_param_count, day, month, year, hour, minute, source_ap, destination_ap);

	result = sqlite3_check_if_value_exists(data_base, flight_table, set, flight_table_identifiable_param_count, &err_code, &has_row);
	exit_on_err(result, err_code);

	printf("   :\n");

	if (has_row) {
		result = sqlite3_delete_value_where_all_and(data_base, flight_table, set, 7, &err_code);
		exit_on_err(result, err_code);

		unsigned long long timestamp = get_timestamp(day, month, year, hour, minute);
		SqlValueParameter* set_b = format_set(seats_table_params, seats_table_identifiable_flight_param_count, timestamp, source_ap, destination_ap);
		result = sqlite3_delete_value_where_all_and(data_base, seats_table, set_b, seats_table_identifiable_flight_param_count, &err_code);
		exit_on_err(result, err_code);

		printf("   : Successfully deleted flight records.\n");

		free_value_set(set_b, seats_table_identifiable_flight_param_count, true);
	}
	else {
		printf("   : Flight record does not exist. (%u/%u/%u, %u:%u  %s ~> %s)\n", day, month, year, hour, minute, source_ap, destination_ap);
	}

	free_value_set(set, flight_table_identifiable_param_count, true);

	printf("\n");
}

void delay_flight_schedule(sqlite3* data_base) {
	char* err_code;
	int result = 0;
	unsigned int day, month, year, hour, minute;
	char source_ap[5];
	char destination_ap[5];
	SqlValueParameter* set;
	bool has_row;

	result = sqlite3_table_if_not_exists(data_base, flight_table, flight_table_params, flight_table_param_count, &err_code);
	exit_on_err(result, err_code);

	printf("Delaying Flight Sched\n");
	get_flight_sched(&day, &month, &year, &hour, &minute, source_ap, destination_ap);

	set = format_set(flight_table_params, flight_table_identifiable_param_count, day, month, year, hour, minute, source_ap, destination_ap);

	result = sqlite3_check_if_value_exists(data_base, flight_table, set, flight_table_identifiable_param_count, &err_code, &has_row);
	exit_on_err(result, err_code);

	printf("   :\n");

	if (!has_row) {
		printf("   : This flight record does not exist. (%u/%u/%u, %u:%u  %s ~> %s)\n", day, month, year, hour, minute, source_ap, destination_ap);
	}
	else {
		int delay;

		printf("   : Delay in minutes - ");
		scanf_s("%d", &delay);

		printf("   :\n");

		SqlParameter* flight_data = &flight_table_params[7];
		int flight_data_count = flight_table_param_count - flight_table_identifiable_param_count;
		SqlValueParameter* flight_arguments = format_set(flight_data, flight_data_count, delay, DELAYED);

		result = sqlite3_set_value_where(
			data_base, flight_table, 
			set, flight_table_identifiable_param_count, 
			flight_arguments, flight_data_count, 
			&err_code);
		exit_on_err(result, err_code);

		free_value_set(flight_arguments, flight_data_count, true);

		printf("   : Delayed the flight (%u/%u/%u, %u:%u  %s ~> %s).\n", day, month, year, hour, minute, source_ap, destination_ap);
		printf("   : By %d minutes.\n", delay);
	}

	free_value_set(set, flight_table_identifiable_param_count, true);
}

void cancel_flight_sched(sqlite3* data_base) {
	char* err_code;
	int result = 0;
	unsigned int day, month, year, hour, minute;
	char source_ap[5];
	char destination_ap[5];
	SqlValueParameter* set;
	sqlite3_stmt* statement;

	result = sqlite3_table_if_not_exists(data_base, flight_table, flight_table_params, flight_table_param_count, &err_code);
	exit_on_err(result, err_code);

	printf("Cancelling Flight Sched\n");
	get_flight_sched(&day, &month, &year, &hour, &minute, source_ap, destination_ap);

	set = format_set(flight_table_params, flight_table_identifiable_param_count, day, month, year, hour, minute, source_ap, destination_ap);

	result = sqlite3_get_row_statement(data_base, flight_table, set, flight_table_identifiable_param_count, &err_code, &statement);
	exit_on_err(result, err_code);

	printf("   :\n");

	if (sqlite3_step(statement) != SQLITE_ROW) {
		printf("   : This flight record does not exist. (%u/%u/%u, %u:%u  %s ~> %s)\n", day, month, year, hour, minute, source_ap, destination_ap);
	}
	else {
		int status = sqlite3_column_int(statement, flight_table_status_index);
		if (status == CANCELLED) {
			printf("   : Flight is already cancelled.\n");
		}
		else {
			SqlParameter* flight_data = &flight_table_params[7];
			int flight_data_count = flight_table_param_count - flight_table_identifiable_param_count;
			SqlValueParameter* flight_arguments = format_set(flight_data, flight_data_count, 0, CANCELLED);

			result = sqlite3_set_value_where(
				data_base, flight_table,
				set, flight_table_identifiable_param_count,
				flight_arguments, flight_data_count,
				&err_code);
			exit_on_err(result, err_code);

			free_value_set(flight_arguments, flight_data_count, true);

			printf("   : Cancelled the flight (%u/%u/%u, %u:%u  %s ~> %s).\n", day, month, year, hour, minute, source_ap, destination_ap);
		}
	}

	sqlite3_finalize(statement);
	free_value_set(set, flight_table_identifiable_param_count, true);
}

void normalize_flight_schedule(sqlite3* data_base) {
	char* err_code;
	int result = 0;
	unsigned int day, month, year, hour, minute;
	char source_ap[5];
	char destination_ap[5];
	SqlValueParameter* set;
	sqlite3_stmt* statement;

	result = sqlite3_table_if_not_exists(data_base, flight_table, flight_table_params, flight_table_param_count, &err_code);
	exit_on_err(result, err_code);

	printf("Normalizing Flight Sched\n");
	get_flight_sched(&day, &month, &year, &hour, &minute, source_ap, destination_ap);

	set = format_set(flight_table_params, flight_table_identifiable_param_count, day, month, year, hour, minute, source_ap, destination_ap);

	result = sqlite3_get_row_statement(data_base, flight_table, set, flight_table_identifiable_param_count, &err_code, &statement);
	exit_on_err(result, err_code);

	printf("   :\n");

	if (sqlite3_step(statement) != SQLITE_ROW) {
		printf("   : This flight record does not exist. (%u/%u/%u, %u:%u  %s ~> %s)\n", day, month, year, hour, minute, source_ap, destination_ap);
	}
	else {
		int status = sqlite3_column_int(statement, flight_table_status_index);

		if (status == ONSCHED) {
			printf("   : Flight is already on-schedule.\n");
		}
		else {
			SqlParameter* flight_data = &flight_table_params[7];
			int flight_data_count = flight_table_param_count - flight_table_identifiable_param_count;
			SqlValueParameter* flight_arguments = format_set(flight_data, flight_data_count, 0, ONSCHED);

			result = sqlite3_set_value_where(
				data_base, flight_table,
				set, flight_table_identifiable_param_count,
				flight_arguments, flight_data_count,
				&err_code);
			exit_on_err(result, err_code);

			free_value_set(flight_arguments, flight_data_count, true);

			printf("   : Normalized the flight (%u/%u/%u, %u:%u  %s ~> %s).\n", day, month, year, hour, minute, source_ap, destination_ap);
		}
	}

	sqlite3_finalize(statement);
	free_value_set(set, flight_table_identifiable_param_count, true);
}

void update_flight_seat(sqlite3* data_base) {
	char* err_code;
	int result = 0;
	unsigned int day, month, year, hour, minute;
	char source_ap[5];
	char destination_ap[5];
	SqlValueParameter* set;
	bool has_row;

	result = sqlite3_table_if_not_exists(data_base, flight_table, flight_table_params, flight_table_param_count, &err_code);
	exit_on_err(result, err_code);

	printf("Updating Flight Seat\n");
	get_flight_sched(&day, &month, &year, &hour, &minute, source_ap, destination_ap);

	set = format_set(flight_table_params, flight_table_identifiable_param_count, day, month, year, hour, minute, source_ap, destination_ap);

	result = sqlite3_check_if_value_exists(data_base, flight_table, set, flight_table_identifiable_param_count, &err_code, &has_row);
	exit_on_err(result, err_code);

	printf("   :\n");

	if (!has_row) {
		printf("   : This flight record does not exist. (%u/%u/%u, %u:%u  %s ~> %s)\n", day, month, year, hour, minute, source_ap, destination_ap);
	}
	else {
		printf("   :\n");

		result = sqlite3_table_if_not_exists(data_base, seats_table, seats_table_params, seats_table_param_count, &err_code);
		exit_on_err(result, err_code);

		long long time_stamp = get_timestamp(day, month, year, hour, minute);

		char given_name[50];
		char middle_name[50];
		char family_name[50];
		char country_name[50];
		char passport_number[50];

		SqlValueParameter* identifiable_seat = get_identifiable_flight_seat(time_stamp, source_ap, destination_ap);
		printf("   : Given Name - ");
		scanf_s("%s", given_name, 50);
		printf("   : Middle Name - ");
		scanf_s("%s", middle_name, 50);
		printf("   : Family Name - ");
		scanf_s("%s", family_name, 50);
		printf("   : Country - ");
		scanf_s("%s", country_name, 50);
		printf("   : Passport No. - ");
		scanf_s("%s", passport_number, 50);

		result = sqlite3_check_if_value_exists(data_base, seats_table, identifiable_seat, seats_table_identifiable_seat_param_count, &err_code, &has_row);
		exit_on_err(result, err_code);

		SqlParameter* seat_data = &seats_table_params[seats_table_identifiable_seat_param_count];
		int data_length = seats_table_identifiable_seat_param_count;
		SqlValueParameter* item_to_set = format_set(seat_data, data_length, given_name, middle_name, family_name, country_name, passport_number);

		if (has_row) {
			result = sqlite3_set_value_where(
				data_base,
				seats_table,
				identifiable_seat, seats_table_identifiable_seat_param_count,
				item_to_set,
				data_length,
				err_code);
			exit_on_err(result, err_code);
		}
		else {
			SqlValueParameter* seat_values = combine_format_set(identifiable_seat, seats_table_identifiable_seat_param_count, item_to_set, data_length);
			result = sqlite3_insert_value(data_base, seats_table, seat_values, seats_table_param_count, &err_code);
			exit_on_err(result, err_code);

			free_value_set(seat_values, seats_table_param_count, true);
		}

		free_value_set(item_to_set, data_length, true);
		free_value_set(identifiable_seat, seats_table_identifiable_seat_param_count, true);
	}

	free_value_set(set, flight_table_identifiable_param_count, true);
}

void view_flight_schedule(sqlite3* data_base) {
	sqlite3_stmt* statement;
	char stuff[80];
	int buffer_size = sizeof(char) * 80;
	int half_buffer_size = buffer_size / 2;

	sprintf_s(stuff, buffer_size, "SELECT * FROM %s", flight_table);

	ft_table_t* table = ft_create_table();
	ft_set_border_style(table, FT_SIMPLE_STYLE);
	ft_set_cell_prop(table, 0, FT_ANY_COLUMN, FT_CPROP_ROW_TYPE, FT_ROW_HEADER);
	ft_write_ln(table, "Date", "Time", "Source Airport Code", "Destination Airport Code", "Delay Time", "Status");

	int result = sqlite3_prepare_v2(data_base, stuff, buffer_size, &statement, NULL);
	exit_on_err(result, NULL);

	while (sqlite3_step(statement) == SQLITE_ROW) {
		int day			= sqlite3_column_int(statement, 0);
		int month		= sqlite3_column_int(statement, 1);
		int year		= sqlite3_column_int(statement, 2);
		int hour		= sqlite3_column_int(statement, 3);
		int minute		= sqlite3_column_int(statement, 4);
		char* source_ap = sqlite3_column_text(statement, 5);
		char* destin_ap = sqlite3_column_text(statement, 6);
		int delay		= sqlite3_column_int(statement, 7);
		int status		= sqlite3_column_int(statement, 8);

		char* date = stuff;
		sprintf_s(date, half_buffer_size, "%d/%d/%d", day, month, year);
		char* time = &stuff[half_buffer_size];
		sprintf_s(time, half_buffer_size, "%d:%d", hour, minute);
		char* status_str;
		switch (status) {
		case ONSCHED:
			status_str = "On Schedule";
			break;
		case DELAYED:
			status_str = "Delayed";
			break;
		case CANCELLED:
			status_str = "Cancelled";
			break;
		default:
			status_str = "Invalid";
			break;
		}

		char delay_str[20];
		if (status != DELAYED) {
			delay_str[0] = '\0';
		}
		else {
			sprintf_s(delay_str, sizeof(char) * 20, "%d minutes", delay);
		}

		ft_write_ln(table, date, time, source_ap, destin_ap, delay_str, status_str);
	}

	printf("%s\n", ft_to_string(table));
	ft_destroy_table(table);
	sqlite3_finalize(statement);
}

void view_flight_seats(sqlite3* data_base) {
	unsigned int day, month, year, hour, minute;
	char source_ap[5];
	char destination_ap[5];

	get_flight_sched(&day, &month, &year, &hour, &minute, source_ap, destination_ap);
	long long time_stamp = get_timestamp(day, month, year, hour, minute);

	sqlite3_stmt* statement;
	char stuff[80];
	int buffer_size = sizeof(char) * 80;
}

void get_flight_sched(unsigned int* day, unsigned int* month, unsigned int* year, unsigned int* hour, unsigned int* minute, char* source_ap, char* destination_ap)
{
	while (true) {
		printf("   : Day/Month/Year (Include Century) - ");
		int res = scanf_s("%u/%u/%u", day, month, year);

		if (res != 3) {
			printf("   : Invalid input.\n");
			empty_stdin();
			continue;
		}

		if (*day > 31) {
			printf("   : Day is bigger than 31. (input: %u)\n", *day);
			continue;
		}

		if (*month > 12) {
			printf("   : Month is bigger than 12. (input: %u)\n", *month);
			continue;
		}

		if (*year < 100) {
			printf("   : Must include the century for the year. (input: %u)\n", *year);
			continue;
		}

		break;
	}

	while (true) {
		printf("   : hh:mm (24 hour system) - ");
		int res = scanf_s("%u:%u", hour, minute);

		if (res != 2) {
			printf("   : Some of the inputs are invalid.\n");
			empty_stdin();
			continue;
		}

		if (*hour > 23) {
			printf("   : An hour can only go up to 23 hrs. (input: %u)\n", *hour);
			continue;
		}

		if (*minute >= 60) {
			printf("   : A minute can only go up to 59. (input: %u)\n", *minute);
			continue;
		}

		break;
	}

	char buffer[50];
	buffer[49] = '\0';

	while (true) {
		reset_loop:

		printf("   : Source Airport ICOA Code (Must be four letters) - ");
		scanf_s("%s", buffer, (unsigned int)sizeof(char) * 50);

		if (strlen(buffer) != 4) {
			printf("   : Airport codes always and strictly contains only four. (input: %s)\n", buffer);
			empty_stdin();
			continue;
		}

		for (int i = 0; i < 4; i++) {
			char curr = buffer[i];
			int index = ((int)curr) - (int)'A';
			int bound = ((int)'Z') - (int)'A';

			if (index < 0 || index > bound) {
				printf("   : Airport codes only contain capital letters. (input: %s)\n", buffer);
				goto reset_loop;
			}
		}

		strcpy_s(source_ap, sizeof(char) * 5, buffer);

		break;
	}

	while (true) {
		reset_loop_b:

		printf("   : Destination Airport ICOA Code (Must be four letters) - ");
		scanf_s("%s", buffer, (unsigned int)sizeof(char) * 50);

		if (strlen(buffer) != 4) {
			printf("   : Airport codes always and strictly contains only four. (input: %s)\n", buffer);
			empty_stdin();
			continue;
		}

		for (int i = 0; i < 4; i++) {
			char curr = buffer[i];
			int index = ((int)curr) - (int)'A';
			int bound = ((int)'Z') - (int)'A';

			if (index < 0 || index > bound) {
				printf("   : Airport codes only contain capital letters. (input: %s)\n", buffer);
				goto reset_loop_b;
			}
		}

		strcpy_s(destination_ap, sizeof(char) * 5, buffer);

		break;
	}
}

SqlValueParameter* get_flight_sched_v2() {
	unsigned int day, month, year, hour, minute;
	char source_ap[5];
	char destination_ap[5];

	get_flight_sched(&day, &month, &year, &hour, &minute, source_ap, destination_ap);

	return format_set(flight_table_params, flight_table_identifiable_param_count, day, month, year, hour, minute, source_ap, destination_ap);
}

SqlValueParameter* get_identifiable_flight_seat(long long time_stamp, char source_ap[5], char destination_ap[5]) {
	int row;
	int column;

	printf("   : Set Row - ");
	scanf_s("%d", &row);
	printf("   : Set Column - ");
	scanf_s("%d", &column);

	return format_set(seats_table_params, seats_table_identifiable_seat_param_count, time_stamp, source_ap, destination_ap, row, column);
}

long long get_timestamp(unsigned int day, unsigned int month, unsigned int year, unsigned int hour, unsigned int minute) {
	long long res = 0;
	res = year;
	res = month + res * 12;
	res = day + res * 31;
	res = hour + res * 24;
	res = minute + res * 60;
	return res;
}

static void empty_stdin(void) {
	int c = getchar();

	while (c != '\n' && c != EOF)
		c = getchar();
}