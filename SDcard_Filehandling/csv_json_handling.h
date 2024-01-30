/*
 * csv_json_handling.h
 *
 *  Created on: Dec 24, 2023
 *      Author: hzaib
 */

#ifndef CSV_JSON_HANDLING_H_
#define CSV_JSON_HANDLING_H_

#include "main.h"

extern uint8_t file_already_exist;
extern char filename_with_format[100];
extern char filename_ver_date[100];
extern uint8_t fileWrite_day;
extern uint8_t fileWrite_month;
extern uint8_t fileWrite_year;

extern uint8_t fileWrite_hour;
extern uint8_t fileWrite_min;
extern uint8_t fileWrite_sec;

extern uint8_t datawritten;

extern void json_update();
extern void csv_update();
extern void csv_header();
extern uint8_t filesaving_process();
extern int8_t createfile(char *filename, char *fileformat);

#endif /* CSV_JSON_HANDLING_H_ */
