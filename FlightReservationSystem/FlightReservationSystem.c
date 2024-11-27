#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <malloc.h>
#include <string.h>
#include <stdbool.h>
#include <conio.h>
#include "console_colour.h"
#include "Sqlite3Helper.h"
#include "StringHelper.h"
#include "D:\TechnicalProjects\C Libs\SQLite\sqlite3\sqlite3.h"
#include "D:\TechnicalProjects\C Libs\LibFort\fort.h";

#define CANCELLED 2
#define DELAYED 1
#define ONSCHED 0

const char* data_base_file = "flights.db";

const char* flight_table = "FLIGHTS";
const SqlParamArray flight_table_parameters = {
	.arr = (SqlParameter[9]){
		[0] = { .name = "Day", .column_type = "INT" },
		[1] = { .name = "Month", .column_type = "INT" },
		[2] = { .name = "Year", .column_type = "INT" },
		[3] = { .name = "Hour", .column_type = "INT" },
		[4] = { .name = "Minute", .column_type = "INT" },
		[5] = { .name = "SourceAirportCode", .column_type = "TEXT" },
		[6] = { .name = "DestinationAirportCode", .column_type = "TEXT" },
		[7] = { .name = "Delay", .column_type = "INT" },
		[8] = { .name = "Status", .column_type = "INT" },
	},
	.length = 9
};
const Range flight_table_identifer_range = { .index = 0, .range = 7 };
const Range flight_table_data_range = { .index = 7, .range = 2 };
const int flight_table_status_index = 8;

const char* seats_table = "SEATS";
const SqlParamArray seats_table_parameters = {
	.arr = (SqlParameter[10]){
		[0] = {.name = "Timestamp", .column_type = "INT" },
		[1] = {.name = "SourceAirportCode", .column_type = "TEXT" },
		[2] = {.name = "DestinationAirportCode", .column_type = "TEXT" },
		[3] = {.name = "Row", .column_type = "INT" },
		[4] = {.name = "Column", .column_type = "INT" },
		[5] = {.name = "GivenName", .column_type = "TEXT" },
		[6] = {.name = "MiddleName", .column_type = "TEXT" },
		[7] = {.name = "FamilyName", .column_type = "TEXT" },
		[8] = {.name = "Country", .column_type = "TEXT" },
		[9] = {.name = "PassportNumber", .column_type = "TEXT" },
	},
	.length = 10
};
const Range seats_table_flight_identifier_range = { .index = 0, .range = 3 };
const Range seats_table_seats_identifier_range = { .index = 3, .range = 2 };
const Range seats_table_combined_identifier_range = { .index = 0, .range = 5 };
const Range seats_table_seats_id_and_data_range = { .index = 3, .range = 7 };
const Range seats_table_data_range = { .index = 5, .range = 5 };

static void print_header_stuff() {
	printf("Welcome to Slugcat Airlines console!\n");

	char* slugcat = "\
####################################################################################################\n\
####################################################################################################\n\
#######################..--#############----########################################################\n\
######################...--############...----######################################################\n\
####################+....--############....---######################################################\n\
####################.....--###########......--###########  ##############  +########################\n\
####################......##-.....+###......--###########  #############   #########################\n\
###################-.......................--.############  ############ +##########################\n\
###################-..+##..................---############. ##. -######  #########.#################\n\
###################+.####......######......--##############  #   #####. #########. #################\n\
###################.#####.....-#######.-----###############+   #  #### .####.      .################\n\
##################.+#####.....########-----.################  ##- -#   ####  ####  .################\n\
#################..#####.....#########------#####################.  .####+ .###.    ################\n\
#################..####-.....########-------#############################  ##   ##+ +###############\n\
################+...#+..##...#######--------.#############################   .#####  ###############\n\
#################..-------------++--------++-#######################################################\n\
#################-.------------------------++#######################################################\n\
##################.----------------------++++###################..############ .####################\n\
###################.--------------------++++++##################  ###########  #####################\n\
####################.----------+----+++++++++-###################  #########. +#####################\n\
###################...--+++++++++++++++++++++-#################### .##  #### .######################\n\
###################...--++++++++++++++++++++++####################  . .  ##  #####       ###########\n\
##################..-.--+++++++++++++++++++++#####################+  -## .  #####  ####   ##########\n\
#################-------+++++++++++++++++++++###########################   +##### +###  #  #########\n\
################+---------+++++++++++++++++++####################################     -##. #########\n\
###############+---------+++++++++++++++++++##############################################  ########\n\
##############++++++----++++++++++++++++++++##############################################--########\n\
\n";

	printf_rich(slugcat, TEXTFORMAT_BOLD, TEXTCOLOR_CYAN, TEXTFORMAT_BLINKING2);

	reset_rich_format();
}

static void empty_stdin(void) {
	int c = getchar();

	while (c != '\n' && c != EOF)
		c = getchar();
}

void display_options(sqlite3* data_base);
void add_flight_sched(sqlite3* data_base);
void delete_flight_sched(sqlite3* data_base);
void delay_flight_schedule(sqlite3* data_base);
void cancel_flight_sched(sqlite3* data_base);
void normalize_flight_schedule(sqlite3* data_base);
void update_flight_seat(sqlite3* data_base);
void remove_flight_seat(sqlite3* data_base);
void clear_flight_seats(sqlite3* data_base);
void view_flight_schedule(sqlite3* data_base);
void view_flight_seats(sqlite3* data_base);

void get_flight_sched(unsigned int* day, unsigned int* month, unsigned int* year, unsigned int* hour, unsigned int* minute, char* source_ap, char* destination_ap);
SqlValueParamArray get_identifiable_combined_seat(long long time_stamp, char source_ap[5], char destination_ap[5], unsigned int* x, unsigned int* y);

long long get_timestamp(unsigned int day, unsigned int month, unsigned int year, unsigned int hour, unsigned int minute);

void print_flight_at(unsigned int day, unsigned int month, unsigned int year, unsigned int hour, unsigned int minute,
	char source_ap[5], char destination_ap[5]);
void print_seats_at(unsigned int row, unsigned int column);

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
	print_header_stuff();

	sqlite3* data_base;
	sqlite3_open(data_base_file, &data_base);
	display_options(data_base);
	sqlite3_close(data_base);
}

void display_options(sqlite3* data_base) {
	while (true) {
		printf_rich("CHOOSE A COMMAND\n", TEXTFORMAT_BOLD, TEXTFORMAT_DOUBLEUNDERLINE);
		printf("\n");
		printf("Negatives. Exit Program\n");
		printf("0. Clear Screen\n");
		printf("1. Add Flight Schedule \n");
		printf("2. Remove Flight Schedule \n");
		printf("3. Delay Flight Schedule \n");
		printf("4. Cancel Flight Schedule \n");
		printf("5. Restore Flight Schedule \n");
		printf("6. Update Flight Seat \n");
		printf("7. Remove Flight Seat \n");
		printf("8. Clear Flight Seat \n");
		printf("9. View Flight Schedule \n");
		printf("10. View Flight Seats \n");
		printf("- ");

		int option;
		begin_rich_printing(TEXTCOLOR_GREEN);
		int res = scanf_s("%d", &option);
		reset_rich_format();

		if (res == 0) {
			empty_stdin();
			printf_rich("Invalid input.\n\n", TEXTCOLOR_RED);
			continue;
		}

		if (option < 0) {
			return;
		}

		switch (option) {
		case 0:
			system("cls");
			print_header_stuff();
			break;
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
			remove_flight_seat(data_base);
			break;
		case 8:
			clear_flight_seats(data_base);
			break;
		case 9:
			view_flight_schedule(data_base);
			break;
		case 10:
			view_flight_seats(data_base);
			break;
		default:
			printf("Invalid input.\n");
			break;
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
	SqlValueParamArray set;
	bool has_row;

	result = sqlite3_table_if_not_exists(data_base, flight_table, flight_table_parameters, &err_code);
	exit_on_err(result, err_code);

	printf("Adding Flight Sched\n");
	printf("   : Input flight schedule to add.\n");
	get_flight_sched(&day, &month, &year, &hour, &minute, source_ap, destination_ap);
	
	set = format_param_set(flight_table_parameters, day, month, year, hour, minute, source_ap, destination_ap, 0, ONSCHED);
	result = sqlite3_check_if_value_exists(data_base, flight_table, set, &err_code, &has_row);
	exit_on_err(result, err_code);

	printf("   :\n");

	if (has_row) {
		printf("   : Flight schedule already exists.\n");
		print_flight_at(day, month,	year, hour, minute, source_ap, destination_ap);
	}
	else {
		result = sqlite3_insert_value(data_base, flight_table, set, &err_code);
		exit_on_err(result, err_code);

		printf("   : Successfully added a flight scheudle.\n");
		print_flight_at(day, month, year, hour, minute, source_ap, destination_ap);
	}

	free_value_set(set);

	printf("\n");
}

void delete_flight_sched(sqlite3* data_base) {
	char* err_code;
	int result = 0;
	unsigned int day, month, year, hour, minute;
	char source_ap[5];
	char destination_ap[5];
	bool has_row;

	result = sqlite3_table_if_not_exists(data_base, flight_table, flight_table_parameters, &err_code);
	exit_on_err(result, err_code);

	result = sqlite3_table_if_not_exists(data_base, seats_table, flight_table_parameters, &err_code);
	exit_on_err(result, err_code);

	printf("Deleting Flight Sched\n");
	printf("   : Input flight schedule to delete.\n");
	get_flight_sched(&day, &month, &year, &hour, &minute, source_ap, destination_ap);

	SqlParamArray flight_sched_identifers = param_array_slice(flight_table_parameters, flight_table_identifer_range);
	SqlValueParamArray flight_sched_value_id = format_param_set(flight_sched_identifers, day, month, year, hour, minute, source_ap, destination_ap);

	result = sqlite3_check_if_value_exists(data_base, flight_table, flight_sched_value_id, &err_code, &has_row);
	exit_on_err(result, err_code);

	printf("   :\n");

	if (has_row) {
		result = sqlite3_delete_value_where_all_and(data_base, flight_table, flight_sched_value_id, &err_code);
		exit_on_err(result, err_code);

		unsigned long long timestamp = get_timestamp(day, month, year, hour, minute);
		SqlParamArray seats_table_flight_identifier_set = param_array_slice(seats_table_parameters, seats_table_flight_identifier_range);
		SqlValueParamArray seats_table_flight_id_val_set = format_param_set(seats_table_flight_identifier_set, timestamp, source_ap, destination_ap);
		result = sqlite3_delete_value_where_all_and(data_base, seats_table, seats_table_flight_id_val_set, &err_code);
		exit_on_err(result, err_code);

		printf("   : Successfully deleted flight records.\n");
		print_flight_at(day, month, year, hour, minute, source_ap, destination_ap);

		free_value_set(seats_table_flight_id_val_set);
	}
	else {
		printf("   : Flight record does not exist.\n");
		print_flight_at(day, month, year, hour, minute, source_ap, destination_ap);
	}

	free_value_set(flight_sched_value_id);

	printf("\n");
}

void delay_flight_schedule(sqlite3* data_base) {
	char* err_code;
	int result = 0;
	unsigned int day, month, year, hour, minute;
	char source_ap[5];
	char destination_ap[5];
	bool has_row;

	result = sqlite3_table_if_not_exists(data_base, flight_table, flight_table_parameters, &err_code);
	exit_on_err(result, err_code);

	printf("Delaying Flight Sched\n");
	printf("   : Input flight schedule to delay.\n");
	get_flight_sched(&day, &month, &year, &hour, &minute, source_ap, destination_ap);

	SqlParamArray flight_id = param_array_slice(flight_table_parameters, flight_table_identifer_range);
	SqlValueParamArray flight_value_id = format_param_set(flight_id, day, month, year, hour, minute, source_ap, destination_ap);

	result = sqlite3_check_if_value_exists(data_base, flight_table, flight_value_id, &err_code, &has_row);
	exit_on_err(result, err_code);

	printf("   :\n");

	if (!has_row) {
		printf("   : This flight record does not exist.\n");
		print_flight_at(day, month, year, hour, minute, source_ap, destination_ap);
	}
	else {
		int delay;

		printf("   : Delay in minutes - ");
		begin_rich_printing(TEXTCOLOR_GREEN);
		scanf_s("%d", &delay);
		reset_rich_format();

		printf("   :\n");

		SqlParamArray flight_data = param_array_slice(flight_table_parameters, flight_table_data_range);
		SqlValueParamArray flight_value_data = format_param_set(flight_data, delay, DELAYED);

		result = sqlite3_set_value_where(data_base, flight_table, flight_value_id, flight_value_data, &err_code);
		exit_on_err(result, err_code);

		free_value_set(flight_value_data);

		printf("   : Delayed the flight by ");
		begin_rich_printing(TEXTCOLOR_GOLD);
		printf("%d", delay);
		reset_rich_format();
		printf(" minutes\n.");
		print_flight_at(day, month, year, hour, minute, source_ap, destination_ap);
	}

	free_value_set(flight_value_id);
}

void cancel_flight_sched(sqlite3* data_base) {
	char* err_code;
	int result = 0;
	unsigned int day, month, year, hour, minute;
	char source_ap[5];
	char destination_ap[5];
	sqlite3_stmt* statement;

	result = sqlite3_table_if_not_exists(data_base, flight_table, flight_table_parameters, &err_code);
	exit_on_err(result, err_code);

	printf("Cancelling Flight Sched\n");
	printf("   : Input flight schedule to cancel.\n");
	get_flight_sched(&day, &month, &year, &hour, &minute, source_ap, destination_ap);

	SqlParamArray flight_id = param_array_slice(flight_table_parameters, flight_table_identifer_range);
	SqlValueParamArray flight_value_id = format_param_set(flight_id, day, month, year, hour, minute, source_ap, destination_ap);

	result = sqlite3_get_row_statement(data_base, flight_table, flight_value_id, &err_code, &statement);
	exit_on_err(result, err_code);

	printf("   :\n");

	if (sqlite3_step(statement) != SQLITE_ROW) {
		printf("   : This flight record does not exist.\n");
		print_flight_at(day, month, year, hour, minute, source_ap, destination_ap);
	}
	else {
		int status = sqlite3_column_int(statement, flight_table_status_index);
		if (status == CANCELLED) {
			printf("   : Flight is already cancelled.\n");
			print_flight_at(day, month, year, hour, minute, source_ap, destination_ap);
		}
		else {
			SqlParamArray flight_data = param_array_slice(flight_table_parameters, flight_table_data_range);
			SqlValueParamArray flight_arguments = format_param_set(flight_data, 0, CANCELLED);

			result = sqlite3_set_value_where(data_base, flight_table, flight_value_id, flight_arguments, &err_code);
			exit_on_err(result, err_code);

			free_value_set(flight_arguments, true);

			printf("   : Successfully cancelled the flight.\n");
			print_flight_at(day, month, year, hour, minute, source_ap, destination_ap);
		}
	}

	sqlite3_finalize(statement);
	free_value_set(flight_value_id);
}

void normalize_flight_schedule(sqlite3* data_base) {
	char* err_code;
	int result = 0;
	unsigned int day, month, year, hour, minute;
	char source_ap[5];
	char destination_ap[5];
	sqlite3_stmt* statement;

	result = sqlite3_table_if_not_exists(data_base, flight_table, flight_table_parameters, &err_code);
	exit_on_err(result, err_code);

	printf("Reset Flight Schedule\n");
	printf("   : Input flight schedule to resets.\n");
	get_flight_sched(&day, &month, &year, &hour, &minute, source_ap, destination_ap);

	SqlParamArray flight_id = param_array_slice(flight_table_parameters, flight_table_identifer_range);
	SqlValueParamArray flight_value_id = format_param_set(flight_id, day, month, year, hour, minute, source_ap, destination_ap);

	result = sqlite3_get_row_statement(data_base, flight_table, flight_value_id, &err_code, &statement);
	exit_on_err(result, err_code);

	printf("   :\n");

	if (sqlite3_step(statement) != SQLITE_ROW) {
		printf("   : This flight record does not exist.\n");
		print_flight_at(day, month, year, hour, minute, source_ap, destination_ap);
	}
	else {
		int status = sqlite3_column_int(statement, flight_table_status_index);

		if (status == ONSCHED) {
			printf("   : Flight is already on-schedule.\n");
			print_flight_at(day, month, year, hour, minute, source_ap, destination_ap);
		}
		else {
			SqlParamArray flight_data = param_array_slice(flight_table_parameters, flight_table_data_range);
			SqlValueParamArray flight_value_data = format_param_set(flight_data, 0, ONSCHED);

			result = sqlite3_set_value_where(data_base, flight_table, flight_value_id, flight_value_data, &err_code);
			exit_on_err(result, err_code);

			free_value_set(flight_value_data);

			printf("   : Successfully continued the flight as scheduled.\n");
			print_flight_at(day, month, year, hour, minute, source_ap, destination_ap);
		}
	}

	sqlite3_finalize(statement);
	free_value_set(flight_value_id);
}

void update_flight_seat(sqlite3* data_base) {
	char* err_code;
	int result = 0;
	unsigned int day, month, year, hour, minute;
	char source_ap[5];
	char destination_ap[5];
	bool has_row;

	result = sqlite3_table_if_not_exists(data_base, flight_table, flight_table_parameters, &err_code);
	exit_on_err(result, err_code);

	printf("Updating Flight Seat\n");
	printf("   : Input flight schedule to update the seat on.\n");
	get_flight_sched(&day, &month, &year, &hour, &minute, source_ap, destination_ap);

	SqlParamArray flight_id = param_array_slice(flight_table_parameters, flight_table_identifer_range);
	SqlValueParamArray flight_value_id = format_param_set(flight_id, day, month, year, hour, minute, source_ap, destination_ap);

	result = sqlite3_check_if_value_exists(data_base, flight_table, flight_value_id, &err_code, &has_row);
	exit_on_err(result, err_code);

	printf("   :\n");

	if (!has_row) {
		printf("   : This flight record does not exist.\n");
		print_flight_at(day, month, year, hour, minute, source_ap, destination_ap);
	}
	else {
		printf("   :\n");

		result = sqlite3_table_if_not_exists(data_base, seats_table, seats_table_parameters, &err_code);
		exit_on_err(result, err_code);

		long long time_stamp = get_timestamp(day, month, year, hour, minute);

		char given_name[50];
		char middle_name[50];
		char family_name[50];
		char country_name[50];
		char passport_number[50];

		unsigned int row;
		unsigned int column;

		printf("   : Seat position.\n");
		SqlValueParamArray identifiable_seat = get_identifiable_combined_seat(time_stamp, source_ap, destination_ap, &row, &column);

		printf("   :\n");

		printf("   : Input Seat Info\n");
		printf("   : Given Name - ");
		begin_rich_printing(TEXTCOLOR_GREEN);
		scanf_s("%s", given_name, 50);
		reset_rich_format();

		printf("   : Middle Name - ");
		begin_rich_printing(TEXTCOLOR_GREEN);
		scanf_s("%s", middle_name, 50);
		reset_rich_format();

		printf("   : Family Name - ");
		begin_rich_printing(TEXTCOLOR_GREEN);
		scanf_s("%s", family_name, 50);
		reset_rich_format();

		printf("   : Country - ");
		begin_rich_printing(TEXTCOLOR_GREEN);
		scanf_s("%s", country_name, 50);
		reset_rich_format();

		printf("   : Passport No. - ");
		begin_rich_printing(TEXTCOLOR_GREEN);
		scanf_s("%s", passport_number, 50);
		reset_rich_format();

		result = sqlite3_check_if_value_exists(data_base, seats_table, identifiable_seat, &err_code, &has_row);
		exit_on_err(result, err_code);

		SqlParamArray seats_data = param_array_slice(seats_table_parameters, seats_table_data_range);
		SqlValueParamArray item_to_set = format_param_set(seats_data, given_name, middle_name, family_name, country_name, passport_number);

		if (has_row) {
			result = sqlite3_set_value_where(data_base, seats_table, identifiable_seat, item_to_set, &err_code);
			exit_on_err(result, err_code);
		}
		else {
			SqlValueParamArray seat_values = combine_param_set(identifiable_seat, item_to_set);
			result = sqlite3_insert_value(data_base, seats_table, seat_values, &err_code);
			exit_on_err(result, err_code);

			free_value_set(seat_values);
		}

		free_value_set(identifiable_seat);
		free_value_set(item_to_set);

		printf("   :\n");
		printf("   : Successfully updated the seat.\n");
		print_flight_at(day, month, year, hour, minute, source_ap, destination_ap);
		print_seats_at(row, column);
	}

	free_value_set(flight_value_id);
}

void remove_flight_seat(sqlite3* data_base) {
	char* err_code;
	int result = 0;
	unsigned int day, month, year, hour, minute;
	char source_ap[5];
	char destination_ap[5];
	bool has_row;

	result = sqlite3_table_if_not_exists(data_base, flight_table, flight_table_parameters, &err_code);
	exit_on_err(result, err_code);

	printf("Remove Flight Seat\n");
	printf("   : Input flight schedule to reset the seat on.\n");
	get_flight_sched(&day, &month, &year, &hour, &minute, source_ap, destination_ap);

	SqlParamArray flight_id = param_array_slice(flight_table_parameters, flight_table_identifer_range);
	SqlValueParamArray flight_value_id = format_param_set(flight_id, day, month, year, hour, minute, source_ap, destination_ap);

	result = sqlite3_check_if_value_exists(data_base, flight_table, flight_value_id, &err_code, &has_row);
	exit_on_err(result, err_code);

	printf("   :\n");

	if (!has_row) {
		printf("   : This flight record does not exist.\n");
		print_flight_at(day, month, year, hour, minute, source_ap, destination_ap);
	}
	else {
		printf("   :\n");

		result = sqlite3_table_if_not_exists(data_base, seats_table, seats_table_parameters, &err_code);
		exit_on_err(result, err_code);

		printf("   : Seat position.\n");
		long long time_stamp = get_timestamp(day, month, year, hour, minute);
		unsigned int row;
		unsigned int column;

		SqlValueParamArray identifiable_seat = get_identifiable_combined_seat(time_stamp, source_ap, destination_ap, &row, &column);

		result = sqlite3_delete_value_where_all_and(data_base, seats_table, identifiable_seat, &err_code);
		exit_on_err(result, err_code);

		printf("   : Successfully delete a seat.\n");
		print_flight_at(day, month, year, hour, minute, source_ap, destination_ap);
		print_seats_at(row, column);

		free_value_set(identifiable_seat);
	}

	free_value_set(flight_value_id);
}

void clear_flight_seats(sqlite3* data_base) {
	char* err_code;
	int result = 0;
	unsigned int day, month, year, hour, minute;
	char source_ap[5];
	char destination_ap[5];
	bool has_row;

	result = sqlite3_table_if_not_exists(data_base, flight_table, flight_table_parameters, &err_code);
	exit_on_err(result, err_code);

	printf("Clear Flight Seat\n");
	printf("   : Input flight schedule to clear the seat on.\n");
	get_flight_sched(&day, &month, &year, &hour, &minute, source_ap, destination_ap);

	SqlParamArray flight_id = param_array_slice(flight_table_parameters, flight_table_identifer_range);
	SqlValueParamArray flight_value_id = format_param_set(flight_id, day, month, year, hour, minute, source_ap, destination_ap);

	result = sqlite3_check_if_value_exists(data_base, flight_table, flight_value_id, &err_code, &has_row);
	exit_on_err(result, err_code);

	printf("   :\n");

	if (!has_row) {
		printf("   : This flight record does not exist.\n");
		print_flight_at(day, month, year, hour, minute, source_ap, destination_ap);
	}
	else {
		result = sqlite3_table_if_not_exists(data_base, seats_table, seats_table_parameters, &err_code);
		exit_on_err(result, err_code);

		long long time_stamp = get_timestamp(day, month, year, hour, minute);
		SqlParamArray flight_identifier = param_array_slice(seats_table_parameters, seats_table_flight_identifier_range);
		SqlValueParamArray flight_value_id = format_param_set(flight_identifier, time_stamp, source_ap, destination_ap);

		result = sqlite3_delete_value_where_all_and(data_base, seats_table, flight_value_id, &err_code);
		exit_on_err(result, err_code);

		printf("   : Cleared all flight seats.\n");
		print_flight_at(day, month, year, hour, minute, source_ap, destination_ap);

		free_value_set(flight_value_id);
	}

	free_value_set(flight_value_id);
}

void view_flight_schedule(sqlite3* data_base) {
	sqlite3_stmt* statement;
	char stuff[80];
	int buffer_size = sizeof(char) * 80;
	int half_buffer_size = buffer_size / 2;

	const SqlParamArray date_time_ordered = {
		.arr = (SqlParameter[5]) { 
			[4] = flight_table_parameters.arr[4], //minute
			[3] = flight_table_parameters.arr[3], //hour
			[2] = flight_table_parameters.arr[0], //day
			[1] = flight_table_parameters.arr[1], //month
			[0] = flight_table_parameters.arr[2], //year
		},
		.length = 5
	};

	char* sql_base = "SELECT * FROM %s ORDER BY %s";
	char* order_by_str = combine_param_names_v2(date_time_ordered);
	char* exec_sql = dynamic_format(sql_base, flight_table, order_by_str);

	printf("Viewing Flight Schedules\n");

	ft_table_t* table = ft_create_table();
	ft_set_border_style(table, FT_SIMPLE_STYLE);
	ft_set_cell_prop(table, 0, FT_ANY_COLUMN, FT_CPROP_ROW_TYPE, FT_ROW_HEADER);
	ft_write_ln(table, "Date", "Time", "Source Airport Code", "Destination Airport Code", "Delay Time", "Status");

	int result = sqlite3_prepare_v2(data_base, exec_sql, -1, &statement, NULL);
	exit_on_err(result, NULL);

	free(order_by_str);
	free(exec_sql);

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

		char delay_str[20] = { 0 };
		if (status != DELAYED) {
			delay_str[0] = '\0';
		}
		else {
			sprintf_s(delay_str, sizeof(char) * 20, "%d minutes", delay);
		}

		ft_write_ln(table, date, time, source_ap, destin_ap, delay_str, status_str);
	}

	printf("\n%s\n", ft_to_string(table));
	ft_destroy_table(table);
	sqlite3_finalize(statement);
}

void view_flight_seats(sqlite3* data_base) {
	unsigned int day, month, year, hour, minute;
	char source_ap[5];
	char destination_ap[5];
	char* err_code;
	int result;
	bool has_row;

	printf("Viewing Flight Seats\n");

	printf("   : Input flight schedule to view the seat on.\n");
	get_flight_sched(&day, &month, &year, &hour, &minute, source_ap, destination_ap);

	SqlParamArray flight_id = param_array_slice(flight_table_parameters, flight_table_identifer_range);
	SqlValueParamArray flight_value_id = format_param_set(flight_id, day, month, year, hour, minute, source_ap, destination_ap);

	result = sqlite3_check_if_value_exists(data_base, flight_table, flight_value_id, &err_code, &has_row);
	exit_on_err(result, err_code);

	if (!has_row) {
		printf("   : This flight record does not exist. (%u/%u/%u, %u:%u  %s ~> %s)\n", day, month, year, hour, minute, source_ap, destination_ap);
	}
	else {
		long long time_stamp = get_timestamp(day, month, year, hour, minute);

		sqlite3_stmt* statement;
		//SELECT columns FROM table_name WHERE conditions ORDER BY columns
		char* sql_exec_base = "SELECT %s FROM %s WHERE %s ORDER BY %s";

		SqlParamArray selector_columns = param_array_slice(seats_table_parameters, seats_table_seats_id_and_data_range);
		char* selected_columns_str = combine_param_names_v2(selector_columns);

		SqlParamArray where_columns = param_array_slice(seats_table_parameters, seats_table_flight_identifier_range);
		SqlValueParamArray where_value_columns = format_param_set(where_columns, time_stamp, source_ap, destination_ap);
		char* flight_sched_condition_str = combine_param_comparisons_and(where_value_columns, "=");
		free_value_set(where_value_columns, true);

		SqlParamArray seat_pos_columns = param_array_slice(seats_table_parameters, seats_table_seats_identifier_range);
		char* seat_pos_str = combine_param_names_v2(seat_pos_columns);

		char* sql_exec = dynamic_format(sql_exec_base, selected_columns_str, seats_table, flight_sched_condition_str, seat_pos_str);

		result = sqlite3_prepare_v2(data_base, sql_exec, -1, &statement, &err_code);
		exit_on_err(result, err_code);

		free(selected_columns_str);
		free(flight_sched_condition_str);
		free(seat_pos_str);
		free(sql_exec);

		ft_table_t* table = ft_create_table();
		ft_set_border_style(table, FT_SIMPLE_STYLE);
		ft_set_cell_prop(table, 0, FT_ANY_COLUMN, FT_CPROP_ROW_TYPE, FT_ROW_HEADER);
		ft_write_ln(table, "Position (Row - Column)", "Name (Given Middle Family)", "Country", "Passport ID");

		while (sqlite3_step(statement) == SQLITE_ROW) {
			int row				= sqlite3_column_int(statement, 0);
			int column			= sqlite3_column_int(statement, 1);
			char* given_name	= sqlite3_column_text(statement, 2);
			char* middle_name	= sqlite3_column_text(statement, 3);
			char* family_name	= sqlite3_column_text(statement, 4);
			char* country		= sqlite3_column_text(statement, 5);
			char* passport_id	= sqlite3_column_text(statement, 6);

			char* pos_str = dynamic_format("%d-%d", row, column);
			char* name_str;
			if (middle_name[0] != '\0') {
				name_str = dynamic_format("%s %s %s", given_name, middle_name, family_name);
			}
			else {
				name_str = dynamic_format("%s %s", given_name, family_name);
			}

			ft_write_ln(table, pos_str, name_str, country, passport_id);
		}

		printf("\n%s\n", ft_to_string(table));

		sqlite3_finalize(statement);
		ft_destroy_table(table);
	}

	free_value_set(flight_value_id);
}

void get_flight_sched(unsigned int* day, unsigned int* month, unsigned int* year, unsigned int* hour, unsigned int* minute, char* source_ap, char* destination_ap)
{
	while (true) {
		printf("   : Day/Month/Year (Include Century) - ");
		begin_rich_printing(TEXTCOLOR_GREEN);
		int res = scanf_s("%u/%u/%u", day, month, year);
		reset_rich_format();

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
		begin_rich_printing(TEXTCOLOR_GREEN);
		int res = scanf_s("%u:%u", hour, minute);
		reset_rich_format();

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

	char buffer[50] = { '\0' };

	while (true) {
		reset_loop:

		printf("   : Source Airport ICOA Code (Must be four letters) - ");
		begin_rich_printing(TEXTCOLOR_GREEN);
		scanf_s("%s", buffer, (unsigned int)sizeof(char) * 50);
		reset_rich_format();

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
		begin_rich_printing(TEXTCOLOR_GREEN);
		scanf_s("%s", buffer, (unsigned int)sizeof(char) * 50);
		reset_rich_format();

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

SqlValueParamArray get_identifiable_combined_seat(long long time_stamp, char source_ap[5], char destination_ap[5], unsigned int* x, unsigned int* y) {
	unsigned int row;
	unsigned int column;

	printf("   : Set Row - ");
	begin_rich_printing(TEXTCOLOR_GREEN);
	while (scanf_s("%u", &row) == 0) {
		reset_rich_format();
		printf("   : Invalid value. Try again.");
		printf("   : Set Row - ");
		empty_stdin();
		begin_rich_printing(TEXTCOLOR_GREEN);
	}
	reset_rich_format();

	printf("   : Set Column - ");
	begin_rich_printing(TEXTCOLOR_GREEN);
	while (scanf_s("%u", &column) == 0) {
		reset_rich_format();
		printf("   : Invalid value. Try again.");
		printf("   : Set Column - ");
		empty_stdin();
		begin_rich_printing(TEXTCOLOR_GREEN);
	}
	reset_rich_format();

	if (x != NULL) {
		*x = row;
	}
	if (y != NULL) {
		*y = column;
	}

	SqlParamArray seat_flight_id = param_array_slice(seats_table_parameters, seats_table_combined_identifier_range);
	return format_param_set(seat_flight_id, time_stamp, source_ap, destination_ap, row, column);
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

void print_flight_at(unsigned int day, unsigned int month, unsigned int year, unsigned int hour, unsigned int minute, 
	char source_ap[5], char destination_ap[5]) {

	printf("   : ");
	begin_rich_printing(TEXTFORMAT_FADED);
	printf("At %s;\n", flight_table);
	reset_rich_format();

	printf("   : ");
	begin_rich_printing(TEXTFORMAT_FADED);
	printf("- %u/%u/%u, %u:%u\n", day, month, year, hour, minute);
	reset_rich_format();

	printf("   : ");
	begin_rich_printing(TEXTFORMAT_FADED);
	printf("- From Airport Code '%s' to '%s'.\n", source_ap, destination_ap);
	reset_rich_format();
}

void print_seats_at(unsigned int row, unsigned int column) {
	printf("   : ");
	begin_rich_printing(TEXTFORMAT_FADED);
	printf("At %s;\n", seats_table);
	reset_rich_format();

	printf("   : ");
	begin_rich_printing(TEXTFORMAT_FADED);
	printf("- %u, %u\n", row, column);
	reset_rich_format();
}